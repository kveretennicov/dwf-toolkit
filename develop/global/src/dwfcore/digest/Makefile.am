AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..

digest_includedir = $(pkgincludedir)/digest
digest_include_HEADERS =  md5.h sha1.h
noinst_LTLIBRARIES =  libdwfcoredigest.la
libdwfcoredigest_la_LDFLAGS = -avoid-version 
libdwfcoredigest_la_SOURCES =  md5.c sha1.c
