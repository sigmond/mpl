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
*   Author: Harald Johansen <hajohans1@gmail.com>
*   Author: Emil B. Viken <emil.b.viken@gmail.com>
*
*/
%{
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mplcomp.hh"
#include "mplcomp.tab.hh"

//#define DBG printf
#define DBG


%}
%option c++
%option yyclass="mpl_lexer"
%option yylineno
%x INCL
%x H
%x C
%x IF_H
%x IF
%x IFDEF
%x EAT_UNTIL_ELSE
%x EAT_UNTIL_ENDIF
%x H_H
%x C_H

%x D_INITIAL
%x D_ALPHA
%x D_LOCALTAG

%%
    if (!is_initialized) {
        if (initial_mode == lexer_mode_doc)
           BEGIN(D_INITIAL);
        else
           BEGIN(INITIAL);

        is_initialized = 1;
    }

category {DBG("lex: %s\n", yytext); return CATEGORY;}
using {DBG("lex: %s\n", yytext); return USING;}
parameter_set {DBG("lex: %s\n", yytext); return PARAMETER_SET;}
command_bag {DBG("lex: %s\n", yytext); return COMMAND_BAG; }
response_bag {DBG("lex: %s\n", yytext); return RESPONSE_BAG; }
event_bag {DBG("lex: %s\n", yytext); return EVENT_BAG;}
prefix {DBG("lex: %s\n", yytext); return PREFIX;}
numeric_id {DBG("lex: %s\n", yytext); return NUMERIC_ID;}
range_id {DBG("lex: %s\n", yytext); return RANGE_ID;}
bag_field_table_suffix {DBG("lex: %s\n", yytext); return BAG_FIELD_TABLE_SUFFIX;}
parameters {DBG("lex: %s\n", yytext); return PARAMETERS;}
commands {DBG("lex: %s\n", yytext); return COMMANDS; }
events {DBG("lex: %s\n", yytext); return EVENTS;}
out {DBG("lex: %s\n", yytext); return OUT; }
inout {DBG("lex: %s\n", yytext); return INOUT; }
enum {DBG("lex: %s\n", yytext); return ENUM;}
enum8 {DBG("lex: %s\n", yytext); return ENUM8;}
enum16 {DBG("lex: %s\n", yytext); return ENUM16;}
enum32 {DBG("lex: %s\n", yytext); return ENUM32;}
signed_enum8 {DBG("lex: %s\n", yytext); return SIGNED_ENUM8;}
signed_enum16 {DBG("lex: %s\n", yytext); return SIGNED_ENUM16;}
signed_enum32 {DBG("lex: %s\n", yytext); return SIGNED_ENUM32;}
bag {DBG("lex: %s\n", yytext); return BAG;}
int {DBG("lex: %s\n", yytext); return INT;}
string {DBG("lex: %s\n", yytext); return STRING;}
wstring {DBG("lex: %s\n", yytext); return WSTRING;}
addr {DBG("lex: %s\n", yytext); return ADDR;}
bool {DBG("lex: %s\n", yytext); return BOOL;}
bool8 {DBG("lex: %s\n", yytext); return BOOL8;}
sint8 {DBG("lex: %s\n", yytext); return SINT8;}
sint16 {DBG("lex: %s\n", yytext); return SINT16;}
sint32 {DBG("lex: %s\n", yytext); return SINT32;}
sint64 {DBG("lex: %s\n", yytext); return SINT64;}
uint8 {DBG("lex: %s\n", yytext); return UINT8;}
uint16 {DBG("lex: %s\n", yytext); return UINT16;}
uint32 {DBG("lex: %s\n", yytext); return UINT32;}
uint64 {DBG("lex: %s\n", yytext); return UINT64;}
uint8_array {DBG("lex: %s\n", yytext); return UINT8_ARRAY;}
uint16_array {DBG("lex: %s\n", yytext); return UINT16_ARRAY;}
uint32_array {DBG("lex: %s\n", yytext); return UINT32_ARRAY;}
string_tuple {DBG("lex: %s\n", yytext); return STRING_TUPLE;}
int_tuple {DBG("lex: %s\n", yytext); return INT_TUPLE;}
strint_tuple {DBG("lex: %s\n", yytext); return STRINT_TUPLE;}
struint8_tuple {DBG("lex: %s\n", yytext); return STRUINT8_TUPLE;}
enumerator_list {DBG("lex: %s\n", yytext); return ENUMERATOR_LIST;}
number_range {DBG("lex: %s\n", yytext); return NUMBER_RANGE;}
range {DBG("lex: %s\n", yytext); return RANGE;}
min {DBG("lex: %s\n", yytext); return MIN;}
max {DBG("lex: %s\n", yytext); return MAX;}
default {DBG("lex: %s\n", yytext); return DEFAULT;}
set {DBG("lex: %s\n", yytext); return SET;}
get {DBG("lex: %s\n", yytext); return GET;}
config {DBG("lex: %s\n", yytext); return CONFIG;}
virtual {DBG("lex: %s\n", yytext); return VIRTUAL;}
in {DBG("lex: %s\n", yytext); return IN;}

true {DBG("lex: %s\n", yytext); yylval.name = strdup(yytext); return BOOLEAN;}
false {DBG("lex: %s\n", yytext); yylval.name = strdup(yytext); return BOOLEAN;}

include BEGIN(INCL);

^%h_begin {DBG("lex: %s\n", yytext); BEGIN(H_H); return H_HEADER;}
<H_H>segment {DBG("lex: %s\n", yytext); return SEGMENT;}
<H_H>= {DBG("lex: %s\n", yytext); return '=';}
<H_H>[ \t]+
<H_H>[a-zA_Z_-]+ {DBG("lex: %s\n", yytext); yylval.name = strdup(yytext); return LITERAL_CODE_OPTION;}
<H_H>\n { DBG("lex: h_header end\n"); BEGIN(H); }
<H>^[^%][^\n]*\n { DBG("lex: h_line: '%s'\n", yytext); yylval.name = strdup(yytext); return HLINE; }
<H>^%h_end[^\n]*\n {DBG("lex: %s\n", yytext); BEGIN(INITIAL); return H_END; }

^%c_begin { DBG("lex: c_begin\n"); BEGIN(C_H); return C_HEADER; }
<C_H>segment {DBG("lex: %s\n", yytext); yylval.name = strdup(yytext); return SEGMENT; }
<C_H>= return '=';
<C_H>[ \t]+
<C_H>[a-zA_Z_-]+ { DBG("lex: c_header option=%s\n", yytext); yylval.name = strdup(yytext); return LITERAL_CODE_OPTION; }
<C_H>\n { DBG("lex: c_header end\n"); BEGIN(C); }
<C>^[^%][^\n]*\n { DBG("lex: c_line: '%s'\n", yytext); yylval.name = strdup(yytext); return CLINE; }
<C>^%c_end[^\n]*\n { DBG("lex: c end\n"); BEGIN(INITIAL); return C_END; }

^%option { DBG("lex: option\n"); return OPTION; }

^%if { DBG("lex: if\n"); BEGIN(IF_H); }
<IF_H>[A-Z_]+ { DBG("lex: if_header name '%s'\n", yytext); compiler_p->add_left_operand_to_if_condition(yytext); }
<IF_H>[0-9]+ { DBG("lex: if_header value '%s'\n", yytext); compiler_p->add_right_operand_to_if_condition(yytext); }
<IF_H>= { DBG("lex: if_header operator '%s'\n", yytext); compiler_p->add_operator_to_if_condition('='); }
<IF_H>\< { DBG("lex: if_header operator '%s'\n", yytext); compiler_p->add_operator_to_if_condition('<'); }
<IF_H>\> { DBG("lex: if_header operator '%s'\n", yytext); compiler_p->add_operator_to_if_condition('>'); }
<IF_H>[ \t]+
<IF_H>\n {
        DBG("lex: if_header end\n");
        if (compiler_p->check_if_condition())
            BEGIN(INITIAL);
        else
            BEGIN(EAT_UNTIL_ELSE);

        compiler_p->reset_if_condition();
    }

^%ifdef {DBG("lex: %s\n", yytext); BEGIN(IFDEF);}

<IFDEF>[^\n]+\n {
        DBG("lex: ifdef end\n");
        if (compiler_p->check_ifdef_condition(strdup(yytext)))
            BEGIN(INITIAL);
        else
            BEGIN(EAT_UNTIL_ELSE);
    }

<EAT_UNTIL_ELSE>^[^%][^\n]*\n
<EAT_UNTIL_ELSE>^%else[ \t]*\n BEGIN(INITIAL);
<EAT_UNTIL_ELSE>^%endif[ \t]*\n BEGIN(INITIAL);

<EAT_UNTIL_ENDIF>^[^%][^\n]*\n
<EAT_UNTIL_ENDIF>^%endif[ \t]*\n BEGIN(INITIAL);

^%else[ \t]*\n BEGIN(EAT_UNTIL_ENDIF);
^%endif[ \t]*\n

<INCL>[ \t*]
<INCL>[^ \t\n]+ {
        char include_file[255];
        sprintf(include_file, "%s/%s", compiler_p->include_dir_name_p, yytext);
        ifstream *input_file_p = new ifstream(include_file, ios::in);
        if (input_file_p->fail()) {
            fprintf(stderr, "%s:%d: Cannot open file '%s'\n",
                    compiler_p->peek_filename(),
                    yylineno,
                    include_file);
            exit(-1);
        }
        DBG("lex: opened file %s\n", include_file);
        compiler_p->add_file_doc();
        compiler_p->push_file((const char *) include_file, compiler_p->lexer_p->lineno(), input_file_p);
        compiler_p->reset_doc_target();
        yypush_buffer_state(yy_create_buffer(input_file_p, YY_BUF_SIZE));
        BEGIN(INITIAL);
    }

<<EOF>> {
        DBG("lex: closing stream %s\n", compiler_p->peek_filename());
        yypop_buffer_state();
        compiler_p->add_file_doc();
        compiler_p->reset_doc_target();
        if (!YY_CURRENT_BUFFER) {
            compiler_p->remember_file(compiler_p->pop_file(), 1);
            yyterminate();
        } else {
            compiler_p->remember_file(compiler_p->pop_file(), 0);
        }
        BEGIN(INITIAL);
    }

"/**\<"([^*]|"*"[^/])*"*/" { DBG("lex: backward doc:\n'%s'\n", yytext); yylval.name = strdup(yytext); return BACKWARD_DOC; }
"/**"([^*]|"*"[^/])*"*/" { DBG("lex: forward doc:\n'%s'\n", yytext); yylval.name = strdup(yytext); return FORWARD_DOC; }
#.*\n { DBG("lex: mpl comment: '%s'\n", yytext); }
\{ { DBG("lex: {\n"); return '{'; }
\} { DBG("lex: }\n"); return '}'; }
; { DBG("lex: ;\n"); return ';'; }
, { DBG("lex: ,\n"); return ','; }
= { DBG("lex: =\n"); return '='; }
\+ { DBG("lex: +\n"); return '+'; }
\( { DBG("lex: (\n"); return '(';}
\) { DBG("lex: )\n"); return ')';}
\: { DBG("lex: :\n"); return ':'; }
\* { DBG("lex: *\n"); return '*'; }
\[ { DBG("lex: [\n"); return '['; }
\] { DBG("lex: ]\n"); return ']'; }
\" { DBG("lex: \"\n"); return '"'; }
[a-zA-Z]+[0-9a-zA-Z_-]* {
              yylval.name = strdup(yytext);
              DBG("lex: name: %s\n", yytext);
              return NAME;
        }

[1-9][a-zA-Z_-]+ {
              yylval.name = strdup(yytext);
              DBG("lex: name: %s\n", yytext);
              return NAME;
        }

-?[0-9]+ {
        DBG("lex: decimal integer: %s\n", yytext); yylval.name = strdup(yytext); return VALUE;
    }

0[xX][0-9a-fA-F]+ {
        DBG("lex: hex integer: %s\n", yytext); yylval.name = strdup(yytext); return VALUE;
    }

\.\. { DBG("lex: dotdot\n"); return DOTDOT; }

\.\.\. { DBG("lex: ellipsis\n"); return ELLIPSIS; }


\n /* ignore */
[ \t]+ /* ignore */
L?\"([^\"\\\n]|(\\['\"?\\abfnrtv])|(\\([0123456]{1,3}))|(\\x[[:xdigit:]]+)|(\\u([[:xdigit:]]{4}))|(\\U([[:xdigit:]]{8})))*\" {
        DBG("lex: literal: '%s'\n", yytext); yylval.name = strdup(yytext); return STRING_LITERAL;
    }

<D_INITIAL>("/**"|"/**\<")[ \t]* { DBG("lex: doc begin\n"); return DOC_BEGIN; }
<D_INITIAL>^[ \n\t]* { DBG("lex: space ignored\n"); }
<D_INITIAL>\n { DBG("lex: newline ignored\n"); }
<D_INITIAL>[ \t*]*\n { DBG("lex: empty line\n"); yylval.name = strdup("\n"); return DOC_TEXT; }
<D_INITIAL>[*][^/][ \t]* { DBG("lex: * ignored\n"); }
<D_INITIAL>"*/" { DBG("lex: doc end\n"); return DOC_END; }
<D_INITIAL>@\[ { DBG("lex: @[\n"); BEGIN(D_LOCALTAG); }
<D_INITIAL>@[\ \t]* { DBG("lex: @\n"); BEGIN(D_ALPHA); }
<D_INITIAL>[^@*\n][^@*\n]+ { DBG("lex: doc text: '%s'\n", yytext); yylval.name = strdup(trimstring(yytext, '\\')); return DOC_TEXT; }
<D_ALPHA>defgroup { DBG("lex: defgroup\n"); return DOC_DEFGROUP; }
<D_ALPHA>ingroup { DBG("lex: ingroup\n"); return DOC_INGROUP; }
<D_ALPHA>param { DBG("lex: param\n"); return DOC_PARAM; }
<D_ALPHA>see { DBG("lex: see\n"); return DOC_SEE; }
<D_ALPHA>file { DBG("lex: file\n"); return DOC_FILE; }
<D_ALPHA>brief { DBG("lex: brief\n"); return DOC_BRIEF; }
<D_ALPHA>ref { DBG("lex: ref\n"); return DOC_REF; }
<D_LOCALTAG>[^ \]\t\n]+ { DBG("lex: localtag '%s'\n", yytext); yylval.name = strdup(yytext); return DOC_LOCALTAG; }
<D_LOCALTAG>[ \t]+ /* ignore */
<D_LOCALTAG>\] { DBG("lex: end localtag\n"); BEGIN(D_ALPHA); }
<D_ALPHA>[ \t]+ /* ignore */
<D_ALPHA>[^ \t\n]+ { DBG("lex: identifier: '%s'\n", yytext); yylval.name = strdup(yytext); BEGIN(D_INITIAL); return DOC_IDENTIFIER; }
%%


