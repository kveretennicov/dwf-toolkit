AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..

xps_includedir = $(pkgincludedir)/xps

xps_include_HEADERS = Constants.h FixedDocument.h FixedDocumentSequence.h FixedPage.h Package.h XPSDocRefResourceExtractor.h XPSFixedDocumentResourceExtractor.h XPSFixedPageResourceExtractor.h XPSFontResourceExtractor.h

noinst_LTLIBRARIES =  libdwfxps.la

libdwfxps_la_LDFLAGS = -avoid-version

libdwfxps_la_SOURCES = Constants.cpp FixedDocument.cpp FixedDocumentSequence.cpp FixedPage.cpp Package.cpp XPSDocRefResourceExtractor.cpp XPSFixedDocumentResourceExtractor.cpp XPSFixedPageResourceExtractor.cpp XPSFontResourceExtractor.cpp
