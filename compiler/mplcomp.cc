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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <fstream>
#include <FlexLexer.h>
#include "mplcomp.tab.hh"
#include "mplcomp.hh"
#include "mplcomp_parameter.hh"

/* Local defines */
#define DBG
//#define DBG printf

#define CHK_FTRACE
#define yylineno compiler_p->lineno()

/* Local datatypes */
typedef struct
{
  char *key_p;
  char *value_p;
} arg_t;

/* Misc local utilities */
static const char *code_segment_string(code_segment_t code_segment);
static char *strchr_escape(char *s, char c, char escape);
static char *get_matching_close_bracket(char open_bracket, char close_bracket, char *str_p, char escape);
static int get_args(arg_t *args,
                    int args_len,
                    char *buf,
                    char equal,
                    char delimiter,
                    char escape);
static int fill_escape(char *str, int size, char delimiter, char escape);
static const char *operator_string(if_operator_t if_operator);
static void add_compiler_define(mpl_list_t **compiler_defines_pp,
                                char *compiler_define_text_p);
static void usage(char *name);
static void dox_dox_list(FILE *f, mpl_list_t *dox_list_p, int is_forward);


/* Exported data */
mpl_compiler *compiler_p;

/* Exported functions */
int main(int argc, char *argv[])
{
    int opt;
    char *mode_p;
    char *mpl_filename_p;
    char *out_name_p = NULL;
    char *suffix_p;
    char prefix[255];
    char *out_dir_name_p;
    FILE *hfile_p = NULL;
    FILE *cfile_p = NULL;
    char hfilename[255];
    char cfilename[255];
    mpl_list_t *compiler_defines_p = NULL;
    char *include_dir_name_p = NULL;
    int experimental = 0;
    codegen_mode_t codegen_mode = codegen_mode_mpl;

    include_dir_name_p = strdup(".");
    mode_p = strdup("c");
    out_dir_name_p = strdup(".");

    if (argc < 2) {
        usage(argv[0]);
        exit(-1);
    }

    while ((opt = getopt(argc, argv, "m:o:d:i:D:e")) != -1) {
        switch (opt) {
            case 'm':
                free(mode_p);
                mode_p = strdup(optarg);
                break;
            case 'o':
                out_name_p = strdup(optarg);
                break;
            case 'd':
                free(out_dir_name_p);
                out_dir_name_p = strdup(optarg);
                break;
            case 'i':
                free(include_dir_name_p);
                include_dir_name_p = strdup(optarg);
                break;
            case 'D':
                if (!experimental) {
                    fprintf(stderr, "Compiler defines not supported in this mode\n");
                    exit(-1);
                }
                add_compiler_define(&compiler_defines_p, optarg);
                break;
            case 'e':
                experimental = 1;
                break;
            default:
                usage(argv[0]);
                exit(-1);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(-1);
    }

    mpl_filename_p = argv[optind];

    if (out_name_p == NULL) {
        char *only_filename_p;

        if (strrchr(mpl_filename_p, '/') != NULL)
            only_filename_p = strrchr(mpl_filename_p, '/') + 1;
        else
            only_filename_p = mpl_filename_p;

        suffix_p = strstr(only_filename_p, ".mpl");
        if (suffix_p && ((strlen(only_filename_p) - (suffix_p - only_filename_p)) == 4)) {
            memset(prefix, 0, 255);
            memcpy(prefix, only_filename_p, strlen(only_filename_p) - 4);
            out_name_p = prefix;
        } else {
            out_name_p = only_filename_p;
        }
    }

    if (!strcmp(mode_p, "cli")) {
        codegen_mode = codegen_mode_cli;
    }
    else if (!strcmp(mode_p, "api")) {
        codegen_mode = codegen_mode_api;
    }

    ifstream input_file(mpl_filename_p, ios::in);
    if (input_file.fail()) {
        fprintf(stderr, "Cannot open input file '%s'\n", mpl_filename_p);
        exit(-1);
    }

    istream input_stream(input_file.rdbuf());

    compiler_p = new mpl_compiler(&input_stream,
                                  strdup(include_dir_name_p),
                                  compiler_defines_p,
                                  experimental,
                                  codegen_mode);

    compiler_p->push_file(mpl_filename_p, 1);
    compiler_p->parse();
    compiler_p->check_parameters();

    if (!strcmp(mode_p, "internals")) {
        compiler_p->print();
    }
    else if (!strcmp(mode_p, "doxygen")) {
        compiler_p->dox(stdout);
    }
    else if (!strcmp(mode_p, "latex")) {
        compiler_p->latex(stdout);
    }
    else if (!strcmp(mode_p, "dejagnu")) {
        compiler_p->dejagnu(stdout);
    }
    else { /* codegen */
        if (codegen_mode == codegen_mode_mpl) {
            sprintf(hfilename, "%s/%s.h", out_dir_name_p, out_name_p);
            sprintf(cfilename, "%s/%s.c", out_dir_name_p, out_name_p);
        }
        else if (codegen_mode == codegen_mode_api) {
            sprintf(hfilename, "%s/%s.hh", out_dir_name_p, out_name_p);
            sprintf(cfilename, "%s/%s.cc", out_dir_name_p, out_name_p);
        }
        else if (codegen_mode == codegen_mode_cli) {
            sprintf(hfilename, "%s/%s_cli.h", out_dir_name_p, out_name_p);
            sprintf(cfilename, "%s/%s_cli.c", out_dir_name_p, out_name_p);
        }

        hfile_p = fopen(hfilename, "w");
        if (hfile_p == NULL){
            fprintf(stderr, "Cannot open output file '%s'\n", hfilename);
            exit(-1);
        }

        cfile_p = fopen(cfilename, "w");
        if (cfile_p == NULL){
            fprintf(stderr, "Cannot open output file '%s'\n", cfilename);
            exit(-1);
        }

        compiler_p->generate_code(hfile_p, cfile_p, out_name_p);

        fclose(hfile_p);
        fclose(cfile_p);
    }
    delete compiler_p;
    if (include_dir_name_p)
        free(include_dir_name_p);
    if (mode_p)
        free(mode_p);
    if (out_dir_name_p)
        free(out_dir_name_p);
    return 0;
}


int yyFlexLexer::yywrap()
{
    return 1;
}

void yyerror(const char *s)
{
    fprintf(stderr,"%s:%d: %s\n", compiler_p->peek_filename(), compiler_p->lineno(), s);
    exit(-1);
}

char *str_toupper(char *str)
{
    char *ustr_p;
    int i;

    ustr_p = (char *) calloc(1, 255);
    for (i = 0; i < strlen(str); i++) {
        ustr_p[i] = toupper(str[i]);
    }
    return ustr_p;
}

char *str_tolower(char *str)
{
    char *lstr_p;
    int i;

    lstr_p = (char *) calloc(1, 255);
    for (i = 0; i < strlen(str); i++) {
        lstr_p[i] = tolower(str[i]);
    }
    return lstr_p;
}

int mpl_prefix2paramsetid(const char *prefix_p)
{
  int id = 0;
  int i;

  for (i = 0; prefix_p[i] != 0; i++)
  {
    id += (prefix_p[i] * (i + 1));
  }

  return (id % 32768);
}

void gc_lines(FILE *f, mpl_list_t *lines_p, code_segment_t code_segment, codegen_mode_t codegen_mode)
{
    mpl_list_t *tmp_p;
    line *line_p;

    if (codegen_mode == codegen_mode_cli)
        return;

    if (lines_p == NULL)
        return;

    fprintf(f,
            "/* Start literal code section (segment = %s) */\n",
            code_segment_string(code_segment)
           );
    MPL_LIST_FOR_EACH(lines_p, tmp_p) {
        line_p = LISTABLE_PTR(tmp_p, line);
        if (line_p->code_segment == code_segment) {
            fprintf(f,
                    "%s",
                    line_p->line_p
                   );
        }
    }
    fprintf(f,
            "/* End literal code section */\n"
           );
    fprintf(f,
            "\n"
           );
}


char *fix_for_header_define(char *name_p)
{
    char *fixed_p;
    int len;
    int i;

    fixed_p = (char *) calloc(1, strlen(name_p) + 1);
    strcpy(fixed_p, name_p);
    len = strlen(fixed_p);

    for (i = 0; i < len; i++) {
        if (!isalpha(fixed_p[i]))
            fixed_p[i] = '_';
    }
    return fixed_p;
}

#define IS_PTR (b || t || a || ((m || o) && !pt))
#define PFX(fn) (!strcmp("class",(fn)) || !strcmp("new",(fn))) ? "_" : ""

void api_parameter_list(FILE *f,
                        mpl_list_t *parameter_list_p,
                        mpl_list_t *exclude_parameter_list_p,
                        direction_t dir)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    const char *sep_p;

    sep_p = "";

    if (parameter_list_p == NULL) {
        fprintf(f,
               "void"
              );
        return;
    }

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;
        int e = parameter_p->is_enum();
        int b = parameter_p->is_bag();
        int t = parameter_p->is_tuple();
        int a = parameter_p->is_array();
        char *fn = parameter_list_entry_p->field_name_p;
        char *pn = parameter_list_entry_p->parameter_name_p;
        int pt = (strstr(parameter_p->get_c_type(), " *") != NULL);

        char *ct;

        if (a || t) {
            int len = strlen(parameter_p->get_c_type()) - 6;
            ct = (char*)calloc(1, len + 1);
            strncpy(ct, (char*)parameter_p->get_c_type() + 4, len);
        }
        else {
            ct = strdup(parameter_p->get_c_type());
        }

        fprintf(f,
                "%s",
                sep_p
               );
        if (e) {
            fprintf(f,
                    "%s_%s_t",
                    parameter_list_entry_p->parameter_set_p->name_p,
                    pn
                   );
        }
        else if (b) {
            fprintf(f,
                    "%s",
                    pn
                   );
        }
        else {
            fprintf(f,
                    "%s",
                    ct
                   );
        }
        fprintf(f,
                "%s",
                pt && !b ? "" : " "
               );
        fprintf(f,
                " %s",
                IS_PTR ? "*" : ""
               );
        fprintf(f,
                "%s%s",
                fn ? PFX(fn) : "_",
                fn ? fn : pn
               );
        if (m)
            fprintf(f,
                    "[], int num_%s",
                    fn ? fn : pn
                   );
        sep_p = ", ";
        free(ct);
    }
}

void api_hh_class_members_parameter_list(FILE *f,
                                         char *indent,
                                         mpl_list_t *parameter_list_p,
                                         mpl_list_t *exclude_parameter_list_p,
                                         parameter_set *parameter_set_p,
                                         direction_t dir)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = parameter_set_p->get_short_name();
    char *snu = str_toupper(snl);

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;
        int b = parameter_p->is_bag();
        int e = parameter_p->is_enum();
        int t = parameter_p->is_tuple();
        int a = parameter_p->is_array();
        char *fn = parameter_list_entry_p->field_name_p;
        char *pn = parameter_list_entry_p->parameter_name_p;
        int pt = (strstr(parameter_p->get_c_type(), " *") != NULL);
        char *ct;

        if (a || t) {
            int len = strlen(parameter_p->get_c_type()) - 6;
            ct = (char*)calloc(1, len + 1);
            strncpy(ct, (char*)parameter_p->get_c_type() + 4, len);
        }
        else {
            ct = strdup(parameter_p->get_c_type());
        }

        if (e) {
            fprintf(f,
                    "%s        %s_%s_t",
                    indent,
                    parameter_list_entry_p->parameter_set_p->name_p,
                    parameter_p->name_p
                   );
        }
        else if (b) {
            fprintf(f,
                    "%s        %s",
                    indent,
                    parameter_p->name_p
                   );
        }
        else {
            fprintf(f,
                    "%s        %s",
                    indent,
                    ct
                   );
        }
        fprintf(f,
                "%s",
                pt && !b ? "" : " "
               );
        fprintf(f,
                " %s",
                IS_PTR ? "*" : ""
               );
        fprintf(f,
                "%s",
                m ? "*" : ""
               );
        fprintf(f,
                "%s%s",
                fn ? PFX(fn) : "_",
                fn ? fn : pn
               );
        if (m)
            fprintf(f,
                    ";\n"
                    "%s        int num_%s",
                    indent,
                    fn ? fn : pn
                   );
        fprintf(f,
                ";\n"
               );
        free(ct);
    }
    free(snu);
}

void api_hh_constructor_parameter_list(FILE *f,
                                       char *indent,
                                       mpl_list_t *parameter_list_p,
                                       mpl_list_t *exclude_parameter_list_p,
                                       inheritable_object *parent_p,
                                       parameter_set *parameter_set_p,
                                       direction_t dir)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = parameter_set_p->get_short_name();
    char *snu = str_toupper(snl);

    const char *sep_p;
    sep_p = "";

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;
        char *fn = parameter_list_entry_p->field_name_p;
        char *pn = parameter_list_entry_p->parameter_name_p;

        if (parent_p == NULL) {
            fprintf(f,
                    "%s",
                    sep_p
                   );
            fprintf(f,
                    "%s    %s%s(%s%s)",
                    indent,
                    fn ? PFX(fn) : "_",
                    fn ? fn : pn,
                    fn ? PFX(fn) : "_",
                    fn ? fn : pn
                   );
            if (m)
                fprintf(f,
                        ",\n"
                        "%s    num_%s(num_%s)",
                        indent,
                        fn ? fn : pn,
                        fn ? fn : pn
                       );
            sep_p = ",\n";
        }
        else {
            fprintf(f,
                    "%s",
                    sep_p
                   );
            fprintf(f,
                    "%s%s",
                    fn ? PFX(fn) : "_",
                    fn ? fn : pn
                   );
            if (m)
                fprintf(f,
                        ", num_%s",
                        fn ? fn : pn
                       );
            sep_p = ", ";
        }
    }
    free(snu);
}

void api_cc_members_allocate_parameter_list(FILE *f,
                                            char *indent,
                                            mpl_list_t *parameter_list_p,
                                            mpl_list_t *exclude_parameter_list_p,
                                            parameter_set *parameter_set_p,
                                            direction_t dir)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = parameter_set_p->get_short_name();
    char *snu = str_toupper(snl);

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;

        int basic = parameter_p->is_basic();
        int e = parameter_p->is_enum();

        int b = parameter_p->is_bag();
        int t = parameter_p->is_tuple();
        int a = parameter_p->is_array();
        int s = parameter_p->is_string();

        char *fn = parameter_list_entry_p->field_name_p;
        char *pn = parameter_list_entry_p->parameter_name_p;
        int pt = (strstr(parameter_p->get_c_type(), " *") != NULL);

        char *ct;

        if (a || t) {
            int len = strlen(parameter_p->get_c_type()) - 6;
            ct = (char*)calloc(1, len + 1);
            strncpy(ct, (char*)parameter_p->get_c_type() + 4, len);
        }
        else {
            ct = strdup(parameter_p->get_c_type());
        }

        if (!m) {
            if (basic && o && !parameter_p->is_addr()) {
                fprintf(f,
                        "%s    if (this->%s%s != NULL) {\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                if (e) {
                    fprintf(f,
                            "%s        %s_%s_t *__%s%s = (%s_%s_t*)calloc(1, sizeof(%s_%s_t));\n",
                            indent,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p
                           );
                    fprintf(f,
                            "%s        *__%s%s = *this->%s%s;\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                    fprintf(f,
                            "%s        this->%s%s = __%s%s;\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                }
                else {
                    fprintf(f,
                            "%s        %s *__%s%s = (%s*)calloc(1, sizeof(%s));\n",
                            indent,
                            ct,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            ct,
                            ct
                           );
                    fprintf(f,
                            "%s        *__%s%s = *this->%s%s;\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                    fprintf(f,
                            "%s        this->%s%s = __%s%s;\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                }
                fprintf(f,
                        "%s    }\n",
                        indent
                       );
            }
            else if (s) {
                fprintf(f,
                        "%s    if (this->%s%s != NULL) {\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s        %s __%s%s = strdup(this->%s%s);\n",
                        indent,
                        ct,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s        this->%s%s = __%s%s;\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s    }\n",
                        indent
                       );
            }
            else if (b || t || a) {
                fprintf(f,
                        "%s    if (this->%s%s != NULL) {\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s        %s *__%s%s = new %s(*this->%s%s);\n",
                        indent,
                        b ? pn : ct,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        b ? pn : ct,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s        this->%s%s = __%s%s;\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s    }\n",
                        indent
                       );
            }
        }
        else { /* m */
            fprintf(f,
                    "%s    if (this->num_%s != 0) {\n",
                    indent,
                    fn ? fn : pn
                   );
            if (basic) {
                if (e) {
                    fprintf(f,
                            "%s        %s_%s_t **__%s%s = (%s_%s_t**)calloc(this->num_%s, sizeof(%s_%s_t*));\n",
                            indent,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p,
                            fn ? fn : pn,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p
                           );
                    fprintf(f,
                            "%s        for (int i = 0; i < this->num_%s; i++) {\n",
                            indent,
                            fn ? fn : pn
                           );
                    fprintf(f,
                            "%s            __%s%s[i] = (%s_%s_t*)calloc(1, sizeof(%s_%s_t));\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p
                           );
                    fprintf(f,
                            "%s            *__%s%s[i] = *this->%s%s[i];\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                    fprintf(f,
                            "%s        }\n",
                            indent
                           );
                    fprintf(f,
                            "%s        this->%s%s = __%s%s;\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                }
                else if (parameter_p->is_addr()) {
                    fprintf(f,
                            "%s        void **__%s%s = (void**)calloc(this->num_%s, sizeof(void*));\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? fn : pn
                           );
                    fprintf(f,
                            "%s        for (int i = 0; i < this->num_%s; i++) {\n",
                            indent,
                            fn ? fn : pn
                           );
                    fprintf(f,
                            "%s            __%s%s[i] = this->%s%s[i];\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                    fprintf(f,
                            "%s        }\n",
                            indent
                           );
                    fprintf(f,
                            "%s        this->%s%s = __%s%s;\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                }
                else {
                    fprintf(f,
                            "%s        %s **__%s%s = (%s**)calloc(this->num_%s, sizeof(%s*));\n",
                            indent,
                            ct,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            ct,
                            fn ? fn : pn,
                            ct
                           );
                    fprintf(f,
                            "%s        for (int i = 0; i < this->num_%s; i++) {\n",
                            indent,
                            fn ? fn : pn
                           );
                    fprintf(f,
                            "%s            __%s%s[i] = (%s*)calloc(1, sizeof(%s));\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            ct,
                            ct
                           );
                    fprintf(f,
                            "%s            *__%s%s[i] = *this->%s%s[i];\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                    fprintf(f,
                            "%s        }\n",
                            indent
                           );
                    fprintf(f,
                            "%s        this->%s%s = __%s%s;\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                }
            }
            else if (s) {
                    fprintf(f,
                            "%s        char **__%s%s = (char**)calloc(this->num_%s, sizeof(char*));\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? fn : pn
                           );
                    fprintf(f,
                            "%s        for (int i = 0; i < this->num_%s; i++) {\n",
                            indent,
                            fn ? fn : pn
                           );
                fprintf(f,
                        "%s            __%s%s[i] = strdup(this->%s%s[i]);\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s        }\n",
                        indent
                       );
                fprintf(f,
                        "%s        this->%s%s = __%s%s;\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            } else if (b || t || a) {
                    fprintf(f,
                            "%s        %s **__%s%s = (%s**)calloc(this->num_%s, sizeof(%s*));\n",
                            indent,
                            b ? pn : ct,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            b ? pn : ct,
                            fn ? fn : pn,
                            b ? pn : ct
                           );
                    fprintf(f,
                            "%s        for (int i = 0; i < this->num_%s; i++) {\n",
                            indent,
                            fn ? fn : pn
                           );
                fprintf(f,
                        "%s            __%s%s[i] = new %s(*this->%s%s[i]);\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        b ? pn : ct,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s        }\n",
                        indent
                       );
                fprintf(f,
                        "%s        this->%s%s = __%s%s;\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            }
            fprintf(f,
                    "%s    }\n",
                    indent
                   );
        }
        free(ct);
    }
    free(snu);
}

void api_cc_copy_constructor_parameter_list(FILE *f,
                                            char *indent,
                                            mpl_list_t *parameter_list_p,
                                            mpl_list_t *exclude_parameter_list_p,
                                            parameter_set *parameter_set_p,
                                            direction_t dir)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = parameter_set_p->get_short_name();
    char *snu = str_toupper(snl);

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;

        int basic = parameter_p->is_basic();
        int e = parameter_p->is_enum();

        int b = parameter_p->is_bag();
        int t = parameter_p->is_tuple();
        int a = parameter_p->is_array();
        int s = parameter_p->is_string();

        char *fn = parameter_list_entry_p->field_name_p;
        char *pn = parameter_list_entry_p->parameter_name_p;
        int pt = (strstr(parameter_p->get_c_type(), " *") != NULL);

        char *ct;

        if (a || t) {
            int len = strlen(parameter_p->get_c_type()) - 6;
            ct = (char*)calloc(1, len + 1);
            strncpy(ct, (char*)parameter_p->get_c_type() + 4, len);
        }
        else {
            ct = strdup(parameter_p->get_c_type());
        }

        if (!m) {
            if (basic && o && !parameter_p->is_addr()) {
                fprintf(f,
                        "%s    if (obj.%s%s == NULL) {\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s        this->%s%s = NULL;\n"
                        "%s    }\n"
                        "%s    else {\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        indent,
                        indent
                       );
                if (e) {
                    fprintf(f,
                            "%s        this->%s%s = (%s_%s_t*)calloc(1, sizeof(%s_%s_t));\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p
                           );
                    fprintf(f,
                            "%s        *this->%s%s = *obj.%s%s;\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                }
                else {
                    fprintf(f,
                            "%s        this->%s%s = (%s*)calloc(1, sizeof(%s));\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            ct,
                            ct
                           );
                    fprintf(f,
                            "%s        *this->%s%s = *obj.%s%s;\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                }
                fprintf(f,
                        "%s    }\n",
                        indent
                       );
            }
            else if (s) {
                fprintf(f,
                        "%s    if (obj.%s%s == NULL) {\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s        this->%s%s = NULL;\n"
                        "%s    }\n"
                        "%s    else {\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        indent,
                        indent
                       );
                fprintf(f,
                        "%s        this->%s%s = strdup(obj.%s%s);\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s    }\n",
                        indent
                       );
            }
            else if (b || t || a) {
                fprintf(f,
                        "%s    if (obj.%s%s == NULL) {\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s        this->%s%s = NULL;\n"
                        "%s    }\n"
                        "%s    else {\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        indent,
                        indent
                       );
                fprintf(f,
                        "%s        this->%s%s = new %s(*obj.%s%s);\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        b ? pn : ct,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
                fprintf(f,
                        "%s    }\n",
                        indent
                       );
            }
            else { /* basic && !o || addr */
                fprintf(f,
                        "%s    this->%s%s = obj.%s%s;\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            }
        }
        else { /* m */
            fprintf(f,
                    "%s    if (obj.num_%s == 0) {\n"
                    "%s        this->num_%s = 0;\n"
                    "%s        this->%s%s = NULL;\n"
                    "%s    }\n"
                    "%s    else {\n",
                    indent,
                    fn ? fn : pn,
                    indent,
                    fn ? fn : pn,
                    indent,
                    fn ? PFX(fn) : "_",
                    fn ? fn : pn,
                    indent,
                    indent
                   );
            fprintf(f,
                    "%s        this->%s%s = (%s%s*)calloc(obj.num_%s, sizeof(%s%s));\n",
                    indent,
                    fn ? PFX(fn) : "_",
                    fn ? fn : pn,
                    b ? pn : ct,
                    IS_PTR ? "*" : "",
                    fn ? fn : pn,
                    b ? pn : ct,
                    IS_PTR ? "*" : ""
                   );
            fprintf(f,
                    "%s        for (int i = 0; i < obj.num_%s; i++) {\n",
                    indent,
                    fn ? fn : pn
                   );
            if (basic && !parameter_p->is_addr()) {
                if (e) {
                    fprintf(f,
                            "%s            this->%s%s[i] = (%s_%s_t*)calloc(1, sizeof(%s_%s_t));\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p,
                            parameter_list_entry_p->parameter_set_p->name_p,
                            parameter_p->name_p
                           );
                    fprintf(f,
                            "%s            *this->%s%s[i] = *obj.%s%s[i];\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                }
                else {
                    fprintf(f,
                            "%s            this->%s%s[i] = (%s*)calloc(1, sizeof(%s));\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            ct,
                            ct
                           );
                    fprintf(f,
                            "%s            *this->%s%s[i] = *obj.%s%s[i];\n",
                            indent,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn,
                            fn ? PFX(fn) : "_",
                            fn ? fn : pn
                           );
                }
            }
            else if (s) {
                fprintf(f,
                        "%s            this->%s%s[i] = strdup(obj.%s%s[i]);\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            } else if (b || t || a) {
                fprintf(f,
                        "%s            this->%s%s[i] = new %s(*obj.%s%s[i]);\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        b ? pn : ct,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            }
            else { /* basic && !o || addr */
                fprintf(f,
                        "%s            this->%s%s[i] = obj.%s%s[i];\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            }
            fprintf(f,
                    "%s        }\n",
                    indent
                   );
            fprintf(f,
                    "%s    }\n",
                    indent
                   );
        }
        free(ct);
    }
    free(snu);
}

void api_cc_encode_parameter_list(FILE *f,
                                  char *indent,
                                  mpl_list_t *parameter_list_p,
                                  mpl_list_t *exclude_parameter_list_p,
                                  parameter_set *parameter_set_p,
                                  char *bag_name_p,
                                  char *bag_param_name_p,
                                  direction_t dir)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = parameter_set_p->get_short_name();
    char *snu = str_toupper(snl);

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;
        char *fn = parameter_list_entry_p->field_name_p;
        char *pn = parameter_list_entry_p->parameter_name_p;
        int e = parameter_p->is_enum();
        int b = parameter_p->is_bag();
        int t = parameter_p->is_tuple();
        int a = parameter_p->is_array();
        int pt = (strstr(parameter_p->get_c_type(), " *") != NULL);

        char *ct;

        if (a || t) {
            int len = strlen(parameter_p->get_c_type()) - 6;
            ct = (char*)calloc(1, len + 1);
            strncpy(ct, (char*)parameter_p->get_c_type() + 4, len);
        }
        else {
            ct = strdup(parameter_p->get_c_type());
        }

        if (o) {
            fprintf(f,
                    "%s    if (%s%s != NULL) {\n",
                    indent,
                    fn ? PFX(fn) : "_",
                    fn ? fn : pn
                   );
        }
        else {
            fprintf(f,
                    "%s    {\n",
                    indent
                   );
        }
        if (fn) {
            if (!m) {
                if (!b) {
                    fprintf(f,
                            "%s        %s_ADD_%s_%s(&%s, %s%s%s);\n",
                            indent,
                            snu,
                            bag_name_p,
                            fn,
                            bag_param_name_p,
                            IS_PTR ? "*" : "",
                            PFX(fn),
                            fn
                           );
                }
                else {
                    fprintf(f,
                            "%s        mpl_list_t *__bag_p = %s%s->encode();\n",
                            indent,
                            PFX(fn),
                            fn
                           );
                    fprintf(f,
                            "%s        %s_ADD_%s_%s(&%s, __bag_p);\n",
                            indent,
                            snu,
                            bag_name_p,
                            fn,
                            bag_param_name_p
                           );
                    fprintf(f,
                            "%s        mpl_param_list_destroy(&__bag_p);\n",
                            indent
                           );
                }
            }
            else {
                fprintf(f,
                        "%s        int i;\n",
                        indent
                       );
                if (!o) {
                    fprintf(f,
                            "%s        if (num_%s <= 0) {\n"
                            "%s            mpl_param_list_destroy(&%s);\n"
                            "%s            return NULL;\n"
                            "%s        }\n",
                            indent,
                            fn,
                            indent,
                            bag_param_name_p,
                            indent,
                            indent
                           );
                }
                fprintf(f,
                        "%s        for (i = 0; i < num_%s; i++) {\n",
                        indent,
                        fn ? fn : pn
                       );
                if (b) {
                    fprintf(f,
                            "%s            mpl_list_t *__bag_p = %s%s[i]->encode();\n",
                            indent,
                            PFX(fn),
                            fn
                           );
                    fprintf(f,
                            "%s            %s_ADD_%s_%s_TAG(&%s, __bag_p, i+1);\n",
                            indent,
                            snu,
                            bag_name_p,
                            fn,
                            bag_param_name_p
                           );
                    fprintf(f,
                            "%s            mpl_param_list_destroy(&__bag_p);\n"
                            "%s        }\n",
                            indent,
                            indent
                           );
                }
                else {
                    fprintf(f,
                            "%s            %s_ADD_%s_%s_TAG(&%s, %s%s%s[i], i+1);\n"
                            "%s        }\n",
                            indent,
                            snu,
                            bag_name_p,
                            fn,
                            bag_param_name_p,
                            parameter_p->is_basic() && !parameter_p->is_addr() ? "*" : "",
                            PFX(fn),
                            fn,
                            indent
                           );
                }
            }
        }
        else {
            if (!m) {
                if (t) {
                    const char *key_field_str;
                    const char *val_field_str;

                    switch(get_type_of_tuple(parameter_p->get_type())) {
                        case 1:
                            key_field_str = "key_p";
                            val_field_str = "value_p";
                            break;
                        case -1:
                            key_field_str = "key";
                            val_field_str = "value";
                            break;
                        case 2:
                            key_field_str = "key_p";
                            val_field_str = "value";
                        case 8:
                            key_field_str = "key_p";
                            val_field_str = "value";
                            break;
                        default:
                            assert(0);
                    }
                    fprintf(f,
                            "%s        %s_ADD_%s(&%s, %s, %s->tuple.%s, %s->tuple.%s);\n",
                            indent,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn,
                            pn,
                            key_field_str,
                            pn,
                            val_field_str
                           );
                }
                else if (a) {
                    fprintf(f,
                            "%s        %s_ADD_%s(&%s, %s, &_%s->array);\n",
                            indent,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn,
                            pn
                           );
                }
                else {
                    if (!b) {
                        fprintf(f,
                                "%s        %s_ADD_%s%s%s(&%s, %s, %s_%s);\n",
                                indent,
                                snu,
                                parameter_p->get_macro_type(),
                                e ? "_" : "",
                                e ? "FROM_VAR" : "",
                                bag_param_name_p,
                                pn,
                                IS_PTR ? "*" : "",
                                pn
                               );
                    }
                    else {
                        fprintf(f,
                                "%s        mpl_list_t *__bag_p = _%s->encode();\n",
                                indent,
                                pn
                               );
                        fprintf(f,
                                "%s        %s_ADD_%s(&%s, %s, __bag_p);\n",
                                indent,
                                snu,
                                parameter_p->get_macro_type(),
                                bag_param_name_p,
                                pn
                               );
                        fprintf(f,
                                "%s        mpl_param_list_destroy(&__bag_p);\n",
                                indent
                               );
                    }
                }
            }
            else {
                fprintf(f,
                        "%s        int i;\n",
                        indent
                       );
                if (!o) {
                    fprintf(f,
                            "%s        if (num_%s <= 0) {\n"
                            "%s            mpl_param_list_destroy(&%s);\n"
                            "%s            return NULL;\n"
                            "%s        }\n",
                            indent,
                            pn,
                            indent,
                            bag_param_name_p,
                            indent,
                            indent
                           );
                }
                fprintf(f,
                        "%s        for (i = 0; i < num_%s; i++) {\n",
                        indent,
                        fn ? fn : pn
                       );
                if (t) {
                    const char *key_field_str;
                    const char *val_field_str;

                    switch(get_type_of_tuple(parameter_p->get_type())) {
                        case 1:
                            key_field_str = "key_p";
                            val_field_str = "value_p";
                            break;
                        case -1:
                            key_field_str = "key";
                            val_field_str = "value";
                            break;
                        case 2:
                            key_field_str = "key_p";
                            val_field_str = "value";
                        case 8:
                            key_field_str = "key_p";
                            val_field_str = "value";
                            break;
                        default:
                            assert(0);
                    }
                    fprintf(f,
                            "%s            %s_ADD_%s_TAG(&%s, %s, _%s[i]->tuple.%s, _%s[i]->tuple.%s, i+1);\n"
                            "%s        }\n",
                            indent,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn,
                            pn,
                            key_field_str,
                            pn,
                            val_field_str,
                            indent
                           );
                }
                else if (a) {
                    fprintf(f,
                            "%s            %s_ADD_%s_TAG(&%s, %s, &_%s[i]->array, i+1);\n"
                            "%s        }\n",
                            indent,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn,
                            pn,
                            indent
                           );
                }
                else if (b) {
                    fprintf(f,
                            "%s            mpl_list_t *__bag_p = _%s[i]->encode();\n",
                            indent,
                            pn
                           );
                    fprintf(f,
                            "%s            %s_ADD_%s_TAG(&%s, %s, __bag_p, i+1);\n",
                            indent,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn
                           );
                    fprintf(f,
                            "%s            mpl_param_list_destroy(&__bag_p);\n"
                            "%s        }\n",
                            indent,
                            indent
                           );
                }
                else {
                    fprintf(f,
                            "%s            %s_ADD_%s%s%s_TAG(&%s, %s, %s_%s[i], i+1);\n"
                            "%s        }\n",
                            indent,
                            snu,
                            parameter_p->get_macro_type(),
                            e ? "_" : "",
                            e ? "FROM_VAR" : "",
                            bag_param_name_p,
                            pn,
                            parameter_p->is_basic() && !parameter_p->is_addr() ? "*" : "",
                            pn,
                            indent
                           );
                }
            }
        }
        fprintf(f,
                "%s    }\n",
                indent
               );
        free(ct);
    }
    free(snu);
}

void api_decode_parameter_list(FILE *f,
                               char *indent,
                               mpl_list_t *parameter_list_p,
                               mpl_list_t *exclude_parameter_list_p,
                               parameter_set *parameter_set_p,
                               char *bag_name_p,
                               char *bag_param_name_p,
                               direction_t dir)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = parameter_set_p->get_short_name();
    char *snu = str_toupper(snl);

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;
        char *fn = parameter_list_entry_p->field_name_p;
        char *pn = parameter_list_entry_p->parameter_name_p;
        int b = parameter_p->is_bag();
        int e = parameter_p->is_enum();
        int t = parameter_p->is_tuple();
        int a = parameter_p->is_array();
        int s = parameter_p->is_string();
        int pt = (strstr(parameter_p->get_c_type(), " *") != NULL);

        char *ct;

        if (a || t) {
            int len = strlen(parameter_p->get_c_type()) - 6;
            ct = (char*)calloc(1, len + 1);
            strncpy(ct, (char*)parameter_p->get_c_type() + 4, len);
        }
        else {
            ct = strdup(parameter_p->get_c_type());
        }

        if (m) {
            fprintf(f,
                    "%s    num_%s = 0;\n",
                    indent,
                    fn ? fn : pn
                   );
        }
        if ((o && !m) || pt) {
            fprintf(f,
                    "%s    %s%s = NULL;\n",
                    indent,
                    fn ? PFX(fn) : "_",
                    fn ? fn : pn
                   );
        }
        if (fn) {
            fprintf(f,
                    "%s    if (%s_%s_%s_EXISTS(%s)) {\n",
                    indent,
                    snu,
                    bag_name_p,
                    fn,
                    bag_param_name_p
                   );
            if (!m) {
                if (o && !pt) {
                    if (e) {
                        fprintf(f,
                                "%s        %s%s = (%s_%s_t*)calloc(1, sizeof(%s_%s_t));\n",
                                indent,
                                PFX(fn),
                                fn,
                                parameter_list_entry_p->parameter_set_p->name_p,
                                parameter_p->name_p,
                                parameter_list_entry_p->parameter_set_p->name_p,
                                parameter_p->name_p
                               );
                    }
                    else {
                        fprintf(f,
                                "%s        %s%s = (%s*)calloc(1, sizeof(%s%s));\n",
                                indent,
                                PFX(fn),
                                fn,
                                b ? pn : ct,
                                b ? pn : ct,
                                IS_PTR ? "*" : ""
                               );
                    }
                    fprintf(f,
                            "%s        if (%s%s != NULL) ",
                            indent,
                            PFX(fn),
                            fn
                           );
                }
                else {
                    fprintf(f,
                            "%s        ",
                            indent
                           );
                }
                if (b) {
                    fprintf(f,
                            "{\n"
                            "%s            %s%s = new %s(%s_GET_%s_%s_PTR(%s));\n"
                            "%s        }\n",
                            indent,
                            PFX(fn),
                            fn,
                            pn,
                            snu,
                            bag_name_p,
                            fn,
                            bag_param_name_p,
                            indent
                           );
                }
                else if (a || t) {
                    fprintf(f,
                            "{\n"
                            "%s            %s%s = new %s(%s_GET_%s_%s_PTR(%s));\n"
                            "%s        }\n",
                            indent,
                            PFX(fn),
                            fn,
                            ct,
                            snu,
                            bag_name_p,
                            fn,
                            bag_param_name_p,
                            indent
                           );
                }
                else if (s) {
                    fprintf(f,
                            "{\n"
                            "%s            %s%s = strdup(%s_GET_%s_%s_PTR(%s));\n"
                            "%s        }\n",
                            indent,
                            PFX(fn),
                            fn,
                            snu,
                            bag_name_p,
                            fn,
                            bag_param_name_p,
                            indent
                           );
                }
                else {
                    fprintf(f,
                            "{\n"
                            "%s            %s%s%s = %s_GET_%s_%s%s(%s);\n"
                            "%s        }\n",
                            indent,
                            IS_PTR && parameter_p->is_basic() ? "*" : "",
                            PFX(fn),
                            fn,
                            snu,
                            bag_name_p,
                            fn,
                            parameter_p->is_basic() ? "" : "_PTR",
                            bag_param_name_p,
                            indent
                           );
                }
            }
            else {
                fprintf(f,
                        "%s        num_%s = %s_%s_%s_FIELD_COUNT(%s);\n",
                        indent,
                        fn,
                        snu,
                        bag_name_p,
                        fn,
                        bag_param_name_p
                       );
                fprintf(f,
                        "%s        %s%s = (%s%s*)calloc(num_%s, sizeof(%s%s));\n",
                        indent,
                        PFX(fn),
                        fn,
                        b ? pn : ct,
                        IS_PTR ? "*" : "",
                        fn,
                        b ? pn : ct,
                        IS_PTR ? "*" : ""
                       );
                fprintf(f,
                        "%s        if (%s%s != NULL) {\n",
                        indent,
                        PFX(fn),
                        fn
                       );
                fprintf(f,
                        "%s            for (int i = 0; i < num_%s; i++)\n",
                        indent,
                        fn
                       );
                if (b) {
                    fprintf(f,
                            "%s                %s%s[i] = new %s(%s_GET_%s_%s_PTR_TAG(%s, i+1));\n",
                            indent,
                            PFX(fn),
                            fn,
                            pn,
                            snu,
                            bag_name_p,
                            fn,
                            bag_param_name_p
                           );
                }
                else if (a || t) {
                    fprintf(f,
                            "%s                %s%s[i] = new %s(%s_GET_%s_%s_PTR_TAG(%s, i+1));\n",
                            indent,
                            PFX(fn),
                            fn,
                            ct,
                            snu,
                            bag_name_p,
                            fn,
                            bag_param_name_p
                           );
                }
                else if (s) {
                    fprintf(f,
                            "%s                %s%s[i] = strdup(%s_GET_%s_%s_PTR_TAG(%s, i+1));\n",
                            indent,
                            PFX(fn),
                            fn,
                            snu,
                            bag_name_p,
                            fn,
                            bag_param_name_p
                           );
                }
                else {
                    fprintf(f,
                            "%s                %s%s%s[i] = %s_GET_%s_%s%s_TAG(%s, i+1);\n",
                            indent,
                            parameter_p->is_basic() && !parameter_p->is_addr() ? "*" : "",
                            PFX(fn),
                            fn,
                            snu,
                            bag_name_p,
                            fn,
                            parameter_p->is_basic() ? "" : "_PTR",
                            bag_param_name_p
                           );
                }
                fprintf(f,
                        "%s        }\n",
                        indent
                       );
            }
        }
        else {
            fprintf(f,
                    "%s    if (%s_EXISTS(%s, %s)) {\n",
                    indent,
                    snu,
                    bag_param_name_p,
                    pn
                   );
            if (!m) {
                if (o && !pt) {
                    if (e) {
                        fprintf(f,
                                "%s        _%s = (%s_%s_t*)calloc(1, sizeof(%s_%s_t));\n",
                                indent,
                                pn,
                                parameter_list_entry_p->parameter_set_p->name_p,
                                parameter_p->name_p,
                                parameter_list_entry_p->parameter_set_p->name_p,
                                parameter_p->name_p
                               );
                    }
                    else {
                        fprintf(f,
                                "%s        _%s = (%s*)calloc(1, sizeof(%s%s));\n",
                                indent,
                                pn,
                                b ? pn : ct,
                                b ? pn : ct,
                                IS_PTR ? "*" : ""
                               );
                    }
                    fprintf(f,
                            "%s        if (_%s != NULL) ",
                            indent,
                            pn
                           );
                }
                else {
                    fprintf(f,
                            "%s        ",
                            indent
                           );
                }
                if (b) {
                    fprintf(f,
                            "{\n"
                            "%s            _%s = new %s(%s_GET_%s_PTR(%s, %s));\n"
                            "%s        }\n",
                            indent,
                            pn,
                            pn,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn,
                            indent
                           );
                }
                else if (a || t) {
                    fprintf(f,
                            "{\n"
                            "%s            _%s = new %s(%s_GET_%s_PTR(%s, %s));\n"
                            "%s        }\n",
                            indent,
                            pn,
                            ct,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn,
                            indent
                           );
                }
                else if (s) {
                    fprintf(f,
                            "{\n"
                            "%s            _%s = strdup(%s_GET_%s_PTR(%s, %s));\n"
                            "%s        }\n",
                            indent,
                            pn,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn,
                            indent
                           );
                }
                else {
                    fprintf(f,
                            "{\n"
                            "%s            %s_%s = %s_GET_%s%s(%s, %s);\n"
                            "%s        }\n",
                            indent,
                            IS_PTR && parameter_p->is_basic() ? "*" : "",
                            pn,
                            snu,
                            parameter_p->get_macro_type(),
                            parameter_p->is_basic() ? "" : "_PTR",
                            bag_param_name_p,
                            pn,
                            indent
                           );
                }
            }
            else {
                fprintf(f,
                        "%s        num_%s = %s_PARAM_COUNT(%s, %s);\n",
                        indent,
                        pn,
                        snu,
                        bag_param_name_p,
                        pn
                       );
                fprintf(f,
                        "%s        _%s = (%s%s*)calloc(num_%s, sizeof(%s%s));\n",
                        indent,
                        pn,
                        b ? pn : ct,
                        IS_PTR ? "*" : "",
                        pn,
                        b ? pn : ct,
                        IS_PTR ? "*" : ""
                       );
                fprintf(f,
                        "%s        if (_%s != NULL) {\n",
                        indent,
                        pn
                       );
                fprintf(f,
                        "%s            for (int i = 0; i < num_%s; i++)\n",
                        indent,
                        pn
                       );
                if (b) {
                    fprintf(f,
                            "%s                _%s[i] = new %s(%s_GET_%s_PTR_TAG(%s, %s, i+1));\n",
                            indent,
                            pn,
                            pn,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn
                           );
                }
                else if (a || t) {
                    fprintf(f,
                            "%s                _%s[i] = new %s(%s_GET_%s_PTR_TAG(%s, %s, i+1));\n",
                            indent,
                            pn,
                            ct,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn
                           );
                }
                else if (s) {
                    fprintf(f,
                            "%s                _%s[i] = strdup(%s_GET_%s_PTR_TAG(%s, %s, i+1));\n",
                            indent,
                            pn,
                            snu,
                            parameter_p->get_macro_type(),
                            bag_param_name_p,
                            pn
                           );
                }
                else {
                    fprintf(f,
                            "%s                %s_%s[i] = %s_GET_%s%s_TAG(%s, %s, i+1);\n",
                            indent,
                            parameter_p->is_basic() && !parameter_p->is_addr() ? "*" : "",
                            pn,
                            snu,
                            parameter_p->get_macro_type(),
                            parameter_p->is_basic() ? "" : "_PTR",
                            bag_param_name_p,
                            pn
                           );
                }
                fprintf(f,
                        "%s        }\n",
                        indent
                       );
            }
        }
        fprintf(f,
                "%s    }\n",
                indent
               );
        free(ct);
    }
    free(snu);
}


void api_free_decoded_parameter_list(FILE *f,
                                     char *indent,
                                     mpl_list_t *parameter_list_p,
                                     mpl_list_t *exclude_parameter_list_p,
                                     parameter_set *parameter_set_p,
                                     direction_t dir)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = parameter_set_p->get_short_name();
    char *snu = str_toupper(snl);

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;

        int basic = parameter_p->is_basic();
        int e = parameter_p->is_enum();

        int b = parameter_p->is_bag();
        int t = parameter_p->is_tuple();
        int a = parameter_p->is_array();
        int s = parameter_p->is_string();

        char *fn = parameter_list_entry_p->field_name_p;
        char *pn = parameter_list_entry_p->parameter_name_p;

        if (!m) {
            if (b || t || a) {
                fprintf(f,
                        "%s    delete %s%s;\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            }
            else if ((o || s) && !parameter_p->is_addr()) {
                fprintf(f,
                        "%s    free(%s%s);\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            }
        }
        else { /* m */
            fprintf(f,
                    "%s    for (int i = 0; i < num_%s; i++) {\n",
                    indent,
                    fn ? fn : pn
                   );
            if (b || t || a) {
                fprintf(f,
                        "%s        delete %s%s[i];\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            }
            else if ((o || s) && !parameter_p->is_addr()) {
                fprintf(f,
                        "%s        free(%s%s[i]);\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            }
            fprintf(f,
                    "%s    }\n",
                    indent
                   );
            fprintf(f,
                    "%s    free(%s%s);\n",
                    indent,
                    fn ? PFX(fn) : "_",
                    fn ? fn : pn
                   );
        }
    }
    free(snu);
}

void api_cc_print_parameter_list(FILE *f,
                                     char *indent,
                                     mpl_list_t *parameter_list_p,
                                     mpl_list_t *exclude_parameter_list_p,
                                     parameter_set *parameter_set_p,
                                     direction_t dir)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = parameter_set_p->get_short_name();
    char *snu = str_toupper(snl);

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;
        char *fn = parameter_list_entry_p->field_name_p;
        char *pn = parameter_list_entry_p->parameter_name_p;
        int b = parameter_p->is_bag();

        fprintf(f,
                "%s    printf(\"%s%s\");\n",
                indent,
                fn ? PFX(fn) : "_",
                fn ? fn : pn
               );
        if (o) {
            fprintf(f,
                    "%s    if (%s%s)\n"
                    "%s        printf(\" (present)\");\n",
                    indent,
                    fn ? PFX(fn) : "_",
                    fn ? fn : pn,
                    indent
                   );
        }
        if (m) {
            fprintf(f,
                    "%s    printf(\" (%%d occurences)\", num_%s);\n",
                    indent,
                    fn ? fn : pn
                   );
        }
        fprintf(f,
                "%s    printf(\"\\n\");\n",
                indent
               );
        if (b) {
            if (!m) {
                fprintf(f,
                        "%s    if (%s%s)\n"
                        "%s        %s%s->print();\n",
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn,
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            }
            else {
                fprintf(f,
                        "%s    for (int i = 0; i < num_%s; i++)\n"
                        "%s        %s%s[i]->print();\n",
                        indent,
                        fn ? fn : pn,
                        indent,
                        fn ? PFX(fn) : "_",
                        fn ? fn : pn
                       );
            }
        }
    }
    free(snu);
}

void dox_doc_f(FILE *f, mpl_list_t *doc_list_p)
{
    mpl_list_t *tmp_p;
    doc *doc_p;

    MPL_LIST_FOR_EACH(doc_list_p, tmp_p) {
        doc_p = LISTABLE_PTR(tmp_p, doc);
        if (doc_p && doc_p->is_forward) {
            doc_p->dox(f);
        }
    }
}

void dox_doc_b(FILE *f, mpl_list_t *doc_list_p)
{
    mpl_list_t *tmp_p;
    doc *doc_p;

    MPL_LIST_FOR_EACH(doc_list_p, tmp_p) {
        doc_p = LISTABLE_PTR(tmp_p, doc);
        if (doc_p && !doc_p->is_forward) {
            doc_p->dox(f);
        }
    }
}

void dox_files(FILE *f, mpl_list_t *files_p)
{
    mpl_list_t *tmp_p;
    file *file_p;

    MPL_LIST_FOR_EACH(files_p, tmp_p) {
        file_p = LISTABLE_PTR(tmp_p, file);
        dox_doc_f(f, file_p->doc_list_p);
        fprintf(f, "\n");
    }
}

void dox_parameter_list(FILE *f, mpl_list_t *parameter_list_p, direction_t dir)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    const char *sep_p;

    sep_p = "";

    if (parameter_list_p == NULL) {
        fprintf(f,
               "void"
              );
        return;
    }

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        fprintf(f,
                "%s%s_t",
                sep_p,
                parameter_list_entry_p->parameter_name_p
               );
        fprintf(f,
                "%s",
                parameter_list_entry_p->direction ? "&" : ""
               );
        fprintf(f,
                "%s",
                parameter_list_entry_p->optional ? "*" : ""
               );
        fprintf(f,
                "%s",
                parameter_list_entry_p->multiple ? "[]" : ""
               );
        fprintf(f,
                " %s",
                parameter_list_entry_p->parameter_name_p
               );
        sep_p = ",";
    }
}

void latex_parameter_list(FILE *f,
                          mpl_list_t *parameter_list_p,
                          mpl_list_t *exclude_parameter_list_p,
                          direction_t dir,
                          parameter_set *default_parameter_set_p,
                          const char *header_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    int o;
    int m;
    int in;
    int out;
    int psdef;

    lx_table_begin(f, 5);
    LX_HLINE;
    LX_TABH(5, "%s",
            header_p
           );
    LX_COL_END;

    LX_HLINE;
    LX_COLH(Parameter);
    LX_COL_SEP;
    LX_COLH(Field);
    LX_COL_SEP;
    LX_COLH(Properties);
    LX_COL_SEP;
    LX_COLH(Type);
    LX_COL_SEP;
    LX_COLH(Description);
    LX_COL_END;
    LX_HLINE;

    if (parameter_list_p == NULL) {
        fprintf(f,
                "\\multicolumn{5}{|l|}{No parameters}"
               );
        LX_COL_END;
    }

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        o = parameter_list_entry_p->optional;
        m = parameter_list_entry_p->multiple;
        in = (parameter_list_entry_p->direction == direction_in) ||
             (parameter_list_entry_p->direction == direction_inout);
        out = (parameter_list_entry_p->direction == direction_inout) ||
              (parameter_list_entry_p->direction == direction_out);
        psdef = (parameter_list_entry_p->parameter_set_p == default_parameter_set_p);

        fprintf_latex(f,
                      "%s%s%s",
                      psdef ? "" : parameter_list_entry_p->parameter_set_p->name_p,
                      psdef ? "" : "::",
                      parameter_list_entry_p->parameter_name_p
                     );
        fprintf(f,
                " \\ref{%s:%s}",
                parameter_list_entry_p->parameter_set_p->name_p,
                parameter_list_entry_p->parameter_name_p
               );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      parameter_list_entry_p->field_name_p ? parameter_list_entry_p->field_name_p : "-"
                     );
        LX_COL_SEP;
        fprintf(f,
                "%s%s%s%s%s%s",
                in ? "in" : "",
                out ? "out" : "",
                o ? "," : "",
                o ? "optional" : "",
                o && m ? "," : "",
                m ? "multiple" : ""
               );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      parameter_p->get_type()
                     );
        LX_COL_SEP;
        latex_latex_list(f, parameter_list_entry_p->latex_list_p);
        LX_COL_END;
    }

    LX_HLINE;
    LX_TAB_END;
}

void lx_table_begin(FILE *f, int cols)
{
    fprintf(f,
            "\\begin{tabular}{"
           );
    for (int i = 0; i < (cols-1); i++) {
        fprintf(f,
                "|"
               );
        LX_NORMAL_COL(cols);
    }

    fprintf(f,
            "|"
           );
    LX_DESCRIPTION_COL(cols);
    fprintf(f,
            "|}\n"
           );
}

int parameter_property_in_list(char *name_p, const char *property_p, mpl_list_t *parameter_list_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    if (parameter_list_p == NULL) {
        return 0;
    }

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if (parameter_list_entry_p->field_name_p) {
            if (strcmp(parameter_list_entry_p->field_name_p, name_p))
                continue;
        }
        else {
            if (strcmp(parameter_list_entry_p->parameter_name_p, name_p))
                continue;
        }

        if (!strcmp(property_p, "optional"))
            return parameter_list_entry_p->optional;
        if (!strcmp(property_p, "direction"))
            return parameter_list_entry_p->direction;
        if (!strcmp(property_p, "multiple"))
            return parameter_list_entry_p->multiple;
        if (!strcmp(property_p, "present"))
            return 1;
    }

    return 0;
}

int parameter_list_entry_is_in_list(parameter_list_entry *entry_p,
                                    mpl_list_t *parameter_list_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    if (parameter_list_p == NULL) {
        return 0;
    }

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if (strcmp(parameter_list_entry_p->parameter_name_p, entry_p->parameter_name_p))
            continue;
        if ((parameter_list_entry_p->field_name_p == NULL) &&
            (entry_p->field_name_p == NULL))
            return 1;
        if (!strcmp(parameter_list_entry_p->field_name_p, entry_p->field_name_p))
            return 1;
    }

    return 0;
}
parameter_list_entry *find_parameter_entry_in_list(mpl_list_t *parameter_list_p,
                                                   char *parameter_set_name_p,
                                                   char *parameter_name_p,
                                                   char *field_name_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if (!strcmp(parameter_list_entry_p->parameter_name_p,
                    parameter_name_p) &&
            !strcmp(parameter_list_entry_p->parameter_set_p->name_p,
                    parameter_set_name_p) &&
            (((field_name_p == NULL) && (parameter_list_entry_p->field_name_p == NULL)) ||
             (((field_name_p != NULL) && (parameter_list_entry_p->field_name_p != NULL)) &&
              !strcmp(parameter_list_entry_p->field_name_p,
                      field_name_p))))
            return parameter_list_entry_p;
    }
    return NULL;
}

parameter_list_entry *find_parameter_entry_in_list(mpl_list_t *parameter_list_p,
                                                   char *name_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if (parameter_list_entry_p->field_name_p) {
            if (!strcmp(parameter_list_entry_p->field_name_p,
                        name_p))
                return parameter_list_entry_p;
        }
        else {
            if (!strcmp(parameter_list_entry_p->parameter_name_p,
                        name_p))
                return parameter_list_entry_p;
        }
    }
    return NULL;
}

void print_parameter_list(int level, mpl_list_t *parameter_list_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_list_entry_p->print(level);
    }
}

int get_type_of_int(const char *type_p)
{
    if (!strcmp(type_p, "int"))
        return 1;

    if (!strncmp(type_p, "uint", 4) && !strstr(type_p, "array"))
        return atoi(type_p + 4);

    if (!strncmp(type_p, "sint", 4))
        return (-atoi(type_p + 4));

    return 0;
}

int get_type_of_bool(const char *type_p)
{
    if (!strcmp(type_p, "bool"))
        return 1;

    if (!strcmp(type_p, "bool8"))
        return 8;

    return 0;
}

int get_type_of_enum(const char *type_p)
{
    if (!strcmp("enum", type_p))
        return 1;

    if (!strncmp("enum", type_p, 4))
        return atoi(type_p + 4);

    if (!strncmp("signed_enum", type_p, 11))
        return (- atoi(type_p + 11));

    return 0;
}

int get_type_of_string(const char *type_p)
{
    if (!strcmp(type_p, "string"))
        return 1;

    if (!strcmp(type_p, "wstring"))
        return 2;

    return 0;
}

int get_type_of_tuple(const char *type_p)
{
    if (!strcmp(type_p, "string_tuple"))
        return 1;

    if (!strcmp(type_p, "int_tuple"))
        return -1;

    if (!strcmp(type_p, "strint_tuple"))
        return 2;

    if (!strcmp(type_p, "struint8_tuple"))
        return 8;

    return 0;
}

int get_type_of_array(const char *type_p)
{
    if (!strcmp(type_p, "uint8_array"))
        return 8;

    if (!strcmp(type_p, "uint16_array"))
        return 16;

    if (!strcmp(type_p, "uint32_array"))
        return 32;

    return 0;
}

int get_type_of_addr(const char *type_p)
{
    if (!strcmp(type_p, "addr"))
        return sizeof(void*);

    return 0;
}

int get_type_of_bag(const char *type_p)
{
    if (!strcmp(type_p, "bag"))
        return 1;

    return 0;
}


void print_enum_values(int level, mpl_list_t *enum_values_p)
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;

    MPL_LIST_FOR_EACH(enum_values_p, tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        printf("%s", spacing(level));
        printf("-> %s = ", enum_value_p->name_p);
        if (enum_value_p->value_p)
            printf("%" PRIi64" \n", *enum_value_p->value_p);
        else
            printf("NULL\n");

        print_doc_list(level, enum_value_p->doc_list_p);
    }
}

void print_doc_list(int level, mpl_list_t *doc_list_p)
{
    mpl_list_t *tmp_p;
    doc *doc_p;

    MPL_LIST_FOR_EACH(doc_list_p, tmp_p) {
        doc_p = LISTABLE_PTR(tmp_p, doc);
        if (doc_p)
            doc_p->print(level);
    }
}

void print_files(mpl_list_t *files_p)
{
    mpl_list_t *tmp_p;
    file *file_p;

    MPL_LIST_FOR_EACH(files_p, tmp_p) {
        file_p = LISTABLE_PTR(tmp_p, file);
        printf("file\n");
        printf("->name_p = %s\n", file_p->filename_p);
        print_doc_list(0, file_p->doc_list_p);
    }
}

int convert_int(const char* value_str, int *value_p)
{
  char* endp;

  /* Empty strings are not allowed */
  if (value_str == NULL || 0 == strlen(value_str))
  {
    return -1;
  }

  *value_p = strtol( value_str, &endp, 0 );

  /* The whole string should be a number */
  if (*endp != '\0')
  {
    return (-1);
  }

  return (0);
}

const char *spacing(int level)
{
    switch(level){
        case 0:
            return "";
        case 1:
            return "    ";
        case 2:
            return "        ";
        case 3:
            return "            ";
        case 4:
            return "                ";
        default:
            return "                    ";
    }
    return "";
}

char *trimstring(char *s, char escape)
{
  char *p = s;
  char *e;

  // Trim start
  while (strlen(s) && isspace(s[0]))
    s++;

  // Trim end, taking escape into account
  p = s + (strlen(s) - 1);

  while ((p > s) && isspace(*p)) {
    // did we find an escaped character?
    e = p;
    while (escape && e && (e > s) && (*(e-1) == escape))
      e--;

    // An odd number of escapes in a row? (then it is escaped)
    if (1 == ((p - e) % 2))
      break;

    *p = '\0';
    p--;
  }

  return s;
}

mpl_list_t *doc_list_clone(mpl_list_t *src_list_p)
{
    mpl_list_t *tmp_p;
    doc *src_obj_p;
    doc *dst_obj_p;
    mpl_list_t *dst_list_p = NULL;

    MPL_LIST_FOR_EACH(src_list_p, tmp_p) {
        src_obj_p = LISTABLE_PTR(tmp_p, doc);
        dst_obj_p = src_obj_p->clone();
        dst_obj_p->append_to(dst_list_p);
    }
    return dst_list_p;
}

mpl_list_t *listable_object_list_clone(mpl_list_t *src_list_p)
{
    mpl_list_t *tmp_p;
    listable_object *src_obj_p;
    listable_object *dst_obj_p;
    mpl_list_t *dst_list_p = NULL;

    MPL_LIST_FOR_EACH(src_list_p, tmp_p) {
        src_obj_p = LISTABLE_PTR(tmp_p, listable_object);
        dst_obj_p = src_obj_p->clone();
        dst_obj_p->append_to(dst_list_p);
    }
    return dst_list_p;
}

mpl_list_t *object_container_list_clone(mpl_list_t *src_list_p)
{
    mpl_list_t *tmp_p;
    object_container *src_container_p;
    object_container *dst_container_p;
    mpl_list_t *dst_list_p = NULL;

    MPL_LIST_FOR_EACH(src_list_p, tmp_p) {
        src_container_p = LISTABLE_PTR(tmp_p, object_container);
        dst_container_p = src_container_p->clone();
        dst_container_p->append_to(dst_list_p);
    }
    return dst_list_p;
}


/* Local functions */

static char *strchr_escape(char *s, char c, char escape)
{
  char *p = s;
  char *e;

  while (p && strlen(p)) {
    p = strchr(p, c);
    // did we find an escaped character?
    e = p;
    while (e && (e > s) && (*(e-1) == escape))
      e--;
    if (1 == ((p - e) % 2)) {
      // An odd number of escapes in a row: yes
      p++;
      continue;
    }
    else
      break;
  }
  return p;
}

static void usage(char *name)
{
    fprintf(stderr, "Usage: %s [options] <yourfile>.mpl\n", name);
    fprintf(stderr,
            "Options: -m <mode> (mode=mpl|cli|dejagnu|cpp|latex) (default mpl)\n"
            "         -i <include-dir>\n"
            "         -d <output-dir>\n"
            "         -o <output-filename-without-suffix> (default <yourfile>)\n"
            "\n"
            "  Modes: mpl     - generate support code for parameter sets and categories (C)\n"
            "         cli     - generate support code for command line interface (C)\n"
            "         api     - generate support code for API (C++)\n"
            "         dejagnu - generate support code for dejagnu test system (TCL)\n"
            "         latex   - generate documentation (latex, suitable for latex2html)\n"
           );
}


static char *get_matching_close_bracket(char open_bracket, char close_bracket, char *str_p, char escape)
{
    int num_open = 0;
    char *p;

    p = str_p;

    if (*p != open_bracket)
        return NULL;

    while (*p) {
        if (*p == escape) {
            p++;
            continue;
        }

        if (*p == open_bracket)
            num_open++;
        if (*p == close_bracket)
            num_open--;
        if (num_open == 0)
            return p;
        p++;
    }

    return NULL;
}



static int get_args(arg_t *args,
                    int args_len,
                    char *buf,
                    char equal,
                    char delimiter,
                    char escape)
{
  char *p;
  char *kp;
  char *vp;
  int i = 0;
  int buflen;

  p = trimstring(buf, escape);

  buflen = strlen(p);

  while (((p - buf) < buflen) && (i < args_len))
  {
    char *mid;
    char *end;
    char end_copy;

    end = strchr_escape( p, delimiter, escape);

    if (NULL == end)
    {
      end = p + strlen(p);
      end_copy = *end;
    }
    else
    {
      end_copy = *end;
      *end = '\0';
    }

    mid = strchr_escape( p, equal, escape );
    if (NULL != mid)
    {
      *mid = '\0';
      kp = trimstring(p, escape);
      args[i].key_p = kp;
      vp = trimstring(mid + 1, escape);
      if (*vp == '{')
      {
          *end = end_copy;
          end = get_matching_close_bracket('{', '}', vp, escape);
          if (NULL == end)
              end = vp + strlen(vp);
          else
              end++;
              *end = '\0';
      }
      args[i].value_p = vp;
    }
    else
    {
      args[i].key_p = trimstring(p, escape);
      args[i].value_p = NULL;
    }

    while ((((end+1) - buf) < buflen) &&
           *(end+1) == delimiter)
      end++;

    p = end + 1;
    i++;
  }

  if ((p - buf) < buflen)
    return -1;
  else
    return i;
}


static int fill_escape(char *str, int size, char delimiter, char escape)
{
    char *s;
    char *d;
    char *src = strdup(str);

    s = src;
    d = str;
    while (*s) {
        if (*s == delimiter) {
            *d = escape;
            d++;
        }
        *d = *s;
        d++;
        s++;
        if ((d - str) > size)
            break;
    }
    *d = '\0';
    free(src);
    return (d - str);
}

int vfprintf_escape(char chars[], int num_chars, char escape,
                    FILE *f, const char *format, va_list ap)
{
    int n;
    int i;
    int size_needed;
    char *tmpstr;
    int num_escapes_needed;
    char *p;
    va_list myap;

    va_copy(myap, ap);
    n = vsnprintf(NULL, 0, format, myap);
    va_end(myap);
    if (n < 0)
        return n;

    tmpstr = (char*) malloc(n + 1);

    va_copy(myap, ap);
    n = vsnprintf(tmpstr, n + 1, format, myap);
    va_end(myap);
    assert(n > 0);

    num_escapes_needed = 0;
    for (i = 0; i < num_chars; i++) {
        p = tmpstr;
        while (p && strlen(p)) {
            p = strchr(p, chars[i]);
            if (!p)
                break;
            num_escapes_needed++;
            p++;
        }
    }
    size_needed = n + num_escapes_needed;

    free(tmpstr);
    tmpstr = (char*) malloc(size_needed + 1);

    va_copy(myap, ap);
    n = vsnprintf(tmpstr, size_needed + 1, format, myap);
    va_end(myap);
    assert(n > 0);
    if (num_escapes_needed) {
        for (i = 0; i < num_chars; i++) {
            n = fill_escape(tmpstr, size_needed + 1, chars[i], escape);
        }
    }

    fprintf(f, "%s", tmpstr);
    free(tmpstr);
    return n;
}

int vsprintf_escape(char chars[], int num_chars, char escape,
                    char **str, const char *format, va_list ap)
{
    int n;
    int i;
    int size_needed;
    char *tmpstr;
    int num_escapes_needed;
    char *p;
    va_list myap;

    va_copy(myap, ap);
    n = vsnprintf(NULL, 0, format, myap);
    va_end(myap);
    if (n <= 0)
        return n;

    tmpstr = (char*) malloc(n + 1);

    va_copy(myap, ap);
    n = vsnprintf(tmpstr, n + 1, format, myap);
    va_end(myap);
    assert(n > 0);

    num_escapes_needed = 0;
    for (i = 0; i < num_chars; i++) {
        p = tmpstr;
        while (p && strlen(p)) {
            p = strchr(p, chars[i]);
            if (!p)
                break;
            num_escapes_needed++;
            p++;
        }
    }
    size_needed = n + num_escapes_needed;

    free(tmpstr);
    tmpstr = (char*) malloc(size_needed + 1);

    va_copy(myap, ap);
    n = vsnprintf(tmpstr, size_needed + 1, format, myap);
    va_end(myap);
    assert(n > 0);
    if (num_escapes_needed) {
        for (i = 0; i < num_chars; i++) {
            n = fill_escape(tmpstr, size_needed + 1, chars[i], escape);
        }
    }

    *str = tmpstr;
    return n;
}

int sprintf_alloc_help(char **str, const char *format, ...)
{
    char chars[] = {'\"'};
    int num_chars = sizeof(chars);
    va_list ap;
    char *s;
    char *s2;
    int n;

    va_start(ap, format);
    n = vsprintf_escape(chars, num_chars, '\\', &s, format, ap);
    va_end(ap);
    if (n <= 0)
        return n;
    s2 = strdup(trimstring(s,0));
    if (strlen(s2)) {
        *str = s2;
        free(s);
        return n;
    }
    else {
        if (s2 != NULL)
            free(s2);
        free(s);
        return 0;
    }
}

int fprintf_dox(FILE *f, const char *format, ...)
{
    char chars[] = {'<','#'};
    int num_chars = sizeof(chars);
    va_list ap;

    va_start(ap, format);
    vfprintf_escape(chars, num_chars, '\\', f, format, ap);
    va_end(ap);
}

int fprintf_latex(FILE *f, const char *format, ...)
{
    /* Escape everything except backslash (useful for e.g. \\newline) */
    char chars[] = {'#', '_', '{', '}', '%', '&', '$', '^', '~'};
    int num_chars = sizeof(chars);
    va_list ap;

    va_start(ap, format);
    vfprintf_escape(chars, num_chars, '\\', f, format, ap);
    va_end(ap);
}



static void add_compiler_define(mpl_list_t **compiler_defines_pp, char *compiler_define_text_p)
{
    arg_t *args_p;
    char *tmp_buf_p;
    int i;
    int numargs;
#define MAX_ARGS 100

    args_p = (arg_t*) malloc(sizeof(arg_t)*MAX_ARGS);
    memset(args_p,0,sizeof(arg_t)*MAX_ARGS);

    tmp_buf_p = strdup(compiler_define_text_p);

    numargs = get_args(args_p,
                       MAX_ARGS,
                       tmp_buf_p,
                       '=',
                       ',',
                       '\\');

    for (i = 0; i < numargs; i++)
    {
        compiler_define *compiler_define_p;

        if (args_p[i].value_p)
            compiler_define_p =
                new compiler_define(compiler_p,
                                    strdup(args_p[i].key_p),
                                    strdup(args_p[i].value_p));
        else
            compiler_define_p =
                new compiler_define(compiler_p,
                                    strdup(args_p[i].key_p), NULL);

        compiler_define_p->add_to(compiler_defines_pp);
    }
    free(tmp_buf_p);
    free(args_p);
}

static const char *code_segment_string(code_segment_t code_segment)
{
    switch (code_segment){
#define CODE_SEGMENT_VALUE_ELEMENT(ELEMENT) \
        case code_segment_##ELEMENT: \
        return #ELEMENT; \

    CODE_SEGMENTS
    }
    return "<unknown>";
}

void dox_dox_list_f(FILE *f, mpl_list_t *dox_list_p)
{
    if (dox_list_p) {
        dox_entry *dox_entry_p;
        dox_entry_p = LISTABLE_PTR(dox_list_p, dox_entry);
        if (dox_entry_p->is_forward)
            dox_dox_list(f, dox_list_p, 1);
    }
}

void dox_dox_list_b(FILE *f, mpl_list_t *dox_list_p)
{
    if (dox_list_p) {
        dox_entry *dox_entry_p;
        dox_entry_p = LISTABLE_PTR(dox_list_p, dox_entry);
        if (!dox_entry_p->is_forward)
            dox_dox_list(f, dox_list_p, 0);
    }
}

static void dox_dox_list(FILE *f, mpl_list_t *dox_list_p, int is_forward)
{
    mpl_list_t *tmp_p;
    dox_entry *dox_entry_p;

    if (mpl_list_len(dox_list_p) == 0)
        return;

    fprintf(f,
            "/**%s\n",
            is_forward ? "" : "<"
           );
    MPL_LIST_FOR_EACH(dox_list_p, tmp_p) {
        dox_entry_p = LISTABLE_PTR(tmp_p, dox_entry);
        if (dox_entry_p->label_p && strlen(dox_entry_p->label_p))
            fprintf_dox(f,
                        "  * %s %s\n",
                        dox_entry_p->label_p,
                        dox_entry_p->value_p ? dox_entry_p->value_p : ""
                       );
        else if (dox_entry_p->value_p && strlen(dox_entry_p->value_p))
            fprintf_dox(f,
                        "  * %s\n",
                        dox_entry_p->value_p ? dox_entry_p->value_p : ""
                       );
    }
    fprintf(f,
            "  */\n"
           );
}

void latex_latex_list(FILE *f, mpl_list_t *latex_list_p)
{
    mpl_list_t *tmp_p;
    latex_entry *latex_entry_p;

    if (!latex_list_p || (mpl_list_len(latex_list_p) == 0))
        return;

    MPL_LIST_FOR_EACH(latex_list_p, tmp_p) {
        latex_entry_p = LISTABLE_PTR(tmp_p, latex_entry);
        if (latex_entry_p->label_p && strlen(latex_entry_p->label_p)) {
            /* Label already latex formatted */
            fprintf(f,
                    "%s",
                    latex_entry_p->label_p
                   );
            fprintf_latex(f,
                          "%s",
                          latex_entry_p->value_p ? latex_entry_p->value_p : ""
                         );
        }
        else {
            fprintf_latex(f,
                          "%s\n",
                          latex_entry_p->value_p ? latex_entry_p->value_p : ""
                         );
        }
    }
}

void help_help_list(ostream &os,
                    mpl_list_t *help_list_p,
                    const char *indent,
                    const char *initial_indent)
{
    mpl_list_t *tmp_p;
    help_entry *help_entry_p;
    int nothing_written = 1;

    if (!help_list_p || (mpl_list_len(help_list_p) == 0)) {
        os << "\\n";
        return;
    }

    MPL_LIST_FOR_EACH(help_list_p, tmp_p) {
        help_entry_p = LISTABLE_PTR(tmp_p, help_entry);
        char *s;
        int written = 0;
        const char *ind = indent;

        if (sprintf_alloc_help(&s, "%s", help_entry_p->label_p) > 0) {
            if (initial_indent) {
                os << initial_indent;
                initial_indent = NULL;
            }
            else if (ind)
                os << ind;
            os << s << " ";
            free(s);
            written = 1;
            indent = NULL;
        }
        if (sprintf_alloc_help(&s, "%s", help_entry_p->value_p) > 0) {
            if (initial_indent) {
                os << initial_indent;
                initial_indent = NULL;
            }
            else if (ind)
                os << ind;
            os << s;
            free(s);
            written = 1;
        }
        if (written) {
            os << "\\n";
            nothing_written = 0;
        }
    }
    if (nothing_written)
        os << "\\n";
}

void help_parameter_list(ostream &os,
                         mpl_list_t *parameter_list_p,
                         mpl_list_t *exclude_parameter_list_p,
                         direction_t dir,
                         parameter_set *default_parameter_set_p,
                         const char *header_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    int o;
    int m;
    int in;
    int out;
    int psdef;

    os << header_p;
    os << "\\n";

    if (parameter_list_p == NULL) {
        os << "  No parameters\\n";
    }

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_list_entry_is_in_list(parameter_list_entry_p,
                                            exclude_parameter_list_p))
            continue;

        if ((dir != direction_none) &&
            ((parameter_list_entry_p->direction != dir) &&
             (parameter_list_entry_p->direction != direction_inout)))
            continue;

        o = parameter_list_entry_p->optional;
        m = parameter_list_entry_p->multiple;
        in = (parameter_list_entry_p->direction == direction_in) ||
             (parameter_list_entry_p->direction == direction_inout);
        out = (parameter_list_entry_p->direction == direction_inout) ||
              (parameter_list_entry_p->direction == direction_out);
        psdef = (parameter_list_entry_p->parameter_set_p == default_parameter_set_p);

        os << "  - ";
        os << (psdef ? "" : parameter_list_entry_p->parameter_set_p->name_p);
        os << (psdef ? "" : "::");
        os << (parameter_list_entry_p->field_name_p ? parameter_list_entry_p->field_name_p : parameter_list_entry_p->parameter_name_p);
        os << " (";
        os << (o ? "optional" : "");
        os << (o && m ? "," : "");
        os << (m ? "multiple" : "");
        os << (o || m ? "," : "");
        os << parameter_p->get_type();
        os << "): ";
        help_help_list(os, parameter_list_entry_p->help_list_p, "    ", "");
    }
}

static void print_x_list(int level, mpl_list_t *x_list_p, const char *label_p)
{
    mpl_list_t *tmp_p;
    x_entry *x_entry_p;

    if (x_list_p == NULL)
        return;

    printf("%s", spacing(level));
    printf("->%s\n", label_p);
    MPL_LIST_FOR_EACH(x_list_p, tmp_p) {
        x_entry_p = LISTABLE_PTR(tmp_p, x_entry);
        printf("%s", spacing(level+1));
        printf("->target_object_type_p = '%s'\n", x_entry_p->target_object_type_p);
        printf("%s", spacing(level+1));
        printf("->label_p = '%s'\n", x_entry_p->label_p);
        printf("%s", spacing(level+1));
        printf("->value_p = '%s'\n", x_entry_p->value_p);
    }
}

void print_dox_list(int level, mpl_list_t *dox_list_p, const char *label_p)
{
    if (label_p == NULL)
        print_x_list(level, dox_list_p, "dox_list");
    else
        print_x_list(level, dox_list_p, label_p);
}

void print_latex_list(int level, mpl_list_t *latex_list_p, const char *label_p)
{
    if (label_p == NULL)
        print_x_list(level, latex_list_p, "latex_list");
    else
        print_x_list(level, latex_list_p, label_p);
}

void print_help_list(int level, mpl_list_t *help_list_p, const char *label_p)
{
    if (label_p == NULL)
        print_x_list(level, help_list_p, "help_list");
    else
        print_x_list(level, help_list_p, label_p);
}


int parameter_list_has_mandatory_elements(mpl_list_t *parameter_list_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if (!parameter_list_entry_p->optional)
            return 1;
    }
    return 0;
}

int parameter_list_has_bag_elements(mpl_list_t *parameter_list_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);
        if (get_type_of_bag(parameter_p->get_type()))
            return 1;
    }
    return 0;
}


int check_paramlist_parameters(mpl_list_t *parameter_list_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    int ret = 0;

    MPL_LIST_FOR_EACH(parameter_list_p, tmp_p) {
        parameter *parameter_p;
        parameter_set *ps_p;

        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        ps_p = parameter_list_entry_p->parameter_set_p;
        if (!(parameter_p = ps_p->find_parameter(parameter_list_entry_p->parameter_name_p))){
            fprintf(stderr, "Parameter %s does not exist in parameter set %s\n",
                    parameter_list_entry_p->parameter_name_p,
                    ps_p->name_p);
            ret++;
        }
        else {
            if (parameter_p->is_virtual) {
                mpl_list_t *tmp_p;
                int non_virtual_child_found = 0;
                mpl_list_t *fcl_p = parameter_p->get_flat_child_list();
                MPL_LIST_FOR_EACH(fcl_p, tmp_p) {
                    object_container *container_p = LISTABLE_PTR(tmp_p, object_container);
                    parameter *child_p = (parameter*) container_p->object_p;
                    if (!child_p->is_virtual) {
                        non_virtual_child_found = 1;
                        break;
                    }
                }
                DELETE_LISTABLE_LIST(&fcl_p, object_container);

                if (!non_virtual_child_found) {
                    fprintf(stderr, "Parameter %s:%s is virtual but has no non-virtual children\n",
                            ps_p->name_p,
                            parameter_list_entry_p->parameter_name_p
                           );
                    exit(-1);
                }
            }

            if (parameter_p->is_bag())
                ret += check_paramlist_parameters((mpl_list_t*) parameter_p->get_property("parameter_list"));
        }
    }
    return ret;
}

int string2int64(char *str_p, int64_t *value_p)
{
    int64_t temp;
    int res;

    assert(value_p);
    if ((NULL != strstr(str_p, "0x")) ||
        (NULL != strstr(str_p, "0X")))
        res = sscanf(str_p, "0x%" SCNx64, &temp);
    else
        res = sscanf(str_p, "%" SCNi64, &temp);

    if (res <= 0)
        return -1;

    *value_p = temp;
    return 0;
}

static latex_entry *find_latex_entry_from_label(mpl_list_t *le_list_p, const char *label_p)
{
    mpl_list_t *tmp_p;
    latex_entry *latex_entry_p;

    MPL_LIST_FOR_EACH(le_list_p, tmp_p) {
        latex_entry_p = LISTABLE_PTR(tmp_p, latex_entry);
        if (!strcmp(latex_entry_p->label_p, label_p))
            return latex_entry_p;
    }
    return NULL;
}


/* This function should somehow call something external (e.g. plugin code), because
   the tags are defined by the mpl *application*, not mpl itself */
int convert_locally_defined_dox_entry(const char *label_p, char *value_p, dox_entry *dox_entry_p)
{
    if (!strcmp(label_p, "at")) {
        dox_entry_p->label_p = (char *) calloc(1, strlen("@see") + 1);
        dox_entry_p->value_p = (char *) calloc(1, strlen(value_p) + 100);
        sprintf(dox_entry_p->label_p, "@see", label_p);
        sprintf(dox_entry_p->value_p, "%s", value_p);
        return 0;
    }
    return -1;
}


/* This function should somehow call something external (e.g. plugin code), because
   the tags are defined by the mpl *application*, not mpl itself */
int convert_locally_defined_latex_entry(const char *object_type_p,
                                        listable_object *object_p,
                                        const char *label_p,
                                        char *value_p,
                                        latex_entry *latex_entry_p)
{
    if (!strcmp(label_p, "at")) {
        const char *atref_label_p = "\n{\\bf AT reference:}\\newline\n";
        latex_entry *le_p = find_latex_entry_from_label(object_p->latex_list_p, atref_label_p);
        if (le_p) {
            le_p->value_p = (char*) realloc(le_p->value_p, strlen(le_p->value_p) + strlen(value_p) + 100);
            sprintf(le_p->value_p + strlen(le_p->value_p), "%s\\newline\n", value_p);
            if (latex_entry_p->label_p)
                free(latex_entry_p->label_p);
            latex_entry_p->label_p = strdup("");
            if (latex_entry_p->value_p)
                free(latex_entry_p->value_p);
            latex_entry_p->value_p = strdup("");
        }
        else {
            if (latex_entry_p->label_p)
                free(latex_entry_p->label_p);
            latex_entry_p->label_p = (char *) calloc(1, strlen(atref_label_p) + 100);
            if (latex_entry_p->value_p)
                free(latex_entry_p->value_p);
            latex_entry_p->value_p = (char *) calloc(1, strlen(value_p) + 100);
            sprintf(latex_entry_p->label_p, atref_label_p, label_p);
            sprintf(latex_entry_p->value_p, "%s\\newline\n", value_p);
        }
        return 0;
    }
    return -1;
}


/* This function should somehow call something external (e.g. plugin code), because
   the tags are defined by the mpl *application*, not mpl itself */
int convert_locally_defined_help_entry(const char *label_p, char *value_p, help_entry *help_entry_p)
{
    return -1;
}


/* This function should somehow call something external (e.g. plugin code), because
   the tags are defined by the mpl *application*, not mpl itself */
void latex_extra(mpl_compiler *compiler_p, FILE *f)
{
    LX_S("%s",
         "External references");
    LX_SS("%s",
          "Commands"
         );
    lx_table_begin(f, 2);
    LX_HLINE;
    LX_TABH(2, "%s",
            "Commands and AT references"
           );
    LX_COL_END;

    LX_HLINE;
    LX_COLH(Command);
    LX_COL_SEP;
    LX_COLH(References);
    LX_COL_END;

    mpl_list_t *tmp_p;
    MPL_LIST_FOR_EACH(compiler_p->categories_p, tmp_p) {
        category *category_p;
        category_p = LISTABLE_PTR(tmp_p, category);

        mpl_list_t *tmp_p;
        MPL_LIST_FOR_EACH(category_p->commands.method_list_p, tmp_p) {
            command *command_p;
            command_p = LISTABLE_PTR(tmp_p, command);
            fprintf_latex(f,
                          "%s",
                          command_p->name_p
                         );
            LX_COL_SEP;
            const char *atref_label_p = "\n{\\bf AT reference:}\\newline\n";
            latex_entry *le_p = find_latex_entry_from_label(command_p->latex_list_p, atref_label_p);
            if (le_p)
                fprintf_latex(f,
                              "%s\n",
                              le_p->value_p
                             );
            LX_COL_END;
        }
    }

    LX_HLINE;
    LX_TAB_END;


    LX_SS("%s",
          "Events"
         );
    lx_table_begin(f, 2);
    LX_HLINE;
    LX_TABH(2, "%s",
            "Events and AT references"
           );
    LX_COL_END;

    LX_HLINE;
    LX_COLH(Event);
    LX_COL_SEP;
    LX_COLH(References);
    LX_COL_END;

    MPL_LIST_FOR_EACH(compiler_p->categories_p, tmp_p) {
        category *category_p;
        category_p = LISTABLE_PTR(tmp_p, category);

        mpl_list_t *tmp_p;
        MPL_LIST_FOR_EACH(category_p->events.method_list_p, tmp_p) {
            event *event_p;
            event_p = LISTABLE_PTR(tmp_p, event);
            fprintf_latex(f,
                          "%s",
                          event_p->name_p
                         );
            LX_COL_SEP;
            const char *atref_label_p = "\n{\\bf AT reference:}\\newline\n";
            latex_entry *le_p = find_latex_entry_from_label(event_p->latex_list_p, atref_label_p);
            if (le_p)
                fprintf_latex(f,
                              "%s\n",
                              le_p->value_p
                             );
            LX_COL_END;
        }
    }

    LX_HLINE;
    LX_TAB_END;
}
