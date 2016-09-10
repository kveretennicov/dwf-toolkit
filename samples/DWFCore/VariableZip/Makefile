CC=g++
VPATH=/usr/local/lib
LIBS=-ldwfcore 
VariableZip: VariableZip.o $(LIBS)
	$(CC) $^  -o $@

clean:
	rm VariableZip *.o Files.txt Out.* FilesArchive.zip

test:
	./VariableZip
