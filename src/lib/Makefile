include ../environments

all:
	for ii in main pen; do\
		(cd $$ii; $(MAKE) all);\
	done

install:
	for ii in main pen; do\
		(cd $$ii; $(MAKE) install);\
	done

clean:
	for ii in main pen; do\
		(cd $$ii; $(MAKE) clean);\
	done
