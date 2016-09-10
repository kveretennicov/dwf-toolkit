AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= 3DPublish
3DPublish_HEADERS =	stdafx.h
3DPublish_SOURCES = stdafx.cpp \
		3DPublish_UTF8.cpp 
3DPublishdir = .
LIBS += -ldwftk -ldwfcore

clean:
	rm 3DPublish *.o
test:
	./3DPublish out.dwf



