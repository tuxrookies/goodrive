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
#ifndef GOODRV_HASHTABLE_H
#define GOODRV_HASHTABLE_H

/*
 * HashTable using Chained Hashing.
 */
typedef struct hashtable *hashtable;

/*
 * Options for creating the hashtable
 * table_size - Default Table Size.
 * load_factor - Load factor of the hash table.
 * hash_fn - Hash function to calculate the hash code for the key.
 * equals - pointer to a function to check whether two values are equal, or not.
 */
struct hashtable_options {
	unsigned int table_size;
	float load_factor;
	int (*hash_fn)(void *key);
	int (*equals)(void *value1, void *value2);
};

typedef struct hashtable_options *ht_options;

/*
 * Get the default hashtable options. The keys should be string for the default options.
 */
ht_options default_ht_options();

/*
 * The options for creating the hashtable.
 */
hashtable ht_create(ht_options options);

/*
 * Get the number of Key-Value pairs in the hashtable.
 */
unsigned int ht_num_entries(hashtable hashtable);

/*
 * Insert a Key-Value pair into the hashtable.
 * If a Key-Value pair is already present, updates it to new value and returns
 *  the previous value.
 */
void *ht_put(hashtable hashtable, void *key, void *value);

/*
 * Get the value for the given key.
 */
void *ht_get(hashtable hashtable, void *key);

/*
 * Remove the Key-Value pair from the hashtable.
 * Returns the Value.
 */
void *ht_remove(hashtable hashtable, void *key);

/*
 * Check whether a key is present in the hashtable.
 */
int ht_exists(hashtable hashtable, void *key);

#endif /* GOODRV_HASHTABLE_H */
