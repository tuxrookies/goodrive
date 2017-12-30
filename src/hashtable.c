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

#include<limits.h>
#include<stddef.h>
#include<stdlib.h>
#include<string.h>
#include "hashtable.h"

/*
 * Hashtable
 */
struct hashtable {
	unsigned int table_size;
	float load_factor;
	int (*hash_fn)(void *key);
	int (*equals)(void *value1, void *value2);
	unsigned int num_entries;
	struct bucket_elem **table;
};

/*
 * Each element in the bucket.
 */
struct bucket_elem {
	struct bucket_elem *next;
	void *key;
	void *value;
};

/*
 * Check whether the addition of 1 more element would cross the threshold of load_factor. If so, increase
 * the table size for the HashTable.
 */
static void check_and_resize(hashtable hashtable);

/*
 * Insert a Key-Value pair into the bucket.
 * bucket_ptr - Pointer to the bucket.
 * equals - A pointer to a function that checks whether the keys are equal (to update existing key, we need this).
 * key - Key
 * value - Value
 */
static void insert_in_bucket(struct bucket_elem **bucket_ptr, int (*equals)(void*, void*), void *key, void *value);

/*
 * Compute the hash code for the input string.
 */
static int hash_fn_string(void *input) {
	char *input_str = input;
	int hash = 0;
	int input_len = strlen(input_str);
	for (char *ptr = input_str; ptr < (input_str + input_len); ptr++) {
		hash = 31 * hash + *ptr;
	}
	return hash;
}

/*
 * Returns 1 if two strings - value1 and value 2 are equal, else returns 0.
 */
static int equals_string(void *value1, void *value2) {
	char *str1 = value1;
	char *str2 = value2;
	return strcmp(str1, str2) == 0;
}

ht_options default_ht_options() {
	ht_options default_options = malloc(sizeof(struct hashtable_options));
	default_options->table_size = 16;
	default_options->load_factor = 0.75f;
	default_options->hash_fn = &hash_fn_string;
	default_options->equals = &equals_string;
	return default_options;
}

hashtable ht_create(ht_options options) {
	if (options == NULL) {
		options = default_ht_options();
	}
	hashtable hashtable = malloc(sizeof(struct hashtable));
	hashtable->table_size = options->table_size;
	hashtable->load_factor = options->load_factor;
	hashtable->hash_fn = options->hash_fn;
	hashtable->equals = options->equals;
	hashtable->num_entries = 0;

	hashtable->table = malloc(sizeof(struct bucket_elem*) * hashtable->table_size);
	return hashtable;
}

void *ht_put(hashtable hashtable, void *key, void *value) {
	check_and_resize(hashtable);
	int hash_code = hashtable->hash_fn(key);
	int hash_value = hash_code % hashtable->table_size;
	void *old_value = ht_get(hashtable, key);
	int exists_already = ht_exists(hashtable, key);
	insert_in_bucket(&hashtable->table[hash_value], hashtable->equals, key, value);
	if (!exists_already) {
		hashtable->num_entries++;
	}
	return old_value;
}

void *ht_get(hashtable hashtable, void *key) {
	int hash_code = hashtable->hash_fn(key);
	int hash_value = hash_code % hashtable->table_size;
	struct bucket_elem *elem = hashtable->table[hash_value];
	while (elem) {
		if (hashtable->equals(elem->key, key)) {
			return elem->value;
		}

		elem = elem->next;
	}
	return NULL;
}

void *ht_remove(hashtable hashtable, void *key) {
	int hash_code = hashtable->hash_fn(key);
	int hash_value = hash_code % hashtable->table_size;
	struct bucket_elem **bucket_ptr = &hashtable->table[hash_value];
	struct bucket_elem *elem = hashtable->table[hash_value];

	while (elem) {
		if (hashtable->equals(elem->key, key)) {
			void *value = elem->value;
			*bucket_ptr = elem->next;

			hashtable->num_entries--;
			free(elem);
			return value;
		}

		bucket_ptr = &elem->next;
		elem = elem->next;
	}

	return NULL;
}

unsigned int ht_num_entries(hashtable hashtable) {
	return hashtable->num_entries;
}

int ht_exists(hashtable hashtable, void *key) {
	int hash_code = hashtable->hash_fn(key);
	int hash_value = hash_code % hashtable->table_size;
	struct bucket_elem *elem = hashtable->table[hash_value];
	while (elem) {
		if (hashtable->equals(elem->key, key)) {
			return 1;
		}

		elem = elem->next;
	}
	return 0;
}

static void insert_in_bucket(struct bucket_elem **bucket_ptr,
		int (*equals)(void*, void*), void *key, void *value) {
	struct bucket_elem *elem = *bucket_ptr;

	while (elem) {
		/* If the key already exists, just update the value and return */
		if (equals(elem->key, key)) {
			elem->value = value;
			return;
		}

		bucket_ptr = &elem->next;
		elem = elem->next;
	}

	struct bucket_elem *new_elem = malloc(sizeof(struct bucket_elem));
	new_elem->key = key;
	new_elem->value = value;
	new_elem->next = NULL;

	*bucket_ptr = new_elem;
}

static void check_and_resize(hashtable hashtable) {
	int threshold = hashtable->table_size * hashtable->load_factor;

	/* If the addition of an entry does not cross the threshold, do not increase the table size */
	if (hashtable->num_entries + 1 <= threshold) {
		return;
	}

	/* If the new size is less than the old size, do nothing */
	unsigned int new_size = hashtable->table_size << 1;
	if (new_size <= hashtable->table_size) {
		return;
	}

	unsigned int old_size = hashtable->table_size;
	struct bucket_elem **new_table = malloc(
			sizeof(struct bucket_elem*) * new_size);
	struct bucket_elem **elem_ptr = hashtable->table;

	struct bucket_elem *elem, *temp;
	int new_index;

	for (; elem_ptr < hashtable->table + old_size; elem_ptr++) {
		elem = *elem_ptr;
		while (elem) {
			new_index = hashtable->hash_fn(elem->key) % new_size;
			insert_in_bucket(new_table + new_index, hashtable->equals,
					elem->key, elem->value);
			temp = elem;
			elem = elem->next;
			free(temp);
		}
	}

	free(hashtable->table);
	hashtable->table = new_table;
	hashtable->table_size = new_size;
}
