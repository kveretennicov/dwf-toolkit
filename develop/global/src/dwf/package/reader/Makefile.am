AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../../..

package_reader_includedir = $(pkgincludedir)/package/reader

package_reader_include_HEADERS = \
	ContentReader.h \
	ContentResourceReader.h \
	DataSectionDescriptorReader.h \
	DuplicateAttributeFilter.h \
	EModelSectionDescriptorReader.h \
	EPlotSectionDescriptorReader.h \
	GlobalSectionDescriptorReader.h \
	ObjectDefinitionReader.h \
	PackageManifestReader.h \
	PackageReader.h \
	SectionDescriptorReader.h \
	SignatureReader.h \
	SignatureSectionDescriptorReader.h

noinst_LTLIBRARIES =  libdwfpackagereader.la

libdwfpackagereader_la_LDFLAGS = -avoid-version

libdwfpackagereader_la_SOURCES = \
	ContentReader.cpp \
	ContentResourceReader.cpp \
	DataSectionDescriptorReader.cpp \
	DuplicateAttributeFilter.cpp \
	EModelSectionDescriptorReader.cpp \
	EPlotSectionDescriptorReader.cpp \
	GlobalSectionDescriptorReader.cpp \
	ObjectDefinitionReader.cpp \
	PackageManifestReader.cpp \
	PackageReader.cpp \
	SectionDescriptorReader.cpp \
	SignatureReader.cpp \
	SignatureSectionDescriptorReader.cpp

