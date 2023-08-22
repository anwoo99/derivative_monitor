include ../environments

all:
	for ii in fep parser; do\
		(cd $$ii; $(MAKE) all);\
	done

install:
	for ii in fep parser; do\
		(cd $$ii; $(MAKE) install);\
	done

clean:
	for ii in fep parser; do\
		(cd $$ii; $(MAKE) clean);\
	done
