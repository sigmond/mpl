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
#ifndef mplcomp_hh
#define mplcomp_hh

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <ostream>
#include <sstream>

#include "mplcomp.tab.hh"
extern "C" {
    #include "mpl_list.h"
    #include "mpl_param.h"
}


//#define CHK_FTRACE

#define NUMBER_RANGE_ID_DEFAULT "NUMBER_RANGE_ID"
#define NUMBER_RANGE_NAME_ANONYMOUS "ANONYMOUS"
#define BAG_FIELD_TABLE_SUFFIX_DEFAULT "FIELD_TABLE"

#define DIRECTIONS \
    DIRECTION_VALUE_ELEMENT(in) \
    DIRECTION_VALUE_ELEMENT(out) \
    DIRECTION_VALUE_ELEMENT(inout) \
    DIRECTION_VALUE_ELEMENT(none)

#define DIRECTION_VALUE_ELEMENT(ELEMENT) \
    direction_##ELEMENT,

typedef enum {
    DIRECTIONS
    number_of_directions
} direction_t;

#undef DIRECTION_VALUE_ELEMENT

#define CODE_SEGMENTS \
    CODE_SEGMENT_VALUE_ELEMENT(top) \
    CODE_SEGMENT_VALUE_ELEMENT(prototypes)

#define CODE_SEGMENT_VALUE_ELEMENT(ELEMENT) \
    code_segment_##ELEMENT,

typedef enum {
    CODE_SEGMENTS
    number_of_code_segments
} code_segment_t;

#undef CODE_SEGMENT_VALUE_ELEMENT

#define VALUE_TYPES \
    VALUE_TYPE_VALUE_ELEMENT(value) \
    VALUE_TYPE_VALUE_ELEMENT(string_literal) \
    VALUE_TYPE_VALUE_ELEMENT(name) \
    VALUE_TYPE_VALUE_ELEMENT(bool) \

#define VALUE_TYPE_VALUE_ELEMENT(ELEMENT) \
    value_type_##ELEMENT,

typedef enum {
    VALUE_TYPES
    number_of_value_types
} value_type_t;
#undef VALUE_TYPE_VALUE_ELEMENT

#define DOC_MODES \
    DOC_MODE_VALUE_ELEMENT(normal) \
    DOC_MODE_VALUE_ELEMENT(commands_or_events) \

#define DOC_MODE_VALUE_ELEMENT(ELEMENT) \
    doc_mode_##ELEMENT,

typedef enum {
    DOC_MODES
    number_of_doc_modes
} doc_mode_t;
#undef DOC_MODE_VALUE_ELEMENT

#define CODEGEN_MODES \
    CODEGEN_MODE_VALUE_ELEMENT(mpl) \
    CODEGEN_MODE_VALUE_ELEMENT(cli) \
    CODEGEN_MODE_VALUE_ELEMENT(api) \

#define CODEGEN_MODE_VALUE_ELEMENT(ELEMENT) \
    codegen_mode_##ELEMENT,

typedef enum {
    CODEGEN_MODES
    number_of_codegen_modes
} codegen_mode_t;
#undef CODEGEN_MODE_VALUE_ELEMENT

#define BLOCK_TYPES \
    BLOCK_TYPE_VALUE_ELEMENT(category) \
    BLOCK_TYPE_VALUE_ELEMENT(parameters_in_file_scope) \
    BLOCK_TYPE_VALUE_ELEMENT(parameters_in_block_scope) \
    BLOCK_TYPE_VALUE_ELEMENT(commands) \
    BLOCK_TYPE_VALUE_ELEMENT(events) \
    BLOCK_TYPE_VALUE_ELEMENT(parameter_set) \
    BLOCK_TYPE_VALUE_ELEMENT(command_bag) \
    BLOCK_TYPE_VALUE_ELEMENT(response_bag) \
    BLOCK_TYPE_VALUE_ELEMENT(event_bag) \
    BLOCK_TYPE_VALUE_ELEMENT(enum) \
    BLOCK_TYPE_VALUE_ELEMENT(bag) \
    BLOCK_TYPE_VALUE_ELEMENT(enumerator_list) \

#define BLOCK_TYPE_VALUE_ELEMENT(ELEMENT) \
    block_type_##ELEMENT,

typedef enum {
    BLOCK_TYPES
    number_of_block_types
} block_type_t;
#undef BLOCK_TYPE_VALUE_ELEMENT

typedef enum {
    parser_mode_normal,
    parser_mode_dox,
    parser_mode_latex,
    parser_mode_help
} parser_mode_t;

/* Destructor utilities */
#define DELETE_LISTABLE_LIST(lst_pp,cls) \
    do {                     \
        mpl_list_t *__tmp_p;   \
        while ((__tmp_p = mpl_list_remove(lst_pp, NULL)) != NULL) { \
            cls *__entry_p;                                  \
            __entry_p = LISTABLE_PTR(__tmp_p, cls);          \
            delete __entry_p;                                \
        }                                                    \
    }                                                        \
    while (0)

/* "Internal print" utilities */
void print_enum_values(int level, mpl_list_t *enum_values_p);
void print_enumerator_lists(int level, mpl_list_t *enumerator_lists_p);
void print_parameter_list(int level, mpl_list_t *parameter_list_p);
void print_files(mpl_list_t *files_p);
void print_doc_list(int level, mpl_list_t *doc_list_p);
void print_dox_list(int level, mpl_list_t *dox_list_p, const char *label_p = NULL);
void print_latex_list(int level, mpl_list_t *latex_list_p, const char *label_p = NULL);
void print_help_list(int level, mpl_list_t *help_list_p, const char *label_p = NULL);

/* Code generation utilities */
void gc_lines(FILE *f, mpl_list_t *lines_p, code_segment_t code_segment, codegen_mode_t codegen_mode);

/* Misc utilities */
int mpl_prefix2paramsetid(const char *prefix_p);
int get_type_of_int(const char *type_p);
int get_type_of_enum(const char *type_p);
int get_type_of_string(const char *type_p);
int get_type_of_tuple(const char *type_p);
int get_type_of_array(const char *type_p);
int get_type_of_bool(const char *type_p);
int get_type_of_addr(const char *type_p);
int get_type_of_bag(const char *type_p);
char *trimstring(char *s, char escape);
const char *spacing(int level);
char *fix_for_header_define(char *name_p);
char *str_toupper(char *str);
char *str_tolower(char *str);
int convert_int(const char* value_str, int *value_p);
int parameter_property_in_list(char *name_p,
                               const char *property_p,
                               mpl_list_t *parameter_list_p);
class parameter_list_entry;
int parameter_list_entry_is_in_list(parameter_list_entry *entry_p,
                                    mpl_list_t *parameter_list_p);
int parameter_list_has_mandatory_elements(mpl_list_t *parameter_list_p);
int parameter_list_has_bag_elements(mpl_list_t *parameter_list_p);
parameter_list_entry *find_parameter_entry_in_list(mpl_list_t *parameter_list_p,
                                                   char *parameter_name_p);
parameter_list_entry *find_parameter_entry_in_list(mpl_list_t *parameter_list_p,
                                                   char *parameter_set_name_p,
                                                   char *parameter_name_p,
                                                   char *field_name_p);
mpl_list_t *doc_list_clone(mpl_list_t *src_list_p);
mpl_list_t *listable_object_list_clone(mpl_list_t *src_list_p);
mpl_list_t *object_container_list_clone(mpl_list_t *src_list_p);
class parameter_set;
int check_paramlist_parameters(mpl_list_t *parameter_list_p);
class dox_entry;
class latex_entry;
class help_entry;
class listable_object;
int convert_locally_defined_dox_entry(const char *label_p, char *value_p, dox_entry *dox_entry_p);
int convert_locally_defined_latex_entry(const char *object_type_p,
                                        listable_object *object_p,
                                        const char *label_p,
                                        char *value_p,
                                        latex_entry *latex_entry_p);
int convert_locally_defined_help_entry(const char *label_p, char *value_p, help_entry *help_entry_p);
class mpl_compiler;
void latex_extra(mpl_compiler *compiler_p, FILE *f);
int string2int64(char *str_p, int64_t *value_p);
int vfprintf_escape(char chars[], int num_chars, char escape,
                    FILE *f, const char *format, va_list ap);

int fprintf_dox(FILE *f, const char *format, ...);
int fprintf_latex(FILE *f, const char *format, ...);


#define CLONE_STR(str) \
    if (o.str) \
        str = strdup(o.str);                    \
    else                                        \
        str = NULL

#define CLONE_INT_P(int_p) \
    if (o.int_p) \
        int_p = new int(*o.int_p);              \
    else                                        \
      int_p = NULL

#define CLONE_INT64_P(int64_p) \
    if (o.int64_p) \
        int64_p = new int64_t(*o.int64_p);              \
    else                                        \
      int64_p = NULL

#define CLONE_LISTABLE_OBJECT(cls,obj) \
    if (o.obj) \
        obj = new cls(*o.obj);                  \
    else                                        \
        obj = NULL

#define CLONE_LISTABLE_OBJECT_CAST(cls,obj) \
    if (o.obj) \
        obj = new cls(*((cls*)o.obj));          \
    else                                        \
        obj = NULL

#define CLONE_DOC_LIST(lst) lst = doc_list_clone(o.lst)
#define CLONE_LISTABLE_OBJECT_LIST(lst) lst = listable_object_list_clone(o.lst)
#define CLONE_OBJECT_CONTAINER_LIST(lst) lst = listable_object_list_clone(o.lst)
#define CLONE_LISTABLE_OBJECT_FUNC_DEFINE(cls) \
    virtual listable_object *clone() { return(new cls(*this)); }

class parameter_set;


class doc;

/* Doxygen generation utilities */
void dox_doc_f(FILE *f, mpl_list_t *doc_list_p);
void dox_doc_b(FILE *f, mpl_list_t *doc_list_p);
void dox_files(FILE *f, mpl_list_t *files_p);
void dox_parameter_list(FILE *f, mpl_list_t *parameter_list_p, direction_t dir);
void dox_dox_list_f(FILE *f, mpl_list_t *dox_list_p);
void dox_dox_list_b(FILE *f, mpl_list_t *dox_list_p);

/* Latex generation utilities */
void latex_parameter_list(FILE *f,
                          mpl_list_t *parameter_list_p,
                          mpl_list_t *exclude_parameter_list_p,
                          direction_t dir,
                          parameter_set *default_parameter_set_p,
                          const char *header_p);
void latex_latex_list(FILE *f, mpl_list_t *latex_list_p);
/* CLI help text generation utilities */
void help_parameter_list(std::ostream &os,
                         mpl_list_t *parameter_list_p,
                         mpl_list_t *exclude_parameter_list_p,
                         direction_t dir,
                         parameter_set *default_parameter_set_p,
                         const char *header_p);
void help_help_list(std::ostream &os,
                    mpl_list_t *help_list_p,
                    const char *indent,
                    const char *initial_indent = NULL);

/* API generation utilities */
void api_parameter_list(FILE *f,
                        mpl_list_t *parameter_list_p,
                        mpl_list_t *exclude_parameter_list_p,
                        direction_t dir);
class category;
void api_cc_encode_parameter_list(FILE *f,
                                  char *indent,
                                  mpl_list_t *parameter_list_p,
                                  mpl_list_t *exclude_parameter_list_p,
                                  parameter_set *parameter_set_p,
                                  char *bag_name_p,
                                  char *bag_param_name_p,
                                  direction_t dir);
void api_decode_parameter_list(FILE *f,
                               char *indent,
                               mpl_list_t *parameter_list_p,
                               mpl_list_t *exclude_parameter_list_p,
                               parameter_set *parameter_set_p,
                               char *bag_name_p,
                               char *bag_param_name_p,
                               direction_t dir);
void api_free_decoded_parameter_list(FILE *f,
                                     char *indent,
                                     mpl_list_t *parameter_list_p,
                                     mpl_list_t *exclude_parameter_list_p,
                                     parameter_set *parameter_set_p,
                                     direction_t dir);
void api_hh_class_members_parameter_list(FILE *f,
                                         char *indent,
                                         mpl_list_t *parameter_list_p,
                                         mpl_list_t *exclude_parameter_list_p,
                                         parameter_set *parameter_set_p,
                                         direction_t dir);
void api_cc_members_allocate_parameter_list(FILE *f,
                                            char *indent,
                                            mpl_list_t *parameter_list_p,
                                            mpl_list_t *exclude_parameter_list_p,
                                            parameter_set *parameter_set_p,
                                            direction_t dir);
void api_cc_copy_constructor_parameter_list(FILE *f,
                                            char *indent,
                                            mpl_list_t *parameter_list_p,
                                            mpl_list_t *exclude_parameter_list_p,
                                            parameter_set *parameter_set_p,
                                            direction_t dir);
class inheritable_object;
void api_hh_constructor_parameter_list(FILE *f,
                                       char *indent,
                                       mpl_list_t *parameter_list_p,
                                       mpl_list_t *exclude_parameter_list_p,
                                       inheritable_object *parent_p,
                                       parameter_set *parameter_set_p,
                                       direction_t dir);
void api_cc_print_parameter_list(FILE *f,
                                 char *indent,
                                 mpl_list_t *parameter_list_p,
                                 mpl_list_t *exclude_parameter_list_p,
                                 parameter_set *parameter_set_p,
                                 direction_t dir);


#define LX_BI                        \
    fprintf(f,                       \
            "\\begin{itemize}\n" \
           )

#define LX_EI                      \
    fprintf(f,                     \
            "\\end{itemize}\n" \
           )

#define LX_BD                        \
    fprintf(f,                       \
            "\\begin{description}\n" \
           )

#define LX_ED                      \
    fprintf(f,                     \
            "\\end{description}\n" \
           )

#define LX_ITEM(format, ...)                             \
    do {                                                       \
        fprintf(f,                                             \
                "\\item["                                      \
               );                                              \
        fprintf_latex(f,                                       \
                      format,                                  \
                      __VA_ARGS__                              \
                     );                                        \
        fprintf(f,                                             \
                "]\n"                                          \
               );                                              \
    } while (0)

#define LX_SSS(format, ...)                                    \
    do {                                                       \
        fprintf(f,                                             \
                "\\subsubsection{"                             \
               );                                              \
        fprintf_latex(f,                                       \
                      format,                                  \
                      __VA_ARGS__                              \
                     );                                        \
        fprintf(f,                                             \
                "}\n"                                          \
               );                                              \
    } while(0)

#define LX_SS(format, ...)                               \
    do {                                                 \
        fprintf(f,                                       \
                "\\subsection{"                          \
               );                                        \
        fprintf_latex(f,                                 \
                      format,                            \
                      __VA_ARGS__                        \
                     );                                  \
        fprintf(f,                                       \
                "}\n"                                    \
               );                                        \
    } while (0)

#define LX_S(format, ...)                                      \
    do {                                                       \
        fprintf(f,                                             \
                "\\section{",                                  \
                __VA_ARGS__                                    \
               );                                              \
        fprintf_latex(f,                                       \
                      format,                                  \
                      __VA_ARGS__                              \
                     );                                        \
        fprintf(f,                                             \
                "}\n"                                          \
               );                                              \
    } while(0)

#define LX_LABEL(format, ...)                             \
    fprintf(f,                                           \
            "\\label{" format "}\n",                      \
            __VA_ARGS__                                  \
           )

#define LX_TABH(cols, format, ...)              \
    do {                                        \
        fprintf(f,                                   \
                "\\multicolumn{" #cols "}{|l|}"      \
                "{\\bf "                             \
               );                                    \
        fprintf_latex(f,                             \
                      format,                        \
                      __VA_ARGS__                    \
                     );                              \
        fprintf(f,                                   \
                "}"                                  \
               );                                    \
    } while (0)

#define LX_COLH(h)                              \
    do {                                        \
        fprintf(f,                              \
                " {\\it "                       \
               );                               \
        fprintf_latex(f,                        \
                      #h                        \
                     );                         \
        fprintf(f,                              \
                "} "                            \
               );                               \
    } while (0)

#define LX_HLINE  fprintf(f, "\\hline\n")
#define LX_COL_END fprintf(f, "\\\\\n")
#define LX_COL_SEP fprintf(f, " & ")
#define LX_TAB_END fprintf(f, "\\end{tabular}\n")
#define LX_NEWLINE  fprintf(f, "\\newline\n")

#define LX_NORMAL_COL(num_cols)                \
    fprintf(f,                                 \
            "l"                                \
           )

#define LX_DESCRIPTION_COL(num_cols)                \
    fprintf(f,                                      \
            "p{0.5\\linewidth}"                   \
           )

void lx_table_begin(FILE *f, int cols);


using namespace std;

void yyerror(const char *error);
extern int yyparse(void *compiler_p);

#define LISTABLE_PTR(list,type) ((type*)         \
                                 MPL_LIST_CONTAINER(list, listable, list_entry))

class listable {
  public:
    listable() {
        clear_list_entry();
    }
    listable(listable &o)
    {
        clear_list_entry();
    }
    void append_to(mpl_list **list_pp) {
        mpl_list_append(list_pp, &list_entry);
    }

    void append_to(mpl_list *&list_p) {
        mpl_list_append(&list_p, &list_entry);
    }

    void add_to(mpl_list **list_pp) {
        mpl_list_add(list_pp, &list_entry);
    }

    void add_to(mpl_list *&list_p) {
        mpl_list_add(&list_p, &list_entry);
    }

    void clear_list_entry() {
        memset(&list_entry, 0, sizeof(mpl_list_t));
    }

    mpl_list_t list_entry;
};

class doc : public listable {
  public:
    virtual ~doc()
    {
        if (text_p) {
            free(text_p);
            text_p = NULL;
        }
    }
    doc(char *text_p, int is_forward) :
        text_p(text_p),
        is_forward(is_forward)
    {
    }
    doc(doc &o) :
        is_forward(o.is_forward)
    {
        CLONE_STR(text_p);
    }
    virtual doc *clone() { return(new doc(*this)); }

    char *text_p;
    int is_forward;

    void print(int level);
    void dox(FILE *f);
    void latex(FILE *f);
};


class x_entry;
class latex_entry;
class group_entry;

class listable_object : public listable {
    private:
    void doc_to_x(parser_mode_t parser_mode,
                  const char *object_type_p,
                  const char *name_p,
                  mpl_list_t **local_x_list_pp,
                  mpl_list_t *local_doc_list_p);

    protected:
    virtual void add_x_info(x_entry *x_entry_p,
                            const char *label_p,
                            char *value_p,
                            char *text_p);

    public:
    virtual ~listable_object();
    listable_object(mpl_compiler *compiler_p) :
        compiler_p(compiler_p),
        group_p(NULL),
        doc_list_p(NULL),
        dox_list_p(NULL),
        latex_list_p(NULL),
        help_list_p(NULL)
    {
    }
    listable_object(listable_object &o) :
        compiler_p(o.compiler_p),
        group_p(o.group_p)
    {
        CLONE_DOC_LIST(doc_list_p);
        CLONE_LISTABLE_OBJECT_LIST(dox_list_p);
        CLONE_LISTABLE_OBJECT_LIST(latex_list_p);
        CLONE_LISTABLE_OBJECT_LIST(help_list_p);
    }

    mpl_compiler *compiler_p;
    group_entry *group_p;
    mpl_list_t *doc_list_p;
    mpl_list_t *dox_list_p;
    mpl_list_t *latex_list_p;
    mpl_list_t *help_list_p;

    virtual char *get_name()
    {
        assert(0);
    }
    virtual listable_object *clone() = 0;
    virtual void doc_to_dox(const char *object_type_p,
                            const char *name_p,
                            mpl_list_t **local_dox_list_pp = NULL,
                            mpl_list_t *local_doc_list_p = NULL);
    virtual void doc_to_latex(const char *object_type_p,
                              const char *name_p,
                              mpl_list_t **local_latex_list_pp = NULL,
                              mpl_list_t *local_doc_list_p = NULL);
    virtual void doc_to_help(const char *object_type_p,
                             const char *name_p,
                             mpl_list_t **local_help_list_pp = NULL,
                             mpl_list_t *local_doc_list_p = NULL);
    virtual void convert_doc(const char *type_p, const char *name_p);
    virtual void add_dox_info(dox_entry *dox_entry_p,
                              const char *label_p,
                              char *value_p,
                              char *text_p);
    virtual void add_latex_info(latex_entry *latex_entry_p,
                                const char *label_p,
                                char *value_p,
                                char *text_p);
    virtual void add_help_info(help_entry *help_entry_p,
                               const char *label_p,
                               char *value_p,
                               char *text_p);
    virtual void print(int level);
};

class inheritable_object : public listable_object {
    public:
    virtual ~inheritable_object();
    inheritable_object(mpl_compiler *compiler_p,
                       char *name_p,
                       inheritable_object *parent_p) :
        listable_object(compiler_p),
        name_p(name_p),
        parent_p(parent_p),
        child_list_p(NULL)
    {
    }
    inheritable_object(inheritable_object &o) :
        listable_object(o),
        parent_p(o.parent_p)
    {
        CLONE_STR(name_p);
        CLONE_OBJECT_CONTAINER_LIST(child_list_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(inheritable_object)

    char *name_p;

    /* Object-inheritance-related stuff */
    /* Pointer to the parent object: */
    inheritable_object *parent_p;

    /* An object container list where each entry refers to a child */
    mpl_list_t *child_list_p;

    virtual char *get_name()
    {
        return name_p;
    }
    virtual void set_parent(inheritable_object *parent_p)
    {
        this->parent_p = parent_p;
    }
    virtual void add_child(inheritable_object *child_p);
    virtual mpl_list_t *get_flat_child_list();
    virtual mpl_list_t *get_parent_list();
    virtual bool is_parent(inheritable_object *obj);
    virtual inheritable_object *get_topmost_parent();
    virtual void *get_property(const char *property_p)
    {
        return NULL;
    }
};

class object_container : public listable {
  public:
    object_container(listable_object *object_p) :
        object_p(object_p)
    {
    }
    object_container(object_container &o) :
        object_p(o.object_p)
    {
    }
    virtual object_container *clone() { return(new object_container(*this)); }

    listable_object *object_p;
    void print(int level);
};



class compiler_define : public listable_object {
  public:
    virtual ~compiler_define()
    {
        if (key_p) {
            free(key_p);
            key_p = NULL;
        }
        if (value_p) {
            free(value_p);
            value_p = NULL;
        }
    }
    compiler_define(mpl_compiler *compiler_p, char *key_p, char *value_p) :
        listable_object(compiler_p),
        key_p(key_p),
        value_p(value_p)
    {
    }
    compiler_define(compiler_define &o) :
        listable_object(o)
    {
        CLONE_STR(key_p);
        CLONE_STR(value_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(compiler_define)

    char *key_p;
    char *value_p;
};

class compiler_option : public listable_object {
  public:
    virtual ~compiler_option()
    {
        if (block_name_p) {
            free(block_name_p);
            block_name_p = NULL;
        }
        if (option_name_p) {
            free(option_name_p);
            option_name_p = NULL;
        }
        if (option_value_p) {
            free(option_value_p);
            option_value_p = NULL;
        }
    }
    compiler_option(mpl_compiler *compiler_p) :
        listable_object(compiler_p),
        block_type_p(NULL),
        block_name_p(NULL),
        option_name_p(NULL),
        option_value_p(NULL)
    {
    }
    compiler_option(compiler_option &o) :
        listable_object(o),
        block_type_p(o.block_type_p)
    {
        CLONE_STR(block_name_p);
        CLONE_STR(option_name_p);
        CLONE_STR(option_value_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(compiler_option)

    const char *block_type_p;
    char *block_name_p;
    char *option_name_p;
    char *option_value_p;

    void print(int level);
    int check();
};

class group_entry : public listable {
  public:
    virtual ~group_entry()
    {
        if (name_p) {
            free(name_p);
            name_p = NULL;
        }
        if (text_p) {
            free(text_p);
            text_p = NULL;
        }
    }
    group_entry(char *name_p,
                char *text_p,
                group_entry *parent_p = NULL) :
        name_p(name_p),
        text_p(text_p),
        parent_p(parent_p)
    {
    }
    group_entry(group_entry &o) :
        listable(o),
        parent_p(o.parent_p)
    {
        CLONE_STR(name_p);
        CLONE_STR(text_p);
    }
    virtual listable *clone() { return(new group_entry(*this)); }

    char *name_p;
    char *text_p;
    group_entry *parent_p;

    void set_parent(group_entry *pg_p)
    {
        parent_p = pg_p;
    }
    void print(int level);
};


class x_entry : public listable_object {
  public:
    virtual ~x_entry()
    {
        if (label_p) {
            free(label_p);
            label_p = NULL;
        }
        if (value_p) {
            free(value_p);
            value_p = NULL;
        }
    }
    x_entry(mpl_compiler *compiler_p,
            const char *target_object_type_p,
            listable_object *target_object_p) :
        listable_object(compiler_p),
        target_object_type_p(target_object_type_p),
        target_object_p(target_object_p),
        label_p(NULL),
        value_p(NULL)
    {
    }
    x_entry(x_entry &o) :
        listable_object(o),
        target_object_type_p(o.target_object_type_p),
        target_object_p(o.target_object_p)
    {
        CLONE_STR(label_p);
        CLONE_STR(value_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(x_entry)

    const char *target_object_type_p;
    listable_object *target_object_p;
    char *label_p;
    char *value_p;

    virtual void clear_info()
    {
        if (label_p)
            free(label_p);
        label_p = NULL;
        if (value_p)
            free(value_p);
        value_p = NULL;
    }

    virtual void add_info(parser_mode_t parser_mode,
                          const char *label_p,
                          char *value_p,
                          char *text_p);
};

class latex_entry : public x_entry {
  public:
    latex_entry(mpl_compiler *compiler_p,
                const char *target_object_type_p,
                listable_object *target_object_p) :
        x_entry(compiler_p, target_object_type_p, target_object_p)
    {
    }
    latex_entry(latex_entry &o) :
        x_entry(o)
    {
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(latex_entry)
};

class help_entry : public x_entry {
  public:
    help_entry(mpl_compiler *compiler_p,
               const char *target_object_type_p,
               listable_object *target_object_p) :
        x_entry(compiler_p, target_object_type_p, target_object_p)
    {
    }
    help_entry(help_entry &o) :
        x_entry(o)
    {
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(help_entry)
};

class dox_entry : public x_entry {
    public:
    dox_entry(mpl_compiler *compiler_p,
              const char *target_object_type_p,
              listable_object *target_object_p,
              int is_forward) :
        x_entry(compiler_p, target_object_type_p, target_object_p),
        is_forward(is_forward)
    {
    }
    dox_entry(dox_entry &o) :
        x_entry(o),
        is_forward(o.is_forward)
    {
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(dox_entry)

    int is_forward;

    void dox(FILE *f);
};

class ellipsis : public listable_object {
  public:
    ellipsis(mpl_compiler *compiler_p, int value) :
        listable_object(compiler_p),
        value(value)
    {
    }
    ellipsis(ellipsis &o) :
        listable_object(o),
        value(o.value)
    {
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(ellipsis)

    int value;

    void print(int level);
};

class string_entry : public listable_object {
  public:
    virtual ~string_entry()
    {
        if (value_p) {
            free(value_p);
            value_p = NULL;
        }
    }
    string_entry(mpl_compiler *compiler_p, char *value_p):
        listable_object(compiler_p),
        value_p(value_p)
    {
    }
    string_entry(string_entry &o) :
        listable_object(o)
    {
        CLONE_STR(value_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(string_entry)

    char *value_p;
    void print(int level, const char *name_p);
};

class enumerator_list : public listable_object {
  public:
    virtual ~enumerator_list();
    enumerator_list(mpl_compiler *compiler_p, char *name_p) :
        listable_object(compiler_p),
        name_p(name_p),
        enumerator_list_values_p(NULL)
    {
    }
    enumerator_list(enumerator_list &o) :
        listable_object(o)
    {
        CLONE_STR(name_p);
        CLONE_LISTABLE_OBJECT_LIST(enumerator_list_values_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(enumerator_list)

    char *name_p;
    mpl_list_t *enumerator_list_values_p;

    void gc_h(FILE *f, char *parameter_set_name_p);

    void print(int level);
};

class parameter;
class enum_parameter;
class number_range;
class integer_range;

class parameter_group : public listable_object {
    public:
    virtual ~parameter_group();
    parameter_group(mpl_compiler *compiler_p,
                    parameter_set *parameter_set_p,
                    category *category_p) :
        listable_object(compiler_p),
        parameter_set_p(parameter_set_p),
        category_p(category_p),
        parameters_p(NULL)
    {
    }
    parameter_group(parameter_group& o) :
        listable_object(o),
        parameter_set_p(o.parameter_set_p),
        category_p(o.category_p)
    {
        CLONE_LISTABLE_OBJECT_LIST(parameters_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(parameter_group)

    parameter_set *parameter_set_p;
    category *category_p;
    mpl_list_t *parameters_p;

    virtual void convert_doc();
    virtual void latex(FILE *f);
    virtual void latex_parameters(FILE *f);
    virtual void print(int level);
};

class parameter_set : public listable_object {
    private:
    void convert_doc_parameters();
    parameter *create_parameter_in_group(const char *type_p, char *name_p, parameter_group *parameter_group_p);

    public:
    virtual ~parameter_set();
    parameter_set(mpl_compiler *compiler_p, char *name_p) :
        listable_object(compiler_p),
        name_p(name_p),
        prefix_p(NULL),
        paramset_id_p(NULL),
        range_id_p(NULL),
        bag_field_table_suffix_p(NULL),
        current_parameter_group_p(NULL),
        default_parameter_group_p(NULL),
        parameter_group_list_p(NULL),
        enumerator_lists_p(NULL),
        number_range_list_p(NULL)
    {
        use_parameter_group(NULL);
    }
    parameter_set(parameter_set &o) :
        listable_object(o),
        current_parameter_group_p(NULL),
        default_parameter_group_p(NULL)
    {
        CLONE_STR(name_p);
        CLONE_LISTABLE_OBJECT(string_entry,prefix_p);
        CLONE_LISTABLE_OBJECT(string_entry,paramset_id_p);
        CLONE_LISTABLE_OBJECT(string_entry,range_id_p);
        CLONE_LISTABLE_OBJECT(string_entry,bag_field_table_suffix_p);
        CLONE_LISTABLE_OBJECT_LIST(parameter_group_list_p);
        CLONE_LISTABLE_OBJECT_LIST(enumerator_lists_p);
        CLONE_LISTABLE_OBJECT_LIST(number_range_list_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(parameter_set)

    char *name_p;
    string_entry *prefix_p;
    string_entry *paramset_id_p;
    string_entry *range_id_p;
    string_entry *bag_field_table_suffix_p;
    parameter_group *current_parameter_group_p;
    parameter_group *default_parameter_group_p;
    mpl_list_t *parameter_group_list_p;
    mpl_list_t *enumerator_lists_p;
    mpl_list_t *number_range_list_p;

    void add_prefix(char *prefix_p);
    void add_id(char *id_p);
    void add_range_id(char *range_id_p);
    void add_bag_field_table_suffix(char *suffix_p);
    enum_parameter *create_field_table_parameter(char *name_p);
    char *get_short_name();
    enumerator_list *get_enumerator_list(char *enumerator_list_name_p);
    enum_parameter *obtain_number_range_parameter();
    void add_number_range(number_range *number_range_p);
    number_range *find_number_range(const char *number_range_name_p);
    int check_parameters();
    parameter *find_parameter(char *name_p);
    parameter_group *find_parameter_group(char *category_name_p);
    parameter *create_parameter_in_current_group(const char *type_p, char *name_p);
    parameter *create_parameter_in_default_group(const char *type_p, char *name_p);
    void add_enumerator_list(enumerator_list *elist_p);
    void use_parameter_group(category *category_p);

    void print(int level);
    void print_parameter_groups(int level);
    void print_enumerator_lists(int level);
    void print_number_ranges(int level);

    void gc_h(FILE *f);
    void gc_h_functions(FILE *f);
    void gc_h_macros(FILE *f);
    void gc_h_bags(FILE *f);
    void gc_h_paramids(FILE *f);
    void gc_h_enums(FILE *f);
    void gc_h_enumerator_lists(FILE *f);

    void gc_c(FILE *f);
    void gc_c_bags(FILE *f);
    void gc_c_param_init(FILE *f);
    void gc_c_param_descr(FILE *f);
    void gc_c_defaults(FILE *f);
    void gc_c_extra(FILE *f);
    void gc_c_ranges(FILE *f);
    void gc_c_field_values(FILE *f);
    void gc_c_children(FILE *f);
    void gc_c_enums(FILE *f);
    void cli_h(FILE *f);
    void cli_c(FILE *f);
    void cli_call_parameter_help(FILE *f);
    void cli_parameter_help_completions(FILE *f);

    void api_hh(FILE *f, char *indent);
    void api_cc(FILE *f, char *indent);

    void wrap_up_definition();

    virtual void convert_doc();

    void deja(FILE *f);

    void dox(FILE *f);
    void dox_parameters(FILE *f, int level);
    virtual void add_dox_info(dox_entry *dox_entry_p,
                              const char *label_p,
                              char *value_p,
                              char *text_p);

    void latex(FILE *f);
};


class enum_value : public listable_object {
  public:
    virtual ~enum_value()
    {
        if (name_p) {
            free(name_p);
            name_p = NULL;
        }
        if (value_p) {
            delete value_p;
            value_p = NULL;
        }
    }
    enum_value(mpl_compiler *compiler_p, char *name_p) :
        listable_object(compiler_p),
        name_p(name_p),
        value_p(NULL)
    {
    }
    enum_value(mpl_compiler *compiler_p, char *name_p, int64_t value) :
        listable_object(compiler_p),
        name_p(name_p)
    {
        value_p = new int64_t(value);
    }
    enum_value(mpl_compiler *compiler_p, char *name_p, int64_t *val_p) :
        listable_object(compiler_p),
        name_p(name_p)
    {
        if (val_p)
            value_p = new int64_t(*val_p);
        else
            value_p = NULL;
    }
    enum_value(enum_value &o) :
        listable_object(o)
    {
        CLONE_STR(name_p);
        CLONE_INT64_P(value_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(enum_value)

    char *name_p;
    int64_t *value_p;

    int64_t *get_value_p()
    {
        return value_p;
    }

    void set_value(int64_t val)
    {
        if (value_p)
            delete value_p;
        value_p = new int64_t(val);
    }

    char *get_name_p()
    {
        return name_p;
    }
};

class integer_range : public listable_object {
  public:
    virtual ~integer_range()
    {
        if (first_p) {
            delete first_p;
            first_p = NULL;
        }
        if (last_p) {
            delete last_p;
            last_p = NULL;
        }
    }
    integer_range(mpl_compiler *compiler_p) :
        listable_object(compiler_p),
        first_p(NULL),
        last_p(NULL)
    {
    }
    integer_range(mpl_compiler *compiler_p, int64_t first, int64_t last) :
        listable_object(compiler_p)
    {
        first_p = new int64_t(first);
        last_p = new int64_t(last);
    }
    integer_range(mpl_compiler *compiler_p, char *first_p, char *last_p);
    integer_range(integer_range &o) :
        listable_object(o)
    {
        CLONE_INT64_P(first_p);
        CLONE_INT64_P(last_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(integer_range)

    int64_t *first_p;
    int64_t *last_p;

    int64_t *get_first_p()
    {
        return first_p;
    }

    void set_first(int64_t first)
    {
        if (first_p)
            delete first_p;
        first_p = new int64_t(first);
    }

    int64_t *get_last_p()
    {
        return last_p;
    }

    void set_last(int64_t last)
    {
        if (last_p)
            delete last_p;
        last_p = new int64_t(last);
    }

    virtual void print(int level);
};

class number_range : public listable_object {
  public:
    virtual ~number_range();
    number_range(mpl_compiler *compiler_p, char *name_p) :
        listable_object(compiler_p),
        name_p(name_p),
        range_list_p(NULL)
    {
    }
    number_range(number_range &o) :
        listable_object(o)
    {
        CLONE_STR(name_p);
        CLONE_LISTABLE_OBJECT_LIST(range_list_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(number_range)

    char *name_p;
    mpl_list_t *range_list_p;

    void add_integer_range(integer_range *range_p)
    {
        range_p->append_to(range_list_p);
    }

    char *get_name_p()
    {
        return name_p;
    }

    virtual void print(int level);
};

class parameter_list_entry : public listable_object {
  public:
    virtual ~parameter_list_entry()
    {
        if (parameter_name_p) {
            free(parameter_name_p);
            parameter_name_p = NULL;
        }
        if (field_name_p) {
            free(field_name_p);
            field_name_p = NULL;
        }
    }
    parameter_list_entry(mpl_compiler *compiler_p,
                         parameter_set *parameter_set_p,
                         char *name_p,
                         char *field_name_p,
                         int optional,
                         int multiple,
                         direction_t direction) :
        listable_object(compiler_p),
        parameter_set_p(parameter_set_p),
        parameter_name_p(name_p),
        field_name_p(field_name_p),
        optional(optional),
        multiple(multiple),
        direction(direction),
        array(1)
    {
    }
    parameter_list_entry(parameter_list_entry &o) :
        listable_object(o),
        parameter_set_p(o.parameter_set_p),
        optional(o.optional),
        multiple(o.multiple),
        array(o.array),
        direction(o.direction)
    {
        CLONE_STR(parameter_name_p);
        CLONE_STR(field_name_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(parameter_list_entry)

    parameter_set *parameter_set_p;
    char *parameter_name_p;
    char *field_name_p;
    int optional;
    int multiple;
    int array;
    direction_t direction;

    virtual void set_option(char *option_p);
    virtual void add_dox_info(dox_entry *dox_entry_p,
                              const char *label_p,
                              char *value_p,
                              char *text_p);
    bool is_same_as(parameter_list_entry *entry_p);
    void print(int level);
};

typedef enum {
    method_type_command,
    method_type_event
} method_type_t;

class bag_parameter;

class method : public listable_object {
  public:
    virtual ~method()
    {
        if (name_p) {
            free(name_p);
            name_p = NULL;
        }
    }
    method(mpl_compiler *compiler_p,
           category *category_p,
           char *name_p,
           method_type_t type) :
        listable_object(compiler_p),
        category_p(category_p),
        name_p(name_p),
        type(type)
    {
    }
    method(method &o) :
        listable_object(o),
        type(o.type),
        category_p(o.category_p)
    {
        CLONE_STR(name_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(method)

    char *name_p;
    category *category_p;
    method_type_t type;

    virtual void wrap_up_definition()
    {
    }
};

class command : public method {
  public:
    virtual ~command();
    command(mpl_compiler *compiler_p, category *category_p, char *name_p) :
        method(compiler_p, category_p, name_p, method_type_command),
        command_bag_p(NULL),
        response_bag_p(NULL),
        cmd_dox_list_p(NULL),
        resp_dox_list_p(NULL),
        last_dox_entry_param_was_skipped(0),
        cmd_latex_list_p(NULL),
        resp_latex_list_p(NULL),
        last_latex_entry_param_was_skipped(0),
        last_parameter_list_entry_latex_target_p(NULL),
        cmd_help_list_p(NULL),
        last_help_entry_param_was_skipped(0),
        last_parameter_list_entry_help_target_p(NULL)
    {
    }
    command(command &o) :
        method(o),
        command_bag_p(o.command_bag_p),
        response_bag_p(o.response_bag_p),
        last_dox_entry_param_was_skipped(o.last_dox_entry_param_was_skipped),
        last_parameter_list_entry_latex_target_p(o.last_parameter_list_entry_latex_target_p)
    {
        CLONE_LISTABLE_OBJECT_LIST(cmd_dox_list_p);
        CLONE_LISTABLE_OBJECT_LIST(resp_dox_list_p);
        CLONE_LISTABLE_OBJECT_LIST(cmd_latex_list_p);
        CLONE_LISTABLE_OBJECT_LIST(resp_latex_list_p);
        CLONE_LISTABLE_OBJECT_LIST(cmd_help_list_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(command)

    bag_parameter *command_bag_p;
    bag_parameter *response_bag_p;
    mpl_list_t *cmd_dox_list_p;
    mpl_list_t *resp_dox_list_p;
    int last_dox_entry_param_was_skipped;
    mpl_list_t *cmd_latex_list_p;
    mpl_list_t *resp_latex_list_p;
    int last_latex_entry_param_was_skipped;
    parameter_list_entry* last_parameter_list_entry_latex_target_p;
    mpl_list_t *cmd_help_list_p;
    int last_help_entry_param_was_skipped;
    parameter_list_entry* last_parameter_list_entry_help_target_p;

    virtual void add_parameter_list_entry(parameter_list_entry *parameter_list_entry_p);
    virtual void wrap_up_definition();

    void api_hh(FILE *f, char *indent);
    void api_cc(FILE *f, char *indent);
    void api_hh_response_class(FILE *f, char *indent);
    void api_cc_response_class(FILE *f, char *indent);

    void print(int level);
    virtual void convert_doc();
    virtual void add_dox_info(dox_entry *dox_entry_p,
                              const char *label_p,
                              char *value_p,
                              char *text_p);
    virtual void add_latex_info(latex_entry *latex_entry_p,
                                const char *label_p,
                                char *value_p,
                                char *text_p);
    virtual void add_help_info(help_entry *help_entry_p,
                               const char *label_p,
                               char *value_p,
                               char *text_p);
    void deja(FILE *f, char *indent);
    void deja_main_proc(FILE *f, char *indent);
    void deja_send_proc(FILE *f, char *indent);
    void dox(FILE *f, int level);
    void latex(FILE *f, parameter_set *default_parameter_set_p);
    void help(std::ostream &os, parameter_set *default_parameter_set_p);
};

class event : public method {
  public:
    event(mpl_compiler *compiler_p, category *category_p, char *name_p) :
        method(compiler_p, category_p, name_p, method_type_event),
        event_bag_p(NULL),
        last_latex_entry_param_was_skipped(0),
        last_parameter_list_entry_latex_target_p(NULL)
    {
    }
    event(event &o) :
        method(o),
        event_bag_p(o.event_bag_p),
        last_latex_entry_param_was_skipped(o.last_latex_entry_param_was_skipped),
        last_parameter_list_entry_latex_target_p(o.last_parameter_list_entry_latex_target_p)
    {
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(event)

    bag_parameter *event_bag_p;
    int last_latex_entry_param_was_skipped;
    parameter_list_entry *last_parameter_list_entry_latex_target_p;

    virtual void add_parameter_list_entry(parameter_list_entry *parameter_list_entry_p);
    virtual void wrap_up_definition();

    void api_hh(FILE *f, char *indent);
    void api_cc(FILE *f, char *indent);
    void api_hh_event_class(FILE *f, char *indent);
    void api_cc_event_class(FILE *f, char *indent);

    void print(int level);
    virtual void convert_doc();
    virtual void add_dox_info(dox_entry *dox_entry_p,
                              const char *label_p,
                              char *value_p,
                              char *text_p);
    virtual void add_latex_info(latex_entry *latex_entry_p,
                                const char *label_p,
                                char *value_p,
                                char *text_p);
    void deja(FILE *f, char *indent);
    void dox(FILE *f, int level);
    void latex(FILE *f, parameter_set *default_parameter_set_p);
};

class methods : public listable_object {
    public:
    virtual ~methods();
    methods(mpl_compiler *compiler_p, category *category_p, method_type_t type) :
        listable_object(compiler_p),
        type(type),
        category_p(category_p),
        method_list_p(NULL)
    {
    }
    methods(methods& o) :
        listable_object(o),
        category_p(o.category_p),
        type(o.type)
    {
        CLONE_LISTABLE_OBJECT_LIST(method_list_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(methods)

    category *category_p;
    method_type_t type;
    mpl_list_t *method_list_p;

    virtual void convert_doc()
    {
        assert(0);
    }
};

class commands_class : public methods {
    public:
    commands_class(mpl_compiler *compiler_p, category *category_p) :
        methods(compiler_p, category_p, method_type_command)
    {
    }
    commands_class(commands_class& o) :
        methods(o)
    {
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(commands_class)


    virtual void convert_doc();
};

class events_class : public methods {
    public:
    events_class(mpl_compiler *compiler_p, category *category_p) :
        methods(compiler_p, category_p, method_type_event)
    {
    }
    events_class(events_class& o) :
        methods(o)
    {
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(events_class)

    virtual void convert_doc();
};

typedef enum {
    message_type_command,
    message_type_response,
    message_type_event
} message_type_t;


class message_spec : public listable_object {
  public:
    message_spec(mpl_compiler *compiler_p, message_type_t type) :
        listable_object(compiler_p),
        type(type),
        id_enumerator_list_p(NULL),
        message_bag_p(NULL)
    {
    }
    message_spec(message_spec &o) :
        listable_object(o),
        type(o.type),
        message_bag_p(o.message_bag_p)
    {
        CLONE_LISTABLE_OBJECT(enumerator_list,id_enumerator_list_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(message_spec)

    message_type_t type;

    /*
      - id enumerator list contains names of all methods and can be used
      for converting between command bag param ids and message bag param ids
    */
    enumerator_list *id_enumerator_list_p; /* enumerator list for messages */

    /*
      - message bag is a bag in the category's parameter set
      and it is a virtual parent for all method bags
    */
    bag_parameter *message_bag_p;

    virtual void add_to_enumerator(char *name_p);
    virtual void convert_doc(const char *name_p);
    virtual void print(int level);
    virtual void wrap_up_definition(block_type_t block_type);
};

class category : public inheritable_object {
    protected:
    bag_parameter *add_message_bag(message_spec *message_spec_p, char *name_p);

    public:
    virtual ~category()
    {
        if (command_spec_p) {
            delete command_spec_p;
            command_spec_p = NULL;
        }
        if (response_spec_p) {
            delete response_spec_p;
            response_spec_p = NULL;
        }
        if (event_spec_p) {
            delete event_spec_p;
            event_spec_p = NULL;
        }
    }
    category(mpl_compiler *compiler_p, char *name_p, category *parent_p) :
        inheritable_object(compiler_p, name_p, parent_p),
        parameter_set_p(NULL),
        command_spec_p(NULL),
        response_spec_p(NULL),
        event_spec_p(NULL),
        commands(compiler_p, this),
        events(compiler_p, this)
    {
        if (this->parent_p)
            this->parent_p->add_child(this);
    }
    category(category &o) :
        inheritable_object(o),
        parameter_set_p(o.parameter_set_p),
        commands(o.commands),
        events(o.events)
    {
        CLONE_LISTABLE_OBJECT(message_spec,command_spec_p);
        CLONE_LISTABLE_OBJECT(message_spec,response_spec_p);
        CLONE_LISTABLE_OBJECT(message_spec,event_spec_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(category)

    parameter_set *parameter_set_p;
    message_spec *command_spec_p;
    message_spec *response_spec_p;
    message_spec *event_spec_p;
    commands_class commands;
    events_class events;

    void print(int level);
    void print_commands(int level);
    void print_events(int level);
    void set_parameter_set(parameter_set *pset_p);
    bag_parameter *add_command_bag(char *name_p);
    bag_parameter *add_response_bag(char *name_p);
    bag_parameter *add_event_bag(char *name_p);
    command *add_command(char *name_p);
    event *add_event(char *name_p);
    int check_parameters();
    parameter_set *get_parameter_set();
    message_spec *get_command_spec();
    message_spec *get_response_spec();
    message_spec *get_event_spec();
    bag_parameter *get_command_bag();
    bag_parameter *get_response_bag();
    bag_parameter *get_event_bag();
    category *get_topmost_category();
    virtual void wrap_up_definition();

    void gc(FILE *hfile_p, FILE *cfile_p, mpl_list_t *categories_p);

    void api_hh(FILE *f, char *indent);
    void api_cc(FILE *f, char *indent);

    void convert_doc();
    void convert_doc_commands();
    void convert_doc_events();

    void deja(FILE *f, char *indent);
    void deja_events(FILE *f, char *indent);
    void deja_commands(FILE *f, char *indent);

    void cli_command_completions(FILE *f);
    void cli_parameter_completions(FILE *f, command *command_p);
    void cli_command_help(FILE *f);
    void cli_parameter_help(FILE *f, command *command_p);

    void dox(FILE *f);
    void dox_events(FILE *f);
    void dox_commands(FILE *f);

    void latex(FILE *f);
    void latex_events(FILE *f);
    void latex_commands(FILE *f);
};

class file : public listable_object {
  public:
    virtual ~file()
    {
        if (filename_p) {
            free(filename_p);
            filename_p = NULL;
        }
        if (stream_p) {
            delete stream_p;
            stream_p = NULL;
        }
    }
    file(mpl_compiler *compiler_p,
         char *filename_p,
         int lineno_offset,
         ifstream *stream_p) :
        listable_object(compiler_p),
        filename_p(filename_p),
        lineno_offset(lineno_offset),
        stream_p(stream_p)
    {
    }
    file(file &o) :
        listable_object(o),
        lineno_offset(o.lineno_offset),
        stream_p(o.stream_p)
    {
        CLONE_STR(filename_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(file)

    char *filename_p;
    int lineno_offset;
    ifstream *stream_p;

    virtual void add_latex_info(latex_entry *latex_entry_p,
                                const char *label_p,
                                char *value_p,
                                char *text_p);
};

class line : public listable_object {
  public:
    virtual ~line()
    {
        if (line_p) {
            free(line_p);
            line_p = NULL;
        }
    }
    line(mpl_compiler *compiler_p, char *line_p, code_segment_t segment) :
        listable_object(compiler_p),
        line_p(line_p),
        code_segment(segment)
    {
    }
    line(line &o) :
        listable_object(o),
        code_segment(o.code_segment)
    {
        CLONE_STR(line_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(line)

    char *line_p;
    code_segment_t code_segment;
};


class block : public listable_object {
  public:
    block(mpl_compiler *compiler_p, block_type_t block_type) :
        listable_object(compiler_p),
        block_type(block_type)
    {
    }
    block(block &o) :
        listable_object(o),
        block_type(o.block_type)
    {
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(block)

    block_type_t block_type;
};

#define IF_OPERATORS \
    IF_OPERATOR_VALUE_ELEMENT(none) \
    IF_OPERATOR_VALUE_ELEMENT(lt) \
    IF_OPERATOR_VALUE_ELEMENT(eq) \
    IF_OPERATOR_VALUE_ELEMENT(gt) \

#define IF_OPERATOR_VALUE_ELEMENT(ELEMENT) \
    if_operator_##ELEMENT,

enum if_operator_t {
    IF_OPERATORS
    number_of_if_operators
};
#undef IF_OPERATOR_VALUE_ELEMENT


class if_condition : listable_object {
  public:
    virtual ~if_condition()
    {
        if (key_p) {
            free(key_p);
            key_p = NULL;
        }
        if (value_p) {
            free(value_p);
            value_p = NULL;
        }
    }
    if_condition(mpl_compiler *compiler_p,
                 char *key_p,
                 if_operator_t if_operator,
                 char *value_p) :
        listable_object(compiler_p),
        key_p(key_p),
        if_operator(if_operator),
        value_p(value_p)
    {
    }
    if_condition(if_condition &o) :
        listable_object(o),
        if_operator(o.if_operator)
    {
        CLONE_STR(key_p);
        CLONE_STR(value_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(if_condition)

    char *key_p;
    if_operator_t if_operator;
    char *value_p;

    int check(compiler_define *compiler_define_p);
};

typedef enum {
    lexer_mode_normal,
    lexer_mode_doc
} lexer_mode_t;

class mpl_lexer : public yyFlexLexer {
  public:
    mpl_lexer(istream* input_stream,
              mpl_compiler *compiler_p,
              lexer_mode_t initial_mode = lexer_mode_normal) :
        yyFlexLexer(input_stream),
        compiler_p(compiler_p),
        initial_mode(initial_mode),
        is_initialized(0)
    {
        stream_is_string = (initial_mode == lexer_mode_doc);
    }

    mpl_compiler *compiler_p;
    YYSTYPE yylval;
    int initial_mode;
    int is_initialized;
    int stream_is_string;

    int yylex();

    int yylex(YYSTYPE *lvalp)
    {
        int token;

        token = yylex();
        *lvalp = yylval;
        return token;
    }
};


class mpl_compiler {
    private:
    void convert_doc_remembered_files();
    void convert_doc_categories();
    void convert_doc_parameter_sets();

    public:
    virtual ~mpl_compiler();
    mpl_compiler(istream *input_stream_p,
                 char *include_dir_name_p,
                 mpl_list_t *compiler_defines_p,
                 int experimental = 0,
                 codegen_mode_t codegen_mode = codegen_mode_mpl) :
        blocks_p(NULL),
        categories_p(NULL),
        current_category_p(NULL),
        direction(direction_in),
        parameter_sets_p(NULL),
        current_parameter_set_p(NULL),
        current_parameter_list_pp(NULL),
        current_parameter_p(NULL),
        current_command_p(NULL),
        current_event_p(NULL),
        compiler_options_p(NULL),
        current_if_operator(if_operator_none),
        files_p(NULL),
        remembered_files_p(NULL),
        compiler_defines_p(compiler_defines_p),
        hlines_p(NULL),
        clines_p(NULL),
        current_code_segment(code_segment_top),
        current_doc_list_target_pp(NULL),
        current_doc_list_p(NULL),
        doc_mode(doc_mode_normal),
        current_dox_list_p(NULL),
        current_latex_list_p(NULL),
        current_help_list_p(NULL),
        include_dir_name_p(include_dir_name_p),
        experimental(experimental),
        codegen_mode(codegen_mode),
        lexer_p(NULL),
        lexer_stream_p(input_stream_p),
        current_if_condition(this, NULL, if_operator_none, NULL),
        current_parameter_list_tmp_p(NULL),
        parser_mode(parser_mode_normal),
        groups_p(NULL),
        flags_p(NULL)
    {
        reset_current_parameter_list();
    }

    mpl_list_t *blocks_p;
    mpl_list_t *categories_p;
    category *current_category_p;
    direction_t direction;
    mpl_list_t *parameter_sets_p;
    parameter_set *current_parameter_set_p;
    mpl_list_t **current_parameter_list_pp;
    parameter *current_parameter_p;
    command *current_command_p;
    event *current_event_p;
    mpl_list_t *compiler_options_p;
    if_operator_t current_if_operator;
    mpl_list_t *files_p;
    mpl_list_t *remembered_files_p;
    mpl_list_t *compiler_defines_p;
    mpl_list_t *hlines_p;
    mpl_list_t *clines_p;
    code_segment_t current_code_segment;
    mpl_list_t **current_doc_list_target_pp;
    mpl_list_t *current_doc_list_p;
    doc_mode_t doc_mode;
    mpl_list_t *current_dox_list_p;
    mpl_list_t *current_latex_list_p;
    mpl_list_t *current_help_list_p;
    char *include_dir_name_p;
    int experimental;
    codegen_mode_t codegen_mode;
    istream *lexer_stream_p;
    mpl_lexer *lexer_p;
    if_condition current_if_condition;
    mpl_list_t *current_parameter_list_tmp_p;
    parser_mode_t parser_mode;
    mpl_list_t *groups_p;
    char *flags_p;

    void parse()
    {
        parse(lexer_mode_normal);
    }

    void parse(lexer_mode_t lexer_mode,
               parser_mode_t parser_mode = parser_mode_normal,
               const char *label_p = NULL)
    {
        this->parser_mode = parser_mode;
        if (label_p)
            push_file(label_p, 1);
        lexer_p = new mpl_lexer(lexer_stream_p, this, lexer_mode);
        yyparse(this);
        delete lexer_p;
    }

    void print()
    {
        convert_doc_remembered_files();
        print_files(remembered_files_p);
        forget_files();
        print_groups();
        convert_doc();
        print_compiler_defines();
        print_compiler_options();
        print_categories();
        printf("\n");
        print_parameter_sets();
        print_files(remembered_files_p);
    }

    void dox(FILE *doxfile_p)
    {
        dox_files(doxfile_p, remembered_files_p);
        forget_files();
        convert_doc();
        dox_parameter_sets(doxfile_p);
        dox_categories(doxfile_p);
    }

    void latex(FILE *latexfile_p)
    {
        convert_doc_remembered_files();
        forget_files();
        fprintf(latexfile_p,
                "\\documentclass[12pt]{article}\n"
                "\\begin{document}\n"
                "\\tableofcontents\n"
                "\\newpage\n"
               );
        convert_doc();
        latex_categories(latexfile_p);
        latex_parameter_sets(latexfile_p);
        latex_extra(this, latexfile_p);
        fprintf(latexfile_p,
                "\\end{document}\n"
               );
    }

    void dejagnu(FILE *expect_file_p, char *flags)
    {
        if (flags != NULL)
            this->flags_p = strdup(flags);
        deja_common(expect_file_p);
        deja_parameter_sets(expect_file_p);
        deja_categories(expect_file_p);
    }

    void generate_code(FILE *hfile_p, FILE *cfile_p, char *out_name_p)
    {
        if ((codegen_mode == codegen_mode_cli) || (codegen_mode == codegen_mode_api))
            convert_doc();
        gc_h_header(hfile_p, out_name_p);
        gc_c_header(cfile_p, out_name_p);
        gc_parameter_sets(hfile_p, cfile_p);
        gc_categories(hfile_p, cfile_p);
        gc_h_footer(hfile_p, out_name_p);
    }

    void convert_doc()
    {
        convert_doc_categories();
        convert_doc_parameter_sets();
    }

    int lineno()
    {
        if (lexer_p)
            return (lexer_p->lineno() - current_lineno_offset() + 1);
        else
            return 0;
    }

    void set_lexer_stream(istream *stream_p)
    {
        lexer_stream_p = stream_p;
    }

    void add_group(char *name_p, char *text_p, char *parent_name_p = NULL);
    void set_group_parent(char *name_p, char *parent_name_p);
    group_entry *get_group(const char *name_p);
    char *get_group_text(char *name_p);
    group_entry *get_group_parent(char *name_p);

    void add_prefix_to_current_parameter_set(char *prefix_p);
    void add_id_to_current_parameter_set(char *id_p);
    void add_range_id_to_current_parameter_set(char *range_id_p);
    void add_bag_field_table_suffix_to_current_parameter_set(char *suffix_p);

    void create_number_range_in_current_parameter_set(char *number_range_name_p);
    void add_integer_range_to_current_number_range(char *first_p, char *last_p);

    void add_enum_value_to_current_parameter(char *name_p, char *value_p);
    void add_option_to_current_parameter(const char *option_p,
                                         const char *value_p,
                                         value_type_t value_type);
    void add_option_to_current_parameter(const char *option_p,
                                         char *value1_p,
                                         value_type_t value_type1,
                                         char *value2_p,
                                         value_type_t value_type2);
    category *get_current_category()
    {
        return current_category_p;
    }
    void add_parameter_set_to_current_category(char *parameter_set_name_p);
    void add_command_bag_to_current_category(char *name_p);
    void add_response_bag_to_current_category(char *name_p);
    void add_event_bag_to_current_category(char *name_p);
    void add_command_to_current_category(char *name_p);
    void add_event_to_current_category(char *name_p);
    void create_parameter_in_current_parameter_set(const char *type_p,
                                                   char *name_p);

    void create_category(char *parent_name_p, char *name_p);
    category *find_category(char *name_p);

    void add_parent_to_current_parameter(char *parent_parameter_set_name_p,
                                         char *parent_name_p);

    void set_direction(direction_t dir) {direction = dir;}
    void check_parameters();

    void create_parameter_set(char *parameter_set_name_p);

    parameter_set *find_parameter_set(char *name_p);
    void set_current_parameter_set(char *name_p);
    void set_current_parameter_group(char *category_name_p);
    parameter_set *get_current_parameter_set();
    parameter_set *get_parameter_set_or_current(char *parameter_set_name_p);
    int num_parameter_sets() { return mpl_list_len(parameter_sets_p); }

    parameter_list_entry *create_parameter_list_entry(char *parameter_set_name_p,
                                                      char *name_p,
                                                      char *field_name_p,
                                                      int optional,
                                                      int multiple);
    void append_to_current_parameter_list(parameter_list_entry *parameter_list_entry_p);
    void set_array_option(char *option_p);


    void move_current_parameter_list_to_current_command();

    void move_current_parameter_list_to_current_event();

    void create_ellipsis();

    void create_compiler_option();
    void add_block_type_to_compiler_option(const char *block_type_p);
    void add_block_name_to_compiler_option(char *block_name_p);
    void add_value_to_compiler_option_and_create_new_if_not_first(char *option_name_p,
                                                                  char *option_value_p);
    int get_compiler_option(const char *block_type_p,
                            char *block_name_p,
                            const char *option_name_p,
                            char **option_value_pp);

    void add_left_operand_to_if_condition(char *left_operand_p);
    void add_right_operand_to_if_condition(char *right_operand_p);
    void add_operator_to_if_condition(char operator_char);
    void reset_if_condition();

    int check_if_condition();
    int check_ifdef_condition(char *if_condition_text_p);
    int check_if_condition_internal(char *if_condition_text_p);


    void create_enumerator_list_in_current_parameter_set(char *enumerator_list_name_p);
    void add_enumerator_list_value_to_current(char *name_p, char *value_p);
    void append_enumerator_list_to_current(char *parameter_set_name_p, char *enumerator_list_name_p);
    void append_enumerator_list_to_current_enum(char *parameter_set_name_p, char *enumerator_list_name_p);
    int is_enumerator_list(char *name_p);

    void push_file(const char *filename_p, int lineno_offset, ifstream *stream_p = NULL);
    char *peek_filename();
    int current_lineno_offset();
    file *pop_file();
    file *current_file();
    void remember_file(file *file_p, int on_top);
    void forget_files();

    void add_hline(char *line_p);
    void add_cline(char *line_p);

    void set_literal_code_option(const char *option_name_p,
                                 char *option_value_p);
    void reset_literal_code_options();


    void set_doc_mode(doc_mode_t mode);
    void create_doc(char *text_p, int is_forward);
    void set_backward_doc_target(mpl_list_t **doc_entry_pp);
    void add_any_forward_doc(mpl_list_t **doc_entry_pp);
    void reset_doc_target();
    void add_file_doc();

    void add_x_entry(const char *label_p, char *value_p, char *text_p = NULL);

    void push_block(block_type_t block_type);
    block *pop_block();
    block *peek_block();
    int is_inside_block(block_type_t block_type);
    void do_block_start(block_type_t block_type);
    void do_block_end();

    void gc_parameter_sets(FILE *hfile_p, FILE *cfile_p);
    void gc_categories(FILE *hfile_p, FILE *cfile_p);
    void gc_h_header(FILE *f, char *out_name_p);
    void gc_h_footer(FILE *f, char *out_name_p);
    void gc_c_header(FILE *f, char *out_name_p);
    void cli_c_common(FILE *f);
    bool bag_is_command(bag_parameter *bag_p);
    bool bag_is_response(bag_parameter *bag_p);
    bool bag_is_event(bag_parameter *bag_p);
    bool param_is_command_parameter(parameter *param_p);

    void deja_common(FILE *expect_file_p);
    void deja_parameter_sets(FILE *expect_file_p);
    void deja_categories(FILE *expect_file_p);

    void print_categories();
    void print_parameter_sets();
    void print_compiler_options();
    void print_compiler_defines();
    void print_groups();

    void reset_current_category();
    void reset_current_parameter_set();
    void reset_current_parameter_list();

    void dox_categories(FILE *f);
    void dox_parameter_sets(FILE *f);

    void latex_categories(FILE *f);
    void latex_parameter_sets(FILE *f);
};



#endif
