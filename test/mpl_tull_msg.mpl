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
#   Author: Harald Johansen <hajohans1@gmail.com>
#   Author: Emil B. Viken <emil.b.viken@gmail.com>
#
#

%option parameter_set tull short_name tll;

# Definition of another parameter set
parameter_set tull {
    prefix tull;

    bag_field_table_suffix FTAB;

    # Parameters in the "tull" parameter set
    parameters {
        # The same name as a parameter in the "test" parameter set,
        # but no name clash because this is another parameter set (namespace).
        string mystring max 20, default "mydefault", set, get, config;
    };

};

# Another RPC category inheriting the "testcat" category, but using
# another parameter set "tull" (overriding)
category dill : testcat using tull {

    command_bag cmd {
        test::myuint8
    };
    
    response_bag resp {
        ...
    };

    commands {
        # Parameters may be from other parameter sets
        mycommand1(myint[], mylist3, test::myint ti, myparent_list[]);
    };
};

# More parameters in the "tull" parameter set
parameters tull {
    int myint max 1000, default 99;

    # An enum inheriting an enum in another parameter set
    # If the other parameter set is defined in the same file (or
    # included file), it must be known at this point
    #
    # If the other parameter set is "external" (defined in another file),
    # the header file of that parameter set must be included in a literal
    # code section.
    # Note that the mpl compiler does limited checking when
    # inheriting from an external parameter set. Also, adding
    # more values is not allowed.
    enum my_enum default val2 : test::my_enum;

    enum my_enum2 default val3 : test::my_enum {
        val3,
        val4
    };

    enumerator_list elist1 {
        val88 = 88,
        val89
    };

    enum tull1 virtual elist1;

    enum my_enum3 default val3 : test::my_enum test::elist1 + {
        val3,
        val4
    };

    enum my_enum4 test::elist1 + {
        val3,
        val4
    } + elist1 + test::elist2;

    enumerator_list elist2 = test::elist1 + elist1 + test::elist2;

    # A bag inheriting from another parameter set, two changed options
    bag mylist get false, max 5 : test::mylist6;

    # A bag inheriting from another parameter set, two changed options, more fields
    bag mylist2 max 10, config false : test::mylist6 {
        myint,
        mylist
    };

    # A bag having member parameters from another parameter set
    bag mylist3 {
        myint,
        test::myint,
        mylist2
    };

    bag myparent_list virtual {
        myint i
    };

    bag mychild_list1 : myparent_list {
        my_enum,
        my_enum2
    };
        
    bag mychild_list2 : myparent_list {
        my_enum3,
        my_enum4 e4
    };

    bag mybag1 {
        mybag2,
        mybag3,
        *mybag4
    };

    bag mybag2 {
        myint
    };
        
    bag mybag3 {
        myint
    };

    bag mybag4 {
        mybag3,
        myint
    };

    bag mynewbag : test::mynewbag {
        myint i3,
        my_enum,
        test::myint i4,
        test::my_enum e1,
        myparent_list l1,
        test::mystring_tup st,
        test::myint_tup it,
        test::mystrint_tup sit,
        test::mystruint8_tup s8t,
        test::myuint8_arr a8,
        test::myuint16_arr a16,
        test::myuint32_arr a32,
        myint *i5[...]
    };
    int my_virtual_int_par virtual;
};
