include ../environments

BIN=$(BIN_DIR)

I=../inc
L=../lib/fep
CFLAGS=$($(PLATFORM)_CFLAGS) $(QUOT_CFLAGS) -I../inc
LFLAGS=$($(PLATFORM)_LFLAGS) -L$(L) -lFEP
AFLAGS=$(MY_AFLAGS)
AROPTS=$(MY_AROPTS)

LIBOBJ=libobj.o
INC= $(I)/push.h $(I)/main.h $(I)/config.h ./rqsvr.h

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

all:		feppush fepreal

feppush:	feppush.o $(LIBOBJ)
		$(CC) -o feppush feppush.o $(LIBOBJ) $(CFLAGS) $(LFLAGS)
foxpush.o:	$(INC)

$(LIBOBJ):	$(LIBOBJ)(config.o) 	$(LIBOBJ)(init.o)	\
		$(LIBOBJ)(client.o)	$(LIBOBJ)(recv.o) 	\
		$(LIBOBJ)(send.o) 	$(LIBOBJ)(board.o) 	\
		$(LIBOBJ)(packet.o) 	$(LIBOBJ)(admin.o)
	   	if [ -s /usr/bin/ranlib ]; then ranlib $(LIBOBJ); fi

$(LIBOBJ)(config.o): 	$(INC)
$(LIBOBJ)(init.o): 	$(INC)
$(LIBOBJ)(client.o): 	$(INC)
$(LIBOBJ)(recv.o): 	$(INC)
$(LIBOBJ)(send.o): 	$(INC)
$(LIBOBJ)(board.o): 	$(INC)
$(LIBOBJ)(packet.o): 	$(INC)
$(LIBOBJ)(admin.o): 	$(INC)

fepreal:	$(INC)  fepreal.c
		$(CC) -o fepreal fepreal.c $(CFLAGS) $(LFLAGS)

install: all
	install feppush $(BIN)
	install fepreal $(BIN)
	rm -f $(BIN)/fepsymb
	ln -s $(BIN)/fepreal $(BIN)/fepsymb
clean:
	-rm -f $(LIBOBJ) *.o feppush 

