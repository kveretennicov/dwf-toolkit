AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..


package_includedir = $(pkgincludedir)/presentation

package_include_HEADERS =  ContentPresentation.h ContentPresentationModelViewNode.h ContentPresentationNode.h ContentPresentationReferenceNode.h ContentPresentationResource.h ContentPresentationView.h ModelScene.h PropertyReference.h ContentPresentationDocument.h PackageContentPresentations.h

SUBDIRS = reader utility

noinst_LTLIBRARIES =  libdwfpresentation.la

libdwfpresentation_la_LDFLAGS = -avoid-version

libdwfpresentation_la_SOURCES =  ContentPresentation.cpp ContentPresentationModelViewNode.cpp ContentPresentationNode.cpp ContentPresentationReferenceNode.cpp ContentPresentationResource.cpp ContentPresentationView.cpp ModelScene.cpp PropertyReference.cpp ContentPresentationDocument.cpp PackageContentPresentations.cpp

libdwfpresentation_la_LIBADD = $(top_builddir)/presentation/reader/libdwfpresentationreader.la $(top_builddir)/presentation/utility/libdwfpresentationutility.la
