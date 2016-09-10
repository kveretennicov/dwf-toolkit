AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= SimpleEPlotWriter
SimpleEPlotWriter_HEADERS =	stdafx.h
SimpleEPlotWriter_SOURCES = stdafx.cpp \
		SimpleEPlotWriter.cpp 
SimpleEPlotWriterdir = .
LIBS += -ldwftk -ldwfcore

clean:
	rm SimpleEPlotWriter *.o
test:
	./SimpleEPlotWriter  test.dwf


