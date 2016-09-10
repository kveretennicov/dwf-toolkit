AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../../..

opc_reader_includedir = $(pkgincludedir)/opc/reader

opc_reader_include_HEADERS = \
 CorePropertiesReader.h \
 RelationshipContainerReader.h \
 XMLElementBuilder.h

noinst_LTLIBRARIES =  libdwfopcreader.la

libdwfopcreader_la_LDFLAGS = -avoid-version

libdwfopcreader_la_SOURCES = \
 CorePropertiesReader.cpp \
 RelationshipContainerReader.cpp


