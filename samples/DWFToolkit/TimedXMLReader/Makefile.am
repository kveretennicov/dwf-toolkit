AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= TimedXMLReader
TimedXMLReader_HEADERS =	stdafx.h
TimedXMLReader_SOURCES = stdafx.cpp \
		TimedXMLReader.cpp 
TimedXMLReaderdir = .
LIBS += -ldwftk -ldwfcore

clean:
	rm TimedXMLReader *.o
test:
	./TimedXMLReader 1\ -\ IRD\ Addition.dwf


