AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I..

pkginclude_HEADERS =  Toolkit.h Version.h

lib_LTLIBRARIES =  libdwftk.la

libdwftk_la_LDFLAGS = -release 7.7.0

libdwftk_la_SOURCES =  Toolkit.cpp

SUBDIRS = package whiptk XAML w3dtk publisher presentation opc xps dwfx 

libdwftk_la_LIBADD = \
				$(top_builddir)/whiptk/libdwfwhiptk.la \
				$(top_builddir)/XAML/libdwfXAML.la \
				$(top_builddir)/w3dtk/libdwfw3dtk.la \
				$(top_builddir)/package/libdwfpackage.la \
				$(top_builddir)/publisher/libdwftkpublisher.la \
				$(top_builddir)/presentation/libdwfpresentation.la \
				$(top_builddir)/opc/libdwfopc.la \
				$(top_builddir)/xps/libdwfxps.la \
				$(top_builddir)/dwfx/libdwfdwfx.la

