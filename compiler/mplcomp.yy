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
*   Author: Per Sigmond <per@sigmond.no>
*   Author: Harald Johansen <harald.johansen@stericsson.com>
*   Author: Emil B. Viken <emil.b.viken@stericsson.com>
*
*/
%{
#include <FlexLexer.h>
#include "mplcomp.hh"
#include "mplcomp.tab.hh"
extern "C" {
    #include "mpl_list.h"
    #include "mpl_param.h"
}

void yyerror(const char *error);
#define YYPARSE_PARAM compiler_p
#define COMPILER_P ((mpl_compiler*)compiler_p)
#define yylex COMPILER_P->lexer_p->yylex

//#define DBG printf
#define DBG

%}
%defines
%define api.pure
%union {
    char* name;
}

%token CATEGORY
%type <name> CATEGORY

%token USING
%type <name> USING

%token PARAMETER_SET
%type <name> PARAMETER_SET

%token PREFIX
%type <name> PREFIX

%token NUMERIC_ID
%type <name> NUMERIC_ID

%token RANGE_ID
%type <name> RANGE_ID

%token BAG_FIELD_TABLE_SUFFIX
%type <name> BAG_FIELD_TABLE_SUFFIX

%token COMMAND_BAG
%type <name> COMMAND_BAG

%token RESPONSE_BAG
%type <name> RESPONSE_BAG

%token EVENT_BAG
%type <name> EVENT_BAG

%token PARAMETERS
%type <name> PARAMETERS

%token COMMANDS
%type <name> COMMANDS

%token EVENTS
%type <name> EVENTS

%token ENUM
%type <name> ENUM

%token ENUM8
%type <name> ENUM8

%token ENUM16
%type <name> ENUM16

%token ENUM32
%type <name> ENUM32

%token SIGNED_ENUM8
%type <name> SIGNED_ENUM8

%token SIGNED_ENUM16
%type <name> SIGNED_ENUM16

%token SIGNED_ENUM32
%type <name> SIGNED_ENUM32

%token BAG
%type <name> BAG

%token INT
%type <name> INT

%token STRING
%type <name> STRING

%token WSTRING
%type <name> WSTRING

%token ADDR
%type <name> ADDR

%token BOOL
%type <name> BOOL

%token BOOL8
%type <name> BOOL8

%token SINT8
%type <name> SINT8

%token SINT16
%type <name> SINT16

%token SINT32
%type <name> SINT32

%token SINT64
%type <name> SINT64

%token UINT8
%type <name> UINT8

%token UINT16
%type <name> UINT16

%token UINT32
%type <name> UINT32

%token UINT64
%type <name> UINT64

%token UINT8_ARRAY
%type <name> UINT8_ARRAY

%token UINT16_ARRAY
%type <name> UINT16_ARRAY

%token UINT32_ARRAY
%type <name> UINT32_ARRAY

%token STRING_TUPLE
%type <name> STRING_TUPLE

%token INT_TUPLE
%type <name> INT_TUPLE

%token STRINT_TUPLE
%type <name> STRINT_TUPLE

%token STRUINT8_TUPLE
%type <name> STRUINT8_TUPLE

%token ENUMERATOR_LIST
%type <name> ENUMERATOR_LIST

%token NUMBER_RANGE
%type <name> NUMBER_RANGE

%token RANGE
%type <name> RANGE

%token DOTDOT
%type <name> DOTDOT

%token MIN
%type <name> MIN

%token MAX
%type <name> MAX

%token DEFAULT
%type <name> DEFAULT

%token SET
%type <name> SET

%token BOOLEAN
%type <name> BOOLEAN

%token GET
%type <name> GET

%token CONFIG
%type <name> CONFIG

%token VALUE
%type <name> VALUE

%token ELLIPSIS
%type <name> ELLIPSIS

%token VIRTUAL
%type <name> VIRTUAL

%token IN
%type <name> IN

%token NAME
%type <name> NAME

%token STRING_LITERAL
%type <name> STRING_LITERAL

%token OUT
%type <name> OUT

%token INOUT
%type <name> INOUT

%token FORWARD_DOC
%type <name> FORWARD_DOC

%token BACKWARD_DOC
%type <name> BACKWARD_DOC

%token H_HEADER
%type <name> H_HEADER

%token C_HEADER
%type <name> C_HEADER

%token SEGMENT
%type <name> SEGMENT

%token LITERAL_CODE_OPTION
%type <name> LITERAL_CODE_OPTION

%token HLINE
%type <name> HLINE

%token CLINE
%type <name> CLINE

%token H_END
%type <name> H_END

%token C_END
%type <name> C_END

%token OPTION
%type <name> OPTION

%token DOC_BEGIN
%type <name> DOC_BEGIN

%token DOC_END
%type <name> DOC_END

%token DOC_DEFGROUP
%type <name> DOC_DEFGROUP

%token DOC_INGROUP
%type <name> DOC_INGROUP

%token DOC_PARAM
%type <name> DOC_PARAM

%token DOC_SEE
%type <name> DOC_SEE

%token DOC_FILE
%type <name> DOC_FILE

%token DOC_BRIEF
%type <name> DOC_BRIEF

%token DOC_REF
%type <name> DOC_REF

%token DOC_LOCALTAG
%type <name> DOC_LOCALTAG

%token DOC_IDENTIFIER
%type <name> DOC_IDENTIFIER

%token DOC_TEXT
%type <name> DOC_TEXT

%%

declaration_list
    : declaration
    | declaration_list declaration
    | x_segment
    ;

declaration
    : category
    | parameters
    | parameter_set
    | literal_code
    | compiler_options
    | file_level_fdoc
    ;

file_level_fdoc
    : FORWARD_DOC { DBG("yacc: forward doc: \n'%s'\n", $1); COMPILER_P->create_doc($1, 1); free($1); }
    ;

fdoc
    :
    | FORWARD_DOC { DBG("yacc: forward doc: \n'%s'\n", $1); COMPILER_P->create_doc($1, 1); free($1); }
    ;

bdoc
    :
    | BACKWARD_DOC { DBG("yacc: backward doc: \n'%s'\n", $1); COMPILER_P->create_doc($1, 0); free($1); }
    ;

literal_code
    : literal_code_header literal_code_contents literal_code_end
    ;

literal_code_header
    : h_header
    | c_header
    ;

literal_code_contents
    : hlines
    | clines
    ;

literal_code_end
    : H_END { COMPILER_P->reset_literal_code_options(); }
    | C_END { COMPILER_P->reset_literal_code_options(); }
    ;


hlines
    : hline
    | hlines hline
    ;

clines
    : cline
    | clines cline
    ;

h_header
    : H_HEADER SEGMENT '=' LITERAL_CODE_OPTION { DBG("yacc: h_header: segment : code_option = %s\n", $4); COMPILER_P->set_literal_code_option("segment", $4); free($4); }
    | H_HEADER
    ;

hline
    : HLINE { DBG("yacc: hline: '%s'\n", $1); COMPILER_P->add_hline($1); free($1); }
    ;

c_header
    : C_HEADER SEGMENT '=' LITERAL_CODE_OPTION { DBG("yacc: c_header: segment : code_option = %s\n", $4); COMPILER_P->set_literal_code_option("segment", $4); free($4); }
    | C_HEADER
    ;

cline
    : CLINE { DBG("yacc: cline: '%s'\n", $1); COMPILER_P->add_cline($1); free($1); }
    ;

compiler_options
    : compiler_option_keyword compiler_option_block_type compiler_option_block_name compiler_option_list ';'
    ;

compiler_option_keyword
    : OPTION { COMPILER_P->create_compiler_option(); }
    ;

compiler_option_block_type
    : PARAMETER_SET { COMPILER_P->add_block_type_to_compiler_option("parameter_set"); }
    | CATEGORY { COMPILER_P->add_block_type_to_compiler_option("category"); }
    ;

compiler_option_block_name
    : NAME { COMPILER_P->add_block_name_to_compiler_option($1); free($1); }
    ;

compiler_option_list
    : compiler_option
    | compiler_option_list ',' compiler_option
    ;

compiler_option
    : NAME { COMPILER_P->add_value_to_compiler_option_and_create_new_if_not_first($1, NULL); free($1); }
    | NAME NAME { COMPILER_P->add_value_to_compiler_option_and_create_new_if_not_first($1, $2); free($1); free($2); }
    | NAME VALUE { COMPILER_P->add_value_to_compiler_option_and_create_new_if_not_first($1, $2); free($1); free($2); }
    ;

block_end
    : '}' ';' { COMPILER_P->do_block_end(); }
    ;

category
    : category_decl '{' category_property_list block_end
    ;

category_decl
    : CATEGORY category_type_name { COMPILER_P->do_block_start(block_type_category); }
    | CATEGORY category_type_name using_parameter_set { COMPILER_P->do_block_start(block_type_category); }
    ;

category_property_list
    : category_property
    | category_property_list category_property
    ;

category_property
    : fdoc commands
    | fdoc events
    | message_bags
    ;

using_parameter_set
    : USING NAME { COMPILER_P->add_parameter_set_to_current_category($2); COMPILER_P->set_current_parameter_group(COMPILER_P->get_current_category()->name_p); free($2); }
    ;

parameter_set
    : parameter_set_declaration '{' parameter_set_property_list block_end

parameter_set_declaration
    : PARAMETER_SET NAME { DBG("yacc: parameter_set: %s\n", $2); COMPILER_P->create_parameter_set($2); COMPILER_P->do_block_start(block_type_parameter_set); free($2); }
    ;

message_bags
    : fdoc message_bag_decl '{' param_list block_end { COMPILER_P->reset_current_parameter_list(); }
    | fdoc message_bag_decl '{' param_list ',' ellipsis block_end { COMPILER_P->reset_current_parameter_list(); }
    | fdoc message_bag_decl '{' ellipsis block_end { COMPILER_P->reset_current_parameter_list(); }
    | fdoc message_bag_decl '{' block_end { COMPILER_P->reset_current_parameter_list(); }
    ;

commands
    : commands_in_category '{' command_list block_end
    ;

commands_in_category
    : COMMANDS { COMPILER_P->do_block_start(block_type_commands); }


events
    : events_in_category '{' event_list block_end
    ;

events_in_category
    : EVENTS { COMPILER_P->do_block_start(block_type_events); }

enum
    : enum_decl parameter_options enum_inherit '{' enum_list block_end
    | enum_decl parameter_options enum_inherit_no_block
    | enum_decl parameter_options enum_inherit enum_enumerator_list_spec
    | enum_decl parameter_options enum_inherit '{' enum_list '}' '+' enum_enumerator_list_spec
    | enum_decl parameter_options enum_inherit enum_enumerator_list_concatination '+' '{' enum_list block_end
    | enum_decl parameter_options enum_inherit enum_enumerator_list_concatination '+' '{' enum_list '}' '+' enum_enumerator_list_spec
    ;

enum_decl
    : ENUM NAME { DBG("yacc: enum %s\n", $2); COMPILER_P->do_block_start(block_type_enum); COMPILER_P->create_parameter_in_current_parameter_set("enum", $2); free($2); }
    | ENUM8 NAME { DBG("yacc: enum8 %s\n", $2); COMPILER_P->do_block_start(block_type_enum); COMPILER_P->create_parameter_in_current_parameter_set("enum8", $2); free($2); }
    | ENUM16 NAME { DBG("yacc: enum16 %s\n", $2); COMPILER_P->do_block_start(block_type_enum); COMPILER_P->create_parameter_in_current_parameter_set("enum16", $2); free($2); }
    | ENUM32 NAME { DBG("yacc: enum32 %s\n", $2); COMPILER_P->do_block_start(block_type_enum); COMPILER_P->create_parameter_in_current_parameter_set("enum32", $2); free($2); }
    | SIGNED_ENUM8 NAME { DBG("yacc: signed_enum8 %s\n", $2); COMPILER_P->do_block_start(block_type_enum); COMPILER_P->create_parameter_in_current_parameter_set("signed_enum8", $2); free($2); }
    | SIGNED_ENUM16 NAME { DBG("yacc: signed_enum16 %s\n", $2); COMPILER_P->do_block_start(block_type_enum); COMPILER_P->create_parameter_in_current_parameter_set("signed_enum16", $2); free($2); }
    | SIGNED_ENUM32 NAME { DBG("yacc: signed_enum32 %s\n", $2); COMPILER_P->do_block_start(block_type_enum); COMPILER_P->create_parameter_in_current_parameter_set("signed_enum32", $2); free($2); }
    ;

enumerator_list
    : enumerator_list_decl '{' enumerator_list_list block_end
    | enumerator_list_decl '=' enumerator_list_concatination ';' { COMPILER_P->do_block_end(); }
    ;

enumerator_list_decl
    : ENUMERATOR_LIST NAME { COMPILER_P->do_block_start(block_type_enumerator_list); COMPILER_P->create_enumerator_list_in_current_parameter_set($2); free($2); }
    ;

enumerator_list_list
    : fdoc enumerator_list_value bdoc
    | enumerator_list_list ',' bdoc fdoc enumerator_list_value bdoc
    ;

enumerator_list_concatination
    : enumerator_list_name
    | enumerator_list_concatination '+' enumerator_list_name
    ;

enumerator_list_name
    : NAME { COMPILER_P->append_enumerator_list_to_current(NULL, $1); free($1); }
    | NAME ':' ':' NAME { COMPILER_P->append_enumerator_list_to_current($1, $4); free($1); free($4); }
    ;

enumerator_list_value
    : NAME { COMPILER_P->add_enumerator_list_value_to_current($1, NULL); free($1); }
    | NAME '=' VALUE { COMPILER_P->add_enumerator_list_value_to_current($1, $3); free($1); free($3); }
    ;

enum_enumerator_list_spec
    : enum_enumerator_list_concatination ';' { COMPILER_P->do_block_end(); }
    ;

enum_enumerator_list_concatination
    : enum_enumerator_list_name
    | enum_enumerator_list_concatination '+' enum_enumerator_list_name
    ;

enum_enumerator_list_name
    : NAME { DBG("yacc: enum enumerator list name %s\n", $1); COMPILER_P->append_enumerator_list_to_current_enum(NULL, $1); free($1); }
    | NAME ':' ':' NAME { DBG("yacc: enum enumerator list name %s::%s\n", $1, $4); COMPILER_P->append_enumerator_list_to_current_enum($1, $4); free($1); free($4); }
    ;

enum_inherit
    :
    | ':' enum_parent_name
    | ':' enum_external_parent_name
    ;

enum_inherit_no_block
    : ':' enum_parent_name ';' { COMPILER_P->do_block_end(); }
    | ':' enum_external_parent_name ';' { COMPILER_P->do_block_end(); }
    ;

enum_parent_name
    : NAME { DBG("yacc: enum inherit -> %s\n", $1); COMPILER_P->add_parent_to_current_parameter(NULL, $1); free($1); }
    ;

enum_external_parent_name
    : NAME ':' ':' NAME { DBG("yacc: enum inherit -> %s::%s\n", $1, $4); COMPILER_P->add_parent_to_current_parameter($1, $4); free($1); free($4); }
    ;

parameters
    : parameters_with_parameter_set_name '{' parameters_contents block_end
    ;

parameters_in_block
    : parameters_without_parameter_set_name '{' parameters_contents block_end
    ;

parameters_with_parameter_set_name
    : PARAMETERS NAME { DBG("yacc: parameters in file scope (parameter_set %s)\n", $2); COMPILER_P->set_current_parameter_set($2); COMPILER_P->set_current_parameter_group(NULL); COMPILER_P->do_block_start(block_type_parameters_in_file_scope); free($2); }
    | PARAMETERS NAME IN NAME { DBG("yacc: parameters in file scope with category ref (parameter_set %s in %s)\n", $2, $4); COMPILER_P->set_current_parameter_set($2); COMPILER_P->set_current_parameter_group($4); COMPILER_P->do_block_start(block_type_parameters_in_file_scope); free($2); free($4); }
    ;

parameters_without_parameter_set_name
    : PARAMETERS { DBG("yacc: parameters in parameter_set block\n"); COMPILER_P->do_block_start(block_type_parameters_in_block_scope); }
    ;

parameters_contents
    : fdoc param_entry bdoc
    | parameters_contents fdoc param_entry bdoc
    ;

param_entry
    : enum
    | bag
    | basic_type
    | enumerator_list
    | number_range
    ;

bag
    : bag_decl parameter_options bag_inherit '{' param_list block_end { COMPILER_P->reset_current_parameter_list(); }
    | bag_decl parameter_options bag_inherit '{' param_list ',' ellipsis block_end { COMPILER_P->reset_current_parameter_list(); }
    | bag_decl parameter_options bag_inherit '{' ellipsis block_end { COMPILER_P->reset_current_parameter_list(); }
    | bag_decl parameter_options bag_inherit '{' block_end { COMPILER_P->reset_current_parameter_list(); }
    | bag_decl parameter_options bag_inherit_no_block
    ;

bag_decl
    : BAG NAME { DBG("yacc: bag %s\n", $2); COMPILER_P->do_block_start(block_type_bag); COMPILER_P->create_parameter_in_current_parameter_set("bag", $2); free($2); }
    ;

bag_inherit
    :
    | ':' bag_parent_name
    | ':' bag_external_parent_name
    ;

bag_inherit_no_block
    : ':' bag_parent_name ';' { COMPILER_P->do_block_end(); }
    | ':' bag_external_parent_name ';' { COMPILER_P->do_block_end(); }
    ;

bag_parent_name
    : NAME { DBG("yacc: bag inherit -> %s\n", $1); COMPILER_P->add_parent_to_current_parameter(NULL, $1); free($1); }
    ;

bag_external_parent_name
    : NAME ':' ':' NAME { DBG("yacc: bag inherit -> %s::%s\n", $1, $4); COMPILER_P->add_parent_to_current_parameter($1, $4); free($1); free($4); }
    ;

basic_type
    : basic_type_decl parameter_options ';'
    | basic_type_decl parameter_options basic_inherit
    ;

basic_type_decl
    : int_decl
    | string_decl
    | wstring_decl
    | addr_decl
    | bool_decl
    | bool8_decl
    | sint8_decl
    | sint16_decl
    | sint32_decl
    | sint64_decl
    | uint8_decl
    | uint16_decl
    | uint32_decl
    | uint64_decl
    | uint8_array_decl
    | uint16_array_decl
    | uint32_array_decl
    | string_tuple_decl
    | int_tuple_decl
    | strint_tuple_decl
    | struint8_tuple_decl
    ;

int_decl
    : INT NAME { DBG("yacc: int %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("int", $2); free($2); }
    ;

string_decl
    : STRING NAME { DBG("yacc: string %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("string", $2); free($2); }
    ;

wstring_decl
    : WSTRING NAME { DBG("yacc: wstring %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("wstring", $2); free($2); }
    ;

addr_decl
    : ADDR NAME { DBG("yacc: addr %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("addr", $2); free($2); }
    ;

bool_decl
    : BOOL NAME { DBG("yacc: bool %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("bool", $2); free($2); }
    ;

bool8_decl
    : BOOL8 NAME { DBG("yacc: bool8 %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("bool8", $2); free($2); }
    ;

sint8_decl
    : SINT8 NAME { DBG("yacc: sint8 %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("sint8", $2); free($2); }
    ;

sint16_decl
    : SINT16 NAME { DBG("yacc: sint16 %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("sint16", $2); free($2); }
    ;

sint32_decl
    : SINT32 NAME { DBG("yacc: sint32 %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("sint32", $2); free($2); }
    ;

sint64_decl
    : SINT64 NAME { DBG("yacc: sint64 %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("sint64", $2); free($2); }
    ;

uint8_decl
    : UINT8 NAME { DBG("yacc: uint8 %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("uint8", $2); free($2); }
    ;

uint16_decl
    : UINT16 NAME { DBG("yacc: uint16 %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("uint16", $2); free($2); }
    ;

uint32_decl
    : UINT32 NAME { DBG("yacc: uint32 %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("uint32", $2); free($2); }
    ;

uint64_decl
    : UINT64 NAME { DBG("yacc: uint64 %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("uint64", $2); free($2); }
    ;

uint8_array_decl
    : UINT8_ARRAY NAME { DBG("yacc: uint8_array %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("uint8_array", $2); free($2); }
    ;

uint16_array_decl
    : UINT16_ARRAY NAME { DBG("yacc: uint16_array %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("uint16_array", $2); free($2); }
    ;

uint32_array_decl
    : UINT32_ARRAY NAME { DBG("yacc: uint32_array %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("uint32_array", $2); free($2); }
    ;

string_tuple_decl
    : STRING_TUPLE NAME { DBG("yacc: string_tuple %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("string_tuple", $2); free($2); }
    ;

int_tuple_decl
    : INT_TUPLE NAME { DBG("yacc: int_tuple %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("int_tuple", $2); free($2); }
    ;

strint_tuple_decl
    : STRINT_TUPLE NAME { DBG("yacc: strint_tuple %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("strint_tuple", $2); free($2); }
    ;

struint8_tuple_decl
    : STRUINT8_TUPLE NAME { DBG("yacc: struint8_tuple %s\n", $2); COMPILER_P->create_parameter_in_current_parameter_set("struint8_tuple", $2); free($2); }
    ;

basic_inherit
    : ':' basic_parent_name ';'
    ;

basic_parent_name
    : NAME ':' ':' NAME { DBG("yacc: basic inherit -> %s::%s\n", $1, $4); COMPILER_P->add_parent_to_current_parameter($1, $4); free($1); free($4); }
    | NAME { DBG("yacc: basic inherit -> %s\n", $1); COMPILER_P->add_parent_to_current_parameter(NULL, $1); free($1); }
    ;

number_range
    : number_range_decl integer_ranges ';'
    ;

number_range_decl
    : NUMBER_RANGE NAME { COMPILER_P->create_number_range_in_current_parameter_set($2); free($2); }
    ;

integer_ranges
    : integer_range_entry
    | integer_ranges ',' integer_range_entry
    ;

integer_range_entry
    : VALUE DOTDOT VALUE { COMPILER_P->add_integer_range_to_current_number_range($1, $3); free($1); free($3); }
    ;


message_bag_decl
    : COMMAND_BAG NAME { DBG("yacc: command_bag %s\n", $2); COMPILER_P->do_block_start(block_type_command_bag); COMPILER_P->add_command_bag_to_current_category($2); free($2); }
    | RESPONSE_BAG NAME { DBG("yacc: response_bag %s\n", $2); COMPILER_P->do_block_start(block_type_response_bag); COMPILER_P->add_response_bag_to_current_category($2);  free($2); }
    | EVENT_BAG NAME { DBG("yacc: event_bag %s\n", $2); COMPILER_P->do_block_start(block_type_event_bag); COMPILER_P->add_event_bag_to_current_category($2);  free($2); }
    ;

ellipsis
    : ELLIPSIS { COMPILER_P->create_ellipsis(); }
    ;



command_list
    : fdoc command
    | command_list fdoc command
    ;

command
    : command_name '(' command_event_param_list ')' ';'{ COMPILER_P->move_current_parameter_list_to_current_command(); }
    | command_name '(' ')' ';'{ COMPILER_P->move_current_parameter_list_to_current_command(); }
    ;

command_name
    : NAME { DBG("yacc: command_name: %s\n", $1); COMPILER_P->add_command_to_current_category($1); free($1); }
    ;

event_list
    : fdoc event
    | event_list fdoc event
    ;

event
    : event_name '(' command_event_param_list ')' ';' { COMPILER_P->move_current_parameter_list_to_current_event(); }
    | event_name '(' ')' ';' { COMPILER_P->move_current_parameter_list_to_current_event(); }
    ;

event_name
    : NAME { DBG("yacc: event_name: %s\n", $1); COMPILER_P->add_event_to_current_category($1); free($1); }
    ;

enum_list
    : fdoc enum_name bdoc
    | enum_list ',' bdoc fdoc enum_name bdoc
    ;

enum_name
    : NAME { DBG("yacc: enum entry %s\n", $1); COMPILER_P->add_enum_value_to_current_parameter($1, NULL); free($1); }
    | NAME '=' VALUE { DBG("yacc: enum entry %s = %s\n", $1, $3); COMPILER_P->add_enum_value_to_current_parameter($1, $3); free($1); free($3); }
    ;

param_list
    : fdoc param bdoc
    | param_list ',' bdoc fdoc param bdoc
    ;

command_event_param_list
    : directional_param
    | command_event_param_list ',' directional_param
    ;

directional_param
    : param
    | direction param
    ;


direction
    : out
    | inout
    ;

out
    : OUT { COMPILER_P->set_direction(direction_out); }
    ;

inout
    : INOUT { COMPILER_P->set_direction(direction_inout); }
    ;

param
    : optional_single_param
    | optional_array_param array_spec
    | mandatory_single_param
    | mandatory_array_param array_spec
    ;

array_spec
    : '[' array_options ']'
    | '[' ']'
    ;

array_options
    : ELLIPSIS { DBG("yacc: array option '%s'\n", "..."); COMPILER_P->set_array_option((char*)"..."); }
    ;

optional_single_param
    : '*' NAME { DBG("yacc: optional single_param: %s\n", $2); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry(NULL, $2, NULL, 1, 0)); COMPILER_P->set_direction(direction_in); free($2); }
    | '*' NAME ':' ':' NAME { DBG("yacc: optional single_param: %s::%s\n", $2, $5); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry($2, $5, NULL, 1, 0)); COMPILER_P->set_direction(direction_in); free($2); free($5); }
    | NAME '*' NAME { DBG("yacc: optional single_param with field: %s %s\n", $1, $3); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry(NULL, $1, $3, 1, 0)); COMPILER_P->set_direction(direction_in); free($1); free($3); }
    | NAME ':' ':' NAME '*' NAME { DBG("yacc: optional single_param with field: %s::%s %s\n", $1, $4, $6); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry($1, $4, $6, 1, 0)); COMPILER_P->set_direction(direction_in); free($1); free($4); free($6); }
    ;

optional_array_param
    : '*' NAME { DBG("yacc: optional array_param: %s\n", $2); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry(NULL, $2, NULL, 1, 1)); COMPILER_P->set_direction(direction_in); free($2); }
    | '*' NAME ':' ':' NAME { DBG("yacc: optional array_param: %s::%s\n", $2, $5); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry($2, $5, NULL, 1, 1)); COMPILER_P->set_direction(direction_in); free($2); free($5); }
    | NAME '*' NAME { DBG("yacc: optional array_param with field: %s %s\n", $1, $3); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry(NULL, $1, $3, 1, 1)); COMPILER_P->set_direction(direction_in); free($1); free($3); }
    | NAME ':' ':' NAME '*' NAME { DBG("yacc: optional array_param with field: %s::%s %s\n", $1, $4, $6); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry($1, $4, $6, 1, 1)); COMPILER_P->set_direction(direction_in); free($1); free($4); free($6); }
    ;

mandatory_single_param
    : NAME { DBG("yacc: mandatory single_param: %s\n", $1); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry(NULL, $1, NULL, 0, 0)); COMPILER_P->set_direction(direction_in); free($1); }
    | NAME ':' ':' NAME { DBG("yacc: mandatory single_param: %s::%s\n", $1, $4); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry($1, $4, NULL, 0, 0)); COMPILER_P->set_direction(direction_in); free($1); free($4); }
    | NAME NAME { DBG("yacc: mandatory single_param with field: %s %s\n", $1, $2); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry(NULL, $1, $2, 0, 0)); COMPILER_P->set_direction(direction_in); free($1); free($2); }
    | NAME ':' ':' NAME NAME { DBG("yacc: mandatory single_param with field: %s::%s %s\n", $1, $4, $5); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry($1, $4, $5, 0, 0)); COMPILER_P->set_direction(direction_in); free($1); free($4); free($5); }
    ;

mandatory_array_param
    : NAME { DBG("yacc: mandatory array_param: %s\n", $1); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry(NULL, $1, NULL, 0, 1)); COMPILER_P->set_direction(direction_in); free($1); }
    | NAME ':' ':' NAME { DBG("yacc: mandatory array_param: %s::%s\n", $1, $4); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry($1, $4, NULL, 0, 1)); COMPILER_P->set_direction(direction_in); free($1); free($4); }
    | NAME NAME { DBG("yacc: mandatory array_param with field: %s %s\n", $1, $2); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry(NULL, $1, $2, 0, 1)); COMPILER_P->set_direction(direction_in); free($1); free($2); }
    | NAME ':' ':' NAME NAME { DBG("yacc: mandatory array_param with field: %s::%s %s\n", $1, $4, $5); COMPILER_P->append_to_current_parameter_list(COMPILER_P->create_parameter_list_entry($1, $4, $5, 0, 1)); COMPILER_P->set_direction(direction_in); free($1); free($4); free($5); }
    ;

category_type_name
    : NAME { DBG("yacc: category '' '%s'\n", $1); COMPILER_P->create_category( NULL, $1 ); free($1); }
    | NAME ':' NAME { DBG("yacc: category '%s' '%s'\n", $3, $1); COMPILER_P->create_category( $3, $1 ); free($1); free($3); }
    ;

parameter_set_property_list
    : parameter_set_property
    | parameter_set_property_list parameter_set_property
    ;

parameter_set_property
    : fdoc parameter_set_prefix_decl ';' bdoc
    | fdoc parameter_set_id_decl ';' bdoc
    | fdoc parameter_set_range_id_decl ';' bdoc
    | fdoc parameter_set_bag_field_table_suffix_decl ';' bdoc
    | fdoc parameters_in_block
    ;

parameter_set_prefix_decl
    : PREFIX NAME { DBG("yacc: parameter_set prefix %s\n", $2); COMPILER_P->add_prefix_to_current_parameter_set($2); free($2); }
    ;

parameter_set_id_decl
    : NUMERIC_ID VALUE { DBG("yacc: parameter_set id %s\n", $2); COMPILER_P->add_id_to_current_parameter_set($2); free($2); }
    ;

parameter_set_range_id_decl
    : RANGE_ID ENUM NAME { DBG("yacc: parameter_set range id enum %s\n", $3); COMPILER_P->add_range_id_to_current_parameter_set($3); free($3); }
    ;

parameter_set_bag_field_table_suffix_decl
    : BAG_FIELD_TABLE_SUFFIX NAME { DBG("yacc: parameter_set bag field table suffix %s\n", $2); COMPILER_P->add_bag_field_table_suffix_to_current_parameter_set($2); free($2); }
    ;

parameter_options
    :
    | option_list
    ;

option_list
    : option
    | option_list ',' option
    ;

option
    : max_option
    | min_option
    | default_option
    | virtual_option
    | get_option
    | set_option
    | config_option
    | range_option
    ;

max_option
    : MAX VALUE { DBG("yacc: option %s %s\n", "max", $2); COMPILER_P->add_option_to_current_parameter("max", $2, value_type_value); free($2); }
    | MAX NAME { DBG("yacc: option %s %s\n", "max", $2); COMPILER_P->add_option_to_current_parameter("max", $2, value_type_name); free($2); }
    ;

min_option
    : MIN VALUE { DBG("yacc: option %s %s\n", "min", $2); COMPILER_P->add_option_to_current_parameter("min", $2, value_type_value); free($2); }
    | MIN NAME { DBG("yacc: option %s %s\n", "min", $2); COMPILER_P->add_option_to_current_parameter("min", $2, value_type_name); free($2); }
    ;

default_option
    : DEFAULT STRING_LITERAL { DBG("yacc: option %s %s\n", "default", $2); COMPILER_P->add_option_to_current_parameter("default", $2, value_type_string_literal); free($2); }
    | DEFAULT VALUE { DBG("yacc: option %s %s\n", "default", $2); COMPILER_P->add_option_to_current_parameter("default", $2, value_type_value); free($2); }
    | DEFAULT '(' STRING_LITERAL ',' STRING_LITERAL ')' { DBG("yacc: option %s %s,%s\n", "default", $3, $5); COMPILER_P->add_option_to_current_parameter("default", $3, value_type_string_literal, $5, value_type_string_literal); free($3); free($5); }
    | DEFAULT '(' STRING_LITERAL ',' VALUE ')' { DBG("yacc: option %s %s,%s\n", "default", $3, $5); COMPILER_P->add_option_to_current_parameter("default", $3, value_type_string_literal, $5, value_type_value); free($3); free($5); }
    | DEFAULT '(' VALUE ',' VALUE ')' { DBG("yacc: option %s %s,%s\n", "default", $3, $5); COMPILER_P->add_option_to_current_parameter("default", $3, value_type_value, $5, value_type_value); free($3); free($5); }
    | DEFAULT NAME { DBG("yacc: option %s %s\n", "default", $2); COMPILER_P->add_option_to_current_parameter("default", $2, value_type_name); free($2); }
    | DEFAULT BOOLEAN { DBG("yacc: option %s %s\n", "default", $2); COMPILER_P->add_option_to_current_parameter("default", $2, value_type_name); free($2); }
    ;

set_option
    : SET { DBG("yacc: option %s true\n", "set"); COMPILER_P->add_option_to_current_parameter("set", "true", value_type_bool); }
    | SET BOOLEAN { DBG("yacc: option %s %s\n", "set", $2); COMPILER_P->add_option_to_current_parameter("set", $2, value_type_bool); free($2); }
    ;

get_option
    : GET { DBG("yacc: option %s true\n", "get"); COMPILER_P->add_option_to_current_parameter("get", "true", value_type_bool); }
    | GET BOOLEAN { DBG("yacc: option %s %s\n", "get", $2); COMPILER_P->add_option_to_current_parameter("get", $2, value_type_bool); free($2); }
    ;

config_option
    : CONFIG { DBG("yacc: option %s true\n", "config"); COMPILER_P->add_option_to_current_parameter("config", "true", value_type_bool); }
    | CONFIG BOOLEAN { DBG("yacc: option %s %s\n", "config", $2); COMPILER_P->add_option_to_current_parameter("config", $2, value_type_bool); free($2); }
    ;

virtual_option
    : VIRTUAL { DBG("yacc: option %s true\n", "virtual"); COMPILER_P->add_option_to_current_parameter("virtual", "true", value_type_bool); }
    ;

range_option
    : RANGE '(' range_list ')'
    ;

range_list
    : range_entry
    | range_list ',' range_entry
    ;

range_entry
    : VALUE DOTDOT VALUE { DBG("yacc: option range %s..%s\n", $1, $3); COMPILER_P->add_option_to_current_parameter("range", $1, value_type_value, $3, value_type_value); free($1); free($3); }
    | NAME { DBG("yacc: option range '%s'\n", $1); COMPILER_P->add_option_to_current_parameter("range", $1, value_type_name); free($1); }
    ;

x_segment
    : x_begin x_element_list x_end
    | x_begin x_end
    ;

x_begin
    : DOC_BEGIN { DBG("yacc: doc_begin\n"); }
    ;

x_end
    : DOC_END { DBG("yacc: doc_end\n"); }
    ;

x_element_list
    : x_element
    | x_element_list x_element
    ;

x_element
    : x_defgroup
    | x_ingroup
    | x_text
    | x_param
    | x_see
    | x_file
    | x_brief
    | x_ref
    | x_localtag
    ;

x_defgroup
    : DOC_DEFGROUP DOC_IDENTIFIER DOC_TEXT { DBG("yacc: doc defgroup '%s' '%s'\n", $2, $3); COMPILER_P->add_x_entry("defgroup", $2, $3); free($2); free($3); }
    ;

x_ingroup
    : DOC_INGROUP DOC_IDENTIFIER { DBG("yacc: doc ingroup '%s'\n", $2); COMPILER_P->add_x_entry("ingroup", $2); free($2); }
    ;

x_text
    : DOC_TEXT { DBG("yacc: doc text: '%s'\n", $1); COMPILER_P->add_x_entry("text", $1); free($1); }
    ;

x_param
    : DOC_PARAM DOC_IDENTIFIER { DBG("yacc: doc param: '%s'\n", $2); COMPILER_P->add_x_entry("param", $2); free($2); }
    ;

x_see
    : DOC_SEE DOC_IDENTIFIER { DBG("yacc: doc see: '%s'\n", $2); COMPILER_P->add_x_entry("see", $2); free($2); }
    ;

x_file
    : DOC_FILE DOC_IDENTIFIER { DBG("yacc: doc file: '%s'\n", $2); COMPILER_P->add_x_entry("file", $2); free($2); }
    ;

x_brief
    : DOC_BRIEF DOC_IDENTIFIER DOC_TEXT { DBG("yacc: doc brief: '%s' '%s'\n", $2, $3); COMPILER_P->add_x_entry("brief", $2, $3); free($2); free($3); }
    ;

x_ref
    : DOC_REF DOC_IDENTIFIER { DBG("yacc: doc ref: '%s'\n", $2); COMPILER_P->add_x_entry("ref", $2); free($2); }
    ;

x_localtag
    : DOC_LOCALTAG DOC_IDENTIFIER { DBG("yacc: doc localtag: '%s' '%s'\n", $1, $2); COMPILER_P->add_x_entry($1, $2); free($1); free($2); }
    ;

%%

