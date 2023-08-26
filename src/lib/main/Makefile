include ../../environments

LIB=$(LIB_DIR)
INC=$(INC_DIR)

LD=ld
IFLAGS=-I../inc -I../../inc
CFLAGS=-O $(IFLAGS) $(MY_CFLAGS) $(MY_SFLAGS)
LFLAGS=$(MY_LFLAGS) -shared -lc -lnsl 
AFLAGS=$(MY_AFLAGS)


X_GFLIB=libMAIN.a
S_GFLIB=libMAIN.so

OBJF=config.o log.o open.o shm.o parson.o close.o file.o time.o folder.o

INCF=../../main.h ../../context.h ../../config.h ../../parson.h ../../schema.h

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

all: $(S_GFLIB)

$(S_GFLIB): $(X_GFLIB)
	$(AR) -x $(X_GFLIB) $(OBJF)
	$(LD) -o $(S_GFLIB) $(OBJF) $(LFLAGS)
	rm -f $(OBJF)

$(X_GFLIB): $(OBJF)
	$(AR) -rcs $@ $(OBJF)
	if [ -s /usr/bin/ranlib ]; then ranlib $@; fi
	@echo $(X_GFLIB) is up-to-date.

install: all
	install -m 0644 ../../inc/main.h $(INC)
	install -m 0644 ../../inc/context.h $(INC)
	install -m 0644 ../../inc/config.h $(INC)
	install -m 0644 ../../inc/parson.h $(INC)
	install -m 0644 ../../inc/schema.h $(INC)
	install $(X_GFLIB) $(LIB)/$(X_GFLIB)
	install $(S_GFLIB) $(LIB)/$(S_GFLIB)

clean:
	rm -f $(X_GFLIB)
	rm -f $(S_GFLIB)
	rm -f $(OBJF)
