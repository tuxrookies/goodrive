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

#ifndef GOODRV_LINUX_API_H
#define GOODRV_LINUX_API_H

#include <fts.h>
#include <sys/stat.h>

#define FULL_ACCESS 07
#define READ_ACCESS 04
#define WRITE_ACCESS 02
#define EXECUTE_ACCESS 01

/*
 * Get the User's home directory
 */
char* get_home_dir(uid_t uid);

/*
 * Get the Home directory of the current user (effective user)
 */
char* get_home_dir_curruser();

/*
 * Get the user's config Directory
 */
char* get_config_dir_curruser();

/*
 * Find the MD5Sum of the file hierarchy within a directory recursively.
 */
char* md5sum_fsh(char *dir_path);

/*
 * Get the MD5 Sum of the file. Returns Null when the file is not accessible
 * for any reason.
 *
 * path - path of the file for which the MD5 checksum is to be calculated.
 */
char* md5sum_file(char* file_path);

/*
 * Traverse the File System Hierarchy within a directory recursively.
 * Call the handle, whenever a child is encountered.
 */
void traverse_fsh(char *dir_path, void (*child_handle)(FTSENT*, void*), void *handle_info);

/*
 * Check whether the user with UID is present in the group with GID?
 * uid - The UID of the user
 * gid - The GID of the group
 */
int is_group_member(uid_t uid, gid_t gid);

/*
 * Check whether the user has file permission for the file
 * uid - The UID of the user for whom the permission needs to be tested
 * permission - Mode of the necessary permissions in Octal
 * file_stat - struct stat for file information
 * Returns 1 if the user has the specified permission, else 0
 */
int has_file_permission(uid_t uid, int permission, struct stat *file_stat);

/*
 * Check whether the current user has the stated permission for the file
 * permission - Mode of the necessary permissions in Octal
 * file_stat - struct stat for file information
 * Returns 1 if the user has the specified permission, else 0
 */
int has_file_permission_curruser(int permission, struct stat *file_stat);

/*
 * Get the Full Path of the FTSENT
 */
char* get_full_path(FTSENT* ftsent);

/*
 * Place watches in the File System Hierarchy represented by the dirpath,
 * and also find the MD5 Checksum of that File System Hierarchy.
 *
 * Both placing watches and finding the MD5 sum are recursive.
 *
 * Params
 * =======
 * fd - File Descriptor for the inotify instance. If this is -1, a new inotify
 * 		instance will be created.
 * md5sum_ptr - Pointer to the char* where MD5 Sum should be stored.
 * dirpath - Directory path for placing watches and finding MD5 sum.
 *
 * Returns
 * ========
 * On success, the File Descriptor for the inotify instance. If for some reason,
 * the inotify instance creation failed, or MD5 checksum calculation failed, then
 * -1 will be returned
 */
int watch_md5sum_fsh(int fd, char **md5sum_ptr, char *dirpath);

#endif /* GOODRV_LINUX_API_H */
