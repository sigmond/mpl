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
CFLAGS += -I$(MPL_DIR) -I$(APR_INCLUDE_DIR)

MPLCOMP_C_GENERATED=\
	mpl_test_msg.c

MPLCOMP_H_GENERATED=$(MPLCOMP_C_GENERATED:.c=.h)

INCLUDED_MPL_FILES= \
	mpl_tull_msg.mpl

MPLCOMP_GENERATED=\
	$(MPLCOMP_C_GENERATED) \
	$(MPLCOMP_H_GENERATED)

LOCAL_OBJS =  mpl_test.o \
              mpl_test_msg.o \
              mpl_test_old_msg.o

all: $(MPLCOMP) $(MPLCOMP_GENERATED) mpl_test

-include $(MPL_OBJS:%.o=%.d)
$(MPL_OBJS):%.o: $(MPL_DIR)/%.c
		$(CC) -c -o $@ -DMPL_MODULE_TEST $(CFLAGS) $<

-include $(LOCAL_OBJS:%.o=%.d)
%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

mpl_test: $(MPL_OBJS:$(MPL_DIR)%.c=%.o) $(LOCAL_OBJS:%.o=%.o)
	$(CC) -rdynamic -o mpl_test $(MPL_OBJS) $(LOCAL_OBJS:%.o=%.o) -lpthread -lapr-1

.PHONY: force
$(MPLCOMP): force
	(cd $(MPL_DIR)/../compiler; make mplcomp)

$(MPLCOMP_H_GENERATED):%.h: %.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -e $<

$(MPLCOMP_C_GENERATED):%.c: %.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -e $<

clean:
	rm -f mpl_test
	rm -f *.o *.d
	rm -f $(MPLCOMP_GENERATED)
