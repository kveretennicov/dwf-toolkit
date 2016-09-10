AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../../..

dwfx_reader_includedir = $(pkgincludedir)/dwfx/reader

dwfx_reader_include_HEADERS = \
 CustomPropertiesReader.h \
 DWFDocumentSequenceReader.h \
 DWFPropertiesReader.h \
 FixedPageResourceExtractor.h

noinst_LTLIBRARIES =  libdwfdwfxreader.la

libdwfdwfxreader_la_LDFLAGS = -avoid-version

libdwfdwfxreader_la_SOURCES = \
 CustomPropertiesReader.cpp \
 DWFDocumentSequenceReader.cpp \
 DWFPropertiesReader.cpp \
 FixedPageResourceExtractor.cpp 


