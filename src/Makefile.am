AM_CFLAGS = --pedantic -Wall -std=c99 -D _GNU_SOURCE $(OPENSSL_CFLAGS)
AM_LDFLAGS = 

# GooDrive Binaries
bin_PROGRAMS = goodrive
goodrive_SOURCES = config.h linux-api.h linux-api.c main.c
goodrive_LDADD = $(OPENSSL_LIBS)
