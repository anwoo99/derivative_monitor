include ./environments

all:
	for ii in lib main quot; do\
		(cd $$ii; $(MAKE) all);\
	done

install:
	for ii in lib main quot; do\
		(cd $$ii; $(MAKE) install);\
	done

clean:
	for ii in lib main quot; do\
		(cd $$ii; $(MAKE) clean);\
	done
