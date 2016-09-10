AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= Aggregate
Aggregate_SOURCES = Aggregate.cpp 
LIBS += -ldwftk -ldwfcore

clean:
	rm Aggregate Aggregated.dwf *.o
test:
	./Aggregate content.dwf -m p 3DPublishOut.dwf



