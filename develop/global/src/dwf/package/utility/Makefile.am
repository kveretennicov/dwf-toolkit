AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../../..


package_utility_includedir = $(pkgincludedir)/package/utility

package_utility_include_HEADERS =  DefinedObjectContainer.h DWFPackageFileDescriptor.h PropertyContainer.h ResourceContainer.h

noinst_LTLIBRARIES =  libdwfpackageutility.la

libdwfpackageutility_la_LDFLAGS = -avoid-version

libdwfpackageutility_la_SOURCES =  DefinedObjectContainer.cpp DWFPackageFileDescriptor.cpp PropertyContainer.cpp ResourceContainer.cpp
