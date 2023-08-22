include ../environments

LIB=$(LIB_DIR)
INC=$(INC_DIR)

LD=ld
IFLAGS=-I../inc -I../../inc
CFLAGS=-O $(IFLAGS) $(MY_CFLAGS) $(MY_SFLAGS)
LFLAGS=$(MY_LFLAGS) -L../lib/main -lMAIN 
AFLAGS=$(MY_AFLAGS)

INCF=../inc/main.h ../inc/context.h ../inc/config.h ../inc/parson.h ../inc/schema.h
CMD=monrecv


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

all: $(CMD)

monrecv: monrecv.o $(INCF)
	$(CC) -o monrecv monrecv.o $(LFLAGS)

install: all
	cp -f $(CMD) $(BIN_DIR)

clean:
	rm -f *.o $(CMD)