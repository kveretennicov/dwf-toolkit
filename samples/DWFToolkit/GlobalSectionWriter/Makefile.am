AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= GlobalSectionWriter
GlobalSectionWriter_HEADERS =	stdafx.h
GlobalSectionWriter_SOURCES = stdafx.cpp \
		GlobalSectionWriter.cpp 
GlobalSectionWriterdir = .
LIBS += -ldwftk -ldwfcore

clean:
	rm GlobalSectionWriter *.o
test:
	cp ../Aggregate/content.dwf .
	chmod +w content.dwf
	./GlobalSectionWriter content.dwf
	rm content.dwf


