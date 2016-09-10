AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../../..

SUBDIRS = extensions

package_writer_includedir = $(pkgincludedir)/package/writer

package_writer_include_HEADERS =  DWF6PackageVersionExtension.h DWF6PackageWriter.h DWFXPackageWriter.h PackageWriter.h DWFXPackageVersionExtension.h

noinst_LTLIBRARIES =  libdwfpackagewriter.la

libdwfpackagewriter_la_LDFLAGS = -avoid-version

libdwfpackagewriter_la_SOURCES =   DWF6PackageWriter.cpp DWFXPackageWriter.cpp PackageWriter.cpp

libdwfpackagewriter_la_LIBADD = $(top_builddir)/package/writer/extensions/libdwfpackagewriterextensions.la
