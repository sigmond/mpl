MPL_DIR=../lib

SRCS := 


CFLAGS :=  $(GCOV_CFLAGS)
CFLAGS += -g -MMD
CFLAGS += -I$(MPL_DIR)

CXXFLAGS=$(CFLAGS)

MPLCOMP_CCFILES=\
	mplcomp.cc \
	mplcomp_compiler.cc \
	mplcomp_category.cc \
	mplcomp_parameter_set.cc \
	mplcomp_parameter.cc \
	mplcomp_classes.cc

MPLCOMP_CFILES=\
	mpl_list.c

MPLCOMP_COBJS = $(MPLCOMP_CFILES:.c=.o)
MPLCOMP_OBJS = $(MPLCOMP_CCFILES:.cc=.o) $(MPLCOMP_COBJS)
DEPS = $(MPLCOMP_CCFILES:.cc=.d) $(MPLCOMP_CFILES:.c=.d)

all: mplcomp

-include $(DEPS)

mplcomp.o: mplcomp.cc mplcomp.hh
mplcomp_compiler.o: mplcomp_compiler.cc mplcomp.hh mplcomp_parameter.hh
mplcomp_category.o: mplcomp_category.cc mplcomp.hh mplcomp_parameter.hh
mplcomp_parameter_set.o: mplcomp_parameter_set.cc mplcomp.hh mplcomp_parameter.hh
mplcomp_parameter.o: mplcomp_parameter.cc mplcomp.hh mplcomp_parameter.hh
mplcomp_classes.o: mplcomp_classes.cc mplcomp.hh

mplcomp: mplcomp.tab.o lex.yy.o $(MPLCOMP_OBJS)
	$(CXX) -o mplcomp lex.yy.o mplcomp.tab.o $(MPLCOMP_OBJS)

$(MPLCOMP_COBJS):%.o: $(MPL_DIR)/%.c
		$(CC) -c -o $@ $(CFLAGS) $<

lex.yy.cc: mplcomp.l
	flex mplcomp.l

mplcomp.tab.cc: mplcomp.yy
	bison mplcomp.yy

clean:
	rm -f mplcomp
	rm -f *.o *.d
	rm -f lex.yy.*
	rm -f mplcomp.tab.*
	rm -f *~ core
