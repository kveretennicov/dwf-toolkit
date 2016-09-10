CC=g++
VPATH=/usr/local/lib

LIBS= -ldwfcore 
Strings: Strings.o $(LIBS)
	$(CC) $^  -o $@

clean:
	rm Strings Strings.o Strings.txt

test:
	./Strings
