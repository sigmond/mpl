MPL_DIR=../lib
MPLCOMP=$(MPL_DIR)/../compiler/mplcomp

CC=gcc

SRCS := mpl_config.c \
	mpl_dbgtrace.c \
	mpl_file.c \
	mpl_list.c \
	mpl_param.c \
	mpl_pthread.c

MPL_OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

ifndef APR_INCLUDE_DIR
APR_INCLUDE_DIR=/usr/include/apr-1.0
endif

CFLAGS :=  $(GCOV_CFLAGS)
CFLAGS += -g -MMD -DMPL_USE_PTHREAD_MUTEX
CFLAGS += -I$(MPL_DIR) -I$(APR_INCLUDE_DIR) -I../example
CXXFLAGS=$(CFLAGS)

MPLCOMP_C_GENERATED=\
	mpl_test_msg.c testprotocol.c

MPLCOMP_H_GENERATED=$(MPLCOMP_C_GENERATED:.c=.h)

TESTPROT_CLI_C_GENERATED=\
	testprotocol_cli.c

TESTPROT_CLI_H_GENERATED=$(TESTPROT_CLI_C_GENERATED:.c=.h)

TESTPROT_API_CC_GENERATED=\
	testprotocol.cc

TESTPROT_API_HH_GENERATED=$(TESTPROT_API_CC_GENERATED:.cc=.hh)

TESTPROT_API_O_GENERATED=$(TESTPROT_API_CC_GENERATED:.cc=_api.o)

INCLUDED_MPL_FILES= \
	mpl_tull_msg.mpl

MPLCOMP_GENERATED=\
	$(MPLCOMP_C_GENERATED) \
	$(MPLCOMP_H_GENERATED)

LOCAL_OBJS= mpl_test.o \
            mpl_test_msg.o \
            mpl_test_old_msg.o

TESTPROT_SERVER_OBJS=testprotocol.o testprot_handlers.o testprot_server.o

TESTPROT_SERVER_CC_OBJS=testprotocol.o $(TESTPROT_API_O_GENERATED) testprot_handlers_cc.o testprot_server_cc.o

TESTPROT_CLI_OBJS=testprotocol.o testprotocol_cli.o testprot_cli.o linenoise.o

all: $(MPLCOMP) $(MPLCOMP_GENERATED) mpl_test testprot_cli testprot_server testprot_server_cc #testprot_api

check: all
	$(MPLCOMP) -m dejagnu testprotocol.mpl > testprotocol.exp

-include $(MPL_OBJS:%.o=%.d)
$(MPL_OBJS):%.o: $(MPL_DIR)/%.c
		$(CC) -c -o $@ -DMPL_MODULE_TEST $(CFLAGS) $<

-include $(LOCAL_OBJS:%.o=%.d)
%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

mpl_test: $(MPL_OBJS:$(MPL_DIR)%.c=%.o) $(LOCAL_OBJS)
	$(CC) -rdynamic -o mpl_test $(MPL_OBJS) $(LOCAL_OBJS) -lpthread -lapr-1

testprot_server: $(MPL_OBJS:$(MPL_DIR)%.c=%.o) $(LOCAL_OBJS) $(TESTPROT_SERVER_OBJS)
	$(CC) -rdynamic -o testprot_server $(MPL_OBJS) $(TESTPROT_SERVER_OBJS) -lpthread -lapr-1

testprot_cli: $(MPL_OBJS:$(MPL_DIR)%.c=%.o) $(TESTPROT_CLI_OBJS)
	$(CC) -rdynamic -o testprot_cli $(MPL_OBJS) $(TESTPROT_CLI_OBJS) -lpthread -lapr-1

testprot_server_cc: $(MPL_OBJS:$(MPL_DIR)%.c=%.o) $(LOCAL_OBJS) $(TESTPROT_SERVER_CC_OBJS) $(TESTPROT_API_O_GENERATED) $(TESTPROT_API_OBJS)
	$(CXX) -rdynamic -o testprot_server_cc $(MPL_OBJS) $(TESTPROT_SERVER_CC_OBJS) $(TESTROT_API_OBJS) $(TESTROT_API_O_GENERATED) -lpthread -lapr-1

testprot_api: $(MPL_OBJS:$(MPL_DIR)%.c=%.o) $(TESTPROT_API_O_GENERATED) $(TESTPROT_API_OBJS)
	$(CXX) -rdynamic -o testprot_api $(MPL_OBJS) $(TESTPROT_API_OBJS) $(TESTROT_API_O_GENERATED) -lpthread -lapr-1

.PHONY: force
$(MPLCOMP): force
	(cd $(MPL_DIR)/../compiler; make mplcomp)

$(MPLCOMP_H_GENERATED):%.h: %.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -e $<

$(MPLCOMP_C_GENERATED):%.c: %.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -e $<

linenoise.o: ../example/linenoise.c
		$(CC) -c -o $@ -DMPL_MODULE_TEST $(CFLAGS) $<

$(TESTPROT_CLI_H_GENERATED):%.h: testprotocol.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -m cli $<

$(TESTPROT_CLI_C_GENERATED):%.c: testprotocol.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -m cli $<

$(TESTPROT_API_O_GENERATED): $(TESTPROT_API_CC_GENERATED)
	$(CXX) -c $(CXXFLAGS) -o $(TESTPROT_API_O_GENERATED) $(TESTPROT_API_CC_GENERATED) 

$(TESTPROT_API_HH_GENERATED):%.hh: testprotocol.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -m api $<

$(TESTPROT_API_CC_GENERATED):%.cc: testprotocol.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -m api $<

clean:
	rm -f mpl_test testprot_server testprot_server_cc testprot_cli testprot_api
	rm -f *.o *.d
	rm -f $(MPLCOMP_GENERATED) $(TESTPROT_CLI_H_GENERATED) $(TESTPROT_CLI_C_GENERATED)
	rm -f $(TESTPROT_API_HH_GENERATED) $(TESTPROT_API_CC_GENERATED)
	rm -f *~ core
