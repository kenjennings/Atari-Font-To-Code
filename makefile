
EXE=atf2code

all: $(EXE) 

# linux CFLAGS=-Wall -I.
# solaris CFLAGS=-I.
CFLAGS=-Wall -I.

ATF2CODE_OBJS=io.o E000.o myglobals.o globals.o main.o 


.c.o: $*.c $*.h
	@echo $*.c
	$(CC) $(CFLAGS) -c $*.c

$(EXE): $(ATF2CODE_OBJS)
	$(CC) $(LDFLAGS) -o $(EXE) $(ATF2CODE_OBJS)

clean: 
	rm -f $(EXE) *.o *~

install:
	cp $(EXE) ~/bin


