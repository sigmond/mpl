MPL_DIR=../lib

all: doc

.PHONY: doc
doc: clean
	cp $(MPL_DIR)/mpl_list.h .
	cp $(MPL_DIR)/mpl_param.h .
	cp $(MPL_DIR)/mpl_file.h .
	cp $(MPL_DIR)/mpl_config.h .
	doxygen Doxyfile

clean:
	rm -f mpl_list.h mpl_param.h mpl_file.h mpl_config.h
	rm -rf html

