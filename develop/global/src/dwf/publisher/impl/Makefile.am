AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../../..

publisher_impl_includedir = $(pkgincludedir)/publisher/impl
publisher_impl_include_HEADERS =  ContentElementPropertyVisitor.h DefinedObjectPropertyVisitor.h DWF6PackagePublisher.h DWFPackagePublisher.h DWFResourceVisitor.h DWFXPackagePublisher.h PublishedContentElement.h PublishedContentElementListener.h PublishedDefinedObject.h
noinst_LTLIBRARIES =  libdwftkpublisherimpl.la
libdwftkpublisherimpl_la_LDFLAGS = -avoid-version 
libdwftkpublisherimpl_la_SOURCES = ContentElementPropertyVisitor.cpp DefinedObjectPropertyVisitor.cpp DWF6PackagePublisher.cpp DWFPackagePublisher.cpp DWFXPackagePublisher.cpp PublishedContentElement.cpp PublishedDefinedObject.cpp
