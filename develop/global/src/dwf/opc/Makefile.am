AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..

opc_includedir = $(pkgincludedir)/opc

SUBDIRS = reader

noinst_LTLIBRARIES =  libdwfopc.la

libdwfopc_la_LDFLAGS = -avoid-version

opc_include_HEADERS = \
	Constants.h \
	ContentTypes.h \
	CoreProperties.h \
	Package.h \
	Part.h \
	PartContainer.h \
	PhysicalLayerReader.h \
	PhysicalLayerWriter.h \
	Relationship.h \
	RelationshipContainer.h \
	RelationshipContainerInterface.h \
	XMLPart.h \
	ZipFileReader.h \
	ZipWriter.h

libdwfopc_la_SOURCES = \
	Constants.cpp \
	ContentTypes.cpp \
	CoreProperties.cpp \
	Package.cpp \
	Part.cpp \
	PartContainer.cpp \
	PhysicalLayerReader.cpp \
	PhysicalLayerWriter.cpp \
	Relationship.cpp \
	RelationshipContainer.cpp \
	XMLPart.cpp \
	ZipFileReader.cpp \
	ZipWriter.cpp

libdwfopc_la_LIBADD = $(top_builddir)/opc/reader/libdwfopcreader.la
