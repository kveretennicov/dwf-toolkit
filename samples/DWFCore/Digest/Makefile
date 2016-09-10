CC=g++
.LIBPATTERNS = lib%.dylib lib%.so lib%.a
VPATH=/usr/lib/ /usr/local/lib
LIBS=-ldwfcore -lexpat
Digest: main.o $(LIBS)
	$(CC) $^  -o $@

clean:
	rm Digest main.o Out_*.txt

test:
	./Digest
