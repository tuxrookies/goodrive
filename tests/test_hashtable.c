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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "test_hashtable.h"
#include "../src/hashtable.h"

/* Helper functions for the test cases */
/* Get the Hash Table options with terrible hash function */
ht_options ht_options_terrible_hashfn();

/* Terrible Hash function */
int terrible_hash_fn(void* key);

/* Test Cases */
/* Test the default Hashtable options */
void test_default_htoptions();
/* Test the hashtable insertion */
void test_hashtable_insertion();
/* Test the hashtable insertion with terrible hash function */
void test_hashtable_insertion_terrible_hashfn();
/* Test hashtable insertion when the key already exists */
void test_hashtable_insert_existing_key();
/* Test hashtable insertion when the key already exists, using terrible hash function */
void test_hashtable_insert_existing_key_terrible_hashfn();
/* Test the hashtable growth */
void test_hashtable_growth();
/* Test the hashtable removal */
void test_hashtable_removal();
/* Test the hashtable removal, with terrible hash function */
void test_hashtable_removal_terrible_hashfn();

/* Register all the test functions here */
void test_hashtable() {
	test_default_htoptions();
	test_hashtable_insertion();
	test_hashtable_insertion_terrible_hashfn();
	test_hashtable_insert_existing_key();
	test_hashtable_insert_existing_key_terrible_hashfn();
	test_hashtable_growth();
	test_hashtable_removal();
	test_hashtable_removal_terrible_hashfn();
}

void test_default_htoptions() {
	ht_options default_options = default_ht_options();
	assert(default_options->table_size == 16);
	assert(default_options->load_factor == 0.75f);
	assert(default_options->hash_fn("goodrive") == 2123187555);
	assert(default_options->equals("foobar", "foobar") == 1);
	assert(default_options->equals("foo", "bar") == 0);
}

void test_hashtable_insertion() {
	hashtable hashtable = ht_create(NULL);
	ht_put(hashtable, "key1", "value1");
	ht_put(hashtable, "key2", "value2");
	ht_put(hashtable, "key3", "value3");

	char* exp_value1 = ht_get(hashtable, "key1");
	char* exp_value2 = ht_get(hashtable, "key2");
	char* exp_value3 = ht_get(hashtable, "key3");

	assert(strcmp(exp_value1, "value1") == 0);
	assert(strcmp(exp_value2, "value2") == 0);
	assert(strcmp(exp_value3, "value3") == 0);
}

void test_hashtable_insertion_terrible_hashfn() {
	ht_options options = ht_options_terrible_hashfn();
	hashtable hashtable = ht_create(options);
	ht_put(hashtable, "key1", "value1");
	ht_put(hashtable, "key2", "value2");
	ht_put(hashtable, "key3", "value3");
	assert(ht_num_entries(hashtable) == 3);

	char* exp_value1 = ht_get(hashtable, "key1");
	char* exp_value2 = ht_get(hashtable, "key2");
	char* exp_value3 = ht_get(hashtable, "key3");

	assert(strcmp(exp_value1, "value1") == 0);
	assert(strcmp(exp_value2, "value2") == 0);
	assert(strcmp(exp_value3, "value3") == 0);
}

void test_hashtable_insert_existing_key() {
	hashtable hashtable = ht_create(NULL);

	char *existing_value;
	existing_value = (char*) ht_put(hashtable, "foo", "bar1");
	assert(existing_value == NULL);
	existing_value = (char*) ht_put(hashtable, "foo", "bar2");
	assert(strcmp(existing_value, "bar1") == 0);

	assert(ht_num_entries(hashtable) == 1);
	char* exp_value = ht_get(hashtable, "foo");
	assert(strcmp(exp_value, "bar2") == 0);
}

void test_hashtable_insert_existing_key_terrible_hashfn() {
	ht_options options = ht_options_terrible_hashfn();
	hashtable hashtable = ht_create(options);

	ht_put(hashtable, "foo1", "bar1");
	ht_put(hashtable, "foo2", "bar2");
	ht_put(hashtable, "foo1", "bar3");

	assert(ht_num_entries(hashtable) == 2);
	char* exp_value = ht_get(hashtable, "foo1");
	assert(strcmp(exp_value, "bar3") == 0);
}

ht_options ht_options_terrible_hashfn() {
	ht_options options = default_ht_options();
	options->hash_fn = &terrible_hash_fn;
	return options;
}

int terrible_hash_fn(void* key) {
	return 10;
}

void test_hashtable_growth() {
	ht_options default_options = default_ht_options();
	default_options->table_size = 2;
	hashtable hashtable = ht_create(default_options);

	ht_put(hashtable, "foo1", "bar1");
	ht_put(hashtable, "foo2", "bar2");
	ht_put(hashtable, "foo3", "bar3");
	ht_put(hashtable, "foo4", "bar4");
	ht_put(hashtable, "foo5", "bar5");
	ht_put(hashtable, "foo6", "bar6");
	ht_put(hashtable, "foo7", "bar7");
	ht_put(hashtable, "foo8", "bar8");
	assert(ht_num_entries(hashtable) == 8);
	assert(strcmp(ht_get(hashtable, "foo1"), "bar1") == 0);
	assert(strcmp(ht_get(hashtable, "foo2"), "bar2") == 0);
	assert(strcmp(ht_get(hashtable, "foo3"), "bar3") == 0);
	assert(strcmp(ht_get(hashtable, "foo4"), "bar4") == 0);
	assert(strcmp(ht_get(hashtable, "foo5"), "bar5") == 0);
	assert(strcmp(ht_get(hashtable, "foo6"), "bar6") == 0);
	assert(strcmp(ht_get(hashtable, "foo7"), "bar7") == 0);
	assert(strcmp(ht_get(hashtable, "foo8"), "bar8") == 0);
}

void test_hashtable_removal() {
	hashtable hashtable = ht_create(NULL);
	ht_put(hashtable, "foo1", "bar1");
	ht_put(hashtable, "foo2", "bar2");

	assert(ht_exists(hashtable, "foo1") == 1);
	ht_remove(hashtable, "foo1");
	assert(ht_exists(hashtable, "foo1") == 0);
}

void test_hashtable_removal_terrible_hashfn() {
	ht_options options = ht_options_terrible_hashfn();
	hashtable hashtable = ht_create(options);
	ht_put(hashtable, "foo1", "bar1");
	ht_put(hashtable, "foo2", "bar2");
	ht_put(hashtable, "foo3", "bar3");
	ht_put(hashtable, "foo4", "bar4");
	ht_put(hashtable, "foo5", "bar5");
	ht_put(hashtable, "foo6", "bar6");
	ht_put(hashtable, "foo7", "bar7");
	ht_put(hashtable, "foo8", "bar8");
	ht_remove(hashtable, "foo4");
	assert(strcmp(ht_get(hashtable, "foo1"), "bar1") == 0);
	assert(strcmp(ht_get(hashtable, "foo2"), "bar2") == 0);
	assert(strcmp(ht_get(hashtable, "foo3"), "bar3") == 0);
	assert(strcmp(ht_get(hashtable, "foo5"), "bar5") == 0);
	assert(strcmp(ht_get(hashtable, "foo6"), "bar6") == 0);
	assert(strcmp(ht_get(hashtable, "foo7"), "bar7") == 0);
	assert(strcmp(ht_get(hashtable, "foo8"), "bar8") == 0);

	assert(ht_exists(hashtable, "foo4") == 0);
}
