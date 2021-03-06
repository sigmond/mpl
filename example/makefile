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
CXXFLAGS=$(CFLAGS)

MPLCOMP_C_GENERATED=\
	personnel.c

MPLCOMP_H_GENERATED=$(MPLCOMP_C_GENERATED:.c=.h)

PERS_CLI_C_GENERATED=\
	personnel_cli.c

PERS_CLI_H_GENERATED=$(PERS_CLI_C_GENERATED:.c=.h)

PERS_API_CC_GENERATED=\
	personnel.cc

PERS_API_HH_GENERATED=$(PERS_API_CC_GENERATED:.cc=.hh)

PERS_API_O_GENERATED=$(PERS_API_CC_GENERATED:.cc=_api.o)

PERS_API_SRCS=pers_cc_api.cc
PERS_API_OBJS=$(PERS_API_SRCS:.cc=.o) personnel.o

INCLUDED_MPL_FILES=

MPLCOMP_GENERATED=\
	$(MPLCOMP_C_GENERATED) \
	$(MPLCOMP_H_GENERATED)

LOCAL_OBJS =  pers_server.o \
	personnel.o \
	pers_handlers.o \
	pers_db.o

PERS_CLI_OBJS = personnel.o personnel_cli.o pers_cli.o linenoise.o

all: $(MPLCOMP) $(MPLCOMP_GENERATED) pers_server pers_cli pers_cc_api

check: all
	$(MPLCOMP) -m dejagnu personnel.mpl > personnel.exp
	expect -f pers_test.exp

memcheck: all
	rm -rf memcheck
	mkdir memcheck
	make VALGRIND="/usr/bin/valgrind --leak-check=full --show-reachable=yes --log-file=memcheck/valgrind-%p.log" check

.PHONY: memcheck
memcheck:
	rm -rf memcheck
	mkdir memcheck
	make check VALGRIND="/usr/bin/valgrind --leak-check=full --show-reachable=yes --log-file=memcheck/valgrind-%p.log"

memcheck/%.logd: memcheck/%.log
	@echo "* Analyzing $^"
	@grep "ERROR SUMMARY: 0 errors" $^ > /dev/null
	@if grep "All heap blocks were freed -- no leaks are possible" $^ > /dev/null; then echo -n ; else \
	grep "LEAK SUMMARY:" $^ > /dev/null && \
	grep "definitely lost: 0 bytes" $^ > /dev/null && \
	grep "indirectly lost: 0 bytes" $^ > /dev/null && \
	grep "possibly lost: 0 bytes" $^ > /dev/null && \
	grep "still reachable: 0 bytes" $^ > /dev/null; \
	fi

VALGRIND_FILES = $(shell echo memcheck/*.log)

.PHONY: analyze_memcheck
analyze_memcheck: $(VALGRIND_FILES:memcheck/%.log=memcheck/%.logd)
	@echo "***"
	@echo "*"
	@echo "* No Valgrind errors found!"
	@echo "*"
	@echo "***"

-include $(MPL_OBJS:%.o=%.d)
$(MPL_OBJS):%.o: $(MPL_DIR)/%.c
		$(CC) -c -o $@ $(CFLAGS) $<

-include $(LOCAL_OBJS:%.o=%.d)
%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

pers_server: $(MPL_OBJS:$(MPL_DIR)%.c=%.o) $(LOCAL_OBJS:%.o=%.o)
	$(CC) -rdynamic -o pers_server $(MPL_OBJS) $(LOCAL_OBJS:%.o=%.o) -lpthread -lapr-1

pers_cli: $(MPL_OBJS:$(MPL_DIR)%.c=%.o) $(PERS_CLI_OBJS)
	$(CC) -rdynamic -o pers_cli $(MPL_OBJS) $(PERS_CLI_OBJS) -lpthread -lapr-1

pers_cc_api: $(MPL_OBJS:$(MPL_DIR)%.c=%.o) $(PERS_API_O_GENERATED) $(PERS_API_OBJS)
	$(CXX) -rdynamic -o pers_cc_api $(MPL_OBJS) $(PERS_API_OBJS) $(PERS_API_O_GENERATED) -lpthread -lapr-1

doc: $(MPLCOMP) $(MPLCOMP_C_GENERATED:.c=.mpl)
	rm -rf personnel.tex
	rm -rf personnel
	$(MPLCOMP) -m latex $(MPLCOMP_C_GENERATED:.c=.mpl) > personnel.tex
	latex personnel.tex > /dev/null
	latex2html -show_section_numbers personnel.tex > /dev/null

.PHONY: force
$(MPLCOMP): force
	(cd $(MPL_DIR)/../compiler; make mplcomp)

$(MPLCOMP_H_GENERATED):%.h: %.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) $<

$(MPLCOMP_C_GENERATED):%.c: %.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) $<

$(PERS_CLI_H_GENERATED):%.h: personnel.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -m cli $<

$(PERS_CLI_C_GENERATED):%.c: personnel.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -m cli $<

$(PERS_API_O_GENERATED): $(PERS_API_CC_GENERATED)
	$(CXX) -c $(CXXFLAGS) -o $(PERS_API_O_GENERATED) $(PERS_API_CC_GENERATED) 

$(PERS_API_HH_GENERATED):%.hh: personnel.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -m api $<

$(PERS_API_CC_GENERATED):%.cc: personnel.mpl $(INCLUDED_MPL_FILES) $(MPLCOMP)
	$(MPLCOMP) -m api $<


clean:
	rm -f pers_server pers_cli pers_cc_api
	rm -f *.o *.d
	rm -f $(MPLCOMP_GENERATED) $(PERS_CLI_C_GENERATED) $(PERS_CLI_H_GENERATED) $(PERS_API_CC_GENERATED) $(PERS_API_HH_GENERATED)
	rm -f *~ core
	rm -f personnel.exp
	rm -rf memcheck
	rm -rf personnel
	rm -rf *.aux *.dvi *.log *.tex *.toc

distclean: clean
	rm -f history.txt
	rm -f personnel.db
