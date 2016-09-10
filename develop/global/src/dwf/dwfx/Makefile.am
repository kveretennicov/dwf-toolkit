AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..

dwfx_includedir = $(pkgincludedir)/dwfx

SUBDIRS = reader

noinst_LTLIBRARIES =  libdwfdwfx.la

libdwfdwfx_la_LDFLAGS = -avoid-version

dwfx_include_HEADERS = \
 Constants.h \
 ContentPart.h \
 CustomProperties.h \
 DocumentPresentationsPart.h \
 DWFDocument.h \
 DWFDocumentSequence.h \
 DWFProperties.h \
 DWFSection.h \
 FixedDocument.h \
 FixedDocumentSequence.h \
 FixedPage.h \
 Package.h \
 ResourcePart.h 

libdwfdwfx_la_SOURCES = \
 Constants.cpp \
 ContentPart.cpp \
 CustomProperties.cpp \
 DocumentPresentationsPart.cpp \
 DWFDocument.cpp \
 DWFDocumentSequence.cpp \
 DWFProperties.cpp \
 DWFSection.cpp \
 FixedDocument.cpp \
 FixedDocumentSequence.cpp \
 FixedPage.cpp \
 Package.cpp \
 ResourcePart.cpp

libdwfdwfx_la_LIBADD = $(top_builddir)/dwfx/reader/libdwfdwfxreader.la
