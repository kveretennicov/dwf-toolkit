AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= ObjectDefinitionReader
ObjectDefinitionReader_HEADERS =	stdafx.h
ObjectDefinitionReader_SOURCES = stdafx.cpp \
		ReadObjectDefinition.cpp 
ObjectDefinitionReaderdir = .
LIBS += -ldwftk -ldwfcore

clean:
	rm ObjectDefinitionReader *.o ObjectDefinitionDump.txt
test:
	./ObjectDefinitionReader Arbor\ Press\ 3D.dwf


