# Not connected with configuration tools

CC=gcc
PROG=tread
LD_LIBRARY_PATH="$(LD_LIBRARY_PATH) ../.libs"

testread:
	${CC} -o $(PROG) ../.libs/ttyproc.o testread.c 
clean:
	rm -f $(PROG) testread.o
