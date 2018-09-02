/*
 *                ______            ____       _
 *               / ____/___  ____  / __ \_____(_)   _____
 *              / / __/ __ \/ __ \/ / / / ___/ / | / / _ \
 * Project     / /_/ / /_/ / /_/ / /_/ / /  / /| |/ /  __/
 *             \____/\____/\____/_____/_/  /_/ |___/\___/
 *
 * Copyright (C) 2017 Pradeep Kumar <pradeep.tux@gmail.com>
 *
 * This file is part of project GooDrive.
 *
 * GooDrive is free software: You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GooDrive is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GooDrive.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "linux-api.h"

#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <malloc.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <openssl/md5.h>

#include "config.h"

/*
 * The information to be passed onto the watch and md5 context handlers
 */
struct watch_md5sum_handle_info {
	// File Descriptor of the inotify instance
	int fd;
	// MD5 Context
	MD5_CTX *md5_ctxt;
};
typedef struct watch_md5sum_handle_info watch_md5sum_handle_info;

/* Get the passwd entry */
struct passwd *get_passwd_entry(uid_t uid);
/* Get the group entry */
struct group *get_group_entry(gid_t gid);

/*
 * 1. Add a watch to the specified path, if it is a directory
 * 2. Update the MD5 Context with the same path
 */
static void watch_and_update_md5ctx_handle(FTSENT *ftsent, void *handle_info);
/* Add a watch to the specified path, if it is a directory */
static void watch_dir_handle(FTSENT *ftsent, void *handle_info);
/* Update the MD5 Context with a path */
static void update_md5ctx_path_handle(FTSENT *ftsent, void *handle_info);

/* Returns maxval if maxval > minval, else returns defval */
static size_t get_max_value(size_t minval, size_t maxval, size_t defval);

char *get_home_dir(uid_t uid) {
	struct passwd *passwd_entry = get_passwd_entry(uid);

	if (passwd_entry != NULL) {
		return passwd_entry->pw_dir;
	}
	return NULL;
}

char *get_home_dir_curruser() {
	char *home_dir = goodrv_config.curr_user_home;
	if (home_dir == NULL) {
		home_dir = get_home_dir(geteuid());
		goodrv_config.curr_user_home = home_dir;
	}
	return home_dir;
}

char *get_config_dir_curruser() {
	char *conf_dir = goodrv_config.config_dir;
	if (conf_dir == NULL) {
		char *home_dir = get_home_dir_curruser();
		const char *CONF_DIR_SUFFIX = "/.goodrive/";
		conf_dir = malloc(strlen(home_dir) + strlen(CONF_DIR_SUFFIX) + 1);
		strcpy(conf_dir, home_dir);
		strcat(conf_dir, CONF_DIR_SUFFIX);
		goodrv_config.config_dir = conf_dir;
	}
	return conf_dir;
}

char *get_abs_path(char *parent_dir, char *file_name) {
	char *result = NULL;
	if (parent_dir != NULL && file_name != NULL) {
		int pdir_len = strlen(parent_dir);
		int fname_len = strlen(file_name);

		if (parent_dir[pdir_len - 1] == '/') {
			result = malloc(pdir_len + fname_len + 1);
			memcpy(result, parent_dir, pdir_len);
			memcpy(result + pdir_len, file_name, fname_len + 1);
		} else {
			result = malloc(pdir_len + fname_len + 2);
			memcpy(result, parent_dir, pdir_len);
			result[pdir_len] = '/';
			memcpy(result + pdir_len + 1, file_name, fname_len + 1);
		}
	}
	return result;
}

char *md5sum_fsh(char *dir_path) {
	struct stat dir_stat;
	if ((stat(dir_path, &dir_stat) == 0) && S_ISDIR(dir_stat.st_mode)) {
		MD5_CTX md5_ctxt;
		MD5_Init(&md5_ctxt);

		traverse_fsh(dir_path, &update_md5ctx_path_handle, &md5_ctxt);

		unsigned char md5sum_bytes[MD5_DIGEST_LENGTH];
		MD5_Final(md5sum_bytes, &md5_ctxt);
		char *md5sum = (char*) malloc(33);
		for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
			/*
			 * For each byte in the array, there will be two hexadecimal characters.
			 */
			sprintf(md5sum + (i * 2), "%02x", md5sum_bytes[i]);
		}
		return md5sum;
	}
	return NULL;
}

char *md5sum_file(char *file_path) {
	FILE *file = fopen(file_path, "r");
	if (file != NULL) {
		MD5_CTX md5_ctxt;
		MD5_Init(&md5_ctxt);

		char buf[MD5_CBLOCK]; // For reading from the file.
		ssize_t bytes; // bytes read from the file

		while ((bytes = fread(buf, 1, MD5_CBLOCK, file)) > 0) {
			MD5_Update(&md5_ctxt, buf, bytes);
		}

		unsigned char md5sum_bytes[MD5_DIGEST_LENGTH];
		MD5_Final(md5sum_bytes, &md5_ctxt);

		// Convert the MD5Sum from bytes form to char array.
		char *md5sum = (char*) malloc(33);
		for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
			/*
			 * For each byte in the array, there will be two hexadecimal characters.
			 */
			sprintf(md5sum + (i * 2), "%02x", md5sum_bytes[i]);
		}
		fclose(file);

		return md5sum;
	}
	return NULL;
}

char *md5sum_str(char *input) {
	if (input != NULL) {
		MD5_CTX md5_ctxt;
		MD5_Init(&md5_ctxt);
		MD5_Update(&md5_ctxt, input, strlen(input));

		unsigned char md5sum_bytes[MD5_DIGEST_LENGTH];
		MD5_Final(md5sum_bytes, &md5_ctxt);

		// Convert the MD5Sum from bytes form to char array.
		char *md5sum = (char*) malloc(33);
		for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
			/*
			 * For each byte in the array, there will be two hexadecimal characters.
			 */
			sprintf(md5sum + (i * 2), "%02x", md5sum_bytes[i]);
		}
		return md5sum;
	}
	return NULL;
}

void traverse_fsh(char *dirpath, void (*child_handle)(FTSENT*, void*), void *handle_info) {
	char *paths[] = { dirpath, NULL };

	/*
	 * FTS_PHYSICAL option is used since it does not make sense to follow
	 * the symbolic links and find the MD5 checksum for the target file.
	 * TODO Need to examine this decision.
	 */
	FTS *fts = fts_open(paths, FTS_PHYSICAL, NULL);
	fts_read(fts);
	FTSENT *child = fts_children(fts, 0);
	while (child_handle != NULL && child != NULL) {
		child_handle(child, handle_info);
		if (S_ISDIR((child->fts_statp)->st_mode)
				&& has_file_permission_curruser(READ_ACCESS | EXECUTE_ACCESS,
						child->fts_statp)) {
			traverse_fsh(get_full_path(child), child_handle, handle_info);
		}
		child = child->fts_link;
	}
}

int is_group_member(uid_t uid, gid_t gid) {
	struct passwd *passwd_entry = get_passwd_entry(uid);
	struct group *group_entry = get_group_entry(gid);
	char *uname;
	while ((uname = *(group_entry->gr_mem++)) != NULL) {
		if (strcmp(uname, passwd_entry->pw_name) == 0) {
			return 1;
		}
	}
	return 0;
}

int has_file_permission(uid_t uid, int permission, struct stat *file_stat) {
	// Check whether the current user is the owner of the file
	if ((file_stat->st_uid == uid)
			&& (((file_stat->st_mode & S_IRWXU) & (permission * 0100))
					== (permission * 0100))) {
		return 1;
	} else if (is_group_member(uid, file_stat->st_gid)
			&& (((file_stat->st_mode & S_IRWXG) & (permission * 0010))
					== (permission * 0010))) {
		// Check whether the user is a part of the group that owns the file.
		return 1;
	} else if (((file_stat->st_mode & S_IRWXO) & permission) == permission) {
		// When the Others have sufficient privilege for the file
		return 1;
	}
	return 0;
}

int has_file_permission_curruser(int permission, struct stat *file_stat) {
	return has_file_permission(geteuid(), permission, file_stat);
}

char *get_full_path(FTSENT *ftsent) {
	if (ftsent != NULL) {
		short int normalized_path = 0;

		/*
		 * Check whether the path is normalized (ends with a /)
		 */
		if (ftsent->fts_path[strlen(ftsent->fts_path) - 1] == '/') {
			normalized_path = 1;
		}

		char *full_path;
		if (normalized_path) {
			full_path = malloc(ftsent->fts_pathlen + ftsent->fts_namelen + 1);
		} else {
			full_path = malloc(ftsent->fts_pathlen + ftsent->fts_namelen + 2);
		}
		strcpy(full_path, ftsent->fts_path);

		if (!normalized_path) {
			strcat(full_path, "/");
		}

		strcat(full_path, ftsent->fts_name);
		return full_path;
	}
	return NULL;
}

int watch_md5sum_fsh(int fd, char **md5sum_ptr, char *dirpath) {
	if (md5sum_ptr != NULL && dirpath != NULL) {
		struct stat dir_stat;
		if ((stat(dirpath, &dir_stat) == 0) && S_ISDIR(dir_stat.st_mode)) {
			if (fd == -1) {
				fd = inotify_init();
			}

			MD5_CTX md5_ctxt;
			MD5_Init(&md5_ctxt);

			watch_md5sum_handle_info handle_info;
			handle_info.fd = fd;
			handle_info.md5_ctxt = &md5_ctxt;

			traverse_fsh(dirpath, &watch_and_update_md5ctx_handle, &handle_info);

			unsigned char md5sum_bytes[MD5_DIGEST_LENGTH];
			MD5_Final(md5sum_bytes, &md5_ctxt);

			char *md5sum = (char*) malloc(33);
			for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
				/*
				 * For each byte in the array, there will be two hexadecimal characters.
				 */
				sprintf(md5sum + (i * 2), "%02x", md5sum_bytes[i]);
			}
			*md5sum_ptr = md5sum;
			return fd;
		}
	}
	return -1;
}

/* Watch the directory and update the MD5 Context */
static void watch_and_update_md5ctx_handle(FTSENT *ftsent, void *handle_info) {
	watch_md5sum_handle_info *hinfo = (watch_md5sum_handle_info *) handle_info;
	watch_dir_handle(ftsent, &hinfo->fd);
	update_md5ctx_path_handle(ftsent, hinfo->md5_ctxt);
}

/* Add a watch to a path, if it is a directory */
static void watch_dir_handle(FTSENT *ftsent, void *handle_info) {
	if (S_ISDIR((ftsent->fts_statp)->st_mode)) {
		int *fd = (int *) handle_info;
		if (*fd > 0) {
			int wd;
			char *full_path = get_full_path(ftsent);
			if((wd = inotify_add_watch(*fd, full_path, IN_ALL_EVENTS)) == -1) {
				printf("\n Cannot add watch for %s", full_path);
			}
			free(full_path);
		}
	}
}

/*
 * Update the MD5 context with the paths of directory's contents.
 */
static void update_md5ctx_path_handle(FTSENT *ftsent, void *handle_info) {
	MD5_CTX *md5_ctxt = handle_info;
	char *full_path;
	full_path = get_full_path(ftsent);
	MD5_Update(md5_ctxt, full_path, strlen(full_path));
	MD5_Update(md5_ctxt, "\n", 1);	// newline character as the delimiter
}

struct passwd *get_passwd_entry(uid_t uid) {
	struct passwd *passwd_entry;
	char *buf;
	size_t buflen;

	passwd_entry = malloc(sizeof(struct passwd));
	buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (buflen == -1) {
		buflen = 1024;
	}
	buf = malloc(buflen);

	while (1) {
		getpwuid_r(uid, passwd_entry, buf, buflen, &passwd_entry);
		if (errno == ERANGE && buflen < ULONG_MAX) {
			buflen = get_max_value(buflen, buflen * 2, ULONG_MAX);
			buf = realloc(buf, buflen);
		} else {
			break;
		}
	}
	return passwd_entry;
}

struct group *get_group_entry(gid_t gid) {
	struct group *group_entry;
	char *buf;
	size_t buflen;

	group_entry = malloc(sizeof(struct group));
	buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
	if (buflen == -1) {
		buflen = 1024;
	}
	buf = malloc(buflen);

	while (1) {
		getgrgid_r(gid, group_entry, buf, buflen, &group_entry);
		if (errno == ERANGE && buflen != ULONG_MAX) {
			buflen = get_max_value(buflen, buflen*2, ULONG_MAX);
			buf = realloc(buf, buflen);
		} else {
			break;
		}
	}
	return group_entry;
}

static size_t get_max_value(size_t minval, size_t maxval, size_t defval) {
	if (minval > maxval) {
		return defval;
	}
	return maxval;
}
