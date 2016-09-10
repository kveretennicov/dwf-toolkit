AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= SimpleXPSPublisher
SimpleXPSPublisher_HEADERS =	stdafx.h
SimpleXPSPublisher_SOURCES = stdafx.cpp \
		SimpleXPSPublisher.cpp 
SimpleXPSPublisherdir = .
LIBS += -ldwftk -ldwfcore

clean:
	rm SimpleXPSPublisher *.o
test:
	./SimpleXPSPublisher out.dwf


