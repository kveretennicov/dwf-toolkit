AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..

utility_includedir = $(pkgincludedir)/utility
utility_include_HEADERS = \
	XMLCompatibilityContext.h \
	XMLCompatibilityExceptions.h \
	XMLCompatibilityInterfaces.h \
	XMLCompatibilityPP.h \
	XMLCompatibilityProcesses.h \
	XMLCompatibilityStack.h \
	XMLCompatibilityStreamFilter.h \
	XMLIdentityWriter.h \
	XMLNamespaceCollection.h \
	XMLQualifiedNameCollection.h 

noinst_LTLIBRARIES =  libdwfcoreutility.la
libdwfcoreutility_la_LDFLAGS = -avoid-version 
libdwfcoreutility_la_SOURCES = \
	XMLCompatibilityContext.cpp \
	XMLCompatibilityPP.cpp \
	XMLCompatibilityProcessChoices.cpp \
	XMLCompatibilityProcessConsider.cpp \
	XMLCompatibilityProcessIgnore.cpp \
	XMLCompatibilityProcessStandard.cpp \
	XMLCompatibilityStack.cpp \
	XMLCompatibilityStreamFilter.cpp \
	XMLIdentityWriter.cpp \
	XMLNamespaceCollection.cpp \
	XMLQualifiedNameCollection.cpp
