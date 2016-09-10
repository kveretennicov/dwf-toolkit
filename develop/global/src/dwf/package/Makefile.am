AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..


package_includedir = $(pkgincludedir)/package

package_include_HEADERS =  Bookmark.h \
	Class.h \
	Constants.h \
	Content.h \
	ContentElement.h \
	ContentManager.h \
	CoordinateSystem.h \
	CustomSection.h \
	DataSection.h \
	DefinedObject.h \
	Dependency.h \
	DwfResult.h \
	EModelSection.h \
	Entity.h \
	EPlotSection.h \
	Feature.h \
	FontResource.h \
	GlobalSection.h \
	GraphicResource.h \
	Group.h \
	Instance.h \
	Interface.h \
	Manifest.h \
	Object.h \
	ObjectDefinition.h \
	ObjectDefinitionResource.h \
	Paper.h \
	Property.h \
	PropertySet.h \
	Renderable.h \
	Resource.h \
	Section.h \
	SectionBuilder.h \
	SectionContentResource.h \
	Signature.h \
	SignatureRequest.h \
	SignatureSection.h \
	SignatureResource.h \
	Source.h \
	Units.h \
	X509.h \
	XML.h

SUBDIRS = reader writer utility

noinst_LTLIBRARIES =  libdwfpackage.la

libdwfpackage_la_LDFLAGS = -avoid-version

libdwfpackage_la_SOURCES =  Bookmark.cpp \
	Class.cpp \
	Constants.cpp \
	Content.cpp \
	ContentElement.cpp \
	ContentManager.cpp \
	CoordinateSystem.cpp \
	CustomSection.cpp \
	DataSection.cpp \
	DefinedObject.cpp \
	EModelSection.cpp \
	Entity.cpp \
	EPlotSection.cpp \
	Feature.cpp \
	FontResource.cpp \
	GlobalSection.cpp \
	GraphicResource.cpp \
	Group.cpp \
	Instance.cpp \
	Interface.cpp \
	Manifest.cpp \
	Object.cpp \
	ObjectDefinition.cpp \
	ObjectDefinitionResource.cpp \
	Paper.cpp \
	Property.cpp \
	PropertySet.cpp \
	Resource.cpp \
	Section.cpp \
	SectionBuilder.cpp \
	SectionContentResource.cpp \
	Signature.cpp \
	SignatureRequest.cpp \
	SignatureResource.cpp \
	SignatureSection.cpp \
	Source.cpp \
	Units.cpp \
	X509.cpp \
	XML.cpp

libdwfpackage_la_LIBADD = $(top_builddir)/package/utility/libdwfpackageutility.la $(top_builddir)/package/writer/libdwfpackagewriter.la $(top_builddir)/package/reader/libdwfpackagereader.la
