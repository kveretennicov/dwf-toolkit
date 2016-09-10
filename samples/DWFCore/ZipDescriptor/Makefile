CC=g++
VPATH=/usr/local/lib
LIBS=-ldwfcore 

ZipDescriptor: ZipDescriptor.o $(LIBS)
	$(CC) $^  -o $@

clean:
	rm ZipDescriptor ZipDescriptor.o *.zip 

test:
	./ZipDescriptor
