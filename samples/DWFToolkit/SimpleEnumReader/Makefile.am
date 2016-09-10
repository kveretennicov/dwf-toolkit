AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= SimpleEnumReader
SimpleEnumReader_HEADERS =	stdafx.h
SimpleEnumReader_SOURCES = stdafx.cpp \
		SimpleEnumReader.cpp 
SimpleEnumReaderdir = .
LIBS += -ldwftk -ldwfcore

clean:
	rm SimpleEnumReader *.o
test:
	./SimpleEnumReader 1\ -\ IRD\ Addition.dwf


