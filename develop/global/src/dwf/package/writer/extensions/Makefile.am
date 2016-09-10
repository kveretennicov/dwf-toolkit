AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../../../..

SUBDIRS = 6.01 6.0 6.11 6.20

package_writer_extensions_includedir = $(pkgincludedir)/package/writer/extensions

package_writer_extensions_include_HEADERS =

noinst_LTLIBRARIES =  libdwfpackagewriterextensions.la

libdwfpackagewriterextensions_la_LDFLAGS = -avoid-version

libdwfpackagewriterextensions_la_SOURCES =

libdwfpackagewriterextensions_la_LIBADD = \
				$(top_builddir)/package/writer/extensions/6.01/libdwfpackagewriterextensions601.la \
				$(top_builddir)/package/writer/extensions/6.0/libdwfpackagewriterextensions60.la \
				$(top_builddir)/package/writer/extensions/6.11/libdwfpackagewriterextensions611.la \
				$(top_builddir)/package/writer/extensions/6.20/libdwfpackagewriterextensions620.la

