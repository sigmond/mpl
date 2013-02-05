/*
 *   Copyright 2013 ST-Ericsson SA
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *   Author: Per Sigmond <per.xx.sigmond@stericsson.com>
 *   Author: Harald Johansen <harald.johansen@stericsson.com>
 *   Author: Emil B. Viken <emil.b.viken@stericsson.com>
 *
 */

/*************************************************************************
 *
 * File name: mpl_param.h
 *
 * Description: MPL parameter system API declarations
 *
 **************************************************************************/
#ifndef _MPL_PARAM_H
#define _MPL_PARAM_H

/**************************************************************************
 * Includes
 *************************************************************************/
#include "mpl_stdint.h"
#include "mpl_stdbool.h"
#include "mpl_list.h"

#include "mpl_dbgtrace.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @file mpl_param.h
 * @brief Marshalling Parameter Library (MPL) API
 */

/**
 * @defgroup MPL Marshalling Parameter Library (MPL) API
 *
 * MPL provides the means to store and transfer data in a generic and
 * platform independent manner.
 *
 * MPL concepts:
 *
 * <ul>
 *   <li> A \b parameter consists of an identifier and a value
 *   <li> A \b parameter \b identifier defines the name, datatype and \b parameter \b set
 *        of the parameter
 *   <li> A \b parameter \b set is a collection of parameter identifiers (a kind of namespace)
 *   <li> A \b parameter \b element is an instance of a parameter (identifier + value)
 *   <li> Parameter elements can be chained to form a \b parameter \b list - an unordered
 *        sequence of parameters
 *   <li> Parameters can be represented using a \b local \b format or a \b transfer \b format
 *   <ul>
 *        <li> The \b local \b format is binary (the "unpacked" format). The exact
 *             representation is cpu/compiler dependant and is thus not suitable
 *             for exchanging data with other systems.
 *        <li> The \b transfer \b format is (currently) text (the "packed" format). This format is
 *             common to all systems using MPL, and is thus suitable for exchanging
 *             data. Because it is text-based it is also convenient for the human
 *             observer (e.g. for logging and debugging purposes). Other non-text
 *             transfer formats may be defined in the future.
 *        <li> Functions are provided for conversion between local format and
 *             transfer format. These operations are often called \b marshalling or
 *             \b serialization.
 *   </ul>
 * </ul>
 *
 * The @ref MPL_LANGUAGE :
 * <ul>
 * <li> This is a formal language (suitable for parsing by a compiler)
 *     <ul>
 *     <li> This language is used to define parameter sets and parameters.
 *     <li> It can also be used to define RPC (Remote Procedure Call) protocols
 *          based on MPL parameters.
 *     <li> A utility (@ref MPL_COMPILER) exists that can generate code and documentation.
 *     </ul>
 * </ul>
 *
 *
 * The @ref MPL_PARAM provides the means to manipulate parameters and parameter
 * lists.
 *
 * MPL used internally in a system:
 *
 * MPL has several properties that makes it convenient for representing and
 * storing data internally in a system (program or groups of programs/processes):
 * <ul>
 *   <li> Data can be represented, stored and exchanged using one formal data type: the \b mpl_list_t*
 *        (see @ref MPL_LIST).
 *        This opens for extendable APIs.
 *   <li> Presence (or absence) of a particular parameter can easily be detected (opens for optional parameters)
 *   <li> File storage support
 *   <li> Configuration support with default values
 *   <li> Value range control
 *   <li> ...
 *</ul>
 * MPL used in a communication system:
 * <ul>
 * <li> MPL can be used to exchange data between heterogeneous systems (systems with
 *      different local data representations)
 * <li> The conversion between local format (unpacked) and transfer format (packed)
 *      corresponds to the OSI model layer 6 functionality (presentation layer).
 * <ul>
 *   <li> The conversion operation is sometimes referred to as "marshalling" or "serializing"
 *   <li> The MPL definition language is used to define the meaning of the data types that are to
 *        be exchanged - this is sometimes referred to as defining the "abstract data types"
 * </ul>
 * <li> The RPC (Remote Procedure Call) part of the MPL language can be used to implement
 *      protocols corresponding to some of the OSI model layer 5 functionality (session layer).
 * </ul>
 *
 * Limitations:
 * <ul>
 *    <li> Note that MPL is optimized for relatively small data volumes, typically for use in
 *         control systems, control protocols and test systems. Using MPL for storing
 *         or exchanging \b large amounts of data is not recommended.
 * </ul>
 *
 * MPL data types:
 * <ul>
 * <li> Basic data types
 *   <ul>
 *     <li> \b int - signed integer (cpu-depandent size)
 *     <li> \b sint8 - signed integer (1 byte)
 *     <li> \b sint16 - signed integer (2 bytes)
 *     <li> \b sint32 - signed integer (4 bytes)
 *     <li> \b sint64 - signed integer (8 bytes)
 *     <li> \b uint8 - unsigned integer (1 byte)
 *     <li> \b uint16 - unsigned integer (2 bytes)
 *     <li> \b uint32 - unsigned integer (4 bytes)
 *     <li> \b uint64 - unsigned integer (8 bytes)
 *     <li> \b enum - enumerated integer (cpu/compiler-depandent size and signedness)
 *     <li> \b enum8 - unsigned enumerated integer (1 byte)
 *     <li> \b enum16 - unsigned enumerated integer (2 bytes)
 *     <li> \b enum32 - unsigned enumerated integer (5 bytes)
 *     <li> \b signed_enum8 - signed enumerated integer (1 byte)
 *     <li> \b signed_enum16 - signed enumerated integer (2 bytes)
 *     <li> \b signed_enum32 - signed enumerated integer (4 bytes)
 *     <li> \b bool - boolean (cpu/compiler-native size)
 *     <li> \b bool8 - boolean (1 byte)
 *     <li> \b string - ascii character string (one byte per character)
 *     <li> \b wstring - wide character string (cpu/compiler dependant size per character)
 *     <li> \b addr - address (cpu-dependant size)
 *   </ul>
 * <li> Structured data types
 *   <ul>
 *     <li> \b uint8_array - array of unsigned integers (1 byte per element)
 *     <li> \b uint16_array - array of unsigned integers (2 bytes per element)
 *     <li> \b uint32_array - array of unsigned integers (4 bytes per element)
 *     <li> \b string_tuple - tuple (key/value) of two ascii character strings
 *     <li> \b int_tuple - tuple (key/value) of two signed integers (cpu-dependant size)
 *     <li> \b strint_tuple - tuple of one ascii string (key) and one integer (value)
 *     <li> \b struint8_tuple - tuple of one ascii string (key) and one unsigned 1-byte integer (value)
 *    </ul>
 * <li> Compound data types
 *   <ul>
 *     <li> \b bag - data type encapsulating other parameters in an unordered manner
 *   </ul>
 * </ul>
 *
 * MPL data representation basics:
 * <ul>
 * <li>Parameter element:
 *    <ul>
 *    <li> Unpacked format:
 * <pre>
 *                           mpl_param_element
 *     param_elem_p ------->+---------------------+
 *                          | id (0x00ab0001)     |
 *                          | tag (0)             |
 *                          | value_p (123)       |
 *                          | ...                 |
 *                          | list_entry          |
 *                          +---------------------+
 * </pre>
 *
 *    <li> Packed format:
 *
 * <pre>
 *      'my_pset.param1=123'
 * </pre>
 *    </ul>
 *
 * <li> Parameter list:
 *    <ul>
 *    <li> Unpacked format:
 * <pre>
 *                       mpl_param_element          mpl_param_element
 *                      +----------------------+   +----------------------+
 *                      |  id (0x00ab0001)     |   |  id (0x00ab0002)     |
 *                      |  tag (0)             |   |  tag (1)             |
 *     param_list_p -+  |  value_p (123)       |   |  value_p ("string1") |
 *                   |  |  ...                 |   |  ...                 |
 *                   +--|->list_entry.next_p-------|->list_entry.next_p------+
 *                      +----------------------+   +----------------------+  |
 *                                                                           |
 *                                          +--------------------------------+
 *                                          |
 *                                          |     mpl_param_element
 *                                          |    +----------------------+
 *                                          |    |  id (0x00ab0002)     |
 *                                          |    |  tag (2)             |
 *                                          |    |  value_p ("string2") |
 *                                          |    |  ...                 |
 *                                          +----|->list_entry.next_p-------+
 *                                               +----------------------+   |
 *                                                                         ---
 * </pre>
 *
 *    <li> Packed format:
 *
 * <pre>
 *      'my_pset.param1=123,my_pset.param2[1]="string1",my_pset.param2[2]="string2"'
 * </pre>
 *    </ul>
 * <li> Bag data type:
 *    <ul>
 *    <li> Unpacked format:
 * <pre>
 *                           mpl_param_element
 *     param_elem_p ------->+---------------------+
 *                          | id (0x00ab0003)     |
 *                          | tag (0)             |
 *                          | value_p----------------+
 *                          | ...                 |  |
 *                          | list_entry          |  |
 *                          +---------------------+  |
 *                                                   |
 *                   +-------------------------------+
 *                   |
 *                   |
 *                   |
 *                   |   mpl_param_element          mpl_param_element
 *                   |  +----------------------+   +----------------------+
 *                   |  |  id (0x00ab0001)     |   |  id (0x00ab0002)     |
 *                   |  |  tag (0)             |   |  tag (1)             |
 *                   |  |  value_p (123)       |   |  value_p ("string1") |
 *                   |  |  ...                 |   |  ...                 |
 *                   +--|->list_entry.next_p-------|->list_entry.next_p------+
 *                      +----------------------+   +----------------------+  |
 *                                                                           |
 *                                          +--------------------------------+
 *                                          |
 *                                          |     mpl_param_element
 *                                          |    +----------------------+
 *                                          |    |  id (0x00ab0002)     |
 *                                          |    |  tag (2)             |
 *                                          |    |  value_p ("string2") |
 *                                          |    |  ...                 |
 *                                          +----|->list_entry.next_p-------+
 *                                               +----------------------+   |
 *                                                                         ---
 * </pre>
 *
 *    <li> Packed format:
 *
 * <pre>
 *      'my_pset.param3={my_pset.param1=123,my_pset.param2[1]="string1",my_pset.param2[2]="string2"}'
 * </pre>
 *    </ul>
 * </ul>
 * MPL packed format details:
 * <ul>
 * <li> Basic data types
 *   <ul>
 *     <li> \b int
 *       <ul>
 *         <li> value: -123
 *         <li> packed: 'my_pset.my_int_param=-123'
 *       </ul>
 *     <li> \b sint8
 *       <ul>
 *         <li> value: -123
 *         <li> packed: 'my_pset.my_sint8_param=-123'
 *       </ul>
 *     <li> \b sint16
 *       <ul>
 *         <li> value: -123
 *         <li> packed: 'my_pset.my_sint16_param=-123'
 *       </ul>
 *     <li> \b sint32
 *       <ul>
 *         <li> value: -123
 *         <li> packed: 'my_pset.my_sint32_param=-123'
 *       </ul>
 *     <li> \b sint64
 *       <ul>
 *         <li> value: -123
 *         <li> packed: 'my_pset.my_sint64_param=-123'
 *       </ul>
 *     <li> \b uint8
 *       <ul>
 *         <li> value: 123
 *         <li> packed: 'my_pset.my_uint8_param=0x7b'
 *       </ul>
 *     <li> \b uint16
 *       <ul>
 *         <li> value: 123
 *         <li> packed: 'my_pset.my_uint16_param=0x7b'
 *       </ul>
 *     <li> \b uint32
 *       <ul>
 *         <li> value: 123
 *         <li> packed: 'my_pset.my_uint32_param=0x7b'
 *       </ul>
 *     <li> \b uint64
 *       <ul>
 *         <li> value: 123
 *         <li> packed: 'my_pset.my_uint64_param=0x7b'
 *       </ul>
 *     <li> \b enum
 *       <ul>
 *         <li> value: 123
 *         <li> packed: 'my_pset.my_enum_param=MY_SYMBOLIC_123_NAME'
 *       </ul>
 *     <li> \b enum8
 *       <ul>
 *         <li> value: 123
 *         <li> packed: 'my_pset.my_enum8_param=MY_SYMBOLIC_123_NAME'
 *       </ul>
 *     <li> \b enum16
 *       <ul>
 *         <li> value: 123
 *         <li> packed: 'my_pset.my_enum16_param=MY_SYMBOLIC_123_NAME'
 *       </ul>
 *     <li> \b enum32
 *       <ul>
 *         <li> value: 123
 *         <li> packed: 'my_pset.my_enum32_param=MY_SYMBOLIC_123_NAME'
 *       </ul>
 *     <li> \b signed_enum8
 *       <ul>
 *         <li> value: -123
 *         <li> packed: 'my_pset.my_signed_enum8_param=MY_SYMBOLIC_MINUS_123_NAME'
 *       </ul>
 *     <li> \b signed_enum16
 *       <ul>
 *         <li> value: -123
 *         <li> packed: 'my_pset.my_signed_enum16_param=MY_SYMBOLIC_MINUS_123_NAME'
 *       </ul>
 *     <li> \b signed_enum32
 *       <ul>
 *         <li> value: -123
 *         <li> packed: 'my_pset.my_signed_enum32_param=MY_SYMBOLIC_MINUS_123_NAME'
 *       </ul>
 *     <li> \b bool
 *       <ul>
 *         <li> value: 1
 *         <li> packed: 'my_pset.my_bool_param=true'
 *       </ul>
 *     <li> \b bool8
 *       <ul>
 *         <li> value: 1
 *         <li> packed: 'my_pset.my_bool8_param=true'
 *       </ul>
 *     <li> \b string
 *       <ul>
 *         <li> value: hello
 *         <li> packed: 'my_pset.my_string_param=hello'
 *       </ul>
 *       <ul>
 *         <li> value: "hello world"
 *         <li> packed: 'my_pset.my_string_param="hello world"'
 *       </ul>
 *       <ul>
 *         <li> value: hello\\ world
 *         <li> packed: 'my_pset.my_string_param=hello world'
 *       </ul>
 *       <ul>
 *         <li> value: "hello, world"
 *         <li> packed: 'my_pset.my_string_param="hello\, world"'
 *       </ul>
 *     <li> \b wstring
 *       <ul>
 *         <li> value: L"hi"
 *         <li> packed: 'my_pset.my_wstring_param=00000003000000480000004900000000'
 *       </ul>
 *     <li> \b addr
 *       <ul>
 *         <li> value: 0xabbababe
 *         <li> packed: 'my_pset.my_addr_param=0xabbababe'
 *       </ul>
 *   </ul>
 * <li> Structured data types
 *   <ul>
 *     <li> \b uint8_array
 *       <ul>
 *         <li> length: 4
 *         <li> value: {0x01,0x02,0x03,0x04}
 *         <li> packed: 'my_pset.my_uint8_array_param=0000000401020304'
 *       </ul>
 *     <li> \b uint16_array
 *       <ul>
 *         <li> length: 4
 *         <li> value: {0x0101,0x0202,0x0303,0x0404}
 *         <li> packed: 'my_pset.my_uint16_array_param=000000040101020203030404'
 *       </ul>
 *     <li> \b uint32_array
 *       <ul>
 *         <li> length: 4
 *         <li> value: {0x01010101,0x02020202,0x03030303,0x04040404}
 *         <li> packed: 'my_pset.my_uint32_array_param=0000000401010101020202020303030304040404'
 *       </ul>
 *     <li> \b string_tuple
 *       <ul>
 *         <li> value: {"thekey","thevalue"]
 *         <li> packed: 'my_pset.my_string_tuple_param=thekey:thevalue'
 *       </ul>
 *     <li> \b int_tuple
 *       <ul>
 *         <li> value: {123,456]
 *         <li> packed: 'my_pset.my_int_tuple_param=123:456'
 *       </ul>
 *     <li> \b strint_tuple
 *       <ul>
 *         <li> value: {"thekey",456]
 *         <li> packed: 'my_pset.my_strint_tuple_param=thekey:456'
 *       </ul>
 *     <li> \b struint8_tuple
 *       <ul>
 *         <li> value: {"192.168.0.1",24]
 *         <li> packed: 'my_pset.my_struint8_tuple_param=192.168.0.1/24'
 *       </ul>
 *   </ul>
 * <li> Tagged parameters
 *   <ul>
 *     <li> \b tag \b 56 (legal values 0-99)
 *       <ul>
 *         <li> value: 123
 *         <li> packed: 'my_pset.my_int_param[56]=123'
 *       </ul>
 *     <li> \b tag \b 0 (the default)
 *       <ul>
 *         <li> value: 123
 *         <li> packed: 'my_pset.my_int_param=123'
 *       </ul>
 *   </ul>
 * <li> Compound data types
 *   <ul>
 *     <li> \b bag - note that bags do not preserve the sequence of the members
 *       <ul>
 *         <li> value: 123 + "s1" + false
 *         <li> packed: 'my_pset.my_bag_param={my_pset.my_bool_param=false,my_pset.my_int_param=123,my_pset.my_string_param=s1}'
 *       </ul>
 *     <li> \b bag \b with \b fields
 *       <ul>
 *         <li> fields: my_pset.my_int_param \b i, my_pset.my_string_param \b s, my_pset.my_bool_param \b b
 *         <li> value: 123 + "s1" + false
 *         <li> packed: 'my_pset.my_bag_param={b=false,i=123,s=s1}'
 *       </ul>
 *   </ul>
 * <li> Commands, responses and events
 *
 *      The messages generated by commands, responses and events are implemented as \b bags.
 *      The encoding of these bags follows the same rules as with any other bag parameter.
 *      Commands and events will typically be specified using field names, resulting in
 *      a "bag with fields" type of encoding as shown above.
 *
 *      The members of a command bag are the in- and inout-parameters specified in the command along
 *      with the members of the "command_bag" (see @ref MPL_LANGUAGE). The "command_bag" of a
 *      category is a parent of all the command bags.
 *      The members of a response bag are the out- and inout-parameters specified in the command along
 *      with the members of the "response_bag". The "response_bag" of a
 *      category is a parent of all the response bags.
 *      The members of an event bag are the parameters specified in the event along
 *      with the members of the "event_bag". The "event_bag" of a
 *      category is a parent of all the event bags.
 *
 *      For a command, the name of the bag parameter will be
 *      the command name with a suffix appended. The suffix is based on the name of the
 *      "command_bag" (the parent).
 *      For example, if the "command_bag" is named "req", then command "dosomething"
 *      would generate a bag named "dosomething_req" in the category's parameter set.
 *      The same goes for responses and events. If the "response_bag" is named
 *      "resp", then the response bag of "dosomething" would be "dosomething_resp".
 *      If the "event_bag" is named "evt", then an event "somethinghappened" would
 *      generate a bag called "somethinghappened_evt".
 *
 * </ul>
 */

/**
 * @defgroup MPL_PARAM MPL Parameter API
 * @ingroup MPL
 * These are the macros and functions needed to manipulate parameters and
 * parameter lists.
 * @note Definition of the parameters and parameter sets is not done via
 * this API, but via the @ref MPL_LANGUAGE and the @ref MPL_COMPILER.
 */

/**
 * @defgroup MPL_LANGUAGE MPL Parameter Definition Language
 * @ingroup MPL
 * This language can be used to define parameter sets and parameters.
 * It can also be used to define RPC (Remote Procedure Call) protocols
 * based on MPL parameters. A utility (@ref MPL_COMPILER) exists that
 * can generate code and documentation from MPL files.
 *
 * Example MPL file:
 <pre>
# Comments start with a hash character

# Literal code section (header code)
\%h_begin
\#include "mpl_config.h"
extern mpl_config_t test_config;
\%h_end

# Literal code section (C code)
\%c_begin
mpl_config_t test_config;
\%c_end

# Options:
# Short name for parameter set "test" intended for naming of functions
# and macros in generated code:
\%option parameter_set test short_name tst;

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
        val14
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

    enum16 my_enum16 default val1 {
        val1,
        val2
    };

    enum32 my_enum32 default val1 {
        val1,
        val2
    };

    signed_enum8 my_senum8 default val1 {
        val1,
        val2
    };

    signed_enum16 my_senum16 default val1 {
        val1,
        val2
    };

    # A parameter with more options:
    # - default is "val1"
    # - "set" of the parameter is allowed
    # - "config" is allowed (can be used in configuration system)
    signed_enum32 my_senum32 default val1, set, config {
        val1,
        val2
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
    uint16 myuint16 max 30000;
    uint32 myuint32 max 5000, config;
    uint64 myuint64 max 5000, config, min 5;
    sint8 mysint8 max 63, config;
    sint16 mysint16 max 30000;
    sint32 mysint32 max 5000, config;

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
*        *mybool
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
\%option parameter_set tull short_name tll;

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
*        *mybag4
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
        test::myuint32_arr a32
    };
};
 </pre>

*/

/**
 * @defgroup MPL_COMPILER MPL Compiler
 * @ingroup MPL
 * The mplcomp compiler can be used to generate different kinds of
 * code based on files that follow the rules of the @ref MPL_LANGUAGE.
 *
 * <pre>
 * Usage: mplcomp [options] \<yourfile>.mpl
 * Options: -m \<mode> (mode=codegen|cli|dejagnu|latex) (default codegen)
 *          -i \<include-dir>
 *          -d \<output-dir>
 *          -o \<output-filename-without-suffix> (default \<yourfile>)
 *
 *   Modes: codegen - generate support code for parameter sets and categories (C)
 *          cli     - generate support code for command line interface (C)
 *          dejagnu - generate support code for dejagnu test system (TCL)
 *          latex   - generate documentation (latex, suitable for latex2html)
 * </pre>
 *
 * In the default mode, the compiler will generate one headerfile
 * (\<yourfile>.h) and one C-file (\<yourfile>.c) that (among other stuff)
 * contains the parameter set definitions and initialization function.
 * Together with the @ref MPL_PARAM this provides all code that is needed
 * to start using the parameter set.
 */

#ifndef DOXYGEN

/*****************************************************************************
 *
 * Defines & Type definitions
 *
 *****************************************************************************/
#define MPL_TYPE_IDS                        \
    /*                type */               \
    MPL_TYPE_ID_ELEMENT(int)                \
    MPL_TYPE_ID_ELEMENT(sint8)              \
    MPL_TYPE_ID_ELEMENT(sint16)             \
    MPL_TYPE_ID_ELEMENT(sint32)             \
    MPL_TYPE_ID_ELEMENT(sint64)             \
    MPL_TYPE_ID_ELEMENT(uint8)              \
    MPL_TYPE_ID_ELEMENT(uint16)             \
    MPL_TYPE_ID_ELEMENT(uint32)             \
    MPL_TYPE_ID_ELEMENT(uint64)             \
    MPL_TYPE_ID_ELEMENT(enum)               \
    MPL_TYPE_ID_ELEMENT(enum8)              \
    MPL_TYPE_ID_ELEMENT(enum16)             \
    MPL_TYPE_ID_ELEMENT(enum32)             \
    MPL_TYPE_ID_ELEMENT(signed_enum8)       \
    MPL_TYPE_ID_ELEMENT(signed_enum16)      \
    MPL_TYPE_ID_ELEMENT(signed_enum32)      \
    MPL_TYPE_ID_ELEMENT(bool)               \
    MPL_TYPE_ID_ELEMENT(bool8)              \
    MPL_TYPE_ID_ELEMENT(string)             \
    MPL_TYPE_ID_ELEMENT(wstring)            \
    MPL_TYPE_ID_ELEMENT(uint8_array)        \
    MPL_TYPE_ID_ELEMENT(uint16_array)       \
    MPL_TYPE_ID_ELEMENT(uint32_array)       \
    MPL_TYPE_ID_ELEMENT(string_tuple)       \
    MPL_TYPE_ID_ELEMENT(int_tuple)          \
    MPL_TYPE_ID_ELEMENT(strint_tuple)       \
    MPL_TYPE_ID_ELEMENT(struint8_tuple)     \
    MPL_TYPE_ID_ELEMENT(bag)                \
    MPL_TYPE_ID_ELEMENT(addr)               \


#define MPL_TYPE_ID_ELEMENT(TYPE)               \
    mpl_type_##TYPE,

typedef enum
{
    MPL_TYPE_IDS
    mpl_end_of_types,
    mpl_type_invalid = (-1)
} mpl_type_t;
#undef MPL_TYPE_ID_ELEMENT


#define max_limit_string  UINT16_MAX
#define max_limit_wstring  UINT16_MAX
#define max_limit_enum    0
#define max_limit_enum8   0
#define max_limit_enum16   0
#define max_limit_enum32   0
#define max_limit_signed_enum8   0
#define max_limit_signed_enum16   0
#define max_limit_signed_enum32   0
#define max_limit_int     INT_MAX
#define max_limit_sint8     SCHAR_MAX
#define max_limit_sint16     SHRT_MAX
#define max_limit_sint32     LONG_MAX
#define max_limit_sint64     INT64_MAX
#define max_limit_uint8   UINT8_MAX
#define max_limit_uint16  UINT16_MAX
#define max_limit_uint32  UINT32_MAX
#define max_limit_uint64  UINT64_MAX
#define max_limit_bool    1
#define max_limit_bool8   1
#define max_limit_uint8_array    UINT32_MAX
#define max_limit_uint16_array   UINT32_MAX
#define max_limit_uint32_array   UINT32_MAX
#define max_limit_string_tuple   UINT8_MAX
#define max_limit_int_tuple   INT_MAX
#define max_limit_strint_tuple   INT_MAX
#define max_limit_struint8_tuple   UINT8_MAX
#define max_limit_bag   UINT8_MAX
#define max_limit_addr 0

#define min_limit_string  0
#define min_limit_wstring  0
#define min_limit_enum    0
#define min_limit_enum8   0
#define min_limit_enum16   0
#define min_limit_enum32   0
#define min_limit_signed_enum8   0
#define min_limit_signed_enum16   0
#define min_limit_signed_enum32   0
#define min_limit_int     INT_MIN
#define min_limit_sint8     SCHAR_MIN
#define min_limit_sint16     SHRT_MIN
#define min_limit_sint32     LONG_MIN
#define min_limit_sint64     INT64_MIN
#define min_limit_uint8   0
#define min_limit_uint16  0
#define min_limit_uint32  0
#define min_limit_uint64  0
#define min_limit_bool    0
#define min_limit_bool8   0
#define min_limit_uint8_array    0
#define min_limit_uint16_array   0
#define min_limit_uint32_array   0
#define min_limit_string_tuple   0
#define min_limit_int_tuple   INT_MIN
#define min_limit_strint_tuple   INT_MIN
#define min_limit_struint8_tuple   0
#define min_limit_bag   0
#define min_limit_addr 0

#define MPL_MAX_ARGS 100
#define MPL_PARAMID_PREFIX_MAXLEN 20

#define MPL_PARAMID_TYPE_BITS 14       /*  0 reserved (means "invalid" or "base"), range 1-16384 */
#define MPL_PARAMID_PARAMSET_BITS 15   /* 0 reserved (means "invalid"), range 1-32768 */
#define MPL_PARAMID_VIRTUAL_BITS 1     /* 0=not virtual, 1=virtual */
#define MPL_PARAMID_BITS (MPL_PARAMID_TYPE_BITS +       \
                          MPL_PARAMID_PARAMSET_BITS +   \
                          MPL_PARAMID_VIRTUAL_BITS)

#define MPL_PARAMID_MAX_NUM_TYPES ((1 << MPL_PARAMID_TYPE_BITS) - 2)
#define MPL_PARAMID_MAX_NUM_PARAMSETS ((1 << MPL_PARAMID_PARAMSET_BITS) - 2)

#define MPL_PARAMID_VIRTUAL_SHIFT (MPL_PARAMID_BITS - MPL_PARAMID_VIRTUAL_BITS)
#define MPL_PARAMID_PARAMSET_SHIFT (MPL_PARAMID_VIRTUAL_SHIFT - MPL_PARAMID_PARAMSET_BITS)
#define MPL_PARAMID_TYPE_SHIFT (MPL_PARAMID_PARAMSET_SHIFT - MPL_PARAMID_TYPE_BITS)

#define MPL_PARAMID_VIRTUAL_MASK (((1 << MPL_PARAMID_VIRTUAL_BITS) - 1) << MPL_PARAMID_VIRTUAL_SHIFT)
#define MPL_PARAMID_PARAMSET_MASK (((1 << MPL_PARAMID_PARAMSET_BITS) - 1) << MPL_PARAMID_PARAMSET_SHIFT)
#define MPL_PARAMID_TYPE_MASK (((1 << MPL_PARAMID_TYPE_BITS) - 1) << MPL_PARAMID_TYPE_SHIFT)

/*
  Parameter id bit positions and meanings:

  |virtual|paramset|type|

  type:       Identifies the parameter in context of <paramset>.
  Value 0 is not used.
  paramset:   The parameter set id of the parameter (a kind of
  namespace).
  Value 0 is not used.
  virtual:    Value 1 means it is not possible to create a parameter
  element (unless forced clear).

*/

/* A valid parameter id will be <> 0 */
#define MPL_PARAM_ID_UNDEFINED 0

#define MPL_PARAMID_POSITION_TYPE(type) ((type) << MPL_PARAMID_TYPE_SHIFT)
#define MPL_PARAMID_POSITION_PARAMSET(pset) ((pset) << MPL_PARAMID_PARAMSET_SHIFT)
#define MPL_PARAMID_POSITION_VIRTUAL(virt) ((virt) << MPL_PARAMID_VIRTUAL_SHIFT)

#define MPL_PARAMID_TO_PARAMSET(paramid) (((paramid) & MPL_PARAMID_PARAMSET_MASK) >> MPL_PARAMID_PARAMSET_SHIFT)
#define MPL_PARAMID_TO_TYPE(paramid) (((paramid) & MPL_PARAMID_TYPE_MASK) >> MPL_PARAMID_TYPE_SHIFT)
#define MPL_PARAMID_TO_VIRTUAL(paramid) (((paramid) & MPL_PARAMID_VIRTUAL_MASK) >> MPL_PARAMID_VIRTUAL_SHIFT)

#define MPL_PARAMID_TYPE_CLEAR(paramid) ((paramid) & ~MPL_PARAMID_TYPE_MASK)
#define MPL_PARAMID_PARAMSET_CLEAR(paramid) ((paramid) & ~MPL_PARAMID_PARAMSET_MASK)
#define MPL_PARAMID_VIRTUAL_CLEAR(paramid) ((paramid) & ~MPL_PARAMID_VIRTUAL_MASK)

#define MPL_PARAM_SET_ID_TO_PARAMID_BASE(param_set_id) MPL_PARAMID_POSITION_PARAMSET(param_set_id)
#define MPL_PARAMID_IS_VIRTUAL(paramid) ((paramid) & MPL_PARAMID_VIRTUAL_MASK)
#define MPL_BUILD_PARAMID(virt,param_set_id,type)               \
    (MPL_PARAMID_POSITION_VIRTUAL(virt) |                       \
     MPL_PARAMID_POSITION_PARAMSET(param_set_id) |              \
     MPL_PARAMID_POSITION_TYPE(type))

#define MESSAGE_DELIMITER ','


/* Macros used in parameter set definitions */
#define mpl_paramset_left_cast_paren_int
#define mpl_paramset_right_cast_paren_int
#define mpl_paramset_left_cast_paren_sint8
#define mpl_paramset_right_cast_paren_sint8
#define mpl_paramset_left_cast_paren_sint16
#define mpl_paramset_right_cast_paren_sint16
#define mpl_paramset_left_cast_paren_sint32
#define mpl_paramset_right_cast_paren_sint32
#define mpl_paramset_left_cast_paren_sint64
#define mpl_paramset_right_cast_paren_sint64
#define mpl_paramset_left_cast_paren_uint8
#define mpl_paramset_right_cast_paren_uint8
#define mpl_paramset_left_cast_paren_uint16
#define mpl_paramset_right_cast_paren_uint16
#define mpl_paramset_left_cast_paren_uint32
#define mpl_paramset_right_cast_paren_uint32
#define mpl_paramset_left_cast_paren_uint64
#define mpl_paramset_right_cast_paren_uint64
#define mpl_paramset_left_cast_paren_string
#define mpl_paramset_right_cast_paren_string
#define mpl_paramset_left_cast_paren_wstring
#define mpl_paramset_right_cast_paren_wstring
#define mpl_paramset_left_cast_paren_bool
#define mpl_paramset_right_cast_paren_bool
#define mpl_paramset_left_cast_paren_bool8
#define mpl_paramset_right_cast_paren_bool8
#define mpl_paramset_left_cast_paren_uint8_array
#define mpl_paramset_right_cast_paren_uint8_array
#define mpl_paramset_left_cast_paren_uint16_array
#define mpl_paramset_right_cast_paren_uint16_array
#define mpl_paramset_left_cast_paren_uint32_array
#define mpl_paramset_right_cast_paren_uint32_array
#define mpl_paramset_left_cast_paren_string_tuple
#define mpl_paramset_right_cast_paren_string_tuple
#define mpl_paramset_left_cast_paren_strint_tuple
#define mpl_paramset_right_cast_paren_strint_tuple
#define mpl_paramset_left_cast_paren_struint8_tuple
#define mpl_paramset_right_cast_paren_struint8_tuple
#define mpl_paramset_left_cast_paren_int_tuple
#define mpl_paramset_right_cast_paren_int_tuple
#define mpl_paramset_left_cast_paren_bag
#define mpl_paramset_right_cast_paren_bag
#define mpl_paramset_left_cast_paren_addr
#define mpl_paramset_right_cast_paren_addr
#define mpl_paramset_left_cast_paren_enum (
#define mpl_paramset_right_cast_paren_enum )
#define mpl_paramset_left_cast_paren_enum8 (
#define mpl_paramset_right_cast_paren_enum8 )
#define mpl_paramset_left_cast_paren_enum16 (
#define mpl_paramset_right_cast_paren_enum16 )
#define mpl_paramset_left_cast_paren_enum32 (
#define mpl_paramset_right_cast_paren_enum32 )
#define mpl_paramset_left_cast_paren_signed_enum8 (
#define mpl_paramset_right_cast_paren_signed_enum8 )
#define mpl_paramset_left_cast_paren_signed_enum16 (
#define mpl_paramset_right_cast_paren_signed_enum16 )
#define mpl_paramset_left_cast_paren_signed_enum32 (
#define mpl_paramset_right_cast_paren_signed_enum32 )

#define mpl_paramset_left_sizeof_paren_int (int
#define mpl_paramset_right_sizeof_paren_int )
#define mpl_paramset_left_sizeof_paren_sint8 (int
#define mpl_paramset_right_sizeof_paren_sint8 )
#define mpl_paramset_left_sizeof_paren_sint16 (int
#define mpl_paramset_right_sizeof_paren_sint16 )
#define mpl_paramset_left_sizeof_paren_sint32 (int
#define mpl_paramset_right_sizeof_paren_sint32 )
#define mpl_paramset_left_sizeof_paren_sint64 (int
#define mpl_paramset_right_sizeof_paren_sint64 )
#define mpl_paramset_left_sizeof_paren_uint8 (int
#define mpl_paramset_right_sizeof_paren_uint8 )
#define mpl_paramset_left_sizeof_paren_uint16 (int
#define mpl_paramset_right_sizeof_paren_uint16 )
#define mpl_paramset_left_sizeof_paren_uint32 (int
#define mpl_paramset_right_sizeof_paren_uint32 )
#define mpl_paramset_left_sizeof_paren_uint64 (int
#define mpl_paramset_right_sizeof_paren_uint64 )
#define mpl_paramset_left_sizeof_paren_string (int
#define mpl_paramset_right_sizeof_paren_string )
#define mpl_paramset_left_sizeof_paren_wstring (int
#define mpl_paramset_right_sizeof_paren_wstring )
#define mpl_paramset_left_sizeof_paren_bool (int
#define mpl_paramset_right_sizeof_paren_bool )
#define mpl_paramset_left_sizeof_paren_bool8 (int
#define mpl_paramset_right_sizeof_paren_bool8 )
#define mpl_paramset_left_sizeof_paren_uint8_array (int
#define mpl_paramset_right_sizeof_paren_uint8_array )
#define mpl_paramset_left_sizeof_paren_uint16_array (int
#define mpl_paramset_right_sizeof_paren_uint16_array )
#define mpl_paramset_left_sizeof_paren_uint32_array (int
#define mpl_paramset_right_sizeof_paren_uint32_array )
#define mpl_paramset_left_sizeof_paren_string_tuple (int
#define mpl_paramset_right_sizeof_paren_string_tuple )
#define mpl_paramset_left_sizeof_paren_strint_tuple (int
#define mpl_paramset_right_sizeof_paren_strint_tuple )
#define mpl_paramset_left_sizeof_paren_struint8_tuple (int
#define mpl_paramset_right_sizeof_paren_struint8_tuple )
#define mpl_paramset_left_sizeof_paren_int_tuple (int
#define mpl_paramset_right_sizeof_paren_int_tuple )
#define mpl_paramset_left_sizeof_paren_bag (int
#define mpl_paramset_right_sizeof_paren_bag )
#define mpl_paramset_left_sizeof_paren_addr (int
#define mpl_paramset_right_sizeof_paren_addr )
#define mpl_paramset_left_sizeof_paren_enum (
#define mpl_paramset_right_sizeof_paren_enum )
#define mpl_paramset_left_sizeof_paren_enum8 (
#define mpl_paramset_right_sizeof_paren_enum8 )
#define mpl_paramset_left_sizeof_paren_enum16 (
#define mpl_paramset_right_sizeof_paren_enum16 )
#define mpl_paramset_left_sizeof_paren_enum32 (
#define mpl_paramset_right_sizeof_paren_enum32 )
#define mpl_paramset_left_sizeof_paren_signed_enum8 (
#define mpl_paramset_right_sizeof_paren_signed_enum8 )
#define mpl_paramset_left_sizeof_paren_signed_enum16 (
#define mpl_paramset_right_sizeof_paren_signed_enum16 )
#define mpl_paramset_left_sizeof_paren_signed_enum32 (
#define mpl_paramset_right_sizeof_paren_signed_enum32 )

#define mpl_paramset_pre_string char
#define mpl_paramset_pre_wstring wchar_t
#define mpl_paramset_pre_int    int
#define mpl_paramset_pre_sint8    sint8_t
#define mpl_paramset_pre_sint16    sint16_t
#define mpl_paramset_pre_sint32    sint32_t
#define mpl_paramset_pre_sint64    int64_t
#define mpl_paramset_pre_uint8  uint8_t
#define mpl_paramset_pre_uint16  uint16_t
#define mpl_paramset_pre_uint32 uint32_t
#define mpl_paramset_pre_uint64 uint64_t
#define mpl_paramset_pre_enum
#define mpl_paramset_pre_enum8
#define mpl_paramset_pre_enum16
#define mpl_paramset_pre_enum32
#define mpl_paramset_pre_signed_enum8
#define mpl_paramset_pre_signed_enum16
#define mpl_paramset_pre_signed_enum32
#define mpl_paramset_pre_bool   bool
#define mpl_paramset_pre_bool8  uint8_t
#define mpl_paramset_pre_uint8_array  mpl_uint8_array_t
#define mpl_paramset_pre_uint16_array  mpl_uint16_array_t
#define mpl_paramset_pre_uint32_array  mpl_uint32_array_t
#define mpl_paramset_pre_string_tuple  mpl_string_tuple_t
#define mpl_paramset_pre_strint_tuple  mpl_strint_tuple_t
#define mpl_paramset_pre_struint8_tuple  mpl_struint8_tuple_t
#define mpl_paramset_pre_int_tuple  mpl_int_tuple_t
#define mpl_paramset_pre_bag mpl_list_t*
#define mpl_paramset_pre_addr void*
#define mpl_paramset_post_string []
#define mpl_paramset_post_wstring []
#define mpl_paramset_post_int
#define mpl_paramset_post_sint8
#define mpl_paramset_post_sint16
#define mpl_paramset_post_sint32
#define mpl_paramset_post_sint64
#define mpl_paramset_post_uint8
#define mpl_paramset_post_uint16
#define mpl_paramset_post_uint32
#define mpl_paramset_post_uint64
#define mpl_paramset_post_enum
#define mpl_paramset_post_enum8
#define mpl_paramset_post_enum16
#define mpl_paramset_post_enum32
#define mpl_paramset_post_signed_enum8
#define mpl_paramset_post_signed_enum16
#define mpl_paramset_post_signed_enum32
#define mpl_paramset_post_bool
#define mpl_paramset_post_bool8
#define mpl_paramset_post_uint8_array
#define mpl_paramset_post_uint16_array
#define mpl_paramset_post_uint32_array
#define mpl_paramset_post_string_tuple
#define mpl_paramset_post_strint_tuple
#define mpl_paramset_post_struint8_tuple
#define mpl_paramset_post_int_tuple
#define mpl_paramset_post_bag
#define mpl_paramset_post_addr

#define mpl_paramset_set_string_default(var) var
#define mpl_paramset_set_wstring_default(var) var
#define mpl_paramset_set_enum_default(var) var
#define mpl_paramset_set_enum8_default(var) var
#define mpl_paramset_set_enum16_default(var) var
#define mpl_paramset_set_enum32_default(var) var
#define mpl_paramset_set_signed_enum8_default(var) var
#define mpl_paramset_set_signed_enum16_default(var) var
#define mpl_paramset_set_signed_enum32_default(var) var
#define mpl_paramset_set_int_default(var) var
#define mpl_paramset_set_sint8_default(var) var
#define mpl_paramset_set_sint16_default(var) var
#define mpl_paramset_set_sint32_default(var) var
#define mpl_paramset_set_sint64_default(var) var
#define mpl_paramset_set_uint8_default(var) var
#define mpl_paramset_set_uint16_default(var) var
#define mpl_paramset_set_uint32_default(var) var
#define mpl_paramset_set_uint64_default(var) var
#define mpl_paramset_set_bool_default(var) var
#define mpl_paramset_set_bool8_default(var) var
#define mpl_paramset_set_string_tuple_default(key,value) {key,value}
#define mpl_paramset_set_strint_tuple_default(key,value) {key,value}
#define mpl_paramset_set_struint8_tuple_default(key,value) {key,value}
#define mpl_paramset_set_bag_default(var) var
#define mpl_paramset_set_int_tuple_default(key,value) {key,value}
#define mpl_paramset_set_addr_default(var) var

#define mpl_paramset_set_string_no_default ""
#define mpl_paramset_set_wstring_no_default L""
#define mpl_paramset_set_enum_no_default 0
#define mpl_paramset_set_enum8_no_default 0
#define mpl_paramset_set_enum16_no_default 0
#define mpl_paramset_set_enum32_no_default 0
#define mpl_paramset_set_signed_enum8_no_default 0
#define mpl_paramset_set_signed_enum16_no_default 0
#define mpl_paramset_set_signed_enum32_no_default 0
#define mpl_paramset_set_int_no_default 0
#define mpl_paramset_set_sint8_no_default 0
#define mpl_paramset_set_sint16_no_default 0
#define mpl_paramset_set_sint32_no_default 0
#define mpl_paramset_set_sint64_no_default 0
#define mpl_paramset_set_uint8_no_default 0
#define mpl_paramset_set_uint16_no_default 0
#define mpl_paramset_set_uint32_no_default 0
#define mpl_paramset_set_uint64_no_default 0
#define mpl_paramset_set_bool_no_default false
#define mpl_paramset_set_bool8_no_default false
#define mpl_paramset_set_uint8_array_no_default {0,0}
#define mpl_paramset_set_uint16_array_no_default {0}
#define mpl_paramset_set_uint32_array_no_default {0}
#define mpl_paramset_set_string_tuple_no_default {0}
#define mpl_paramset_set_strint_tuple_no_default {0}
#define mpl_paramset_set_struint8_tuple_no_default {0,0}
#define mpl_paramset_set_int_tuple_no_default {0}
#define mpl_paramset_set_bag_no_default NULL
#define mpl_paramset_set_addr_no_default NULL

#define mpl_paramset_string_min_max_type int
#define mpl_paramset_wstring_min_max_type int
#define mpl_paramset_enum_min_max_type int
#define mpl_paramset_enum8_min_max_type int
#define mpl_paramset_enum16_min_max_type int
#define mpl_paramset_enum32_min_max_type int
#define mpl_paramset_signed_enum8_min_max_type int
#define mpl_paramset_signed_enum16_min_max_type int
#define mpl_paramset_signed_enum32_min_max_type int
#define mpl_paramset_int_min_max_type int
#define mpl_paramset_sint8_min_max_type sint8_t
#define mpl_paramset_sint16_min_max_type sint16_t
#define mpl_paramset_sint32_min_max_type sint32_t
#define mpl_paramset_sint64_min_max_type int64_t
#define mpl_paramset_uint8_min_max_type uint8_t
#define mpl_paramset_uint16_min_max_type uint16_t
#define mpl_paramset_uint32_min_max_type uint32_t
#define mpl_paramset_uint64_min_max_type uint64_t
#define mpl_paramset_bool_min_max_type int
#define mpl_paramset_bool8_min_max_type int
#define mpl_paramset_uint8_array_min_max_type int
#define mpl_paramset_uint16_array_min_max_type int
#define mpl_paramset_uint32_array_min_max_type int
#define mpl_paramset_string_tuple_min_max_type int
#define mpl_paramset_strint_tuple_min_max_type int
#define mpl_paramset_struint8_tuple_min_max_type uint8_t
#define mpl_paramset_int_tuple_min_max_type int
#define mpl_paramset_bag_min_max_type int
#define mpl_paramset_addr_min_max_type int

#define mpl_paramset_is_default_pre_string
#define mpl_paramset_is_default_pre_wstring &
#define mpl_paramset_is_default_pre_int &
#define mpl_paramset_is_default_pre_sint8 &
#define mpl_paramset_is_default_pre_sint16 &
#define mpl_paramset_is_default_pre_sint32 &
#define mpl_paramset_is_default_pre_sint64 &
#define mpl_paramset_is_default_pre_uint8 &
#define mpl_paramset_is_default_pre_uint16 &
#define mpl_paramset_is_default_pre_uint32 &
#define mpl_paramset_is_default_pre_uint64 &
#define mpl_paramset_is_default_pre_enum &
#define mpl_paramset_is_default_pre_enum8 &
#define mpl_paramset_is_default_pre_enum16 &
#define mpl_paramset_is_default_pre_enum32 &
#define mpl_paramset_is_default_pre_signed_enum8 &
#define mpl_paramset_is_default_pre_signed_enum16 &
#define mpl_paramset_is_default_pre_signed_enum32 &
#define mpl_paramset_is_default_pre_bool &
#define mpl_paramset_is_default_pre_bool8 &
#define mpl_paramset_is_default_pre_uint8_array &
#define mpl_paramset_is_default_pre_uint16_array &
#define mpl_paramset_is_default_pre_uint32_array &
#define mpl_paramset_is_default_pre_string_tuple &
#define mpl_paramset_is_default_pre_strint_tuple &
#define mpl_paramset_is_default_pre_struint8_tuple &
#define mpl_paramset_is_default_pre_int_tuple &
#define mpl_paramset_is_default_pre_bag &
#define mpl_paramset_is_default_pre_addr &

#define mpl_paramset_set_max(var) var
#define mpl_paramset_set_vmax(var) var
#define mpl_paramset_set_no_max 0
#define mpl_paramset_set_min(var) var
#define mpl_paramset_set_vmin(var) var
#define mpl_paramset_set_no_min 0

#define mpl_paramset_is_default(...) true
#define mpl_paramset_is_no_default false
#define mpl_paramset_is_max(var) true
#define mpl_paramset_is_vmax(var) true
#define mpl_paramset_is_no_max false
#define mpl_paramset_is_min(var) true
#define mpl_paramset_is_vmin(var) true
#define mpl_paramset_is_no_min false

#ifndef TYPEDEF_ENUM
#define TYPEDEF_ENUM enum
#endif
#ifndef ENUM8
#define ENUM8(t) ;typedef uint8_t t
#endif
#ifndef ENUM16
#define ENUM16(t) ;typedef uint16_t t
#endif
#ifndef ENUM32
#define ENUM32(t) ;typedef uint32_t t
#endif
#ifndef SIGNED_ENUM8
#define SIGNED_ENUM8(t) ;typedef sint8_t t
#endif
#ifndef SIGNED_ENUM16
#define SIGNED_ENUM16(t) ;typedef sint16_t t
#endif
#ifndef SIGNED_ENUM32
#define SIGNED_ENUM32(t) ;typedef sint32_t t
#endif


/**
 * This macro can be used to define the structure
 * 'paramsetname_param_descr_set'
 *
 * Note that both lowercase name and uppercase name for the set are required.
 * @note Deprecated, see MPL_DEFINE_PARAM_DESCR_SET2
 *
 */
#define MPL_DEFINE_PARAM_DESCR_SET(paramsetname, PARAMSETNAME)      \
    static mpl_param_descr_set_t paramsetname ##_param_descr_set =  \
    {                                                               \
        paramsetname ##_param_descr,                                \
        NULL,                                                       \
        false,                                                      \
        PARAMSETNAME ##_PARAMID_PREFIX,                             \
        PARAMSETNAME ##_PARAM_SET_ID,                               \
        paramsetname ##_end_of_paramids,                            \
        {                                                           \
            NULL                                                    \
        }                                                           \
    }

/**
 * This macro can be used to define the structure
 * 'paramsetname_param_descr_set'
 *
 * Note that both lowercase name and uppercase name for the set are required.
 *
 */
#define MPL_DEFINE_PARAM_DESCR_SET2(paramsetname, PARAMSETNAME)     \
    static mpl_param_descr_set_t paramsetname ##_param_descr_set =  \
    {                                                               \
        paramsetname ##_param_descr,                                \
        paramsetname ##_param_descr2,                               \
        false,                                                      \
        PARAMSETNAME ##_PARAMID_PREFIX,                             \
        PARAMSETNAME ##_PARAM_SET_ID,                               \
        paramsetname ##_enum_size_paramids,                         \
        {                                                           \
            NULL                                                    \
        }                                                           \
    }

typedef enum {
    field_pack_mode_context,
    field_pack_mode_autonomous
} mpl_field_pack_mode_t;

typedef struct
{
    bool no_prefix;
    char message_delimiter;
    int param_set_id;
    mpl_field_pack_mode_t field_pack_mode;
    bool force_field_pack_mode;
} mpl_pack_options_t;

#define MPL_PACK_OPTIONS_DEFAULT {false,MESSAGE_DELIMITER,-1,field_pack_mode_autonomous,false}

/**
 * mpl_param_element_id_t
 *
 * Parameter id
 *
 */
typedef uint32_t mpl_param_element_id_t;



/**
 * mpl_param_element_t
 *
 * Listable parameter element
 *
 *     id         parameter identifier
 *     tag        tag of this parameter, 1-99 (used to identify specific
 *                instances in a list of parameters of the same type (id))
 *                Note: Use of the tag is optional, default value is 0 (means
 *                no tag).
 *     context    parameter id of a compount type (0 means not member of
 *                a compound type)
 *     id_in_context identifier that has a meaning in the context
 *     value_p    pointer to parameter value
 *     list_entry list field
 *
 */
typedef struct
{
    mpl_param_element_id_t id;
    int                      tag;
    mpl_param_element_id_t context;
    int                      id_in_context;
    void*                    value_p;
    mpl_list_t              list_entry;
} mpl_param_element_t;


/**
 * mpl_uint8_array_t
 *
 * Parameter type for array of uint8
 *
 */
typedef struct
{
    int len;
    uint8_t *arr_p;
} mpl_uint8_array_t;

/**
 * mpl_uint16_array_t
 *
 * Parameter type for array of uint16
 *
 */
typedef struct
{
    int len;
    uint16_t *arr_p;
} mpl_uint16_array_t;


/**
 * mpl_uint32_array_t
 *
 * Parameter type for array of uint32
 *
 */
typedef struct
{
    uint32_t len;
    uint32_t *arr_p;
} mpl_uint32_array_t;


/**
 * mpl_string_tuple_t
 *
 * Parameter type for tuple of key/value strings
 *
 */
typedef struct
{
    char *key_p;
    char *value_p;
} mpl_string_tuple_t;


/**
 * mpl_int_tuple_t
 *
 * Parameter type for tuple of key/value integers
 *
 */
typedef struct
{
    int key;
    int value;
} mpl_int_tuple_t;


/**
 * mpl_strint_tuple_t
 *
 * Parameter type for tuple of key/value where key is a string and value is
 * an integer
 *
 */
typedef struct
{
    char *key_p;
    int value;
} mpl_strint_tuple_t;

/**
 * mpl_struint8_tuple_t
 *
 * Parameter type for tuple of key/value where key is a string and value is
 * an uint8
 *
 */
typedef struct
{
    char *key_p;
    uint8_t value;
} mpl_struint8_tuple_t;

typedef mpl_list_t mpl_bag_t;

/**
 * mpl_arg_t - array of key/value pairs
 *
 * @key_p     pointing to key string
 * @value_p   pointing to value string
 *
 **/
typedef mpl_string_tuple_t mpl_arg_t;

/* For enums: */
typedef struct
{
    const char *name_p;
    int64_t value; /* We should cover all enum values with this */
} mpl_enum_value_t;

/* For integers (and friends): */
typedef struct
{
    int64_t first;
    int64_t last;
    int64_t id;
} mpl_integer_range_t;

/* For bags: */
typedef struct
{
    const char *name_p;
    int field_id;
    mpl_param_element_id_t param_id;
    mpl_param_element_id_t context_id;
} mpl_field_value_t;

/**
 * mpl_param_descr2_t - additional description of a specific parameter
 *
 * min_p Pointer to min value (NULL if no min)
 * max_p Pointer to max value (NULL if no min)
 * enum_values Array of name<->value for enums
 * integer_range Array of integer ranges for integers (signed and unsigned)
 *
 **/
typedef struct
{
    int is_virtual;
    const void *min_p;
    const void *max_p;
    const mpl_enum_value_t *enum_values;
    int enum_values_size;
    size_t enum_representation_bytesize;
    bool enum_representation_signed;
    const mpl_integer_range_t *integer_ranges;
    int integer_ranges_size;
    const mpl_field_value_t *field_values;
    int field_values_size;
    const mpl_param_element_id_t *children;
    int children_size;
} mpl_param_descr2_t;


/**
 * mpl_pack_param_fp
 *
 * Pack function (method) for a specific parameter
 *
 * Parameters:
 *     param_value_p:     Pointer to the parameter value
 *     buf:               Buffer to pack into
 *     buflen:            Length of buffer
 *     descr_p:           parameter description
 *     options_p:         Pack options
 *
 * @return Number of bytes written to buf (or the number
 *                 that would have been written if buflen is too
 *                 small). Returns negative value on error.
 *
 */
typedef int (*mpl_pack_param_fp)(const void* param_value_p,
                                 char *buf, size_t buflen,
                                 const mpl_param_descr2_t *descr_p,
                                 const mpl_pack_options_t *options_p);


/**
 * mpl_unpack_param_fp
 *
 * Unpack function (method) for a specific parameter
 *
 * Parameters:
 *     value_str:         String containing the value (to unpack from)
 *     value_pp:          Pointing to the variable where the value is unpacked
 *                        into
 *     descr_p:           parameter description
 *     options_p:         (Un)pack options
 *
 * @return 0 on success, -1 on error.
 *
 */
typedef int (*mpl_unpack_param_fp)(const char* value_str,
                                   void **value_pp,
                                   const mpl_param_descr2_t *descr_p,
                                   const mpl_pack_options_t *options_p,
                                   mpl_param_element_id_t unpack_context);

/**
 * mpl_clone_param_fp
 *
 * Clone function (method) for a specific parameter
 *
 * Parameters:
 *     new_value_pp:      The parameter-copy is returned here
 *     old_value_p:       Pointer to the parameter to be cloned
 *
 * @return 0 on success, -1 on error.
 *
 */
typedef int (*mpl_clone_param_fp)(void **new_value_pp,
                                  const void* old_value_p,
                                  const mpl_param_descr2_t *descr_p);


/**
 * mpl_copy_param_fp
 *
 * Copy function (method) for a specific parameter
 *
 * Parameters:
 *     to_value_p:      The value is copied into this memory area
 *     from_value_p:    The value is copied from this memory area
 *     size:            Size of the destination memory in bytes.
 *                      If input too small, copying will not be complete.
 *
 * @return On success, size of the value in bytes on success, negative
 *                 on error.
 *                 If return value is -1 it means that the parameter does not
 *                 support copy.
 *                 Note that supplying a too small 'size' is not an error, it
 *                 just means that
 *                 not all bytes are copied (the function will in this case
 *                 return the number of bytes that would have been copied).
 *
 */
typedef int (*mpl_copy_param_fp)(void *to_value_p,
                                 const void* from_value_p,
                                 int size,
                                 const mpl_param_descr2_t *descr_p);


/**
 * mpl_compare_param_fp
 *
 * Compare function (method) for a specific parameter
 *
 * Parameters:
 *     value1_p:      Parameter 1
 *     value2_p:      Parameter 2
 *
 * @return 0 on success (equal), -1 on error.
 *
 */
typedef int (*mpl_compare_param_fp)(const void *value1_p,
                                    const void* value2_p,
                                    const mpl_param_descr2_t *descr_p);


/**
 * mpl_sizeof_param_fp
 *
 * Sizeof function (method) for a specific parameter
 *
 * Parameters:
 *        parameter descriptor 2
 *
 * @return size of parameter value (or 0 if not appropriate)
 *
 */
typedef size_t (*mpl_sizeof_param_fp)(const mpl_param_descr2_t *descr_p);

/**
 * mpl_free_param_fp
 *
 * Free function (method) for a specific parameter
 *
 * Parameters:
 *     value_p:      Parameter to be freed
 *
 * @return -
 *
 */
typedef void (*mpl_free_param_fp)(void *value_p);


/**
 * mpl_param_descr_t - description of a specific parameter
 *
 * @name              name of parameter
 * @type              type (string) of parameter
 * @allow_set         Is it allowed to set this parameter
 * @allow_get         Is it allowed to get this parameter
 * @allow_config      Is it allowed to get this parameter from config file
 * @default_value_p   Pointer to default value (NULL if no default)
 * @max_p             Pointer to max value or stringlen (NULL if no max)
 *                    (deprecated)
 * @pack_func         pack method
 * @unpack_func       unpack method
 * @clone_func        clone method
 * @copy_func         copy method
 * @compare_func      compare method
 * @sizeof_func       sizeof method
 * @free_func         free method
 * @stringarr         array of valid strings-names of parameter
 *                    (deprecated)
 * @stringarr_size    size of array
 *                    (deprecated)
 *
 **/
typedef struct
{
    char *name;
    mpl_type_t type;
    bool allow_set;
    bool allow_get;
    bool allow_config;
    const void *default_value_p;
    const uint64_t *max_p; /* Deprecated */
    mpl_pack_param_fp pack_func;
    mpl_unpack_param_fp unpack_func;
    mpl_clone_param_fp clone_func;
    mpl_copy_param_fp copy_func;
    mpl_compare_param_fp compare_func;
    mpl_sizeof_param_fp sizeof_func;
    mpl_free_param_fp free_func;
    const char **stringarr; /* Deprecated */
    int stringarr_size; /* Deprecated */
} mpl_param_descr_t;

/**
 * mpl_param_descr_set_t - a set of parameter decriptors
 *
 * @array              array of parameter decriptors
 * @size               size of array
 *
 **/
typedef struct
{
    const mpl_param_descr_t *array;
    const mpl_param_descr2_t *array2;
    bool is_dynamic_array2;
    char paramid_prefix[MPL_PARAMID_PREFIX_MAXLEN + 1];
    int param_set_id;
    int paramid_enum_size;
    mpl_list_t list_entry;
} mpl_param_descr_set_t;


/* List of parameter elements representing a blacklist. Only the parameter id
   matters in such a list, so the elements may be empty (no value).
*/
typedef mpl_list_t *mpl_blacklist_t;

/*
  deprecated, only kept for backwards compatibility
*/
typedef enum
{
    _MPL_ENUM_
} mpl_enum_t;

#endif /* !DOXYGEN */
/****************************************************************************
 *
 * Functions and macros
 *
 ****************************************************************************/
/**
 * @ingroup MPL_PARAM
 * mpl_param_init - Initiate library with new parameter set
 *
 * @param param_descr_set_p    pointer to parameter descriptor set struct
 * @return  0 on success, -1 on failure
 *
 **/
int mpl_param_init(mpl_param_descr_set_t *param_descr_set_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_deinit - Deinitiate library
 *
 *
 **/
void mpl_param_deinit(void);


/**
 * @ingroup MPL_PARAM
 * mpl_param_index_to_paramid
 *
 * @param index index of parameter id in parameter set
 * @param param_set_id parameter set id
 * @return parameter id
 * Returns MPL_PARAM_ID_UNDEFINED if index is not valid
 *
 **/
mpl_param_element_id_t mpl_param_index_to_paramid(int index, int param_set_id);

/**
 * @ingroup MPL_PARAM
 * mpl_param_num_parameters
 *
 * @param param_set_id parameter set id
 * @return number of parameters in the parameter set
 *
 **/
int mpl_param_num_parameters(int param_set_id);

/**
 * @ingroup MPL_PARAM
 * mpl_param_first_paramid
 *
 * @param param_set_id parameter set id
 * @return first parameter id of the given parameter set.
 * Returns MPL_PARAM_ID_UNDEFINED if parameter set is not valid
 *
 **/
mpl_param_element_id_t mpl_param_first_paramid(int param_set_id);

/**
 * @ingroup MPL_PARAM
 * mpl_param_last_paramid
 *
 * @param param_set_id parameter set id
 * @return last parameter id of the given parameter set.
 * Returns MPL_PARAM_ID_UNDEFINED if parameter set is not valid
 *
 **/
mpl_param_element_id_t mpl_param_last_paramid(int param_set_id);


/**
 * @ingroup MPL_PARAM
 * mpl_paramset_prefix
 *
 * @param param_set_id Parameter set id
 * @return String prefix of the parameter set
 *
 **/
const char *mpl_paramset_prefix(int param_set_id);

/**
 * @ingroup MPL_PARAM
 * mpl_prefix2paramsetid
 *
 * @param prefix_p Parameter set prefix string
 * @return parameter set id
 *
 * Convert parameter set prefix (string) to parameter set id
 *
 **/
int mpl_prefix2paramsetid(const char *prefix_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_pack - pack a parameter
 *
 * @param element_p The parameter element (id + tag + value) to pack
 * @param buf     buffer to store packed message
 * @param buflen  length of the buffer
 *
 *          The function does not write more than buflen bytes to the buffer
 *          (including '\0').
 *          If more would have been written, it is truncated.
 * @return   Bytes written to buf (excluding '\0') on success (-1) on error
 *           If the output was truncated due 'buflen' then the return value is
 *           the number of characters which would have been written to the
 *           final string if enough space had been available
 *
 **/
int mpl_param_pack(const mpl_param_element_t* element_p,
                   char *buf,
                   size_t buflen);


/**
 * @ingroup MPL_PARAM
 * mpl_param_pack_no_prefix - pack a parameter, omitting the prefix
 *
 * @param element_p The parameter element (id + tag + value) to pack
 * @param buf     buffer to store packed message
 * @param buflen  length of the buffer
 *          The function does not write more than buflen bytes to the buffer
 *          (including '\0').
 *          If more would have been written, it is truncated.
 * @return  Bytes written to buf (excluding '\0') on success (-1) on error
 *           If the output was truncated due 'buflen' then the return value is
 *           the number of characters which would have been written to the
 *           final string if enough space had been available
 *
 **/
int mpl_param_pack_no_prefix(const mpl_param_element_t* element_p,
                             char *buf,
                             size_t buflen);

int mpl_param_pack_internal(const mpl_param_element_t* element_p,
                            char *buf,
                            size_t buflen,
                            const mpl_pack_options_t *options_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_unpack - unpack received parameter strings
 *
 * @param id_str        Parameter set and name (null terminated string)
 * @param value_str      Parameter value (null terminated string)
 *                 NULL means no value (i.e. element value
 *                 is set to NULL)
 * @param element_pp     Returned unpacked parameter element (id + tag + value)
 *                 This is allocated by the function.
 *
 * @return (0) on success, (-1) on error
 *
 * @note '*element_pp' needs to be freed with
 *       mpl_param_element_destroy()
 *
 **/
int mpl_param_unpack(const char* id_str,
                     const char* value_str,
                     mpl_param_element_t** element_pp);

/**
 * @ingroup MPL_PARAM
 * mpl_param_unpack_param_set - unpack received parameter strings
 *                              falling back to default parameter set
 *                              if not indicated by key string
 *
 * @param id_str        Parameter set and name (null terminated string)
 * @param value_str      Parameter value (null terminated string)
 *                 NULL means no value (i.e. element value
 *                 is set to NULL)
 * @param element_pp     Returned unpacked parameter element (id + tag + value)
 *                 This is allocated by the function.
 * @param param_set_id   default parameter set id (fallback)
 *
 * @return (0) on success, (-1) on error
 *
 * @note '*element_pp' needs to be freed with
 *       mpl_param_element_destroy()
 *
 **/
int mpl_param_unpack_param_set(const char* id_str,
                               const char* value_str,
                               mpl_param_element_t** element_pp,
                               int param_set_id);

int mpl_param_unpack_internal(const char* id_str,
                              const char* value_str,
                              mpl_param_element_t** element_pp,
                              const mpl_pack_options_t *options_p,
                              mpl_param_element_id_t unpack_context);

/**
 * @ingroup MPL_PARAM
 * mpl_param_id_get_string
 *
 * Get string corresponding to the parameter ID
 *
 * @param param_id       Param ID
 * @return String (statically allocated, overwritten every 4th call
 *                 in the same thread)
 *
 */
const char *mpl_param_id_get_string(mpl_param_element_id_t param_id);

/**
 * @ingroup MPL_PARAM
 * mpl_param_id_get_prefix
 *
 * Get string (the prefix) corresponding to the parameter set ID
 *
 * @param param_id       Param ID
 * @return String (statically allocated)
 *
 */
const char *mpl_param_id_get_prefix(mpl_param_element_id_t param_id);

/**
 * @ingroup MPL_PARAM
 * mpl_param_id_get_type
 *
 * Get type corresponding to the parameter id
 *
 * @param param_id       Param ID
 * @return type (enum)
 */
mpl_type_t mpl_param_id_get_type(mpl_param_element_id_t param_id);

/**
 * @ingroup MPL_PARAM
 * mpl_param_id_get_type_string
 *
 * Get type string corresponding to the parameter id
 *
 * @param param_id       Param ID
 * @return the string
 *
 */
const char *mpl_param_id_get_type_string(mpl_param_element_id_t param_id);

/**
 * @ingroup MPL_PARAM
 * mpl_param_id_sizeof_param_value
 *
 * Get sizeof param values for a certain Parameter ID
 * Please note that the size is only possible to determine
 * for parameter IDs with fixed size types.
 *
 * @param param_id       Param ID
 *
 * @return 0 Parameter value is not fixed size (e.g. string, etc)
 *                 >= On success
 *
 */
size_t mpl_param_id_sizeof_param_value(mpl_param_element_id_t param_id);

/**
 * @ingroup MPL_PARAM
 * mpl_param_id_is_child
 *
 * Investigates if a param id is a child of another
 *
 * @param parent_param_id       Parent Param ID
 * @param child_param_id        Child Param ID
 *
 * @return 1 Yes
 *         0 No or error
 *
 */
#define mpl_param_id_is_child(parent_param_id, child_param_id)        \
    (mpl_param_get_child_index(parent_param_id, child_param_id) >= 0)

/**
 * @ingroup MPL_PARAM
 * mpl_param_id_is_same_or_child
 *
 * Investigates if a param id is the same or a child
 *
 * @param first_param_id       First (parent) param ID
 * @param second_param_id        Second (child) param ID
 *
 * @return 1 Yes
 *         0 No or error
 *
 */
#define mpl_param_id_is_same_or_child(first_param_id, second_param_id)  \
    ((first_param_id == second_param_id) || mpl_param_id_is_child((first_param_id),(second_param_id)))

/**
 * @ingroup MPL_PARAM
 * mpl_param_get_child_id
 *
 * Get param id of child given child index
 *
 * @param parent_param_id       Parent Param ID
 * @param child_index           Child Index
 *
 * @return child param id
 *         0 on error
 *
 */
int mpl_param_get_child_id(mpl_param_element_id_t parent_param_id,
                           int child_index);

/**
 * @ingroup MPL_PARAM
 * mpl_param_get_child_index
 *
 * Get index of child given parent amd child id
 *
 * @param parent_param_id       Parent Param ID
 * @param child_param_id        Child ID
 *
 * @return child index
 *         -1 on error
 *
 *
 */
int mpl_param_get_child_index(mpl_param_element_id_t parent_param_id,
                              mpl_param_element_id_t child_param_id);

/**
 * @ingroup MPL_PARAM
 * mpl_param_get_bag_field_id
 *
 * Get param id of bag-field given bag and index
 *
 * @param bag_param_id           Bag Param ID
 * @param field_index            Field Index
 *
 * @return field param id
 *         0 on error
 *
 */
mpl_param_element_id_t mpl_param_get_bag_field_id(mpl_param_element_id_t bag_param_id,
                                                  int field_index);

/**
 * @ingroup MPL_PARAM
 * mpl_param_get_bag_field_context
 *
 * Get param id of context of bag-field given bag and index
 *
 * @param bag_param_id           Bag Param ID
 * @param field_index            Field Index
 *
 * @return context param id
 *         0 on error
 *
 */
mpl_param_element_id_t mpl_param_get_bag_field_context(mpl_param_element_id_t bag_param_id,
                                                       int field_index);

/**
 * @ingroup MPL_PARAM
 * mpl_param_get_bag_field_name
 *
 * Get name of bag-field given bag and index
 *
 * @param bag_param_id           Bag Param ID
 * @param field_index            Field Index
 *
 * @return field name
 *
 */
const char *mpl_param_get_bag_field_name(mpl_param_element_id_t bag_param_id,
                                         int field_index);

/**
 * @ingroup MPL_PARAM
 * mpl_param_value_get_string
 *
 * Get string corresponding to the parameter value
 *
 * @param param_id       Param ID
 * @param value_p        Param value
 *
 * @return String (statically allocated, overwritten by each call)
 *
 */
const char *mpl_param_value_get_string(mpl_param_element_id_t param_id,
                                       void* value_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_value_get_range_id
 *
 * Get range id corresponding to the parameter value
 *
 * @param param_id       Param ID
 * @param value_p        Param value
 *
 * @return Range id
 *
 */
int mpl_param_value_get_range_id(mpl_param_element_id_t param_id,
                                 void* value_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_value_copy_out
 *
 * Copy value to user allocated memory
 * For structured MPL types (eg. uint8_array) this function the
 * value referes to the "raw" value (eg. arr_p) not the whole MPL
 * structure.
 *
 * @param element_p       Paramameter element to copy from
 * @param to_value_p            The value is copied into this memory area
 * @param size            Size of the destination memory in bytes.
 *                      If too small to contain the value, the copying will
 *                      not be complete.
 *
 * @return On success, number of bytes occupied by the value.
 *                 Negative return values means error (-1 = copy not supported,
 *                 -2 = parameter error).
 *                 Note that supplying a too small 'size' is not an error, it
 *                 just means that not all bytes are copied (the function will
 *                 in this case return the number of bytes that would have been
 *                 copied).
 *
 */
int mpl_param_value_copy_out(const mpl_param_element_t* element_p,
                             void *to_value_p,
                             int size);

/**
 * @ingroup MPL_PARAM
 * mpl_param_allow_get
 *
 * Check if the parameter is allowed to be fetched
 *              with the 'get' command
 *
 * @param param_id     Param ID
 *
 * @return true/false
 *
 */
#define mpl_param_allow_get(param_id) mpl_param_allow_get_bl(param_id, NULL)

/**
 * @ingroup MPL_PARAM
 * mpl_param_allow_get_bl
 *
 * Check if the parameter is allowed to be fetched
 *              with the 'get' command.
 *              Results are filtered through a blacklist.
 *
 * @param param_id     Param ID
 * @param blacklist    The blacklist
 *
 * @return true/false
 *
 */
bool mpl_param_allow_get_bl(mpl_param_element_id_t param_id,
                            mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_PARAM
 * mpl_param_allow_set
 *
 * Check if the parameter is allowed to be fetched
 *              with the 'set' command
 *
 * @param param_id     Param ID
 *
 * @return true/false
 *
 */
#define mpl_param_allow_set(param_id) mpl_param_allow_set_bl(param_id, NULL)

/**
 * @ingroup MPL_PARAM
 * mpl_param_allow_set_bl
 *
 * Check if the parameter is allowed to be fetched
 *              with the 'set' command
 *              Results are filtered through a blacklist.
 *
 * @param param_id     Param ID
 * @param blacklist     Parameters present in the blacklist will return false
 *
 * @return true/false
 *
 */
bool mpl_param_allow_set_bl(mpl_param_element_id_t param_id,
                            mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_PARAM
 * mpl_param_allow_config
 *
 * Check if it is allowed to get this parameter from config file
 *
 * @param param_id     Param ID
 *
 * @return true/false
 *
 */
#define mpl_param_allow_config(param_id)        \
    mpl_param_allow_config_bl(param_id, NULL)

/**
 * @ingroup MPL_PARAM
 * mpl_param_allow_config_bl
 *
 * Check if it is allowed to get this parameter from config file
 *              Results are filtered through a blacklist.
 *
 * @param param_id     Param ID
 * @param blacklist     Parameters present in the blacklist will return false
 *
 * @return true/false
 *
 */
bool mpl_param_allow_config_bl(mpl_param_element_id_t param_id,
                               mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_stringn
 *
 * Allocate and fill in a parameter element with
 *              string type value. Allocates 'len' bytes for the string.
 *
 * @param param_id     Param ID
 * @param value        Value
 * @param len           string length
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_stringn(param_id, value, len)    \
    mpl_param_element_create_stringn_tag(param_id, 0, value, len)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_stringn_tag
 *
 * Allocate and fill in a parameter element with
 *              string type value. Allocates 'len' bytes for the string.
 *              With tag.
 *
 * @param    param_id     Param ID
 * @param    tag The tag
 * @param    value        Value
 * @param    len           string length
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_stringn_tag(mpl_param_element_id_t param_id,
                                                          int tag,
                                                          const char* value, size_t len);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_wstringn
 *
 * Allocate and fill in a parameter element with
 *              widestring type value. Allocates 'len' wchars for the string.
 *
 * @param    param_id     Param ID
 * @param    value        Value
 * @param    len           string length
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_wstringn(param_id, value, len)     \
    mpl_param_element_create_wstringn_tag(param_id, 0, value, len)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_wstringn_tag
 *
 * Allocate and fill in a parameter element with
 *              widestring type value. Allocates 'len' wchars for the string.
 *              With tag.
 *
 * @param    param_id     Param ID
 * @param    tag        Tag
 * @param    value        Value
 * @param    len           string length
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_wstringn_tag(mpl_param_element_id_t param_id,
                                                           int tag,
                                                           const wchar_t* value, size_t len);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_uint8_array
 *
 * Allocate and fill in a parameter element with
 *              uint8 array type value. Allocates 'size' uint8 for the array.
 *
 * @param    param_id     Param ID
 * @param    value        Value
 * @param    size         array size
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_uint8_array(param_id, value, size)     \
    mpl_param_element_create_uint8_array_tag(param_id, 0, value, size)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_uint8_array_tag
 *
 * Allocate and fill in a parameter element with
 *              uint8 array  type value. Allocates 'size' uint8 for the array.
 *              With tag.
 *
 * @param    param_id     Param ID
 * @param    tag          Tag
 * @param    value        Value
 * @param    size         array size
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_uint8_array_tag(mpl_param_element_id_t param_id,
                                                              int tag,
                                                              const uint8_t* value,
                                                              size_t size);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_uint16_array
 *
 * Allocate and fill in a parameter element with
 *              uint16 array type value. Allocates 'size' uint16 for the array.
 *
 * @param param_id     Param ID
 * @param value        Value
 * @param size         array size
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_uint16_array(param_id, value, size)    \
    mpl_param_element_create_uint16_array_tag(param_id, 0, value, size)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_uint16_array_tag
 *
 * Allocate and fill in a parameter element with
 *              uint16 array  type value. Allocates 'size' uint16 for the
 *              string. With tag.
 *
 * @param    param_id     Param ID
 * @param    tag          Tag
 * @param    value        Value
 * @param    size         array size
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_uint16_array_tag(mpl_param_element_id_t param_id,
                                                               int tag,
                                                               const uint16_t* value,
                                                               size_t size);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_uint32_array
 *
 * Allocate and fill in a parameter element with
 *              uint32 array type value. Allocates 'size' uint32 for the array.
 *
 * @param    param_id     Param ID
 * @param    value        Value
 * @param    size         array size
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_uint32_array(param_id, value, size)    \
    mpl_param_element_create_uint32_array_tag(param_id, 0, value, size)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_uint32_array_tag
 *
 * Allocate and fill in a parameter element with
 *              uint32 array  type value. Allocates 'size' uint32 for the
 *              string. With tag.
 *
 * @param    param_id     Param ID
 * @param    tag          Tag
 * @param    value        Value
 * @param    size         array size
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_uint32_array_tag(mpl_param_element_id_t param_id,
                                                               int tag,
                                                               const uint32_t* value,
                                                               size_t size);


/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_string_tuple
 *
 * Allocate and fill in a parameter element with
 *              string tuple value.
 *
 * @param    param_id     Param ID
 * @param    key          tuple key
 * @param    val          tuple value
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_string_tuple(param_id, key, val)     \
    mpl_param_element_create_string_tuple_tag(param_id, 0, key, val)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_string_tuple_tag
 *
 * Allocate and fill in a parameter element with
 *              string tuple value. With tag.
 *
 * @param    param_id     Param ID
 * @param    tag          Tag
 * @param    key          tuple key
 * @param    val          tuple value
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_string_tuple_tag(mpl_param_element_id_t param_id,
                                                               int tag,
                                                               const char* key,
                                                               const char* val);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_int_tuple
 *
 * Allocate and fill in a parameter element with
 *              int tuple value.
 *
 * @param    param_id     Param ID
 * @param    key          tuple key
 * @param    val          tuple value
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_int_tuple(param_id, key, val)    \
    mpl_param_element_create_int_tuple_tag(param_id, 0, key, val)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_int_tuple_tag
 *
 * Allocate and fill in a parameter element with
 *              int tuple value. With tag.
 *
 * @param    param_id     Param ID
 * @param    tag          Tag
 * @param    key          tuple key
 * @param    val          tuple value
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_int_tuple_tag(mpl_param_element_id_t param_id,
                                                            int tag,
                                                            int key, int val);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_strint_tuple
 *
 * Allocate and fill in a parameter element with
 *              strint tuple value.
 *
 * @param    param_id     Param ID
 * @param    key          tuple key
 * @param    val          tuple value
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_strint_tuple(param_id, key, val)     \
    mpl_param_element_create_strint_tuple_tag(param_id, 0, key, val)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_strint_tuple_tag
 *
 * Allocate and fill in a parameter element with
 *              strint tuple value. With tag.
 *
 * @param    param_id     Param ID
 * @param    tag          Tag
 * @param    key          tuple key
 * @param    val          tuple value
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_strint_tuple_tag(mpl_param_element_id_t param_id,
                                                               int tag,
                                                               const char* key, int val);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_struint8_tuple
 *
 * Allocate and fill in a parameter element with
 *              struint8 tuple value.
 *
 * @param    param_id     Param ID
 * @param    key          tuple key
 * @param    val          tuple value
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_struint8_tuple(param_id, key, val)     \
    mpl_param_element_create_struint8_tuple_tag(param_id, 0, key, val)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_struint8_tuple_tag
 *
 * Allocate and fill in a parameter element with
 *              struint8 tuple value. With tag.
 *
 * @param    param_id     Param ID
 * @param    tag          Tag
 * @param    key          tuple key
 * @param    val          tuple value
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_struint8_tuple_tag(mpl_param_element_id_t param_id,
                                                                 int tag,
                                                                 const char* key, uint8_t val);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_empty
 *
 * Allocate and fill in a parameter element
 *              without any value.
 *
 * @param    param_id     Param ID
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_empty(param_id)    \
    mpl_param_element_create_empty_tag(param_id, 0)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_empty
 *
 * Allocate and fill in a parameter element
 *              without any value, but with a given tag.
 *
 * @param    param_id     Param ID
 * @param    tag        Tag
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_empty_tag(mpl_param_element_id_t param_id, int tag);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_n
 *
 * Allocate and fill in a parameter element
 *              including the value, with value length check.
 *
 * @param    param_id     Param ID
 * @param    value_p      Param value
 * @param    len          Param len (ignored for parameter types with built-in size)
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create_n(param_id, value_p, len)    \
    mpl_param_element_create_n_tag(param_id, 0, value_p, len)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_n_tag
 *
 * Allocate and fill in a parameter element
 *              including the value, with value length check. With tag.
 *
 * @param    param_id     Param ID
 * @param    tag          Tag
 * @param    value_p      Param value
 * @param    len          Param len (ignored for parameter types with built-in size)
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_n_tag(mpl_param_element_id_t param_id,
                                                    int tag,
                                                    const void* value_p,
                                                    size_t len);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create
 *
 * Allocate and fill in a parameter element
 *              including the value
 *
 * @param    param_id     Param ID
 * @param    value_p      Param value
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_create(param_id, value_p)     \
    mpl_param_element_create_tag(param_id, 0, value_p)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_create_tag
 *
 * Allocate and fill in a parameter element
 *              including the value. With tag.
 *
 * @param    param_id     Param ID
 * @param    tag        Tag
 * @param    value_p      Param value
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_create_tag(mpl_param_element_id_t param_id,
                                                  int tag,
                                                  const void* value_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_clone
 *
 * Creates (and allocates) a clone of the param element
 *
 * @param    element_p     Paramameter element to clone
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_clone(const mpl_param_element_t* element_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_compare
 *
 * Compares two param elements
 *
 * @param    element1_p     Paramameter element 1
 * @param    element2_p     Paramameter element 2
 *
 * @return 0 if equal, -1 if not
 *
 *
 */
int mpl_param_element_compare(const mpl_param_element_t* element1_p,
                              const mpl_param_element_t* element2_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_get_default
 *
 * Get param element with default value
 *
 * @param    param_id     Param ID
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
#define mpl_param_element_get_default(param_id)       \
    mpl_param_element_get_default_bl(param_id, NULL)

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_get_default
 *
 * Get param element with default value
 *              Results are filtered through a blacklist
 *
 * @param    param_id     Param ID
 *  @param   blacklist    Parameters present in the blacklist will return NULL
 *
 * @return Parameter element or NULL
 *
 * @note The returned parameter element needs to be freed with
 *       mpl_param_element_destroy()
 *
 */
mpl_param_element_t *mpl_param_element_get_default_bl(mpl_param_element_id_t param_id,
                                                      mpl_blacklist_t blacklist);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_destroy
 *
 * Free parameter element (including value)
 *
 * @param    element_p     Paramameter element to destroy
 *
 * @return -
 *
 */
void mpl_param_element_destroy(mpl_param_element_t* element_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_set_tag
 *
 * Set the tag of an existing parameter element
 *
 * @param    element_p    Paramameter element
 * @param    tag        tag value
 *
 * @return 0 on success, -1 on failure.
 *
 */
int mpl_param_element_set_tag(mpl_param_element_t* element_p, int tag);

/**
 * @ingroup MPL_PARAM
 * mpl_param_element_get_tag
 *
 * Get the tag of a parameter element
 *
 * @param    element_p     Paramameter element
 *
 * @return tag
 *
 */
#define mpl_param_element_get_tag(element_p) (element_p)->tag

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_pack
 *
 * Pack a parameter list
 *
 * @param    param_list_p parameter list to pack
 * @param    buf_p        buffer to store packed list
 * @param    buflen       length of the buffer
 *
 * @return  Number of bytes written to buf (excluding '\0') on success.
 *           -1 on error.
 *           If the output was truncated due to 'buflen' then the return value
 *           is the number of characters which would have been written to the
 *           final string if enough space had been available
 *
 *                   The function does not write more than buflen bytes to the
 *                   buffer (including '\0').
 *                   If more would have been written, it is truncated.
 *
 */
int mpl_param_list_pack(mpl_list_t *param_list_p,
                        char *buf_p,
                        int buflen);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_pack_no_prefix
 *
 * Pack a parameter list without writing the prefix
 *
 * @param    param_list_p parameter list to pack
 * @param    buf_p        buffer to store packed list
 * @param    buflen       length of the buffer
 *                   The function does not write more than buflen bytes to the
 *                   buffer (including '\0').
 *                   If more would have been written, it is truncated.
 * @return  Number of bytes written to buf (excluding '\0') on success.
 *           -1 on error.
 *           If the output was truncated due to 'buflen' then the return value
 *           is the number of characters which would have been written to the
 *           final string if enough space had been available
 */
int mpl_param_list_pack_no_prefix(mpl_list_t *param_list_p,
                                  char *buf_p,
                                  int buflen);

int mpl_param_list_pack_extended(mpl_list_t *param_list_p,
                                 char *buf_p,
                                 int buflen,
                                 const mpl_pack_options_t *options_p);

/* for backward compatibility */
int mpl_param_list_pack_internal(mpl_list_t *param_list_p,
                                 char *buf_p,
                                 int buflen,
                                 char message_delimiter,
                                 bool no_prefix);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_unpack - unpack packed parameter list
 *
 * @param     buf_p           message to be unpacked (zero terminated)
 *
 * @return parameter list on success, NULL on failure (or no params)
 *
 **/
mpl_list_t *mpl_param_list_unpack(char *buf_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_unpack_error - unpack packed parameter list with error
 *                               indication
 *
 * @param     buf_p           message to be unpacked (zero terminated)
 * @param     has_error_p     Did the unpack fail with errors
 *
 * @return parameter list on success, NULL on failure (or no params)
 *
 **/
mpl_list_t *mpl_param_list_unpack_error(char *buf_p, bool *has_error_p);


/**
 * @ingroup MPL_PARAM
 * mpl_param_list_unpack_param_set - unpack packed parameter list
 *                              falling back to default parameter set
 *                              if not indicated by key string
 *
 * @param     buf_p           message to be unpacked (zero terminated)
 * @param     param_set_id    default parameter set id (fallback)
 *
 * @return parameter list on success, NULL on failure (or no params)
 *
 **/
mpl_list_t *mpl_param_list_unpack_param_set(char *buf_p, int param_set_id);


/**
 * @ingroup MPL_PARAM
 * mpl_param_list_unpack_param_set_error -
 *                              unpack packed parameter list with
 *                              error indication
 *                              falling back to default parameter set
 *                              if not indicated by key string
 *
 * @param     buf_p           message to be unpacked (zero terminated)
 * @param     param_set_id    default parameter set id (fallback)
 * @param     has_error_p     Did the unpack fail with errors
 *
 * @return parameter list on success, NULL on failure (or no params)
 *
 **/
mpl_list_t *mpl_param_list_unpack_param_set_error(char *buf_p,
                                                  int param_set_id,
                                                  bool *has_error_p);

mpl_list_t *mpl_param_list_unpack_internal(char *buf_p,
                                           const mpl_pack_options_t *options_p,
                                           mpl_param_element_id_t unpack_context,
                                           bool *has_error_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_clone
 *
 * Clone a parameter list
 *
 * @param    param_list_p parameter list to clone
 *
 * @return pointer to cloned parameter list, NULL on error
 */
mpl_list_t *mpl_param_list_clone(mpl_list_t *param_list_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_destroy
 *
 * Destroy all parameters elements in list
 *
 * @param    param_list_pp List of parameters
 *
 * @return -
 *
 */
void mpl_param_list_destroy(mpl_list_t **param_list_pp);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find
 *
 * Find a parameter element identified by param_id in list
 *
 * @param    param_id     Param ID to search for
 * @param    param_list_p List of parameters
 *
 * @return Found matching parameter element or NULL
 *
 */
mpl_param_element_t *mpl_param_list_find(mpl_param_element_id_t param_id,
                                         mpl_list_t *param_list_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_next
 *
 * Find the next parameter element identified by param_id in list
 *
 * @param    param_id     Param ID to search for
 * @param    elem         Current element
 *
 * @return Found matching parameter element or NULL
 *
 */
#define mpl_param_list_find_next(param_id,elem)                         \
    mpl_param_list_find((param_id),                                     \
                        ((elem)==NULL?NULL:(elem)->list_entry.next_p))

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_next_tag
 *
 * Find the next parameter element identified by param_id and tag
 *              in list
 *
 * @param    param_id     Param ID to search for (or MPL_PARAM_ID_UNDEFINED to find
 *                   any parameters having the tag)
 * @param    tag          tag to look for
 * @param    elem         Current element
 *
 * @return Found matching parameter element or NULL
 *
 */
#define mpl_param_list_find_next_tag(param_id,tag,elem)                 \
    mpl_param_list_find_tag((param_id),                                 \
                            (tag),                                      \
                            ((elem)==NULL?NULL:(elem)->list_entry.next_p))

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_all
 *
 * Find all parameter elements identified by param_id in list
 *
 * @param    param_id     Param ID to search for
 * @param    param_list_p List of parameters
 *
 * @return Parameter list of any found elements or NULL
 *
 */
mpl_list_t *mpl_param_list_find_all(mpl_param_element_id_t param_id,
                                    mpl_list_t *param_list_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_all_tag
 *
 * Find all parameter elements identified by param_id in list
 *
 * @param    param_id     Param ID to search for (or MPL_PARAM_ID_UNDEFINED to find
 *                   any parameters having the tag)
 * @param    tag          tag to look for
 * @param    param_list_p List of parameters
 *
 * @return Parameter list of any found elements or NULL
 *
 */
mpl_list_t *mpl_param_list_find_all_tag(mpl_param_element_id_t param_id,
                                        int tag,
                                        mpl_list_t *param_list_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_param_count
 *
 * Return number of parameter elements identified by param_id in list
 *
 * @param    param_id     Param ID to search for
 * @param    param_list_p List of parameters
 *
 * @return Number of parameter elements
 *
 */
int mpl_param_list_param_count(mpl_param_element_id_t param_id,
                               mpl_list_t *param_list_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_param_count_tag
 *
 * Return number of parameter elements identified by param_id in list
 *
 * @param    param_id     Param ID to search for (or MPL_PARAM_ID_UNDEFINED to find
 *                        any parameters having the tag)
 * @param    tag        Tag to match
 * @param    param_list_p List of parameters
 *
 * @return Number of parameter elements
 *
 */
int mpl_param_list_param_count_tag(mpl_param_element_id_t param_id,
                                   int tag,
                                   mpl_list_t *param_list_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_tag
 *
 * Find a parameter element identified by param_id and tag in list
 *
 * @param    param_id     Param ID to search for (or MPL_PARAM_ID_UNDEFINED to find
 *                   any parameters having the tag)
 * @param    tag        Tag to match
 * @param    param_list_p List of parameters
 *
 * @return Found matching parameter element or NULL
 *
 */
mpl_param_element_t* mpl_param_list_find_tag( mpl_param_element_id_t param_id,
                                              int tag,
                                              mpl_list_t *param_list_p );

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_field
 *
 * Find a parameter element identified by field in list
 *
 * @param    context        Param ID of context
 * @param    id_in_context  id in the context
 * @param    param_list_p List of parameters
 *
 * @return Found matching parameter element or NULL
 *
 */
mpl_param_element_t *mpl_param_list_find_field( mpl_param_element_id_t context,
                                                int id_in_context,
                                                mpl_list_t *param_list_p );

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_field_next
 *
 * Find the next parameter element identified by field in list
 *
 * @param    context        Param ID of context
 * @param    id_in_context  id in the context
 * @param    elem         Current element
 *
 * @return Found matching parameter element or NULL
 *
 */
#define mpl_param_list_find_field_next(context,id_in_context,elem)      \
    mpl_param_list_find_field((context),(id_in_context),                \
                              ((elem)==NULL?NULL:(elem)->list_entry.next_p))

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_field_next_tag
 *
 * Find the next parameter element identified by field and tag in list
 *
 * @param    context        Param ID of context
 * @param    id_in_context  id in the context
 * @param    tag          tag to look for
 * @param    elem         Current element
 *
 * @return Found matching parameter element or NULL
 *
 */
#define mpl_param_list_find_field_next_tag(context,id_in_context,tag,elem)  \
    mpl_param_list_find_field_tag((context),(id_in_context),            \
                                  tag,                                  \
                                  ((elem)==NULL?NULL:(elem)->list_entry.next_p))

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_field_all
 *
 * Find all parameter elements identified by field in list
 *
 * @param    context        Param ID of context
 * @param    id_in_context  id in the context
 * @param    param_list_p List of parameters
 *
 * @return Parameter list of any found elements or NULL
 *
 */
mpl_list_t *mpl_param_list_find_field_all( mpl_param_element_id_t context,
                                           int id_in_context,
                                           mpl_list_t *param_list_p );

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_field_all
 *
 * Find all parameter elements identified by field in list
 *
 * @param    context        Param ID of context
 * @param    id_in_context  id in the context
 * @param    tag        Tag to match
 * @param    param_list_p List of parameters
 *
 * @return Parameter list of any found elements or NULL
 *
 */
mpl_list_t *mpl_param_list_find_field_all_tag( mpl_param_element_id_t context,
                                               int id_in_context,
                                               int tag,
                                               mpl_list_t *param_list_p );

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_field_tag
 *
 * Find a parameter element identified by field and tag in list
 *
 * @param    context        Param ID of context
 * @param    id_in_context  id in the context
 * @param    tag         Tag to match
 * @param    param_list_p List of parameters
 *
 * @return Found matching parameter element or NULL
 *
 */
mpl_param_element_t *mpl_param_list_find_field_tag( mpl_param_element_id_t context,
                                                    int id_in_context,
                                                    int tag,
                                                    mpl_list_t *param_list_p );

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_field_count
 *
 * Return number of parameter elements identified by field in list
 *
 * @param    context        Param ID of context
 * @param    id_in_context  id in the context
 * @param    param_list_p List of parameters
 *
 * @return Number of parameters
 *
 */
int mpl_param_list_field_count( mpl_param_element_id_t context,
                                int id_in_context,
                                mpl_list_t *param_list_p );

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_find_field_count_tag
 *
 * Return number of parameter elements identified by field in list
 *
 * @param    context     Param ID to search for
 * @param    id_in_context  id in the context
 * @param    tag        Tag to match
 * @param    param_list_p List of parameters
 *
 * @return Number of parameters
 *
 */
int mpl_param_list_field_count_tag( mpl_param_element_id_t context,
                                    int id_in_context,
                                    int tag,
                                    mpl_list_t *param_list_p );

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_*_tuple_key_find
 *
 * Find a parameter element identified by param_id and tuple key
 *              in list
 *              Note that the parameter must be of type '*_tuple'
 *
 * @param    param_id     Param ID to search for
 * @param    key_p        Key to search for
 * @param    param_list_p List of parameters
 *
 * @return Found matching parameter element or NULL
 *
 */
mpl_param_element_t *mpl_param_list_string_tuple_key_find(mpl_param_element_id_t param_id,
                                                          char *key_p,
                                                          mpl_list_t *param_list_p);

mpl_param_element_t *mpl_param_list_strint_tuple_key_find(mpl_param_element_id_t param_id,
                                                          char *key_p,
                                                          mpl_list_t *param_list_p);

mpl_param_element_t *mpl_param_list_struint8_tuple_key_find(mpl_param_element_id_t param_id,
                                                            char *key_p,
                                                            mpl_list_t *param_list_p);

mpl_param_element_t *mpl_param_list_int_tuple_key_find(mpl_param_element_id_t param_id,
                                                       int key,
                                                       mpl_list_t *param_list_p);

/* For backward compatibility */
#define mpl_param_list_tuple_key_find mpl_param_list_string_tuple_key_find


/**
 * @ingroup MPL_PARAM
 * mpl_param_list_*_tuple_find
 *
 * Find a parameter element identified by param_id and tuple
 *              (key and value) in list
 *              Note that the parameter must be of type '*_tuple'
 *
 * @param    param_id     Param ID to search for
 * @param    key_p        Key to search for
 * @param    value_p      Value to search for
 * @param    param_list_p List of parameters
 *
 * @return Found matching parameter element or NULL
 *
 */
mpl_param_element_t *mpl_param_list_string_tuple_find(mpl_param_element_id_t param_id,
                                                      char *key_p,
                                                      char *value_p,
                                                      mpl_list_t *param_list_p);
mpl_param_element_t *mpl_param_list_strint_tuple_find(mpl_param_element_id_t param_id,
                                                      char *key_p,
                                                      int value,
                                                      mpl_list_t *param_list_p);
mpl_param_element_t *mpl_param_list_int_tuple_find(mpl_param_element_id_t param_id,
                                                   int key, int value,
                                                   mpl_list_t *param_list_p);
mpl_param_element_t *mpl_param_list_struint8_tuple_find(mpl_param_element_id_t param_id,
                                                        char *key_p,
                                                        uint8_t value,
                                                        mpl_list_t *param_list_p);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_tuple_key_find_wildcard
 *
 * Find a parameter element identified by param_id and tuple key
 *              in list, but with wildcard fallback.
 *              Note that the parameter must be of type 'string_tuple'
 *
 * @param    param_id     Param ID to search for
 * @param    key_p        Key to search for
 * @param    wildcard_p   Wildcard to search for
 * @param    param_list_p List of parameters
 *
 * @return Found matching parameter element or NULL
 *
 */
mpl_param_element_t *mpl_param_list_tuple_key_find_wildcard(mpl_param_element_id_t param_id,
                                                            char *key_p,
                                                            char *wildcard_p,
                                                            mpl_list_t *param_list_p);

#ifndef DOXYGEN
/**
 * @ingroup MPL_PARAM
 * mpl_pack_param_value_*
 *
 * Encode (pack) a value of a specific type from local format to
 *              "on the wire" format
 *
 * See mpl_pack_param_fp for more details.
 *
 */
int mpl_pack_param_value_string(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p);
int mpl_pack_param_value_wstring(const void* param_value_p,
                                 char *buf,
                                 size_t buflen,
                                 const mpl_param_descr2_t *descr_p,
                                 const mpl_pack_options_t *options_p);
int mpl_pack_param_value_int(const void* param_value_p,
                             char *buf,
                             size_t buflen,
                             const mpl_param_descr2_t *descr_p,
                             const mpl_pack_options_t *options_p);
int mpl_pack_param_value_sint8(const void* param_value_p,
                               char *buf,
                               size_t buflen,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p);
int mpl_pack_param_value_sint16(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p);
int mpl_pack_param_value_sint32(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p);
int mpl_pack_param_value_sint64(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p);
int mpl_pack_param_value_uint8(const void* param_value_p,
                               char *buf,
                               size_t buflen,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p);
int mpl_pack_param_value_uint16(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p);
int mpl_pack_param_value_uint32(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p);
int mpl_pack_param_value_uint64(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p);
int mpl_pack_param_value_enum(const void* param_value_p,
                              char *buf,
                              size_t buflen,
                              const mpl_param_descr2_t *descr_p,
                              const mpl_pack_options_t *options_p);
int mpl_pack_param_value_enum8(const void* param_value_p,
                               char *buf,
                               size_t buflen,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p);
int mpl_pack_param_value_enum16(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p);
int mpl_pack_param_value_enum32(const void* param_value_p,
                                char *buf,
                                size_t buflen,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p);
int mpl_pack_param_value_signed_enum8(const void* param_value_p,
                                      char *buf,
                                      size_t buflen,
                                      const mpl_param_descr2_t *descr_p,
                                      const mpl_pack_options_t *options_p);
int mpl_pack_param_value_signed_enum16(const void* param_value_p,
                                       char *buf,
                                       size_t buflen,
                                       const mpl_param_descr2_t *descr_p,
                                       const mpl_pack_options_t *options_p);
int mpl_pack_param_value_signed_enum32(const void* param_value_p,
                                       char *buf,
                                       size_t buflen,
                                       const mpl_param_descr2_t *descr_p,
                                       const mpl_pack_options_t *options_p);
int mpl_pack_param_value_bool(const void* param_value_p,
                              char *buf,
                              size_t buflen,
                              const mpl_param_descr2_t *descr_p,
                              const mpl_pack_options_t *options_p);
int mpl_pack_param_value_bool8(const void* param_value_p,
                               char *buf,
                               size_t buflen,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p);
int mpl_pack_param_value_uint8_array(const void* param_value_p,
                                     char *buf,
                                     size_t buflen,
                                     const mpl_param_descr2_t *descr_p,
                                     const mpl_pack_options_t *options_p);
int mpl_pack_param_value_uint16_array(const void* param_value_p,
                                      char *buf,
                                      size_t buflen,
                                      const mpl_param_descr2_t *descr_p,
                                      const mpl_pack_options_t *options_p);
int mpl_pack_param_value_uint32_array(const void* param_value_p,
                                      char *buf,
                                      size_t buflen,
                                      const mpl_param_descr2_t *descr_p,
                                      const mpl_pack_options_t *options_p);
int mpl_pack_param_value_string_tuple(const void* param_value_p,
                                      char *buf,
                                      size_t buflen,
                                      const mpl_param_descr2_t *descr_p,
                                      const mpl_pack_options_t *options_p);
int mpl_pack_param_value_int_tuple(const void* param_value_p,
                                   char *buf,
                                   size_t buflen,
                                   const mpl_param_descr2_t *descr_p,
                                   const mpl_pack_options_t *options_p);
int mpl_pack_param_value_strint_tuple(const void* param_value_p,
                                      char *buf,
                                      size_t buflen,
                                      const mpl_param_descr2_t *descr_p,
                                      const mpl_pack_options_t *options_p);
int mpl_pack_param_value_struint8_tuple(const void* param_value_p,
                                        char *buf,
                                        size_t buflen,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p);
int mpl_pack_param_value_bag(const void* param_value_p,
                             char *buf,
                             size_t buflen,
                             const mpl_param_descr2_t *descr_p,
                             const mpl_pack_options_t *options_p);
int mpl_pack_param_value_addr(const void* param_value_p,
                              char *buf,
                              size_t buflen,
                              const mpl_param_descr2_t *descr_p,
                              const mpl_pack_options_t *options_p);

/**
 * @ingroup MPL_PARAM
 * mpl_unpack_param_value_*
 *
 * Decode (unpack) a value of a specific type from "on the wire"
 *              format to local format
 *
 * See mpl_unpack_param_fp for more details.
 *
 */
int mpl_unpack_param_value_string(const char* value_str,
                                  void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_wstring(const char* value_str,
                                   void **value_pp,
                                   const mpl_param_descr2_t *descr_p,
                                   const mpl_pack_options_t *options_p,
                                   mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_int(const char* value_str,
                               void **value_pp,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p,
                               mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_sint8(const char* value_str,
                                 void **value_pp,
                                 const mpl_param_descr2_t *descr_p,
                                 const mpl_pack_options_t *options_p,
                                 mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_sint16(const char* value_str,
                                  void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_sint32(const char* value_str,
                                  void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_sint64(const char* value_str,
                                  void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_uint8(const char* value_str,
                                 void **value_pp,
                                 const mpl_param_descr2_t *descr_p,
                                 const mpl_pack_options_t *options_p,
                                 mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_uint16(const char* value_str,
                                  void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_uint32(const char* value_str,
                                  void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_uint64(const char* value_str,
                                  void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_enum(const char* value_str,
                                void **value_pp,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p,
                                mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_enum8(const char* value_str,
                                 void **value_pp,
                                 const mpl_param_descr2_t *descr_p,
                                 const mpl_pack_options_t *options_p,
                                 mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_enum16(const char* value_str,
                                  void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_enum32(const char* value_str,
                                  void **value_pp,
                                  const mpl_param_descr2_t *descr_p,
                                  const mpl_pack_options_t *options_p,
                                  mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_signed_enum8(const char* value_str,
                                        void **value_pp,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p,
                                        mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_signed_enum16(const char* value_str,
                                         void **value_pp,
                                         const mpl_param_descr2_t *descr_p,
                                         const mpl_pack_options_t *options_p,
                                         mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_signed_enum32(const char* value_str,
                                         void **value_pp,
                                         const mpl_param_descr2_t *descr_p,
                                         const mpl_pack_options_t *options_p,
                                         mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_bool(const char* value_str,
                                void **value_pp,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p,
                                mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_bool8(const char* value_str,
                                 void **value_pp,
                                 const mpl_param_descr2_t *descr_p,
                                 const mpl_pack_options_t *options_p,
                                 mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_uint8_array(const char* value_str,
                                       void **value_pp,
                                       const mpl_param_descr2_t *descr_p,
                                       const mpl_pack_options_t *options_p,
                                       mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_uint16_array(const char* value_str,
                                        void **value_pp,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p,
                                        mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_uint32_array(const char* value_str,
                                        void **value_pp,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p,
                                        mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_string_tuple(const char* value_str,
                                        void **value_pp,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p,
                                        mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_int_tuple(const char* value_str,
                                     void **value_pp,
                                     const mpl_param_descr2_t *descr_p,
                                     const mpl_pack_options_t *options_p,
                                     mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_strint_tuple(const char* value_str,
                                        void **value_pp,
                                        const mpl_param_descr2_t *descr_p,
                                        const mpl_pack_options_t *options_p,
                                        mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_struint8_tuple(const char* value_str,
                                          void **value_pp,
                                          const mpl_param_descr2_t *descr_p,
                                          const mpl_pack_options_t *options_p,
                                          mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_bag(const char* value_str,
                               void **value_pp,
                               const mpl_param_descr2_t *descr_p,
                               const mpl_pack_options_t *options_p,
                               mpl_param_element_id_t unpack_context);
int mpl_unpack_param_value_addr(const char* value_str,
                                void **value_pp,
                                const mpl_param_descr2_t *descr_p,
                                const mpl_pack_options_t *options_p,
                                mpl_param_element_id_t unpack_context);

/**
 * @ingroup MPL_PARAM
 * mpl_clone_param_value_*
 *
 * Clone a parameter of a specific type
 *
 * See mpl_clone_param_fp for more details.
 *
 */
int mpl_clone_param_value_string(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_wstring(void **new_value_pp,
                                  const void* old_value_p,
                                  const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_int(void **new_value_pp,
                              const void* old_value_p,
                              const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_sint8(void **new_value_pp,
                                const void* old_value_p,
                                const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_sint16(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_sint32(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_sint64(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_uint8(void **new_value_pp,
                                const void* old_value_p,
                                const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_uint16(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_uint32(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_uint64(void **new_value_pp,
                                 const void* old_value_p,
                                 const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_enum(void **new_value_pp,
                               const void* old_value_p,
                               const mpl_param_descr2_t *descr_p);
#define mpl_clone_param_value_enum8 mpl_clone_param_value_uint8
#define mpl_clone_param_value_enum16 mpl_clone_param_value_uint16
#define mpl_clone_param_value_enum32 mpl_clone_param_value_uint32
#define mpl_clone_param_value_signed_enum8 mpl_clone_param_value_sint8
#define mpl_clone_param_value_signed_enum16 mpl_clone_param_value_sint16
#define mpl_clone_param_value_signed_enum32 mpl_clone_param_value_sint32
int mpl_clone_param_value_bool(void **new_value_pp,
                               const void* old_value_p,
                               const mpl_param_descr2_t *descr_p);
#define mpl_clone_param_value_bool8 mpl_clone_param_value_uint8
int mpl_clone_param_value_uint8_array(void **new_value_pp,
                                      const void* old_value_p,
                                      const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_uint16_array(void **new_value_pp,
                                       const void* old_value_p,
                                       const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_uint32_array(void **new_value_pp,
                                       const void* old_value_p,
                                       const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_string_tuple(void **new_value_pp,
                                       const void* old_value_p,
                                       const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_int_tuple(void **new_value_pp,
                                    const void* old_value_p,
                                    const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_strint_tuple(void **new_value_pp,
                                       const void* old_value_p,
                                       const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_struint8_tuple(void **new_value_pp,
                                         const void* old_value_p,
                                         const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_bag(void **new_value_pp,
                              const void* old_value_p,
                              const mpl_param_descr2_t *descr_p);
int mpl_clone_param_value_addr(void **new_value_pp,
                               const void* old_value_p,
                               const mpl_param_descr2_t *descr_p);

/**
 * @ingroup MPL_PARAM
 * mpl_copy_param_value_*
 *
 * Copy a parameter of a specific type
 *
 * See mpl_copy_param_fp for more details.
 *
 */
int mpl_copy_param_value_string(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_wstring(void *to_value_p,
                                 const void* from_value_p,
                                 int size,
                                 const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_int(void *to_value_p,
                             const void* from_value_p,
                             int size,
                             const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_sint8(void *to_value_p,
                               const void* from_value_p,
                               int size,
                               const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_sint16(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_sint32(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_sint64(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_uint8(void *to_value_p,
                               const void* from_value_p,
                               int size,
                               const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_uint16(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_uint32(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_uint64(void *to_value_p,
                                const void* from_value_p,
                                int size,
                                const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_enum(void *to_value_p,
                              const void* from_value_p,
                              int size,
                              const mpl_param_descr2_t *descr_p);
#define mpl_copy_param_value_enum8 mpl_copy_param_value_uint8
#define mpl_copy_param_value_enum16 mpl_copy_param_value_uint16
#define mpl_copy_param_value_enum32 mpl_copy_param_value_uint32
#define mpl_copy_param_value_signed_enum8 mpl_copy_param_value_sint8
#define mpl_copy_param_value_signed_enum16 mpl_copy_param_value_sint16
#define mpl_copy_param_value_signed_enum32 mpl_copy_param_value_sint32
int mpl_copy_param_value_bool(void *to_value_p,
                              const void* from_value_p,
                              int size,
                              const mpl_param_descr2_t *descr_p);
#define mpl_copy_param_value_bool8 mpl_copy_param_value_uint8
int mpl_copy_param_value_uint8_array(void *to_value_p,
                                     const void* from_value_p,
                                     int size,
                                     const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_uint16_array(void *to_value_p,
                                      const void* from_value_p,
                                      int size,
                                      const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_uint32_array(void *to_value_p,
                                      const void* from_value_p,
                                      int size,
                                      const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_string_tuple(void *to_value_p,
                                      const void* from_value_p,
                                      int size,
                                      const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_int_tuple(void *to_value_p,
                                   const void* from_value_p,
                                   int size,
                                   const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_strint_tuple(void *to_value_p,
                                      const void* from_value_p,
                                      int size,
                                      const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_struint8_tuple(void *to_value_p,
                                        const void* from_value_p,
                                        int size,
                                        const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_bag(void *to_value_p,
                             const void* from_value_p,
                             int size,
                             const mpl_param_descr2_t *descr_p);
int mpl_copy_param_value_addr(void *to_value_p,
                              const void* from_value_p,
                              int size,
                              const mpl_param_descr2_t *descr_p);

/**
 * @ingroup MPL_PARAM
 * mpl_compare_param_value_*
 *
 * Compare parameters of a specific type
 *
 * See mpl_compare_param_fp for more details.
 *
 */
int mpl_compare_param_value_string(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_wstring(const void *value1_p,
                                    const void* value2_p,
                                    const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_int(const void *value1_p,
                                const void* value2_p,
                                const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_sint8(const void *value1_p,
                                  const void* value2_p,
                                  const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_sint16(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_sint32(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_sint64(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_uint8(const void *value1_p,
                                  const void* value2_p,
                                  const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_uint16(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_uint32(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_uint64(const void *value1_p,
                                   const void* value2_p,
                                   const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_enum(const void *value1_p,
                                 const void* value2_p,
                                 const mpl_param_descr2_t *descr_p);
#define mpl_compare_param_value_enum8 mpl_compare_param_value_uint8
#define mpl_compare_param_value_enum16 mpl_compare_param_value_uint16
#define mpl_compare_param_value_enum32 mpl_compare_param_value_uint32
#define mpl_compare_param_value_signed_enum8 mpl_compare_param_value_sint8
#define mpl_compare_param_value_signed_enum16 mpl_compare_param_value_sint16
#define mpl_compare_param_value_signed_enum32 mpl_compare_param_value_sint32
int mpl_compare_param_value_bool(const void *value1_p, const void* value2_p,
                                 const mpl_param_descr2_t *descr_p);
#define mpl_compare_param_value_bool8 mpl_compare_param_value_uint8
int mpl_compare_param_value_uint8_array(const void *value1_p,
                                        const void* value2_p,
                                        const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_uint16_array(const void *value1_p,
                                         const void* value2_p,
                                         const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_uint32_array(const void *value1_p,
                                         const void* value2_p,
                                         const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_string_tuple(const void *value1_p,
                                         const void* value2_p,
                                         const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_int_tuple(const void *value1_p,
                                      const void* value2_p,
                                      const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_strint_tuple(const void *value1_p,
                                         const void* value2_p,
                                         const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_struint8_tuple(const void *value1_p,
                                           const void* value2_p,
                                           const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_bag(const void *value1_p,
                                const void* value2_p,
                                const mpl_param_descr2_t *descr_p);
int mpl_compare_param_value_addr(const void *value1_p,
                                 const void* value2_p,
                                 const mpl_param_descr2_t *descr_p);

/**
 * @ingroup MPL_PARAM
 * mpl_sizeof_param_value_*
 *
 * Size of parameters of a specific type
 *
 * See mpl_sizeof_param_fp for more details.
 *
 */
size_t mpl_sizeof_param_value_string(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_wstring(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_int(const mpl_param_descr2_t *descr_p);
#define mpl_sizeof_param_value_sint8 mpl_sizeof_param_value_uint8
#define mpl_sizeof_param_value_sint16 mpl_sizeof_param_value_uint16
#define mpl_sizeof_param_value_sint32 mpl_sizeof_param_value_uint32
size_t mpl_sizeof_param_value_sint64(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_uint8(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_uint16(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_uint32(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_uint64(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_enum(const mpl_param_descr2_t *descr_p);
#define mpl_sizeof_param_value_enum8 mpl_sizeof_param_value_uint8
#define mpl_sizeof_param_value_enum16 mpl_sizeof_param_value_uint16
#define mpl_sizeof_param_value_enum32 mpl_sizeof_param_value_uint32
#define mpl_sizeof_param_value_signed_enum8 mpl_sizeof_param_value_sint8
#define mpl_sizeof_param_value_signed_enum16 mpl_sizeof_param_value_sint16
#define mpl_sizeof_param_value_signed_enum32 mpl_sizeof_param_value_sint32
size_t mpl_sizeof_param_value_bool(const mpl_param_descr2_t *descr_p);
#define mpl_sizeof_param_value_bool8 mpl_sizeof_param_value_uint8
size_t mpl_sizeof_param_value_uint8_array(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_uint16_array(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_uint32_array(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_string_tuple(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_int_tuple(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_strint_tuple(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_struint8_tuple(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_bag(const mpl_param_descr2_t *descr_p);
size_t mpl_sizeof_param_value_addr(const mpl_param_descr2_t *descr_p);

/**
 * @ingroup MPL_PARAM
 * mpl_free_param_value_*
 *
 * Free parameters of a specific type
 *
 * See mpl_free_param_fp for more details.
 *
 */
#define mpl_free_param_value_string free
#define mpl_free_param_value_wstring free
#define mpl_free_param_value_int free
#define mpl_free_param_value_sint8 free
#define mpl_free_param_value_sint16 free
#define mpl_free_param_value_sint32 free
#define mpl_free_param_value_sint64 free
#define mpl_free_param_value_uint8 free
#define mpl_free_param_value_uint16 free
#define mpl_free_param_value_uint32 free
#define mpl_free_param_value_uint64 free
#define mpl_free_param_value_enum free
#define mpl_free_param_value_enum8 free
#define mpl_free_param_value_enum16 free
#define mpl_free_param_value_enum32 free
#define mpl_free_param_value_signed_enum8 free
#define mpl_free_param_value_signed_enum16 free
#define mpl_free_param_value_signed_enum32 free
#define mpl_free_param_value_bool free
#define mpl_free_param_value_bool8 free
void mpl_free_param_value_uint8_array(void *value_p);
void mpl_free_param_value_uint16_array(void *value_p);
void mpl_free_param_value_uint32_array(void *value_p);
void mpl_free_param_value_string_tuple(void *value_p);
#define mpl_free_param_value_int_tuple free
void mpl_free_param_value_strint_tuple(void *value_p);
void mpl_free_param_value_struint8_tuple(void *value_p);
void mpl_free_param_value_bag(void *value_p);
#define mpl_free_param_value_addr free

#endif /* ! DOXYGEN */


/**
 * @ingroup MPL_PARAM
 * mpl_get_args
 *
 * Split argument-buffer into array of key/value pairs
 *
 * @param    args         Array of key/value pairs
 * @param    args_len     Length of array
 * @param    buf          Null-terminated buffer containing the original arguments
 *                   Note that the buffer is changed.
 * @param    equal        Character used as "equal sign" between key and value
 *                   (argument format could be key=value)
 * @param    delimiter    Character used as delimiter between arguments
 * @param    escape       Character used as escape
 *
 * @return Number of key/value pairs found or -1 on error.
 *
 */
int mpl_get_args(mpl_arg_t *args,
                 int args_len,
                 char *buf,
                 char equal,
                 char delimiter,
                 char escape);


/**
 * @ingroup MPL_PARAM
 * mpl_add_param_to_list - add parameter to param list
 *
 * @param param_list_pp List to add to
 * @param param_id Parameter id
 * @param value_p Pointer to value
 *
 * @return 0 on success
 *
 */
#define mpl_add_param_to_list(param_list_pp, param_id, value_p)     \
    mpl_add_param_to_list_tag(param_list_pp, param_id, 0, value_p)

/**
 * @ingroup MPL_PARAM
 * mpl_add_param_to_list_tag - add parameter with a given tag to param list
 *
 * @param param_list_pp List to add to
 * @param param_id Parameter id
 * @param tag The tag to give to the parameter
 * @param value_p Pointer to value
 *
 * @return 0 on success
 */
int mpl_add_param_to_list_tag(mpl_list_t **param_list_pp,
                              mpl_param_element_id_t param_id,
                              int tag,
                              const void *value_p);

/**
 * @ingroup MPL_PARAM
 * mpl_add_param_to_list_n - add parameter to param list with value size check
 *
 * @param param_list_pp List to add to
 * @param param_id Parameter id
 * @param value_p Pointer to value
 * @param len size of value
 *
 * @return 0 on success
 */
#define mpl_add_param_to_list_n(param_list_pp, param_id, value_p, len)  \
    mpl_add_param_to_list_n_tag(param_list_pp, param_id, 0, value_p, len)

/**
 * @ingroup MPL_PARAM
 * mpl_add_param_to_list_n_tag - add parameter with a given tag to param list
 *                               with value size check
 *
 * @param param_list_pp List to add to
 * @param param_id Parameter id
 * @param tag The tag to give to the parameter
 * @param value_p Pointer to value
 * @param len size of value
 *
 * @return 0 on success
 */
int mpl_add_param_to_list_n_tag(mpl_list_t **param_list_pp,
                                mpl_param_element_id_t param_id,
                                int tag,
                                const void *value_p,
                                size_t len);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_add_* - add parameter of given type (*) to param list
 *
 * @param param_list_pp List to add to
 * @param param_id Parameter id
 * @param value Value (must be of the given type)
 * @return 0 on success
 */
#define mpl_param_list_add_int(param_list_pp, param_id, value)    \
    mpl_param_list_add_int_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_uint8(param_list_pp, param_id, value)    \
    mpl_param_list_add_uint8_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_uint16(param_list_pp, param_id, value)     \
    mpl_param_list_add_uint16_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_uint32(param_list_pp, param_id, value)     \
    mpl_param_list_add_uint32_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_uint64(param_list_pp, param_id, value)     \
    mpl_param_list_add_uint64_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_sint8(param_list_pp, param_id, value)    \
    mpl_param_list_add_sint8_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_sint16(param_list_pp, param_id, value)     \
    mpl_param_list_add_sint16_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_sint32(param_list_pp, param_id, value)     \
    mpl_param_list_add_sint32_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_sint64(param_list_pp, param_id, value)     \
    mpl_param_list_add_sint64_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_bool8(param_list_pp, param_id, value)    \
    mpl_param_list_add_bool8_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_bool(param_list_pp, param_id, value)     \
    mpl_param_list_add_bool_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_enum(param_list_pp, param_id, value)         \
    mpl_param_list_add_enum_tag(param_list_pp, param_id, 0, (int64_t)value)
#define mpl_param_list_add_enum8(param_list_pp, param_id, value)    \
    mpl_param_list_add_enum8_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_enum16(param_list_pp, param_id, value)     \
    mpl_param_list_add_enum16_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_enum32(param_list_pp, param_id, value)     \
    mpl_param_list_add_enum32_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_signed_enum8(param_list_pp, param_id, value) \
    mpl_param_list_add_signed_enum8_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_signed_enum16(param_list_pp, param_id, value) \
    mpl_param_list_add_signed_enum16_tag(param_list_pp, param_id, 0, value)
#define mpl_param_list_add_signed_enum32(param_list_pp, param_id, value) \
    mpl_param_list_add_signed_enum32_tag(param_list_pp, param_id, 0, value)

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_add_*_tag - add parameter of given type (*)
 * and tag to param list
 *
 * @param param_list_pp List to add to
 * @param param_id Parameter id
 * @param tag The tag to give to the parameter
 * @param value Value (must be of the given type)
 * @return 0 on success
 */
int mpl_param_list_add_int_tag(mpl_list_t **param_list_pp,
                               mpl_param_element_id_t param_id,
                               int tag,
                               int value);
int mpl_param_list_add_uint8_tag(mpl_list_t **param_list_pp,
                                 mpl_param_element_id_t param_id,
                                 int tag,
                                 uint8_t value);
int mpl_param_list_add_uint16_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  uint16_t value);
int mpl_param_list_add_uint32_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  uint32_t value);
int mpl_param_list_add_uint64_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  uint64_t value);
int mpl_param_list_add_sint8_tag(mpl_list_t **param_list_pp,
                                 mpl_param_element_id_t param_id,
                                 int tag,
                                 sint8_t value);
int mpl_param_list_add_sint16_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  sint16_t value);
int mpl_param_list_add_sint32_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  sint32_t value);
int mpl_param_list_add_sint64_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  int64_t value);
int mpl_param_list_add_bool8_tag(mpl_list_t **param_list_pp,
                                 mpl_param_element_id_t param_id,
                                 int tag,
                                 uint8_t value);
int mpl_param_list_add_bool_tag(mpl_list_t **param_list_pp,
                                mpl_param_element_id_t param_id,
                                int tag,
                                bool value);
int mpl_param_list_add_enum_tag(mpl_list_t **param_list_pp,
                                mpl_param_element_id_t param_id,
                                int tag,
                                int64_t value);
int mpl_param_list_add_enum8_tag(mpl_list_t **param_list_pp,
                                 mpl_param_element_id_t param_id,
                                 int tag,
                                 uint8_t value);
int mpl_param_list_add_enum16_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  uint16_t value);
int mpl_param_list_add_enum32_tag(mpl_list_t **param_list_pp,
                                  mpl_param_element_id_t param_id,
                                  int tag,
                                  uint32_t value);
int mpl_param_list_add_signed_enum8_tag(mpl_list_t **param_list_pp,
                                        mpl_param_element_id_t param_id,
                                        int tag,
                                        sint8_t value);
int mpl_param_list_add_signed_enum16_tag(mpl_list_t **param_list_pp,
                                         mpl_param_element_id_t param_id,
                                         int tag,
                                         sint16_t value);
int mpl_param_list_add_signed_enum32_tag(mpl_list_t **param_list_pp,
                                         mpl_param_element_id_t param_id,
                                         int tag,
                                         sint32_t value);

/**
 * @ingroup MPL_PARAM
 * mpl_param_list_get_int - Get int value of a parameter on a list
 *
 * @param paramid Parameter id
 * @param param_list_p List to extract from
 * @return The integer value
 * @note The parameter must be present on the list (or a crash
 * may occur)
 *
 */

#define mpl_param_list_get_int(paramid,param_list_p)        \
    MPL_GET_PARAM_VALUE_FROM_LIST(int,paramid,param_list_p)

bool _mpl_is_enum_type(mpl_param_element_id_t param_id);
bool _mpl_equal_typesize(size_t typesize, mpl_param_element_id_t param_id);
bool _mpl_assert_typesize(size_t typesize, mpl_param_element_id_t param_id);

/**
 * @ingroup MPL_PARAM
 * MPL_PARAM_ELEMENT_IS_FIELD - Check if a parameter element is a field
 *
 * @param param_element_p The parameter element pointer
 * @return true if field
 *
 */
#define MPL_PARAM_ELEMENT_IS_FIELD(param_element_p)       \
    (param_element_p->context != MPL_PARAM_ID_UNDEFINED)

/**
 * @ingroup MPL_PARAM
 * MPL_FIELD_PRESENT_IN_LIST - Check if a field is on a list
 *
 * @param context param id of context
 * @param id_in_context identifier in the context
 * @param list List to extract from
 * @return true if on the list
 *
 */
#define MPL_FIELD_PRESENT_IN_LIST(context, id_in_context, list)     \
    (mpl_param_list_find_field(context,                             \
                               id_in_context,                       \
                               list) != NULL)

/**
 * @ingroup MPL_PARAM
 * MPL_FIELD_PRESENT_IN_LIST_TAG - Check if a field is on a list
 *
 * @param context param id of context
 * @param id_in_context identifier in the context
 * @param tag tag to look for
 * @param list List to extract from
 * @return true if on the list
 *
 */
#define MPL_FIELD_PRESENT_IN_LIST_TAG(context, id_in_context, tag, list) \
    (mpl_param_list_find_field_tag(context,                             \
                                   id_in_context,                       \
                                   tag,                                 \
                                   list) != NULL)


/**
 * @ingroup MPL_PARAM
 * MPL_PARAM_ELEMENT_SET_FIELD_INFO - Set context and field info in element
 *
 * @param elem_p Parameter element pointer
 * @param ctx param id of context
 * @param id_in_ctx identifier in the context
 *
 */
#define MPL_PARAM_ELEMENT_SET_FIELD_INFO(elem_p,ctx,id_in_ctx)          \
    do {                                                                \
        elem_p->context = ctx ?                                         \
                          mpl_param_get_bag_field_context(ctx, id_in_ctx) : \
                          MPL_PARAM_ID_UNDEFINED;                       \
        elem_p->id_in_context = ctx ? id_in_ctx : 0;                    \
    } while(0)

/**
 * @ingroup MPL_PARAM
 * MPL_PARAM_ELEMENT_CLEAR_FIELD_INFO - Clear context and field info from element
 *
 * @param elem_p Parameter element pointer
 *
 */
#define MPL_PARAM_ELEMENT_CLEAR_FIELD_INFO(elem_p)                      \
    do {                                                                \
        elem_p->context = MPL_PARAM_ID_UNDEFINED;                       \
        elem_p->id_in_context = 0;                                      \
    } while(0)

/**
 * @ingroup MPL_PARAM
 * MPL_PARAM_PRESENT_IN_LIST - Check if a parameter is on a list
 *
 * @param paramid Parameter id
 * @param param_list_p List to extract from
 * @return true if on the list
 *
 */
#define MPL_PARAM_PRESENT_IN_LIST(paramid,param_list_p)       \
    (NULL != mpl_param_list_find((paramid), (param_list_p)))

/**
 * @ingroup MPL_PARAM
 * MPL_PARAM_PRESENT_IN_LIST_TAG - Check if a parameter with a given tag
 * is on a list
 *
 * @param paramid Parameter id
 * @param tag The tag that must match
 * @param param_list_p List to extract from
 * @return true if on the list
 *
 */
#define MPL_PARAM_PRESENT_IN_LIST_TAG(paramid,tag,param_list_p)         \
    (NULL != mpl_param_list_find_tag((paramid), (tag), (param_list_p)))

/**
 * @ingroup MPL_PARAM
 * MPL_PARAM_VALUE_PRESENT_IN_LIST - Check if a parameter with that
 * is on a list has a value
 *
 * @param paramid Parameter id
 * @param param_list_p List to extract from
 * @return true if the parameter has a value
 * @note The parameter must be present on the list (or a crash
 * may occur)
 *
 */
#define MPL_PARAM_VALUE_PRESENT_IN_LIST(paramid,param_list_p)           \
    (NULL != MPL_GET_PARAM_VALUE_PTR_FROM_LIST(void*,paramid,param_list_p))

/**
 * @ingroup MPL_PARAM
 * MPL_GET_VALUE_FROM_PARAM_ELEMENT - Get the value from a parameter element
 *
 * @param type The C type to cast to
 * @param param_element_p The parameter element
 * @return The value
 * @note The value must exist (not be NULL) (or a crash
 * may occur)
 *
 */
#define MPL_GET_VALUE_FROM_PARAM_ELEMENT(type,param_element_p) \
    (_mpl_assert_typesize(sizeof(type), param_element_p->id) ? \
     *((type*)(param_element_p)->value_p) :                    \
     *((type*)(param_element_p)->value_p))

/**
 * @ingroup MPL_PARAM
 * MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT - Get pointer to the value from
 * a parameter element
 *
 * @param reftype The C type (must be pointer) to cast to
 * @param param_element_p The parameter element
 * @return The pointer value
 *
 */
#define MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(reftype,param_element_p) \
    ((reftype)(param_element_p)->value_p)
#ifndef DOXYGEN
/* backwards compatibility */
#define MPL_GET_VALUE_REF_FROM_PARAM_ELEMENT    \
    MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT
#endif
/**
 * @ingroup MPL_PARAM
 * MPL_GET_PARAM_VALUE_FROM_LIST - Get the value from a parameter
 * present on a parameter list
 *
 * @param type The C type to cast to
 * @param paramid Parameter id
 * @param param_list_p The parameter list
 * @return The value
 * @note The parameter must exist on the list (or a crash
 * may occur)
 *
 */
#define MPL_GET_PARAM_VALUE_FROM_LIST(type,paramid,param_list_p) /*lint -save -e666 */ \
    MPL_GET_VALUE_FROM_PARAM_ELEMENT(type,                              \
                                     mpl_param_list_find((paramid),     \
                                                         (param_list_p)))/*lint -restore */
/**
 * @ingroup MPL_PARAM
 * MPL_GET_PARAM_VALUE_FROM_LIST_TAG - Get the value from a parameter with a
 * given tag present on a parameter list
 *
 * @param type The C type to cast to
 * @param paramid Parameter id
 * @param tag The tag
 * @param param_list_p The parameter list
 * @return The value
 * @note The parameter must exist on the list (or a crash
 * may occur)
 *
 */
#define MPL_GET_PARAM_VALUE_FROM_LIST_TAG(type,paramid,tag,param_list_p) /*lint -save -e666 */ \
    MPL_GET_VALUE_FROM_PARAM_ELEMENT(type,                              \
                                     mpl_param_list_find_tag((paramid), \
                                                             (tag),     \
                                                             (param_list_p)))/*lint -restore */

/**
 * @ingroup MPL_PARAM
 * MPL_GET_PARAM_VALUE_PTR_FROM_LIST - Get pointer to the value from a
 * parameter present on a parameter list
 *
 * @param reftype The C type to cast to
 * @param paramid Parameter id
 * @param param_list_p The parameter list
 * @return The pointer value
 * @note The parameter must exist on the list (or a crash
 * may occur)
 *
 */
#define MPL_GET_PARAM_VALUE_PTR_FROM_LIST(reftype,paramid,param_list_p) \
    MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(reftype,                       \
                                         mpl_param_list_find((paramid), \
                                                             (param_list_p)))

/**
 * @ingroup MPL_PARAM
 * MPL_GET_PARAM_VALUE_PTR_FROM_LIST_TAG - Get pointer to the value from a
 * parameter with a given tag present on a parameter list
 *
 * @param reftype The C type to cast to
 * @param paramid Parameter id
 * @param tag The tag
 * @param param_list_p The parameter list
 * @return The pointer value
 * @note The parameter must exist on the list (or a crash
 * may occur)
 *
 */
#define MPL_GET_PARAM_VALUE_PTR_FROM_LIST_TAG(reftype,                  \
                                              paramid,                  \
                                              tag,                      \
                                              param_list_p)             \
    MPL_GET_VALUE_PTR_FROM_PARAM_ELEMENT(reftype,                       \
                                         mpl_param_list_find_tag((paramid), \
                                                                 (tag), \
                                                                 (param_list_p)))
#ifndef DOXYGEN
/* backwards compatibility */
#define MPL_GET_PARAM_VALUE_REF_FROM_LIST       \
    MPL_GET_PARAM_VALUE_PTR_FROM_LIST
#define MPL_GET_PARAM_VALUE_REF_FROM_LIST_TAG   \
    MPL_GET_PARAM_VALUE_PTR_FROM_LIST_TAG
#endif

/**
 * @ingroup MPL_PARAM
 * MPL_PARAM_ELEMENT_CLONE_TO_NEW_PARAMID - Return a copy of a parameter
 * element and give it a new parameter id
 *
 * @param param_element_p Parameter element (source)
 * @param new_paramid The new parameter id
 * @return The new parameter element
 *
 */
#define MPL_PARAM_ELEMENT_CLONE_TO_NEW_PARAMID(param_element_p, new_paramid) \
    mpl_param_element_create_tag((new_paramid),                         \
                                 mpl_param_element_get_tag(param_element_p), \
                                 (param_element_p)->value_p)

/**
 * @ingroup MPL_PARAM
 * MPL_DESTROY_PARAM_FROM_LIST - Remove first occurrence of parameter
 * from a list and deallocate it
 *
 * @param paramid The new parameter id
 * @param param_list_p The list
 *
 */
#define MPL_DESTROY_PARAM_FROM_LIST(paramid,param_list_p)               \
    do                                                                  \
    {                                                                   \
        mpl_param_element_t *tmp_elem_p;                                \
        tmp_elem_p = mpl_param_list_find((paramid), (param_list_p));    \
        if (tmp_elem_p != NULL)                                         \
        {                                                               \
            (void)mpl_list_remove(&(param_list_p), &tmp_elem_p->list_entry); \
            mpl_param_element_destroy(tmp_elem_p);                      \
        }                                                               \
    } while (0)

/**
 * @ingroup MPL_PARAM
 * MPL_DESTROY_PARAM_FROM_LIST_TAG - Remove first occurrence of parameter
 * from a list and deallocate it
 *
 * @param paramid The new parameter id
 * @param tag tag to look for
 * @param param_list_p The list
 *
 */
#define MPL_DESTROY_PARAM_FROM_LIST_TAG(paramid,tag,param_list_p)       \
    do                                                                  \
    {                                                                   \
        mpl_param_element_t *tmp_elem_p;                                \
        tmp_elem_p = mpl_param_list_find_tag((paramid), (tag), (param_list_p)); \
        if (tmp_elem_p != NULL)                                         \
        {                                                               \
            (void)mpl_list_remove(&(param_list_p), &tmp_elem_p->list_entry); \
            mpl_param_element_destroy(tmp_elem_p);                      \
        }                                                               \
    } while (0)

/**
 * @ingroup MPL_PARAM
 * MPL_DESTROY_FIELD_FROM_LIST - Remove field parameter
 * from a list and deallocate it
 *
 * @param context param id of context
 * @param id_in_context identifier in the context
 * @param param_list_p The list
 *
 */
#define MPL_DESTROY_FIELD_FROM_LIST(context, id_in_context, param_list_p) \
    do                                                                  \
    {                                                                   \
        mpl_param_element_t *tmp_elem_p;                                \
        tmp_elem_p = mpl_param_list_find_field((context),               \
                                               (id_in_context),         \
                                               (param_list_p));         \
        if (tmp_elem_p != NULL)                                         \
        {                                                               \
            (void)mpl_list_remove(&(param_list_p), &tmp_elem_p->list_entry); \
            mpl_param_element_destroy(tmp_elem_p);                      \
        }                                                               \
    } while (0)

/**
 * @ingroup MPL_PARAM
 * MPL_DESTROY_FIELD_FROM_LIST_TAG - Remove field parameter
 * from a list and deallocate it
 *
 * @param context param id of context
 * @param id_in_context identifier in the context
 * @param tag tag to look for
 * @param param_list_p The list
 *
 */
#define MPL_DESTROY_FIELD_FROM_LIST_TAG(context, id_in_context, tag, param_list_p) \
    do                                                                  \
    {                                                                   \
        mpl_param_element_t *tmp_elem_p;                                \
        tmp_elem_p = mpl_param_list_find_field_tag((context),           \
                                                   (id_in_context),     \
                                                   (tag),               \
                                                   (param_list_p));     \
        if (tmp_elem_p != NULL)                                         \
        {                                                               \
            (void)mpl_list_remove(&(param_list_p), &tmp_elem_p->list_entry); \
            mpl_param_element_destroy(tmp_elem_p);                      \
        }                                                               \
    } while (0)

/**
 * @ingroup MPL_PARAM
 * MPL_DESTROY_PARAM_FROM_LIST_TAG - Remove first occurrence of parameter
 * with a given tag from a list and deallocate it
 *
 * @param paramid The new parameter id
 * @param tag The tag
 * @param param_list_p The list
 *
 */
#define MPL_DESTROY_PARAM_FROM_LIST_TAG(paramid,tag,param_list_p)       \
    do                                                                  \
    {                                                                   \
        mpl_param_element_t *tmp_elem_p;                                \
        tmp_elem_p = mpl_param_list_find_tag((paramid), (tag), (param_list_p)); \
        if (tmp_elem_p != NULL)                                         \
        {                                                               \
            (void)mpl_list_remove(&(param_list_p), &tmp_elem_p->list_entry); \
            mpl_param_element_destroy(tmp_elem_p);                      \
        }                                                               \
    } while (0)

/**
 * @ingroup MPL_PARAM
 * mpl_add_to_blacklist - add parameter id to blacklist
 *
 * @param blacklist_p The blacklist
 * @param param_id Parameter id
 * @return 0 on success
 *
 */
#define mpl_add_to_blacklist(blacklist_p, param_id)     \
    mpl_add_param_to_list(blacklist_p, param_id, NULL)

/**
 * @ingroup MPL_PARAM
 * mpl_remove_from_blacklist - remove parameter id from blacklist
 *
 * @param blacklist_p The blacklist
 * @param param_id Parameter id
 * @return 0 on success
 */
int mpl_remove_from_blacklist(mpl_blacklist_t *blacklist_p,
                              mpl_param_element_id_t param_id);

/**
 * @ingroup MPL_PARAM
 * mpl_trimstring
 *
 * Takes away whitespace in front and rear, unless last
 *              whitespace is escaped.
 *
 * @param str The string
 * @param escape The escape character
 *
 * @return Pointer to where non-whitespace string begins.
 */
char *mpl_trimstring(char *str, char escape);

/**
 * @ingroup MPL_PARAM
 * mpl_compare_param_lists
 *
 * Compare lists (including values)
 *
 * Returns  0 if equal
 */
int mpl_compare_param_lists(mpl_list_t *list1_p, mpl_list_t *list2_p);

/**
 * @ingroup MPL_PARAM
 * mpl_convert_int
 *
 * Parse a string and convert to integer
 *
 * @param value_str The source string
 * @param value_p Pointer to returned value (int)
 * @return 0 on success, -1 on error
 */
int mpl_convert_int(const char* value_str, int *value_p);

/**
 * @ingroup MPL_PARAM
 * ARRAY_SIZE
 *
 * Calculate the number of elements in an array
 *
 * @param    arr      The array
 *
 * @return Number of elements
 *
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


/**
 * @ingroup MPL_PARAM
 * MPL_IDENTIFIER_NOT_USED
 *
 * Report to compiler that an input parameter is not used by
 *              function
 *
 * @param P The parameter
 *
 * @return void
 *
 */
#define MPL_IDENTIFIER_NOT_USED(P) (void)(P);

/**
 * @ingroup MPL_PARAM
 * mpl_get_errno
 *
 * Retrieve error code of last error
 *
 * @return error code on success, -1 on error.
 */
int mpl_get_errno(void);

void mpl_set_errno(int error_value);

#if 1
void mpl_dbg_param_list_print(mpl_list_t *list_p);
#endif
#ifdef UNIT_TEST
void unittest_force_mpl_init(void);
void unittest_force_mpl_set(mpl_param_element_id_t param_id,
                            MPL_ErrorCode_t errno_code);
boolean unittest_force_mpl_errno_response(mpl_param_element_id_t param_id,
                                          MPL_ErrorCode_t *errno_code);
#endif

#ifdef  __cplusplus
}
#endif

#endif /* _MPL_PARAM_H */
