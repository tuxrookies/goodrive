AM_CFLAGS = --pedantic -Wall -std=c99 -D _GNU_SOURCE $(OPENSSL_CFLAGS)

JSONC_CFLAGS = $(shell pkg-config --cflags json-c)
AM_CFLAGS += $(JSONC_CFLAGS) -g3 -O0

AM_LDFLAGS =

# GooDrive Binaries
bin_PROGRAMS = goodrive
goodrive_SOURCES = base64url.h base64url.c config.h linux-api.h linux-api.c jwt.h jwt.c main.c

goodrive_LDADD = $(OPENSSL_LIBS) -ljson-c
