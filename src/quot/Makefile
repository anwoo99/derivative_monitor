include ../environments

LIB=$(LIB_DIR)
INC=$(INC_DIR)

CFLAGS=-O -I../inc -I../../inc $(MY_CFLAGS)
LFLAGS=$(MY_LFLAGS) -L../lib/main -lMAIN -L../lib/pen -lpen -lcurses
AFLAGS=$(MY_AFLAGS)
AROPTS=$(MY_AROPTS)

LIBOBJ=libOBJ.o
INCINC=../inc/main.h ../inc/context.h ../inc/config.h ../inc/parson.h ../inc/schema.h ../inc/extpkt.h \
       ../inc/oldpkt.h ../inc/quot.h ../inc/pen.h ../inc/push.h
CMD=monquot

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

all:		$(CMD)

monquot:	monquot.c $(INCINC) $(LIBOBJ)
		$(CC) -o monquot monquot.c $(LIBOBJ) $(CFLAGS) $(LFLAGS)

$(LIBOBJ): 	$(LIBOBJ)(map.o)	$(LIBOBJ)(realtime.o)	\
	   	$(LIBOBJ)(query.o)	$(LIBOBJ)(update.o)	
	   	if [ -s /usr/bin/ranlib ]; then ranlib $(LIBOBJ); fi
	   	@echo $(LIBOBJ) is up-to-date.

$(LIBOBJ)(map.o): 		$(INCINC)
$(LIBOBJ)(query.o): 	$(INCINC)
$(LIBOBJ)(realtime.o): 	$(INCINC)
$(LIBOBJ)(update.o): 	$(INCINC)

install: all
	install $(CMD) $(BIN_DIR)
clean:	
	rm -f *.o $(CMD)
