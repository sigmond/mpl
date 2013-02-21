DIRS=compiler test example doc

.PHONY: force

all: force
	for dir in $(DIRS) ; do \
		(cd $$dir; make) \
	done

doc: force
	for dir in $(DIRS) ; do \
		(cd $$dir; make doc) \
	done

check: force
	for dir in $(DIRS) ; do \
		(cd $$dir; make check) \
	done

memcheck: force
	for dir in $(DIRS) ; do \
		(cd $$dir; make memcheck) \
	done

analyze_memcheck: force
	for dir in $(DIRS) ; do \
		(cd $$dir; make analyze_memcheck) \
	done

clean: force
	for dir in $(DIRS) ; do \
		(cd $$dir; make clean) \
	done
	rm -f lib/*~
	rm -f *~

distclean: force clean
	for dir in $(DIRS) ; do \
		(cd $$dir; make distclean) \
	done
