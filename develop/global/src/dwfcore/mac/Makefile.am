AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..

mac_includedir = $(pkgincludedir)/mac
mac_include_HEADERS = Core.h Types.h

noinst_LTLIBRARIES =  libdwfcoremac.la
libdwfcoremac_la_LDFLAGS = -avoid-version 
libdwfcoremac_la_SOURCES = Core.cpp Timer.cpp
