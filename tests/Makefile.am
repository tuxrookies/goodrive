AM_CFLAGS = --pedantic -Wall -std=c99 -D _GNU_SOURCE $(OPENSSL_CFLAGS) -I../src/
AM_LDFLAGS = 
AM_TESTS_FD_REDIRECT = 9>&2

#
TESTS = $(check_PROGRAMS)

check_PROGRAMS = hashtable_test linux_api_test
hashtable_test_SOURCES = ../src/hashtable.h ../src/hashtable.c test_hashtable.c

linux_api_test_SOURCES = ../src/linux-api.h ../src/linux-api.c test_linux_api.c
linux_api_test_LDADD = $(OPENSSL_LIBS) 
