#
#   Copyright 2013 ST-Ericsson SA
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Author: Per Sigmond <per@sigmond.no>
#   Author: Harald Johansen <harald.johansen@stericsson.com>
#   Author: Emil B. Viken <emil.b.viken@stericsson.com>
#
#

# Comments start with a hash character

# Literal code section (header code)
%h_begin
#include "mpl_config.h"
extern mpl_config_t test_config;
extern mpl_param_descr_set_t *test_param_descr_set_external_p;
extern const char test_event_failure_buf[];
%h_end

# Literal code section (C code)
%c_begin
const char test_event_failure_buf[] = "message=event_failure";

static mpl_param_descr_set_t test_param_descr_set;
mpl_param_descr_set_t *test_param_descr_set_external_p = &test_param_descr_set;
mpl_config_t test_config;
%c_end

# Options:
# Short name for parameter set "test" intended for naming of functions
# and macros in generated code:
%option parameter_set test short_name tst;

# Definition of a parameter set named "test".
# A parameter set can be regarded as a namespace for parameters.
parameter_set test {
    prefix test;

    # Parameters in the parameter set
    parameters {

        # An enum
        enum msgtype {
            req,
            resp,
            event
        };

        # Another enum
        enum result {
            ok,
            failed_for_a_reason
        };
    };

};

# Definition of an RPC protocol.
# RPC definitions are encapsulated in "categories".
# The default parameter set for category "testcat" is "test"
category testcat using test {

    # Define which parameters make up a command (request) message
    command_bag cmd {
        ...          # This means that more parameters are allowed in a command
    };

    # Define which parameters make up a response message
    response_bag resp {
        result,
        ...
    };

    # Define which parameters make up an event message
    event_bag evt {
        ...
    };

    # Define commands
    commands {
        # myint is a parameter to go in the command (request) message
        # myint2 is a parameter to go in the response message
        mycommand(myint, out myint2, out mynewbag b);

        # mystring is a parameter to go in the command (request) message
        # myint is a parameter to go in the response message
        # my_enum is an optional parameter to go in the response message
        #
        mycommand2(mystring s, out myint i, out my_enum *e);
    };

    # Define events
    events {
        # myint is a parameter to go in the event message
        myevent(out myint i1, out myint i2);

        # multiple instances of the myint parameter may go in the event
        # message; each instance will be tagged from 1 to N
        myevent2(out myint i[]);
    };

};

# More parameters in the parameter set "test"
parameters test {
    # A parameter with the default value defined
    enum my_enum default val1 {
        val1,
        val2
    };

    enum my_enum_2 {
        minval = -2147483648,
        maxval = 2147483647
    };

    # An enum inheriting another, only default changed
    enum my_enum2 default val2 : my_enum;

    # An enum inheriting another, new default and new enum values
    # this time with specific numeric values
    enum my_enum3 default val4 : my_enum2 {
        val3 = 33,
        val4
    };

    # An enum inheriting another, even more enum values
    enum my_enum4 set : my_enum3 {
        val5 = 0xff,
        val6
    };

    # An enumerator list for use with later enum definitions
    enumerator_list elist1 {
        val7,
        val8 = 0xa,
        val9
    };

    # Another enumerator list
    enumerator_list elist2 {
        val10 = -22,
        val11 = -33,
        val12
    };

    # An enumerator list combining two others
    enumerator_list elist3 = elist1 + elist2;

    # An enum made up by inherit plus two enumerator lists
    enum my_enum5 get : my_enum elist1 + elist2;

    # An enum inheriting this one again
    enum my_enum6 config : my_enum5 {
        val13,
        val14,
        val_last = -1
    };

    # An enum put together by adding an enumerator list
    # to an enum block
    enum my_enum7 {
        val1,
        val2
    } + elist1;

    # A variation of the same
    enum my_enum9 elist1 + {
        val3,
        val4
    };

    # Another variation
    enum my_enum10 : my_enum elist1 + {
        val3,
        val4
    } + elist2;

    # Yet another
    enum my_enum11 elist1 + elist2 + {
        val3,
        val4
    };

    # And another
    enum my_enum12 {
        val1,
        val2
    } + elist1 + elist2;

    # Different enum types
    enum8 my_enum8 default smallval1 {
        smallval1,
        smallval2
    };

    enum8 my_enum8_2 {
        minval = 0,
        maxval = 255
    };

    enum16 my_enum16 default val1 {
        val1,
        val2
    };

    enum16 my_enum16_2 {
        minval = 0,
        maxval = 65535
    };

    enum32 my_enum32 default val1 {
        val1,
        val2
    };

    enum32 my_enum32_2 {
        minval = 0,
        maxval = 4294967295
    };

    signed_enum8 my_senum8 default val1 {
        val1,
        val2
    };

    signed_enum8 my_senum8_2 {
        minval = -128,
        maxval = 127
    };

    signed_enum16 my_senum16 default val1 {
        val1,
        val2
    };

    signed_enum16 my_senum16_2 {
        minval = -32768,
        maxval = 32767
    };

    # A parameter with more options:
    # - default is "val1"
    # - "set" of the parameter is allowed
    # - "config" is allowed (can be used in configuration system)
    signed_enum32 my_senum32 default val1, set, config {
        val1,
        val2
    };

    signed_enum32 my_senum32_2 {
        minval = -2147483648,
        maxval = 2147483647
    };

    string mystring min 5, max 20, default "mydefault", set, get, config;

    # A parameter with more options:
    # - max for a string means max string length
    # - "get" of the parameter is allowed
    wstring mywstring max 20, default L"mydefault", set, get, config;
    int myint max 1000, default 99, config, min -500;

    # A parameter with negative min/max options
    int myint1 min -2999, max -1999;

    # A parameter with no options 
    int myint2;

    # A basic parameter inheriting another, adding options
    int myint3 max 50 : myint2;

    # A basic parameter inheriting another, adding options
    int myint4 default 25 : myint3;

    bool mybool config;
    bool8 mybool8 config;
    uint8 myuint8 min 5, max 63, config;
    uint8 myuint8_2;
    uint16 myuint16 max 30000;
    uint16 myuint16_2;
    uint32 myuint32 max 5000, config;
    uint32 myuint32_2;
    uint64 myuint64 max 5000, config, min 5;
    uint64 myuint64_2;
    sint8 mysint8 max 63, config;
    sint8 mysint8_2;
    sint16 mysint16 max 30000;
    sint16 mysint16_2;
    sint32 mysint32 max 5000, config;
    sint32 mysint32_2;
    sint64 mysint64 max 5000, config, min 5;
    sint64 mysint64_2;
    
    # An array of uint8
    # "max 20" means maximum 20 elements
    uint8_array myuint8_arr max 20, set, get, config, min 2;
    uint16_array myuint16_arr max 20, set, get, config;
    uint32_array myuint32_arr max 20, set, get, config;

    # Tuple of two strings (key and value)
    # "max 20" means max length of each string
    string_tuple mystring_tup max 20, min 2, set, get, config;
    string_tuple mystring_tup2 max 20, default("heisann","hoppsann"), set, get, config;

    # Tuple of two integers (key and value)
    # "max 20" means max value of each integer
    int_tuple myint_tup max 1000, default (11,22), min -1000, set, get, config;

    # Tuple of a string (key) and an integer (value)
    # "max 1000" means max value of the integer
    strint_tuple mystrint_tup max 1000, min -500, default ("tjohei",33), set, get, config;

    # Tuple of a string (key) and an uint8 (value)
    # "max 200" means max value of the uint8
    struint8_tuple mystruint8_tup min 5, max 200, default("tjohei", 33), set, get, config;

    # A "bag" is a compound type parameter that can contain other parameters
    # "..." means that the enclosed parameters are not known
    bag mylist1 set, get, config { ... };

    # "max 3" means that the bag can at most contain 3 parameters
    # (including any multiple instances, see below)
    bag mylist2 min 2, max 3, set, get, config {...};
    
    # A bag with specific members:
    # - myint
    # - mystring (multiple, tagged from 1 to N)
    # - mybool (optional)
    bag mylist3 set, get, config, max 5 {
        myint,
        mystring[],
        *mybool
    };
    # A bag inheriting another, only options changed
    bag mylist4 get false, max 10 : mylist3;
    # A bag inheriting another, options changed and two new fields
    bag mylist5 get, set false, max 20 : mylist4 {
        myuint8,
        myuint16
    };
    # A bag inheriting another, even one more field
    bag mylist6: mylist5{
        myuint32
    };

    bag mynewbag {
        myint i1,
        myint i2,
        mystring s[],
        mybool *b
    };

    # An int with a number range
    int my_ranged_int1 range (10..20);

    # An int with a number range
    int my_ranged_int_child1 : my_ranged_int1;

    # An int with a number range
    int my_ranged_int_child2 range(-5..5) : my_ranged_int1;

    # An int with a number range
    int my_ranged_int2 range (10..20,30..40,50..60);

    # A named number range
    number_range range1 70..80;

    # A named number range
    number_range range2 100..110, -100..-90;

    # An int with a number range
    int my_ranged_int3 range(10..20,range1,-50..5,range2), set;

    # An int with a number range
    int my_ranged_int4 get, range(range1,-50..5,range2);

    # An uint8 with a number range
    uint8 my_ranged_uint8 config, range(5..20, range1,30..40);    

    addr myaddr set, get, config;
    struint8_tuple mylast max 200, default ("tjohei",33), set, get, config;
};

# Include another mpl-file
include mpl_tull_msg.mpl
