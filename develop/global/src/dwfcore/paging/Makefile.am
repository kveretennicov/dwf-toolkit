AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..

paging_includedir = $(pkgincludedir)/paging
paging_include_HEADERS =    Adapter.h \
							Archive.h \
							FileAdapter.h \
							MemoryManager.h \
							PagingFwd.h \
							PagingStrategy.h \
							Proxy.h \
							SqlAdapter.h

noinst_LTLIBRARIES =  libdwfcorepaging.la
libdwfcorepaging_la_LDFLAGS = -avoid-version 
libdwfcorepaging_la_SOURCES = FileAdapter.cpp \
							  MemoryManager.cpp \
							  PagingStrategy.cpp \
							  SqlAdapter.cpp
