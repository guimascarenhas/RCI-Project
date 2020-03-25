CC=gcc
CFLAGS=
LIBS= 
SOURCES = dkt.c
OBJECTS = dkt.o


dkt: $(OBJECTS)
		gcc $(CFLAGS) -o $@ $(OBJECTS) $(LIBS)
		
dkt.o = dkt.c

clean::
		rm -f *.o core a.out dkt*~
		
depende::
		makedepend $(SOURCES)

	
