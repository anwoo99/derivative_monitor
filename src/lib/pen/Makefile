include ../../environments

LIB=$(LIB_DIR)
INC=$(INC_DIR)

CFLAGS=-O $(MY_CFLAGS) $(MY_SFLAGS) -I../../inc
LFLAGS=$(MY_LFLAGS) 
AFLAGS=$(MY_AFLAGS)
AROPTS=$(MY_AROPTS)

LIBOBJ=libobj.o
X_PENLIB=libpen.so


LOBJS=$(LIBOBJ) $(X_BUSLIB) *.o *.a *.so
PENINC=../../inc/pen.h

PENOBJ= term.o getkey.o

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@
.c~:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(CC) $(CFLAGS) $(LFLAGS) $*.c -o $@
	-rm -f $*.c
.c.a:
	$(CC) -c $(CFLAGS) $<
	$(AR) $(AFLAGS) $@ $*.o; rm -f $*.o 
.s~.s:
	$(GET) $(GFLAGS) $<
.h~.h:
	$(GET) $(GFLAGS) $<
.c~.c:
	$(GET) $(GFLAGS) $<

all:	$(X_PENLIB)
$(X_PENLIB): $(LIBOBJ)
	$(AR) $(AROPTS) -x $(LIBOBJ) $(PENOBJ)
	if [ "${PLATFORM}" = "AIX" ]; then \
		$(CC) -o $(X_PENLIB) $(PENOBJ) -bE:export.pen.aix -bM:SRE -bnoentry -lc $(CFLAGS) $(LFLAGS); \
	fi;

	if [ "${PLATFORM}" = "SunOS" ]; then \
		ld -b -G -o $(X_PENLIB) $(PENOBJ) -lc ${socketlib} -Mexport.pen.sun; \
	fi;

	if [ "${PLATFORM}" = "HPUX" ]; then \
		ld -b -o $(X_PENLIB) $(PENOBJ) -c export.pen; \
	fi;
	if [ "${PLATFORM}" = "LINUX" ]; then \
		ld -shared -o $(X_PENLIB) $(PENOBJ) -lc -lnsl -Map export.pen; \
	fi;
	rm -f $(PENOBJ);

$(LIBOBJ): $(LIBOBJ)(term.o)		$(LIBOBJ)(getkey.o)
	   if [ -s /usr/bin/ranlib ]; then ranlib $(LIBOBJ); fi
	   @echo $(LIBOBJ) is up-to-date.

$(LIBOBJ)(term.o): 	$(PENINC)
$(LIBOBJ)(getkey.o): 	$(PENINC)

install: all
	install $(X_PENLIB) $(LIB)
clean:	
	rm -f $(LOBJS) *.o *.a *.so
