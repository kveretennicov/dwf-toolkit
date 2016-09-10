AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= SimpleW3DStreamProcessor
SimpleW3DStreamProcessor_HEADERS =	stdafx.h
SimpleW3DStreamProcessor_SOURCES = stdafx.cpp \
		SimpleW3DStreamProcessor.cpp 
SimpleW3DStreamProcessordir = .
LIBS += -ldwftk -ldwfcore

clean:
	rm SimpleW3DStreamProcessor *.o
test:
	./SimpleW3DStreamProcessor ../Aggregate/3DPublishOut.dwf


