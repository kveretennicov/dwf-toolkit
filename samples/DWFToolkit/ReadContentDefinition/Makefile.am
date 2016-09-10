AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= ReadContentDefinition
ReadContentDefinition_SOURCES = ReadContentDefinition.cpp 

LIBS += -ldwftk -ldwfcore

clean:
	rm ReadContentDefinition *.o
test:
	./ReadContentDefinition content.dwf
	./ReadContentDefinition 3DPublishOut.dwf


