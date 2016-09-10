AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..

ansi_includedir = $(pkgincludedir)/ansi
zip_includedir = zip

ansi_include_HEADERS = Core.h Mutex.h Semaphore.h Signal.h StandardFileDescriptor.h Thread.h Types.h UUID.h
noinst_LTLIBRARIES =  libdwfcoreansi.la
libdwfcoreansi_la_LDFLAGS = -avoid-version 
libdwfcoreansi_la_SOURCES = Core.cpp Mutex.cpp Semaphore.cpp Signal.cpp StandardFileDescriptor.cpp StreamFileDescriptor.cpp String.cpp TempFile.cpp Thread.cpp UUID.cpp
