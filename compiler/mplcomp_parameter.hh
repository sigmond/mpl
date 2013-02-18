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
#ifndef mplcomp_parameter_hh
#define mplcomp_parameter_hh

#include "mplcomp.hh"
#include <assert.h>

/* Virtual base class for parameters */
class parameter : public inheritable_object {
  public:
    virtual ~parameter();
    parameter(mpl_compiler *compiler_p, char *name_p, const char *type_p, parameter_set *parameter_set_p) :
        inheritable_object(compiler_p, name_p, NULL),
        tcl_param_id_p(NULL),
        type_p(type_p),
        parameter_set_p(parameter_set_p),
        set_p(NULL),
        get_p(NULL),
        config_p(NULL),
        is_virtual(0),
        external_parent_parameter_name_p(NULL),
        external_parent_parameter_set_name_p(NULL)    {
    }
    parameter(parameter &o) :
        inheritable_object(o),
        type_p(o.type_p),
        is_virtual(o.is_virtual)
    {
        CLONE_STR(tcl_param_id_p);
        CLONE_LISTABLE_OBJECT(parameter_set,parameter_set_p);
        CLONE_INT_P(set_p);
        CLONE_INT_P(get_p);
        CLONE_INT_P(config_p);
        CLONE_STR(external_parent_parameter_name_p);
        CLONE_STR(external_parent_parameter_set_name_p);
    }

    char *tcl_param_id_p;
    parameter_set *parameter_set_p;
    const char *type_p;

    /* Properties that can be object-inherited: */
    int *set_p;
    int *get_p;
    int *config_p;


    /* A virtual parameter means that a parameter id is not generated */
    int is_virtual;
    /* Parameter inherits a parameter from another parameter set,
      no parent object, only names are known:
    */
    char *external_parent_parameter_name_p;
    char *external_parent_parameter_set_name_p;

    virtual const char *get_type() = 0;
    virtual const char *get_c_type() = 0;
    virtual const char *get_macro_type() = 0;
    virtual void print_type_description(FILE *f) = 0;
    virtual int is_basic() { return 1; }
    virtual int is_int() { return 0; }
    virtual int is_enum() { return 0; }
    virtual int is_bag() { return 0; }
    virtual int is_tuple() { return 0; }
    virtual int is_string() { return 0; }
    virtual int is_bool() { return 0; }
    virtual int is_array() { return 0; }
    virtual int is_addr() { return 0; }

    virtual int has_children() { return (child_list_p != NULL); }

    virtual void add_option(const char *option_p,
                            const char *value_p,
                            value_type_t value_type);
    virtual void add_option(const char *option_p,
                            char *value1_p,
                            value_type_t value_type1,
                            char *value2_p,
                            value_type_t value_type2);
    virtual int check_option(const char *option_p,
                             const char *value_p,
                             value_type_t value_type);
    virtual int check_option(const char *option_p,
                             const char *value1_p,
                             value_type_t value_type1,
                             char *value2_p,
                             value_type_t value_type2)
    {
        return -1;
    }

    virtual void gc_c_ranges(FILE *f, char *parameter_set_name_p);
    virtual void gc_c_field_values(FILE *f, char *parameter_set_name_p);
    virtual void gc_c_children(FILE *f, char *parameter_set_name_p);
    virtual void cli_c_write_value_help(FILE *f) { }
    virtual void cli_h_help(FILE *f);
    virtual void cli_c_help(FILE *f);
    virtual void help(ostream &os);

    virtual void api_hh(FILE* f, char *indent) { }
    virtual void api_cc(FILE* f, char *indent) { }

    virtual void latex_options(FILE *f);

    virtual void add_parent_parameter(char *parent_parameter_set_name_p,
                                      char *parent_name_p);
    virtual void wrap_up_definition() { }
    virtual void *get_property(const char *property_p);

    virtual void print(int level);
    virtual void convert_doc();
    virtual void deja(FILE *f);
    virtual const char *tcl_param_id();
    virtual void dox(FILE *f, int level);
    virtual void latex(FILE *f, parameter_set *default_parameter_set_p);
};


class int_parameter : public parameter {
  public:
    virtual ~int_parameter();
    int_parameter(mpl_compiler *compiler_p, char *name_p, const char *type_p, parameter_set *parameter_set_p) :
        parameter(compiler_p, name_p, type_p, parameter_set_p),
        default_p(NULL),
        max_p(NULL),
        min_p(NULL),
        number_ranges_p(NULL),
        type_min_str_p(NULL),
        type_max_str_p(NULL)
    {
    }
    int_parameter(int_parameter &o) :
        parameter(o),
        max_p(o.max_p),
        min_p(o.min_p),
        default_p(o.default_p)
    {
        CLONE_LISTABLE_OBJECT_CAST(int_parameter,parent_p);
        CLONE_LISTABLE_OBJECT_LIST(number_ranges_p);
        CLONE_STR(type_min_str_p);
        CLONE_STR(type_max_str_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(int_parameter)

    /* Properties that can be object-inherited: */
    const char *max_p;
    const char *min_p;
    const char *default_p;
    mpl_list_t *number_ranges_p;
    char *type_min_str_p;
    char *type_max_str_p;

    virtual const char *get_type() { return type_p; }
    virtual const char *get_c_type();
    virtual const char *get_macro_type() { return "INT"; }

    virtual void print_type_description(FILE *f);

    virtual int is_int() { return 1; }

    virtual const char *type_min();
    virtual const char *type_max();

    virtual void add_integer_range(char *first_p, char *last_p);
    virtual void add_number_range(const char *number_range_name_p);
    virtual void add_option(const char *option_p,
                            const char *value_p,
                            value_type_t value_type);
    virtual int check_option(const char *option_p,
                             const char *value_p,
                             value_type_t value_type);
    virtual void add_option(const char *option_p,
                            char *value1_p,
                            value_type_t value_type1,
                            char *value2_p,
                            value_type_t value_type2);
    virtual int check_option(const char *option_p,
                             const char *value1_p,
                             value_type_t value_type1,
                             char *value2_p,
                             value_type_t value_type2);
    virtual void *get_property(const char *property_p);
    virtual void gc_c_ranges(FILE *f, char *parameter_set_name_p);
    virtual void latex_options(FILE *f);
    virtual void cli_c_write_value_help(FILE *f);
    virtual void help(ostream &os);

    virtual void convert_doc();
    virtual void print(int level);
    virtual void print_number_ranges(int level);
};

class string_parameter : public parameter {
  public:
    virtual ~string_parameter();
    string_parameter(mpl_compiler *compiler_p, char *name_p, const char *type_p, parameter_set *parameter_set_p) :
        parameter(compiler_p, name_p, type_p, parameter_set_p),
        default_p(NULL),
        max_p(NULL),
        min_p(NULL)
    {
    }
    string_parameter(string_parameter &o) :
        parameter(o),
        max_p(o.max_p),
        min_p(o.min_p),
        default_p(o.default_p)
    {
        CLONE_LISTABLE_OBJECT_CAST(string_parameter,parent_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(string_parameter)

    /* Properties that can be object-inherited: */
    const char *max_p;
    const char *min_p;
    const char *default_p;

    virtual const char *get_type() { return type_p; }
    virtual const char *get_c_type();
    virtual const char *get_macro_type() { return "STRING"; }

    virtual void print_type_description(FILE *f);
    virtual int is_string() { return 1; }
    virtual int is_basic() { return 0; }

    virtual void add_option(const char *option_p,
                            const char *value_p,
                            value_type_t value_type);
    virtual int check_option(const char *option_p,
                             const char *value_p,
                             value_type_t value_type);
    virtual void *get_property(const char *property_p);
    virtual void latex_options(FILE *f);
    virtual void cli_c_write_value_help(FILE *f);
    virtual void help(ostream &os);
    virtual void deja(FILE *f);
    virtual void print(int level);
};

class bool_parameter : public parameter {
  public:
    virtual ~bool_parameter();
    bool_parameter(mpl_compiler *compiler_p, char *name_p, const char *type_p, parameter_set *parameter_set_p) :
        parameter(compiler_p, name_p, type_p, parameter_set_p),
        default_p(NULL)
    {
    }
    bool_parameter(bool_parameter &o) :
        parameter(o),
        default_p(o.default_p)
    {
        CLONE_LISTABLE_OBJECT_CAST(bool_parameter,parent_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(bool_parameter)

    /* Properties that can be object-inherited: */
    const char *default_p;

    virtual const char *get_type() { return type_p; }
    virtual const char *get_c_type();
    virtual const char *get_macro_type() { return "BOOL"; }

    virtual void print_type_description(FILE *f);
    virtual int is_bool() { return 1; }

    virtual void add_option(const char *option_p,
                            const char *value_p,
                            value_type_t value_type);
    virtual int check_option(const char *option_p,
                             const char *value_p,
                             value_type_t value_type);
    virtual void *get_property(const char *property_p);
    virtual void latex_options(FILE *f);
    virtual void deja(FILE *f);
    virtual void print(int level);
};

class array_parameter : public parameter {
  public:
    virtual ~array_parameter();
    array_parameter(mpl_compiler *compiler_p, char *name_p, const char *type_p, parameter_set *parameter_set_p) :
        parameter(compiler_p, name_p, type_p, parameter_set_p),
        max_p(NULL),
        min_p(NULL)
    {
    }
    array_parameter(array_parameter &o) :
        parameter(o),
        max_p(o.max_p),
        min_p(o.min_p)
    {
        CLONE_LISTABLE_OBJECT_CAST(array_parameter,parent_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(array_parameter)

    /* Properties that can be object-inherited: */
    const char *max_p;
    const char *min_p;

    virtual const char *get_type() { return type_p; }
    virtual const char *get_c_type();
    virtual const char *get_macro_type();

    virtual void print_type_description(FILE *f);
    virtual int is_array() { return 1; }
    virtual int is_basic() { return 0; }

    virtual void add_option(const char *option_p,
                            const char *value_p,
                            value_type_t value_type);
    virtual int check_option(const char *option_p,
                             const char *value_p,
                             value_type_t value_type);
    virtual void *get_property(const char *property_p);
    virtual void cli_c_write_value_help(FILE *f);
    virtual void latex_options(FILE *f);
    virtual void deja(FILE *f);
    virtual void print(int level);
};

class addr_parameter : public parameter {
  public:
    addr_parameter(mpl_compiler *compiler_p, char *name_p, const char *type_p, parameter_set *parameter_set_p) :
        parameter(compiler_p, name_p, type_p, parameter_set_p)
    {
    }
    addr_parameter(addr_parameter &o) :
        parameter(o)
    {
        CLONE_LISTABLE_OBJECT_CAST(addr_parameter,parent_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(addr_parameter)

    virtual const char *get_type() { return type_p; }
    virtual const char *get_c_type();
    virtual const char *get_macro_type() { return "ADDR"; }

    virtual void print_type_description(FILE *f);
    virtual int is_addr() { return 1; }

    virtual int check_option(const char *option_p,
                             const char *value_p,
                             value_type_t value_type);
    virtual void cli_c_write_value_help(FILE *f);
};

class enum_parameter : public parameter {
  public:
    virtual ~enum_parameter();
    enum_parameter(mpl_compiler *compiler_p, char *name_p, const char *type_p, parameter_set *parameter_set_p) :
        parameter(compiler_p, name_p, type_p, parameter_set_p),
        default_p(NULL),
        enum_values_p(NULL)
    {
    }
    enum_parameter(enum_parameter &o) :
        parameter(o),
        default_p(o.default_p)
    {
        CLONE_LISTABLE_OBJECT_LIST(enum_values_p);
        CLONE_LISTABLE_OBJECT_CAST(enum_parameter,parent_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(enum_parameter)

    /* Properties that can be object-inherited: */
    const char *default_p;
    mpl_list_t *enum_values_p;

    virtual const char *get_type() { return type_p; }
    virtual const char *get_c_type();
    virtual const char *get_macro_type() { return "ENUM"; }

    virtual void print_type_description(FILE *f);
    virtual int is_enum() { return 1; }

    virtual void add_option(const char *option_p,
                            const char *value_p,
                            value_type_t value_type);
    virtual int check_option(const char *option_p,
                             const char *value_p,
                             value_type_t value_type);
    virtual void wrap_up_definition();
    virtual void *get_property(const char *property_p);
    virtual void latex_options(FILE *f);

    void add_enum_value(char *name_p, char *value_p);
    enum_value *get_enum_value(const char *name_p);
    void add_enumerator_list_values(enumerator_list *enumerator_list_p);
    int num_entries_with_value(int64_t value);
    int num_entries_with_name(char *name_p);
    int64_t min_numeric_value();
    int64_t max_numeric_value();

    void gc_h_enum(FILE *f, char *parameter_set_name_p);
    void gc_c_enum(FILE *f, char *parameter_set_name_p);
    void cli_h_completions(FILE *f);
    void cli_c_completions(FILE *f);
    virtual void help(ostream &os);

    virtual void convert_doc();
    virtual void print(int level);
    virtual void deja(FILE *f);
    virtual void dox(FILE *f, int level);
    virtual void latex(FILE *f, parameter_set *default_parameter_set_p);
    void dox_enum_values(FILE *f, int level);
    void latex_enum_values(FILE *f);
};


class bag_parameter : public parameter {
    protected:
    int gc_c_check_parameter_list(FILE *f,
                                  const char *print_this_first_p);
    public:
    virtual ~bag_parameter();
    bag_parameter(mpl_compiler *compiler_p, char *name_p, const char *type_p, parameter_set *parameter_set_p) :
        parameter(compiler_p, name_p, "bag", parameter_set_p),
        bag_parameter_list_p(NULL),
        ellipsis_p(NULL),
        max_p(NULL),
        min_p(NULL),
        field_table_parameter_p(NULL),
        method_ref_p(NULL)
    {
    }
    bag_parameter(bag_parameter &o) :
        parameter(o),
        max_p(o.max_p),
        min_p(o.min_p),
        field_table_parameter_p(o.field_table_parameter_p),
        method_ref_p(o.method_ref_p)
    {
        CLONE_LISTABLE_OBJECT_CAST(bag_parameter,parent_p);
        CLONE_LISTABLE_OBJECT_LIST(bag_parameter_list_p);
        CLONE_LISTABLE_OBJECT(ellipsis,ellipsis_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(bag_parameter)

    /* Properties that can be object-inherited: */
    const char *max_p;
    const char *min_p;
    mpl_list_t *bag_parameter_list_p;
    ellipsis *ellipsis_p;

    /* Not inherited; cloned for each level */
    enum_parameter *field_table_parameter_p;

    const method *method_ref_p;

    virtual const char *get_type() { return type_p; }
    virtual const char *get_c_type();
    virtual const char *get_macro_type() { return "BAG"; }

    virtual void print_type_description(FILE *f);
    virtual int is_bag() { return 1; }
    virtual int is_basic() { return 0; }

    virtual void add_option(const char *option_p,
                            const char *value_p,
                            value_type_t value_type);
    virtual int check_option(const char *option_p,
                             const char *value_p,
                             value_type_t value_type);
    virtual void wrap_up_definition();
    void force_parameter_list_direction_unless_inout(direction_t direction);
    virtual void *get_property(const char *property_p);
    virtual bool have_own_parameters();
    virtual bag_parameter* field_context(char *field_name_p);
    virtual bag_parameter* parameter_list_entry_context(parameter_list_entry *entry_p);
    mpl_list_t *get_possible_parameters();
    virtual void latex_options(FILE *f);

    void gc_h_bag(FILE *f);
    void gc_c_bag(FILE *f);
    void cli_h_completions(FILE *f);
    void cli_c_completions(FILE *f);
    virtual void cli_h_help(FILE *f);
    virtual void cli_c_help(FILE *f);
    virtual void help(ostream &os);

    virtual void api_hh(FILE* f, char *indent);
    virtual void api_cc(FILE* f, char *indent);

    bool param_is_field_or_subfield(parameter *param_p);
    virtual void gc_c_field_values(FILE *f, char *parameter_set_name_p);
    virtual void print(int level);
    virtual void convert_doc();
    virtual void dox(FILE *f, int level);
    virtual void latex(FILE *f, parameter_set *default_parameter_set_p);
    void dox_bag_parameter_list(FILE *f, int level);
    void latex_bag_parameter_list(FILE *f, parameter_set *default_parameter_set_p);
    virtual void deja(FILE *f);
};

class tuple_parameter : public parameter {
  public:
    virtual ~tuple_parameter();
    tuple_parameter(mpl_compiler *compiler_p, char *name_p, const char *type_p, parameter_set *parameter_set_p) :
        parameter(compiler_p, name_p, type_p, parameter_set_p),
        default_p(NULL),
        default2_p(NULL),
        max_p(NULL),
        min_p(NULL)
    {
    }
    tuple_parameter(tuple_parameter &o) :
        parameter(o),
        max_p(o.max_p),
        min_p(o.min_p),
        default_p(o.default_p),
        default2_p(o.default2_p)
    {
        CLONE_LISTABLE_OBJECT_CAST(tuple_parameter,parent_p);
    }
    CLONE_LISTABLE_OBJECT_FUNC_DEFINE(tuple_parameter)

    /* Properties that can be object-inherited: */
    const char *max_p;
    const char *min_p;
    const char *default_p;
    const char *default2_p;

    virtual const char *get_type() { return type_p; }
    virtual const char *get_c_type();
    virtual const char *get_macro_type();

    virtual void print_type_description(FILE *f);
    virtual int is_tuple() { return 1; }
    virtual int is_basic() { return 0; }

    virtual void add_option(const char *option_p,
                            const char *value_p,
                            value_type_t value_type);
    virtual void add_option(const char *option_p,
                            char *value1_p,
                            value_type_t value_type1,
                            char *value2_p,
                            value_type_t value_type2);
    virtual int check_option(const char *option_p,
                             const char *value_p,
                             value_type_t value_type);
    virtual int check_option(const char *option_p,
                             const char *value1_p,
                             value_type_t value_type1,
                             char *value2_p,
                             value_type_t value_type2);
    virtual void *get_property(const char *property_p);
    virtual void cli_c_write_value_help(FILE *f);
    virtual void latex_options(FILE *f);

    virtual void deja(FILE *f);
    virtual void print(int level);
};


#endif
