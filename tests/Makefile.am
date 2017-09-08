AM_CFLAGS = --pedantic -Wall -std=c99 -D _GNU_SOURCE $(OPENSSL_CFLAGS)
AM_LDFLAGS = 
AM_TESTS_FD_REDIRECT = 9>&2

#
TESTS = tests

check_PROGRAMS = tests
tests_SOURCES = ../src/hashtable.h ../src/hashtable.c main.c test_hashtable.c
