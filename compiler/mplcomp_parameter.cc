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
#define __STDC_LIMIT_MACROS
#include <FlexLexer.h>
#include "mplcomp_parameter.hh"
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <sstream>

parameter::~parameter()
{
    if (tcl_param_id_p)
        free(tcl_param_id_p);
    if (set_p)
        delete set_p;
    if (get_p)
        delete get_p;
    if (config_p)
        delete config_p;
    if (external_parent_parameter_name_p)
        free(external_parent_parameter_name_p);
    if (external_parent_parameter_set_name_p)
        free(external_parent_parameter_set_name_p);
}

int_parameter::~int_parameter()
{
    if (max_p)
        free((char*)max_p);
    if (min_p)
        free((char*)min_p);
    if (default_p)
        free((char*)default_p);
    DELETE_LISTABLE_LIST(&number_ranges_p, number_range);
    if (type_min_str_p)
        free(type_min_str_p);
    if (type_max_str_p)
        free(type_max_str_p);
}

string_parameter::~string_parameter()
{
    if (max_p)
        free((char*)max_p);
    if (min_p)
        free((char*)min_p);
    if (default_p)
        free((char*)default_p);
}

bool_parameter::~bool_parameter()
{
    if (default_p)
        free((char*)default_p);
}

array_parameter::~array_parameter()
{
    if (max_p)
        free((char*)max_p);
    if (min_p)
        free((char*)min_p);
}

enum_parameter::~enum_parameter()
{
    if (default_p)
        free((char*)default_p);
    DELETE_LISTABLE_LIST(&enum_values_p, enum_value);
}

bag_parameter::~bag_parameter()
{
    if (max_p)
        free((char*)max_p);
    if (min_p)
        free((char*)min_p);
    DELETE_LISTABLE_LIST(&bag_parameter_list_p, parameter_list_entry);
    if (ellipsis_p)
        delete ellipsis_p;
}

tuple_parameter::~tuple_parameter()
{
    if (max_p)
        free((char*)max_p);
    if (min_p)
        free((char*)min_p);
    if (default_p)
        free((char*)default_p);
    if (default2_p)
        free((char*)default2_p);
}

const char *int_parameter::get_c_type()
{
    switch(get_type_of_int(get_type())) {
        case 1:
            return "int";
        case 8:
            return "uint8_t";
        case 16:
            return "uint16_t";
        case 32:
            return "uint32_t";
        case 64:
            return "uint64_t";
        case -8:
            return "sint8_t";
        case -16:
            return "sint16_t";
        case -32:
            return "sint32_t";
        case -64:
            return "int64_t";
        default:
            assert(0);
    }
    return NULL;
}

const char *bool_parameter::get_c_type()
{
    switch(get_type_of_bool(get_type())) {
        case 1:
            return "bool";
        case 8:
            return "uint8_t";
        default:
            assert(0);
    }
    return NULL;
}

const char *enum_parameter::get_c_type()
{
    switch(get_type_of_enum(get_type())) {
        case 1:
            return "int64_t";
        case 8:
            return "uint8_t";
        case 16:
            return "uint16_t";
        case 32:
            return "uint32_t";
        case -8:
            return "sint8_t";
        case -16:
            return "sint16_t";
        case -32:
            return "sint32_t";
        default:
            assert(0);
    }
    return NULL;
}

const char *string_parameter::get_c_type()
{
    switch(get_type_of_string(get_type())) {
        case 1:
            return "char *";
        case 2:
            return "wchar_t *";
        default:
            assert(0);
    }
    return NULL;
}

const char *tuple_parameter::get_c_type()
{
    switch(get_type_of_tuple(get_type())) {
        case 1:
            return "mpl_string_tuple_t";
        case -1:
            return "mpl_int_tuple_t";
        case 2:
            return "mpl_strint_tuple_t";
        case 8:
            return "mpl_struint8_tuple_t";
        default:
            assert(0);
    }
    return NULL;
}

const char *array_parameter::get_c_type()
{
    switch(get_type_of_array(get_type())) {
        case 8:
            return "mpl_uint8_array_t";
        case 16:
            return "mpl_uint16_array_t";
        case 32:
            return "mpl_uint32_array_t";
        default:
            assert(0);
    }
    return NULL;
}

const char *addr_parameter::get_c_type()
{
    if (get_type_of_addr(get_type()) == 0)
        return NULL;

    return "void *";
}

const char *bag_parameter::get_c_type()
{
    if (get_type_of_bag(get_type()) == 0)
        return NULL;

    return "mpl_bag_t *";
}

const char *array_parameter::get_macro_type()
{
    switch(get_type_of_array(get_type())) {
        case 8:
            return "UINT8_ARRAY";
        case 16:
            return "UINT16_ARRAY";
        case 32:
            return "UINT32_ARRAY";
        default:
            assert(0);
    }
    return NULL;
}

const char *tuple_parameter::get_macro_type()
{
    switch(get_type_of_tuple(get_type())) {
        case 1:
            return "STRING_TUPLE";
        case -1:
            return "INT_TUPLE";
        case 2:
            return "STRINT_TUPLE";
        case 8:
            return "STRUINT8_TUPLE";
        default:
            assert(0);
    }
    return NULL;
}

void int_parameter::print_type_description(FILE *f)
{
    int t = get_type_of_int(get_type());
    assert(t != 0);

    if (t <= 1)
        fprintf(f,
                "Signed integer type. "
               );
    else
        fprintf(f,
                "Unsigned integer type. "
               );

    switch (abs(t)) {
        case 1:
            fprintf(f,
                    "Size >= 4 bytes (compiler/cpu-dependant)."
                   );
            break;
        default:
                fprintf(f,
                        "Size = %d byte%s.",
                        abs(t) / 8,
                        abs(t) == 8 ? "" : "s"
                       );
    }
}

void string_parameter::print_type_description(FILE *f)
{
    int t = get_type_of_string(get_type());
    assert(t != 0);

    if (t == 1)
        fprintf(f,
                "Ascii string type."
               );
    else
        fprintf(f,
                "Wide character string type."
               );
}

void bool_parameter::print_type_description(FILE *f)
{
    int t = get_type_of_bool(get_type());
    assert(t != 0);

    if (t == 8)
        fprintf(f,
                "Boolean type. Size = 1 byte."
               );
    else
        fprintf(f,
                "Boolean type. Size is compiler/cpu-dependant."
               );
}

void enum_parameter::print_type_description(FILE *f)
{
    int t = get_type_of_enum(get_type());
    assert(t != 0);

    if (t <= 1)
        fprintf(f,
                "Signed enum type. "
               );
    else
        fprintf(f,
                "Unsigned enum type. "
               );

    switch (abs(t)) {
        case 1:
            fprintf(f,
                    "Size is compiler/cpu-dependant."
                   );
            break;
        default:
                fprintf(f,
                        "Size = %d byte%s.",
                        abs(t) / 8,
                        abs(t) == 8 ? "" : "s"
                       );
    }
}

void array_parameter::print_type_description(FILE *f)
{
    int t = get_type_of_array(get_type());
    assert(t != 0);

    fprintf(f,
            "Array of unsigned integers. "
            "Size of each array element = %d byte%s.",
                        abs(t) / 8,
                        abs(t) == 8 ? "" : "s"
                       );
}

void bag_parameter::print_type_description(FILE *f)
{
    int t = get_type_of_bag(get_type());
    assert(t != 0);

    fprintf(f,
            "Compound type that encapsulates other parameters. Members of the bag "
            "may occur in any order (sequence is not preserved). "
            "Members can be marked as being optional."
           );
}

void tuple_parameter::print_type_description(FILE *f)
{
    int t = get_type_of_tuple(get_type());
    assert(t != 0);

    fprintf(f,
            "Tuple type (key and value). "
           );

    switch (t) {
        case 1:
            fprintf(f,
                    "Key type = string. Value type = string."
                   );
            break;
        case -1:
            fprintf(f,
                    "Key type = int. Value type = int."
                   );
            break;
        case 2:
            fprintf(f,
                    "Key type = string. Value type = int."
                   );
            break;
        case 8:
            fprintf(f,
                    "Key type = string. Value type = uint8."
                   );
            break;
        default:
            assert(0);
    }
}

void addr_parameter::print_type_description(FILE *f)
{
    int t = get_type_of_addr(get_type());
    assert(t != 0);

    fprintf(f,
            "Address type. Size is compiler/cpu-dependant."
           );
}

void parameter::add_parent_parameter(char *parent_parameter_set_name_p,
                                     char *parent_name_p)
{
    parameter_set *parent_parameter_set_p;

    if (parent_p) {
        fprintf(stderr, "%s:%d: parent parameter '%s' already defined for '%s'\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                parent_p->name_p,
                name_p
               );
        exit(-1);
    }

    if (external_parent_parameter_name_p || external_parent_parameter_set_name_p) {
        fprintf(stderr, "%s:%d: external parent parameter %s::%s already defined for this parameter\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                external_parent_parameter_set_name_p,
                external_parent_parameter_name_p
               );
        exit(-1);
    }

    if (parent_parameter_set_name_p) {
        parent_parameter_set_p = compiler_p->find_parameter_set(parent_parameter_set_name_p);
        if (parent_parameter_set_p == NULL) {
            external_parent_parameter_name_p = strdup(parent_name_p);
            external_parent_parameter_set_name_p = strdup(parent_parameter_set_name_p);
            return;
        }
    }
    else {
        parent_parameter_set_p = parameter_set_p;
    }

    set_parent((inheritable_object *)parent_parameter_set_p->find_parameter(parent_name_p));
    if (parent_p == NULL) {
        fprintf(stderr, "%s:%d: parent-parameter %s does not exist in parameter set %s\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                parent_name_p, parent_parameter_set_p->name_p);
        exit(-1);
    }
    if (strcmp(get_type(), ((parameter*)parent_p)->get_type())) {
        fprintf(stderr, "%s:%d: parent parameter is not the same type (%s != %s)\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                ((parameter*)parent_p)->get_type(), get_type()
               );
        exit(-1);
    }
    ((parameter*)parent_p)->add_child(this);
}

void enum_parameter::add_enumerator_list_values(enumerator_list *enumerator_list_p)
{
    mpl_list_t *tmp_p;
    enum_value *src_enum_value_p;
    enum_value *dst_enum_value_p;

    MPL_LIST_FOR_EACH(enumerator_list_p->enumerator_list_values_p, tmp_p) {
        src_enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        dst_enum_value_p = new enum_value(*src_enum_value_p);
        dst_enum_value_p->append_to(enum_values_p);
    }
}

void enum_parameter::add_enum_value(char *name_p, char *value_p)
{
    int64_t value;
    enum_value *enum_value_p;

    if (value_p != NULL) {
        if (string2int64(value_p, &value) != 0) {
            fprintf(stderr, "%s:%d: Could not convert string '%s' to integer\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    value_p
                   );
            exit(-1);
        }

        enum_value_p = new enum_value(compiler_p, strdup(name_p), value);
    }
    else {
        enum_value_p = new enum_value(compiler_p, strdup(name_p));
    }

    compiler_p->add_any_forward_doc(&enum_value_p->doc_list_p);
    enum_value_p->append_to(enum_values_p);
}

enum_value *enum_parameter::get_enum_value(const char *name_p)
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("enum_values"), tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        if (!strcmp(enum_value_p->name_p, name_p))
            return enum_value_p;
    }
    return NULL;
}


void parameter::add_option(const char *option_p,
                           const char *value_p,
                           value_type_t value_type)
{
    if (check_option(option_p, value_p, value_type) != 0) {
        fprintf(stderr, "%s:%d: Invalid option %s = %s for parameter %s of type %s\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                option_p,
                value_p,
                name_p,
                type_p);
        exit(-1);
    }

    if (!strcmp(option_p, "set")) {
        set_p = new int((strcmp(value_p, "true") == 0));
    }
    else if (!strcmp(option_p, "get")) {
        get_p = new int((strcmp(value_p, "true") == 0));
    }
    else if (!strcmp(option_p, "config")) {
        config_p = new int((strcmp(value_p, "true") == 0));
    }
    else if (!strcmp(option_p, "virtual"))
        is_virtual = 1;
}

void int_parameter::add_option(const char *option_p,
                               const char *value_p,
                               value_type_t value_type)
{
    parameter::add_option(option_p, value_p, value_type);
    if (!strcmp(option_p, "max"))
        max_p = strdup(value_p);
    else if (!strcmp(option_p, "min"))
        min_p = strdup(value_p);
    else if (!strcmp(option_p, "default"))
        default_p = strdup(value_p);
    else if (!strcmp(option_p, "range"))
        add_number_range(value_p);
}

void string_parameter::add_option(const char *option_p,
                               const char *value_p,
                               value_type_t value_type)
{
    parameter::add_option(option_p, value_p, value_type);
    if (!strcmp(option_p, "max"))
        max_p = strdup(value_p);
    else if (!strcmp(option_p, "min"))
        min_p = strdup(value_p);
    else if (!strcmp(option_p, "default"))
        default_p = strdup(value_p);
}

void bool_parameter::add_option(const char *option_p,
                                const char *value_p,
                                value_type_t value_type)
{
    parameter::add_option(option_p, value_p, value_type);
    if (!strcmp(option_p, "default"))
        default_p = strdup(value_p);
}

void enum_parameter::add_option(const char *option_p,
                                const char *value_p,
                                value_type_t value_type)
{
    parameter::add_option(option_p, value_p, value_type);
    if (!strcmp(option_p, "default"))
        default_p = strdup(value_p);
}

void array_parameter::add_option(const char *option_p,
                                 const char *value_p,
                                 value_type_t value_type)
{
    parameter::add_option(option_p, value_p, value_type);
    if (!strcmp(option_p, "max"))
        max_p = strdup(value_p);
    else if (!strcmp(option_p, "min"))
        min_p = strdup(value_p);
}

void bag_parameter::add_option(const char *option_p,
                                 const char *value_p,
                                 value_type_t value_type)
{
    parameter::add_option(option_p, value_p, value_type);
    if (!strcmp(option_p, "max"))
        max_p = strdup(value_p);
    else if (!strcmp(option_p, "min"))
        min_p = strdup(value_p);
}

void tuple_parameter::add_option(const char *option_p,
                                 const char *value_p,
                                 value_type_t value_type)
{
    parameter::add_option(option_p, value_p, value_type);
    if (!strcmp(option_p, "max"))
        max_p = strdup(value_p);
    else if (!strcmp(option_p, "min"))
        min_p = strdup(value_p);
}

/* Second flavor of add_option (more parameters) */
void parameter::add_option(const char *option_p,
                           char *value1_p,
                           value_type_t value_type1,
                           char *value2_p,
                           value_type_t value_type2)
{
    fprintf(stderr, "%s:%d: Invalid option %s = %s,%s for parameter %s of type %s\n",
            compiler_p->peek_filename(), compiler_p->lineno(),
            option_p,
            value1_p,
            value2_p,
            name_p,
            type_p);
    exit(-1);
}

void int_parameter::add_option(const char *option_p,
                               char *value1_p,
                               value_type_t value_type1,
                               char *value2_p,
                               value_type_t value_type2)
{
    if (check_option(option_p,
                     value1_p,
                     value_type1,
                     value2_p,
                     value_type2) != 0) {
        fprintf(stderr, "%s:%d: Invalid option %s = %s,%s for parameter %s of type %s\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                option_p,
                value1_p,
                value2_p,
                name_p,
                type_p);
        exit(-1);
    }

    add_integer_range(value1_p, value2_p);
}

void tuple_parameter::add_option(const char *option_p,
                                 char *value1_p,
                                 value_type_t value_type1,
                                 char *value2_p,
                                 value_type_t value_type2)
{
    if (check_option(option_p,
                     value1_p,
                     value_type1,
                     value2_p,
                     value_type2) != 0) {
        fprintf(stderr, "%s:%d: Invalid option %s = %s,%s for parameter %s of type %s\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                option_p,
                value1_p,
                value2_p,
                name_p,
                type_p);
        exit(-1);
    }

    default_p = strdup(value1_p);
    default2_p = strdup(value2_p);
}

int parameter::check_option(const char *option_p, const char *value_p, value_type_t value_type)
{
    /* All types */
    if ((!strcmp(option_p, "set") ||
         !strcmp(option_p, "get") ||
         !strcmp(option_p, "config")) &&
        (value_type == value_type_bool))
            return 0;

    if (!strcmp(option_p, "virtual") && (value_type == value_type_bool))
        return 0;

    return -1;
}

int bool_parameter::check_option(const char *option_p, const char *value_p, value_type_t value_type)
{
    if (!strcmp(option_p, "default") && (value_type == value_type_name) &&
        (!strcmp(value_p,"true") || !strcmp(value_p, "false")))
        return 0;

    return parameter::check_option(option_p, value_p, value_type);
}

int array_parameter::check_option(const char *option_p, const char *value_p, value_type_t value_type)
{
    if (!strcmp(option_p, "max") && ((value_type == value_type_value)  || (value_type == value_type_name)))
        return 0;

    if (!strcmp(option_p, "min") && ((value_type == value_type_value)  || (value_type == value_type_name)))
        return 0;

    return parameter::check_option(option_p, value_p, value_type);
}

int string_parameter::check_option(const char *option_p, const char *value_p, value_type_t value_type)
{
    if (!strcmp(type_p, "string")) {
        if (!strcmp(option_p, "max") &&
            ((value_type == value_type_value) || (value_type == value_type_name)))
            return 0;
        if (!strcmp(option_p, "min") &&
            ((value_type == value_type_value) || (value_type == value_type_name)))
            return 0;
        if (!strcmp(option_p, "default") && (value_type == value_type_string_literal))
            return 0;
    }
    else if (!strcmp(type_p, "wstring")) {
        if (!strcmp(option_p, "max") && ((value_type == value_type_value)  || (value_type == value_type_name)))
            return 0;
        if (!strcmp(option_p, "min") && ((value_type == value_type_value)  || (value_type == value_type_name)))
            return 0;
        if (!strcmp(option_p, "default") && (value_type == value_type_string_literal) && (value_p[0] == 'L'))
            return 0;
    }

    return parameter::check_option(option_p, value_p, value_type);
}

int int_parameter::check_option(const char *option_p, const char *value_p, value_type_t value_type)
{
    if (!strcmp(option_p, "max") &&
        ((value_type == value_type_value) || (value_type == value_type_name)))
        return 0;
    if (!strcmp(option_p, "min") &&
        ((value_type == value_type_value) || (value_type == value_type_name)))
        return 0;
    if (!strcmp(option_p, "default") &&
        ((value_type == value_type_value) || (value_type == value_type_name)))
        return 0;
    if (!strcmp(option_p, "range") &&
        (value_type == value_type_name))
        return 0;

    return parameter::check_option(option_p, value_p, value_type);
}

int enum_parameter::check_option(const char *option_p, const char *value_p, value_type_t value_type)
{
    if (!strcmp(option_p, "default") && (value_type == value_type_name))
        return 0;

    return parameter::check_option(option_p, value_p, value_type);
}

int bag_parameter::check_option(const char *option_p, const char *value_p, value_type_t value_type)
{
    if (!strcmp(option_p, "max") && ((value_type == value_type_value)  || (value_type == value_type_name)))
            return 0;

    if (!strcmp(option_p, "min") && ((value_type == value_type_value)  || (value_type == value_type_name)))
            return 0;

    return parameter::check_option(option_p, value_p, value_type);
}

int addr_parameter::check_option(const char *option_p, const char *value_p, value_type_t value_type)
{
    return parameter::check_option(option_p, value_p, value_type);
}

int tuple_parameter::check_option(const char *option_p, const char *value_p, value_type_t value_type)
{
    if (!strcmp(option_p, "max") && ((value_type == value_type_value) || (value_type == value_type_name)))
        return 0;

    if (!strcmp(option_p, "min") && ((value_type == value_type_value) || (value_type == value_type_name)))
        return 0;

    return parameter::check_option(option_p, value_p, value_type);
}

int int_parameter::check_option(const char *option_p,
                                const char *value1_p,
                                value_type_t value_type1,
                                char *value2_p,
                                value_type_t value_type2)
{
    if (!strcmp(option_p, "range") &&
        (value_type1 == value_type_value) &&
        (value_type2 == value_type_value))
        return 0;

    return -1;
}

int tuple_parameter::check_option(const char *option_p,
                                  const char *value1_p,
                                  value_type_t value_type1,
                                  char *value2_p,
                                  value_type_t value_type2)
{
    /* Tuple types */
    if (!strcmp(type_p, "string_tuple")) {
        if (!strcmp(option_p, "default") &&
            (value_type1 == value_type_string_literal) &&
            (value_type2 == value_type_string_literal))
            return 0;
    }

    if (!strcmp(type_p, "int_tuple")) {
        if (!strcmp(option_p, "default") &&
            (value_type1 == value_type_value) &&
            (value_type2 == value_type_value))
            return 0;
    }

    if (!strcmp(type_p, "strint_tuple") ||
        !strcmp(type_p, "struint8_tuple")) {
        if (!strcmp(option_p, "default") &&
            (value_type1 == value_type_string_literal) &&
            (value_type2 == value_type_value))
            return 0;
    }

    return -1;
}

void *parameter::get_property(const char *property_p)
{
    if (!strcmp(property_p, "type")) {
        assert(type_p);
        return (void *) get_type();
    }

    if (!strcmp(property_p, "set"))
        if (set_p)
            return (void *) set_p;

    if (!strcmp(property_p, "get"))
        if (get_p)
            return (void *) get_p;

    if (!strcmp(property_p, "config"))
        if (config_p)
            return (void *) config_p;

    return NULL;
}

void *int_parameter::get_property(const char *property_p)
{
    void *ret_p;

    if ((ret_p = parameter::get_property(property_p)) != NULL)
        return ret_p;

    if (!strcmp(property_p, "min"))
        if (min_p)
            return (void *) min_p;

    if (!strcmp(property_p, "max"))
        if (max_p)
            return (void *) max_p;

    if (!strcmp(property_p, "default"))
        if (default_p)
            return (void *) default_p;

    if (!strcmp(property_p, "number_ranges"))
        if (number_ranges_p)
            return (void *) number_ranges_p;

    return parent_p ? parent_p->get_property(property_p) : NULL;
}

void *string_parameter::get_property(const char *property_p)
{
    void *ret_p;

    if ((ret_p = parameter::get_property(property_p)) != NULL)
        return ret_p;

    if (!strcmp(property_p, "min"))
        if (min_p)
            return (void *) min_p;

    if (!strcmp(property_p, "max"))
        if (max_p)
            return (void *) max_p;

    if (!strcmp(property_p, "default"))
        if (default_p)
            return (void *) default_p;

    return parent_p ? parent_p->get_property(property_p) : NULL;
}

void *bool_parameter::get_property(const char *property_p)
{
    void *ret_p;

    if ((ret_p = parameter::get_property(property_p)) != NULL)
        return ret_p;

    if (!strcmp(property_p, "default"))
        if (default_p)
            return (void *) default_p;

    return parent_p ? parent_p->get_property(property_p) : NULL;
}

void *array_parameter::get_property(const char *property_p)
{
    void *ret_p;

    if ((ret_p = parameter::get_property(property_p)) != NULL)
        return ret_p;

    if (!strcmp(property_p, "min"))
        if (min_p)
            return (void *) min_p;

    if (!strcmp(property_p, "max"))
        if (max_p)
            return (void *) max_p;

    return parent_p ? parent_p->get_property(property_p) : NULL;
}

void *bag_parameter::get_property(const char *property_p)
{
    void *ret_p;

    if ((ret_p = parameter::get_property(property_p)) != NULL)
        return ret_p;

    if (!strcmp(property_p, "min"))
        if (min_p)
            return (void *) min_p;

    if (!strcmp(property_p, "max"))
        if (max_p)
            return (void *) max_p;

    if (!strcmp(property_p, "parameter_list"))
        if (bag_parameter_list_p)
            return (void *) bag_parameter_list_p;

    if (!strcmp(property_p, "ellipsis"))
        if (ellipsis_p)
            return (void *) ellipsis_p;

    if (!strcmp(property_p, "field_table_parameter"))
        if (field_table_parameter_p)
            return (void *) field_table_parameter_p;

    return parent_p ? parent_p->get_property(property_p) : NULL;
}

void *enum_parameter::get_property(const char *property_p)
{
    void *ret_p;

    if ((ret_p = parameter::get_property(property_p)) != NULL)
        return ret_p;

    if (!strcmp(property_p, "default"))
        if (default_p)
            return (void *) default_p;

    if (!strcmp(property_p, "enum_values"))
        if (enum_values_p)
            return (void *) enum_values_p;

    return parent_p ? parent_p->get_property(property_p) : NULL;
}

void *tuple_parameter::get_property(const char *property_p)
{
    void *ret_p;

    if ((ret_p = parameter::get_property(property_p)) != NULL)
        return ret_p;

    if (!strcmp(property_p, "min"))
        if (min_p)
            return (void *) min_p;

    if (!strcmp(property_p, "max"))
        if (max_p)
            return (void *) max_p;

    if (!strcmp(property_p, "default"))
        if (default_p)
            return (void *) default_p;

    if (!strcmp(property_p, "default2"))
        if (default2_p)
            return (void *) default2_p;

    return parent_p ? parent_p->get_property(property_p) : NULL;
}

const char *int_parameter::type_min()
{
    if (type_min_str_p == NULL) {
        type_min_str_p = (char*) calloc(24, sizeof(char));
        switch(get_type_of_int(get_type())) {
            case 1: /* int */
                sprintf(type_min_str_p, "%d", INT_MIN);
                break;
            case 8: /* uint8 */
            case 16: /* uint16 */
            case 32: /* uint32 */
            case 64: /* uint64 */
                sprintf(type_min_str_p, "0");
                break;
            case -8: /* sint8 */
                sprintf(type_min_str_p, "%d", INT8_MIN);
                break;
            case -16: /* sint16 */
                sprintf(type_min_str_p, "%d", INT16_MIN);
                break;
            case -32: /* sint32 */
                sprintf(type_min_str_p, "%d", INT32_MIN);
                break;
            case -64: /* sint64 */
                sprintf(type_min_str_p, "%" PRIi64, INT64_MIN);
                break;
            default:
                assert(0);
        }
    }
    return type_min_str_p;
}

const char *int_parameter::type_max()
{
    if (type_max_str_p == NULL) {
        type_max_str_p = (char*) calloc(24, sizeof(char));
        switch(get_type_of_int(get_type())) {
            case 1: /* int */
                sprintf(type_max_str_p, "%d", INT_MAX);
                break;
            case 8: /* uint8 */
                sprintf(type_max_str_p, "%d", UINT8_MAX);
                break;
            case 16: /* uint16 */
                sprintf(type_max_str_p, "%d", UINT16_MAX);
                break;
            case 32: /* uint32 */
                sprintf(type_max_str_p, "%d", UINT32_MAX);
                break;
            case 64: /* uint64 */
                sprintf(type_max_str_p, "%" PRIu64, UINT64_MAX);
                break;
            case -8: /* sint8 */
                sprintf(type_max_str_p, "%d", INT8_MAX);
                break;
            case -16: /* sint16 */
                sprintf(type_max_str_p, "%d", INT16_MAX);
                break;
            case -32: /* sint32 */
                sprintf(type_max_str_p, "%d", INT32_MAX);
                break;
            case -64: /* sint64 */
                sprintf(type_max_str_p, "%" PRIi64, INT64_MAX);
                break;
            default:
                assert(0);
        }
    }
    return type_max_str_p;
}


void int_parameter::add_integer_range(char *first_p, char *last_p)
{
    number_range *number_range_p = new number_range(compiler_p,
                                                    strdup(NUMBER_RANGE_NAME_ANONYMOUS));
    integer_range *integer_range_p = new integer_range(compiler_p, first_p, last_p);
    number_range_p->add_integer_range(integer_range_p);
    number_range_p->add_to(number_ranges_p);
}

void int_parameter::add_number_range(const char *number_range_name_p)
{
    number_range *number_range_p = parameter_set_p->find_number_range(number_range_name_p);
    listable_object *nr_copy_p;
    if (number_range_p == NULL) {
        fprintf(stderr, "%s:%d: No number range %s in parameter set %s.\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                number_range_name_p,
                parameter_set_p->name_p);
        exit(-1);
    }
    nr_copy_p = number_range_p->clone();
    nr_copy_p->add_to(number_ranges_p);
}


void parameter::print(int level)
{
    mpl_list_t *tmp_p;

    printf("%s", spacing(level));
    printf("parameter\n");
    printf("%s", spacing(level));
    printf("->name_p = %s\n", name_p);
    printf("%s", spacing(level));
    printf("->type_p = %s\n", type_p);
    printf("%s", spacing(level));
    printf("->is_virtual = %d\n", is_virtual);
    printf("%s", spacing(level));
    printf("->set = %d\n", set_p ? *set_p : -1);
    printf("%s", spacing(level));
    printf("->get = %d\n", get_p ? *get_p : -1);
    printf("%s", spacing(level));
    printf("->config = %d\n", config_p ? *config_p : -1);
    listable_object::print(level);

    MPL_LIST_FOR_EACH(child_list_p, tmp_p) {
        object_container *child_p = LISTABLE_PTR(tmp_p, object_container);
        child_p->print(level);
    }

    if (child_list_p) {
        printf("%s", spacing(level));
        printf("flat child list:\n");
        mpl_list_t *fcl_p = get_flat_child_list();
        MPL_LIST_FOR_EACH(fcl_p, tmp_p) {
            object_container *child_p = LISTABLE_PTR(tmp_p, object_container);
            child_p->print(level);
        }
        DELETE_LISTABLE_LIST(&fcl_p, object_container);
    }

    if (parent_p) {
        printf("%s", spacing(level));
        printf("->parent %s\n", parent_p->name_p);
        return;
    }
    if (external_parent_parameter_name_p) {
        printf("%s", spacing(level));
        printf("->external_parent_parameter_name_p %s\n", external_parent_parameter_name_p);
        printf("%s", spacing(level));
        printf("->external_parent_parameter_set_name_p %s\n", external_parent_parameter_set_name_p);
        return;
    }
}

void int_parameter::print(int level)
{
    parameter::print(level);
    printf("%s", spacing(level));
    printf("->min_p = %s\n", min_p);
    printf("%s", spacing(level));
    printf("->max_p = %s\n", max_p);
    printf("%s", spacing(level));
    printf("->default_p = %s\n", default_p);
    if (number_ranges_p)
        print_number_ranges(level);
}

void int_parameter::print_number_ranges(int level)
{
    mpl_list_t *tmp_p;
    number_range *number_range_p;

    printf("%s", spacing(level));
    printf("number_ranges\n");
    MPL_LIST_FOR_EACH(number_ranges_p, tmp_p) {
        number_range_p = LISTABLE_PTR(tmp_p, number_range);
        number_range_p->print(level+1);
    }
}

void string_parameter::print(int level)
{
    parameter::print(level);
    printf("%s", spacing(level));
    printf("->min_p = %s\n", min_p);
    printf("%s", spacing(level));
    printf("->max_p = %s\n", max_p);
    printf("%s", spacing(level));
    printf("->default_p = %s\n", default_p);
}

void bool_parameter::print(int level)
{
    parameter::print(level);
    printf("%s", spacing(level));
    printf("->default_p = %s\n", default_p);
}

void array_parameter::print(int level)
{
    parameter::print(level);
    printf("%s", spacing(level));
    printf("->min_p = %s\n", min_p);
    printf("%s", spacing(level));
    printf("->max_p = %s\n", max_p);
}

void bag_parameter::print(int level)
{
    parameter::print(level);
    printf("%s", spacing(level));
    printf("->min_p = %s\n", min_p);
    printf("%s", spacing(level));
    printf("->max_p = %s\n", max_p);
    if (ellipsis_p)
        ellipsis_p->print(level);
    print_parameter_list(level+1, bag_parameter_list_p);
}

void enum_parameter::print(int level)
{
    parameter::print(level);
    printf("%s", spacing(level));
    printf("->default_p = %s\n", default_p);
    print_enum_values(level+1, enum_values_p);
}

void tuple_parameter::print(int level)
{
    parameter::print(level);
    printf("%s", spacing(level));
    printf("->min_p = %s\n", min_p);
    printf("%s", spacing(level));
    printf("->max_p = %s\n", max_p);
    printf("%s", spacing(level));
    printf("->default_p = %s\n", default_p);
    printf("%s", spacing(level));
    printf("->default2_p = %s\n", default2_p);
}

void parameter::gc_c_ranges(FILE *f, char *parameter_set_name_p)
{
    char *psl = parameter_set_name_p;
    char *pn = name_p;

    fprintf(f,
            "#define %s_ranges_%s NULL\n",
            psl,
            pn
           );
    fprintf(f,
            "#define %s_range_size_%s 0\n",
            psl,
            pn
           );
}

void int_parameter::gc_c_ranges(FILE *f, char *parameter_set_name_p)
{
    char *psl = parameter_set_name_p;
    char *pn = name_p;
    mpl_list_t *nr_p = (mpl_list_t*) get_property("number_ranges");

    if (nr_p == NULL) {
        parameter::gc_c_ranges(f, parameter_set_name_p);
        return;
    }

    fprintf(f,
            "\n/* %s range%s */\n",
            name_p,
            parent_p ? " (inherited)" : ""
           );

    if (parent_p && (nr_p == NULL)) {
        fprintf(f,
                "#define %s_ranges_%s %s_ranges_%s\n\n",
                psl,
                pn,
                ((parameter*)parent_p)->parameter_set_p->name_p,
                parent_p->name_p
               );
        fprintf(f,
                "#define %s_range_size_%s %s_range_size_%s\n",
                psl,
                pn,
                ((parameter*)parent_p)->parameter_set_p->name_p,
                parent_p->name_p
               );
        return;
    }

    fprintf(f,
            "const mpl_integer_range_t %s_ranges_%s[] =\n"
            "{\n",
            psl,
            pn
           );

    mpl_list_t *tmp_p;
    number_range *number_range_p;
    const char *sep_p = "";
    int range_size = 0;
    enum_parameter *number_range_parameter_p =
        (enum_parameter*) parameter_set_p->find_parameter(parameter_set_p->range_id_p->value_p);
    assert(number_range_parameter_p);

    MPL_LIST_FOR_EACH(nr_p, tmp_p) {
        number_range_p = LISTABLE_PTR(tmp_p, number_range);

        mpl_list_t *tmp_p;
        integer_range *integer_range_p;
        MPL_LIST_FOR_EACH(number_range_p->range_list_p, tmp_p) {
            integer_range_p = LISTABLE_PTR(tmp_p, integer_range);
            assert(integer_range_p->get_first_p());
            assert(integer_range_p->get_last_p());
            enum_value *enum_value_p =
                number_range_parameter_p->get_enum_value(number_range_p->name_p);
            assert(enum_value_p);
            assert(enum_value_p->get_value_p());
            fprintf(f,
                    "%s    {%" PRIi64 ",%" PRIi64 ", %" PRIi64 "}",
                    sep_p,
                    *integer_range_p->get_first_p(),
                    *integer_range_p->get_last_p(),
                    *enum_value_p->get_value_p()
                   );
            range_size++;
            sep_p = ",\n";
        }
    }
    fprintf(f,
            "\n"
            "};\n"
           );
    fprintf(f,
            "#define %s_range_size_%s %d\n",
            psl,
            pn,
            range_size
           );
    fprintf(f,
            "\n"
           );
}

void enum_parameter::gc_h_enum(FILE *f, char *parameter_set_name_p)
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;
    mpl_list_t *enum_values_p;
    int external_parent = (external_parent_parameter_name_p != NULL);
    char *epn = external_parent_parameter_name_p;
    char *epsl = external_parent_parameter_set_name_p;
    char *epsu = NULL;
    char *psl = parameter_set_name_p;
    char *psu = str_toupper(psl);
    char *pn = name_p;

    if (epsl)
        epsu = str_toupper(epsl);

    fprintf(f,
            "/* %s_%s_t %s */\n",
            psl,
            name_p,
            is_virtual ? "(virtual)" : ""
           );

    if (!external_parent) {
        fprintf(f,
                "#define %s_%sS \\\n",
                psu,
                pn
               );

        enum_values_p = (mpl_list_t *) get_property("enum_values");

        MPL_LIST_FOR_EACH(enum_values_p, tmp_p) {
            enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
            fprintf(f,
                    "  %s_%s_VALUE_ELEMENT(%s, %" PRIi64 ") \\\n",
                    psu,
                    pn,
                    enum_value_p->name_p,
                    *enum_value_p->value_p
                   );
        }
        fprintf(f,
                "\n"
               );
    }

    fprintf(f,
            "#define %s_%s_VALUE_ELEMENT(NAME, VALUE) \\\n",
            external_parent ? epsu : psu,
            external_parent ? epn : pn
           );
    fprintf(f,
            "  %s_%s_##NAME = VALUE,\n",
            psl,
            pn
           );
    fprintf(f,
            "\n"
           );

    if (get_type_of_enum(type_p) == 1) {
        fprintf(f,
                "typedef enum\n"
                "{\n"
                "  %s_%sS\n",
                external_parent ? epsu : psu,
                external_parent ? epn : pn
               );
    } else {
        fprintf(f,
                "TYPEDEF_ENUM\n"
                "{\n"
                "  %s_%sS\n",
                external_parent ? epsu : psu,
                external_parent ? epn : pn
               );
    }

    if (get_type_of_enum(type_p) == 1) {
        fprintf(f,
                "} %s_%s_t;\n",
                psl,
                pn
               );
    } else {
        int enum_size = get_type_of_enum(type_p);
        fprintf(f,
                "} %s%d (%s_%s_t);\n",
                enum_size > 0 ? "ENUM" : "SIGNED_ENUM",
                abs(enum_size),
                psl,
                pn
               );
    }
    fprintf(f,
            "\n"
           );
    fprintf(f,
            "#undef %s_%s_VALUE_ELEMENT\n",
            external_parent ? epsu : psu,
            external_parent ? epn : pn
            );
    fprintf(f,
            "\n"
           );
    if (external_parent) {
        fprintf(f,
                "#define %s_min_value_%s %s_min_value_%s\n",
                psl,
                pn,
                epsl,
                epn
               );
        fprintf(f,
                "#define %s_max_value_%s %s_max_value_%s\n",
                psl,
                pn,
                epsl,
                epn
               );
        fprintf(f,
                "#define %s_enum_size_%s %s_enum_size_%s\n",
                psl,
                pn,
                epsl,
                epn
               );
    } else {
        fprintf(f,
                "#define %s_min_value_%s %" PRIi64 "\n",
                psl,
                pn,
                min_numeric_value()
               );
        fprintf(f,
                "#define %s_max_value_%s %" PRIi64 "\n",
                psl,
                pn,
                max_numeric_value()
               );
        fprintf(f,
                "#define %s_enum_size_%s %d\n",
                psl,
                pn,
                (int) mpl_list_len(enum_values_p)
               );
    }

    if (!external_parent) {
        if (parent_p && (this->enum_values_p == NULL)) {
            fprintf(f,
                    "#define %s_names_%s %s_names_%s\n\n",
                    psl,
                    pn,
                    ((parameter*)parent_p)->parameter_set_p->name_p,
                    parent_p->name_p
                   );
        }
        else
            fprintf(f,
                    "extern const mpl_enum_value_t %s_names_%s[];\n",
                    psl,
                    pn
                   );
    }

    fprintf(f,
            "\n"
           );
    free(psu);
    if (epsu)
        free(epsu);
}

void enum_parameter::gc_c_enum(FILE *f, char *parameter_set_name_p)
{
    mpl_list_t *tmp_p;
    int external_parent = (external_parent_parameter_name_p != NULL);
    char *epn = external_parent_parameter_name_p;
    char *epsl = external_parent_parameter_set_name_p;
    char *epsu = NULL;
    char *psl = parameter_set_name_p;
    char *psu = str_toupper(psl);
    char *pn = name_p;

    if (epsl)
        epsu = str_toupper(epsl);

    fprintf(f,
            "/* %s names%s */\n",
            name_p,
            parent_p || external_parent ? " (inherited)" : ""
           );

    if (parent_p && (enum_values_p == NULL)) {
        fprintf(f,
                "#define %s_names_%s %s_names_%s\n\n",
                psl,
                pn,
                ((parameter*)parent_p)->parameter_set_p->name_p,
                parent_p->name_p
               );
        free(psu);
        if (epsu)
            free(epsu);
        return;
    }

    if (external_parent) {
        fprintf(f,
                "#define %s_names_%s %s_names_%s\n\n",
                psl,
                pn,
                epsl,
                epn
               );
        free(psu);
        if (epsu)
            free(epsu);
        return;
    }

    fprintf(f,
            "#define %s_%s_VALUE_ELEMENT(NAME, VALUE) { #NAME, VALUE }, \n",
            psu,
            pn
           );
    fprintf(f,
            "const mpl_enum_value_t %s_names_%s[] =\n"
            "{\n",
            psl,
            pn
           );
    fprintf(f,
            "  %s_%sS\n"
            "};\n",
            psu,
            pn
           );
    fprintf(f,
            "#undef %s_%s_VALUE_ELEMENT\n",
            psu,
            pn
           );
    fprintf(f,
            "\n"
           );
    free(psu);
    if (epsu)
        free(epsu);
}

bool bag_parameter::have_own_parameters()
{
    parameter_list_entry *parameter_list_entry_p;
    mpl_list_t *tmp_p;

    MPL_LIST_FOR_EACH((mpl_list_t*) get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if (parameter_list_entry_context(parameter_list_entry_p) == this)
            return true;
    }
    return false;
}

bag_parameter* bag_parameter::parameter_list_entry_context(parameter_list_entry *entry_p)
{
    mpl_list_t *tmp_p;
    object_container *oc_p;
    parameter_list_entry *parameter_list_entry_p;
    mpl_list_t *pl_p = get_parent_list();

    oc_p = new object_container(this);
    oc_p->append_to(pl_p);

    MPL_LIST_FOR_EACH(pl_p, tmp_p) {
        object_container *param_container_p = LISTABLE_PTR(tmp_p, object_container);
        bag_parameter *bag_p = (bag_parameter*) param_container_p->object_p;

        mpl_list_t *tmp_p;
        MPL_LIST_FOR_EACH((mpl_list_t*) bag_p->get_property("parameter_list"), tmp_p) {
            parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
            if (parameter_list_entry_p->is_same_as(entry_p)) {
                DELETE_LISTABLE_LIST(&pl_p, object_container);
                return bag_p;
            }
        }
        if (bag_p == this)
            break;
    }
    DELETE_LISTABLE_LIST(&pl_p, object_container);

    return NULL;
}

bag_parameter* bag_parameter::field_context(char *field_name_p)
{
    mpl_list_t *tmp_p;
    object_container *oc_p;
    parameter_list_entry *parameter_list_entry_p;
    mpl_list_t *pl_p = get_parent_list();

    oc_p = new object_container(this);
    oc_p->append_to(pl_p);

    MPL_LIST_FOR_EACH(pl_p, tmp_p) {
        object_container *param_container_p = LISTABLE_PTR(tmp_p, object_container);
        bag_parameter *bag_p = (bag_parameter*) param_container_p->object_p;

        mpl_list_t *tmp_p;
        MPL_LIST_FOR_EACH((mpl_list_t*) bag_p->get_property("parameter_list"), tmp_p) {
            parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
            if (parameter_list_entry_p->field_name_p == NULL)
                continue;
            if (!strcmp(parameter_list_entry_p->field_name_p, field_name_p)) {
                DELETE_LISTABLE_LIST(&pl_p, object_container);
                return bag_p;
            }
        }
        if (bag_p == this)
            break;
    }
    DELETE_LISTABLE_LIST(&pl_p, object_container);

    return NULL;
}

void parameter::gc_c_field_values(FILE *f, char *parameter_set_name_p)
{
    char *psl = parameter_set_name_p;
    char *pn = name_p;

    fprintf(f,
            "#define %s_field_values_%s NULL\n",
            psl,
            pn
           );
    fprintf(f,
            "#define %s_field_values_size_%s 0\n",
            psl,
            pn
           );
}

void bag_parameter::gc_c_field_values(FILE *f, char *parameter_set_name_p)
{
    char *psl = parameter_set_name_p;
    char *pn = name_p;
    mpl_list_t *pl_p = (mpl_list_t*) get_property("parameter_list");

    if (field_table_parameter_p == NULL) {
        parameter::gc_c_field_values(f, parameter_set_name_p);
        return;
    }

    fprintf(f,
            "\n/* %s field values */\n",
            name_p
           );

    fprintf(f,
            "const mpl_field_value_t %s_field_values_%s[] =\n"
            "{\n",
            psl,
            pn
           );

    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    const char *sep_p = "";
    int field_values_size = 0;
    int param_set_id = atoi(parameter_set_p->paramset_id_p->value_p);

    MPL_LIST_FOR_EACH(pl_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if (parameter_list_entry_p->field_name_p == NULL)
            continue;

        char *lnl = parameter_list_entry_p->parameter_set_p->name_p;
        char *lnu = str_toupper(lnl);
        char *pn = parameter_list_entry_p->parameter_name_p;
        parameter *parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(pn);
        int virt = (parameter_p->is_virtual != 0);
        char *fn = parameter_list_entry_p->field_name_p;
        bag_parameter *field_context_parameter_p = field_context(fn);
        char *plnl = field_context_parameter_p->parameter_set_p->name_p;
        char *plnu = str_toupper(plnl);
        char *ppn = field_context_parameter_p->name_p;
        int pvirt = (field_context_parameter_p->is_virtual != 0);

        assert(field_table_parameter_p);
        enum_value *field_enum_value_p =
            field_table_parameter_p->get_enum_value(parameter_list_entry_p->field_name_p);
        assert(field_enum_value_p && field_enum_value_p->get_value_p());
        int64_t field_id = *field_enum_value_p->get_value_p();

        fprintf(f,
                "%s    {"
                "\"%s\", "
                "%" PRIi64 ", "
                "MPL_BUILD_PARAMID(%d,%s_PARAM_SET_ID,MPL_PARAMID_TO_TYPE(%s_paramid_%s)), "
                "MPL_BUILD_PARAMID(%d,%s_PARAM_SET_ID,MPL_PARAMID_TO_TYPE(%s_paramid_%s))"
                "}",
                sep_p,
                fn,
                field_id,
                virt,
                lnu,
                lnl,
                pn,
                pvirt,
                plnu,
                plnl,
                ppn
               );
        field_values_size++;
        sep_p = ",\n";
        free(lnu);
        free(plnu);
    }
    fprintf(f,
            "\n"
            "};\n"
           );
    fprintf(f,
            "#define %s_field_values_size_%s %d\n",
            psl,
            pn,
            field_values_size
           );

    fprintf(f,
            "\n"
           );
}

void parameter::gc_c_children(FILE *f, char *parameter_set_name_p)
{
    char *psl = parameter_set_name_p;
    char *pn = name_p;

    if (child_list_p == NULL) {
        fprintf(f,
                "#define %s_children_%s NULL\n",
                psl,
                pn
               );
        fprintf(f,
                "#define %s_children_size_%s 0\n",
                psl,
                pn
               );
        return;
    }

    fprintf(f,
            "\n/* %s children */\n",
            name_p
           );

    fprintf(f,
            "const mpl_param_element_id_t %s_children_%s[] =\n"
            "{\n",
            psl,
            pn
           );

    const char *sep_p = "";
    mpl_list_t *tmp_p;
    int children_size = 0;
    mpl_list_t *fcl_p = get_flat_child_list();
    MPL_LIST_FOR_EACH(fcl_p, tmp_p) {
        object_container *child_container_p = LISTABLE_PTR(tmp_p, object_container);
        parameter *child_p = (parameter*) child_container_p->object_p;
        fprintf(f,
                "%s    %s_paramid_%s",
                sep_p,
                child_p->parameter_set_p->name_p,
                child_p->name_p
               );
        children_size++;
        sep_p = ",\n";
    }
    DELETE_LISTABLE_LIST(&fcl_p, object_container);
    fprintf(f,
            "\n"
            "};\n"
           );
    fprintf(f,
            "#define %s_children_size_%s %d\n",
            psl,
            pn,
            children_size
           );
}

static const char *param_c_type(parameter *parameter_p)
{
    static char t[100];
    sprintf(t, "%s%s%s%s",
            parameter_p->is_enum() ? parameter_p->parameter_set_p->name_p : "",
            parameter_p->is_enum() ? "_" : "",
            parameter_p->is_enum() ? parameter_p->name_p : "",
            parameter_p->is_enum() ? "_t" : parameter_p->get_c_type()
           );
    return t;
}


void bag_parameter::gc_h_bag(FILE *f)
{
    char *short_name_p = parameter_set_p->get_short_name();
    char *snu = str_toupper(short_name_p);
    char *lnl = parameter_set_p->name_p;
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    mpl_list_t *pl_p = (mpl_list_t*) get_property("parameter_list");
    parameter_set *ps_p;
    char group[100];

    /* In group *_UTIL */
    sprintf(group, "%s_FM_UTIL", snu);

    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Check bag for protocol errors.\n"
            "  * @param bag_elem_p (in) Parameter element of the bag\n"
            "  * @param result_list_pp (out) Parameter list containing parameters that are\n"
            "  *                       either missing or should not be present.\n"
            "  * @return 0 on success, >0 on protocol errors, <0 on memory allocation error\n"
            "  */\n",
            group
           );
    fprintf(f,
            "int %s_checkBag_%s(mpl_param_element_t *bag_elem_p, mpl_list_t **result_list_pp);\n",
            parameter_set_p->name_p,
            name_p
           );
    fprintf(f,
            "int %s_checkBag_%s_params(mpl_param_element_t *bag_elem_p, mpl_list_t **result_list_pp, mpl_list_t **allowed_parameters_pp);\n",
            parameter_set_p->name_p,
            name_p
           );

    MPL_LIST_FOR_EACH(pl_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if (parameter_list_entry_p->field_name_p == NULL)
            continue;

        bag_parameter *field_context_bag_p = field_context(parameter_list_entry_p->field_name_p);
        char *csnu = str_toupper(field_context_bag_p->parameter_set_p->get_short_name());

        ps_p = parameter_list_entry_p->parameter_set_p;
        parameter_p = ps_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_p->is_int() ||
            parameter_p->is_bool() ||
            parameter_p->is_enum() ||
            parameter_p->is_addr()) {

            /* In group *_ADD */
            sprintf(group, "%s_FM_ADD", snu);

            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Add value or variable of type %s to field '%s' of the parameters\n"
                    "  * '%s'.\n"
                    "  * Allocates list memory.\n"
                    "  *\n"
                    "  * @param paramsPtr (in/out) The parameters to add to\n"
                    "  * @param val (in) The %s to add (value or variable)\n"
                    "  */\n",
                    group,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    name_p,
                    param_c_type(parameter_p)
                   );
            fprintf(f,
                    "#define %s_ADD_%s_%s(paramsPtr, val) \\\n"
                    "    do { \\\n"
                    "        %s __val = val; \\\n"
                    "        %s_ADD_BAG_FIELD(paramsPtr, %s, %s, &__val); \\\n"
                    "    } while (0)\n",
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    param_c_type(parameter_p),
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p
                   );

            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Add value or variable of type %s to field '%s' of the parameters\n"
                    "  * '%s'.\n"
                    "  * Allocates list memory.\n"
                    "  *\n"
                    "  * @param paramsPtr (in/out) The parameters to add to\n"
                    "  * @param val (in) The %s to add (value or variable)\n"
                    "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                    "  */\n",
                    group,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    name_p,
                    param_c_type(parameter_p)
                   );
            fprintf(f,
                    "#define %s_ADD_%s_%s_TAG(paramsPtr, val, tag) \\\n"
                    "    do { \\\n"
                    "        %s __val = val; \\\n"
                    "        %s_ADD_BAG_FIELD_TAG(paramsPtr, %s, %s, &__val, tag); \\\n"
                    "    } while (0)\n",
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_p->get_c_type(),
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p
                   );
            if (parameter_p->has_children()) {
                fprintf(f,
                        "/**\n"
                        "  * @ingroup %s\n"
                        "  * Add value or variable of type %s to field '%s' of the parameters\n"
                        "  * '%s', where the value belongs to a child parameter.\n"
                        "  * Note that to use this macro, the child and parent parameters\n"
                        "  * must belong to the same parameter set.\n"
                        "  * Allocates list memory.\n"
                        "  *\n"
                        "  * @param paramsPtr (in/out) The parameters to add to\n"
                        "  * @param child (in) The child parameter name\n"
                        "  * @param val (in) The %s to add (value or variable)\n"
                        "  */\n",
                        group,
                        param_c_type(parameter_p),
                        parameter_list_entry_p->field_name_p,
                        name_p,
                        param_c_type(parameter_p)
                       );
                fprintf(f,
                        "#define %s_ADD_%s_%s_CHILD(paramsPtr, child, val) \\\n"
                        "    do { \\\n"
                        "        %s __val = val; \\\n"
                        "        %s_ADD_BAG_FIELD_CHILD(paramsPtr, %s, %s, %s_PARAM_ID(child), &__val); \\\n"
                        "    } while (0)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        parameter_p->get_c_type(),
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu
                       );

                fprintf(f,
                        "/**\n"
                        "  * @ingroup %s\n"
                        "  * Add value or variable of type %s to field '%s' of the parameters\n"
                        "  * '%s', where the value belongs to a child parameter.\n"
                        "  * Note that to use this macro, the child and parent parameters\n"
                        "  * must belong to the same parameter set.\n"
                        "  * Allocates list memory.\n"
                        "  *\n"
                        "  * @param paramsPtr (in/out) The parameters to add to\n"
                        "  * @param child (in) The child parameter name\n"
                        "  * @param val (in) The %s to add (value or variable)\n"
                        "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                        "  */\n",
                        group,
                        param_c_type(parameter_p),
                        parameter_list_entry_p->field_name_p,
                        name_p,
                        param_c_type(parameter_p)
                       );
                fprintf(f,
                        "#define %s_ADD_%s_%s_CHILD_TAG(paramsPtr, child, val, tag) \\\n"
                        "    do { \\\n"
                        "        %s __val = val; \\\n"
                        "        %s_ADD_BAG_FIELD_CHILD_TAG(paramsPtr, %s, %s, %s_PARAM_ID(child), &__val, tag); \\\n"
                        "    } while (0)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        parameter_p->get_c_type(),
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu
                       );
            }

            if (parameter_p->is_enum()) {
                fprintf(f,
                        "/**\n"
                        "  * @ingroup %s\n"
                        "  * Add named enum value of type %s to field '%s' of the parameters\n"
                        "  * '%s'.\n"
                        "  * Allocates list memory.\n"
                        "  *\n"
                        "  * @param paramsPtr (in/out) The parameters to add to\n"
                        "  * @param valName (in) The named enum value to add\n"
                        "  */\n",
                        group,
                        param_c_type(parameter_p),
                        parameter_list_entry_p->field_name_p,
                        name_p
                       );
                fprintf(f,
                        "#define %s_ADD_%s_%s_ENUM(paramsPtr, valName) \\\n"
                        "    do { \\\n"
                        "        %s_%s_t __val = %s_%s_ ##valName; \\\n"
                        "        %s_ADD_BAG_FIELD(paramsPtr, %s, %s, &__val); \\\n"
                        "    } while (0)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        lnl,
                        parameter_p->name_p,
                        lnl,
                        parameter_p->name_p,
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p
                       );
                fprintf(f,
                        "/**\n"
                        "  * @ingroup %s\n"
                        "  * Add named enum value of type %s to field '%s' of the parameters\n"
                        "  * '%s'.\n"
                        "  * Allocates list memory.\n"
                        "  *\n"
                        "  * @param paramsPtr (in/out) The parameters to add to\n"
                        "  * @param valName (in) The named enum value to add\n"
                        "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                        "  */\n",
                        group,
                        param_c_type(parameter_p),
                        parameter_list_entry_p->field_name_p,
                        name_p
                       );
                fprintf(f,
                        "#define %s_ADD_%s_%s_ENUM_TAG(paramsPtr, valName, tag) \\\n"
                        "    do { \\\n"
                        "        %s __val = %s_%s_ ##valName; \\\n"
                        "        %s_ADD_BAG_FIELD_TAG(paramsPtr, %s, %s, &__val, tag); \\\n"
                        "    } while (0)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        parameter_p->get_c_type(),
                        lnl,
                        parameter_p->name_p,
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p
                       );
                if (parameter_p->has_children()) {
                    fprintf(f,
                            "/**\n"
                            "  * @ingroup %s\n"
                            "  * Add named enum value of type %s to field '%s' of the parameters\n"
                            "  * '%s', where the value belongs to a child parameter.\n"
                            "  * Note that to use this macro, the child and parent parameters\n"
                            "  * must belong to the same parameter set.\n"
                            "  * Allocates list memory.\n"
                            "  *\n"
                            "  * @param paramsPtr (in/out) The parameters to add to\n"
                            "  * @param child (in) The child parameter name\n"
                            "  * @param valName (in) The named enum value to add\n"
                            "  */\n",
                            group,
                            param_c_type(parameter_p),
                            parameter_list_entry_p->field_name_p,
                            name_p
                           );
                    fprintf(f,
                            "#define %s_ADD_%s_%s_ENUM_CHILD(paramsPtr, child, valName) \\\n"
                            "    do { \\\n"
                            "        %s __val = %s_%s_ ##valName; \\\n"
                            "        %s_ADD_BAG_FIELD_CHILD(paramsPtr, %s, %s, %s_PARAM_ID(child), &__val); \\\n"
                            "    } while (0)\n",
                            snu,
                            name_p,
                            parameter_list_entry_p->field_name_p,
                            parameter_p->get_c_type(),
                            lnl,
                            parameter_p->name_p,
                            snu,
                            name_p,
                            parameter_list_entry_p->field_name_p,
                            snu
                           );
                    fprintf(f,
                            "/**\n"
                            "  * @ingroup %s\n"
                            "  * Add named enum value of type %s to field '%s' of the parameters\n"
                            "  * '%s', where the value belongs to a child parameter.\n"
                            "  * Note that to use this macro, the child and parent parameters\n"
                            "  * must belong to the same parameter set.\n"
                            "  * Allocates list memory.\n"
                            "  *\n"
                            "  * @param paramsPtr (in/out) The parameters to add to\n"
                            "  * @param child (in) The child parameter name\n"
                            "  * @param valName (in) The named enum value to add\n"
                            "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                            "  */\n",
                            group,
                            param_c_type(parameter_p),
                            parameter_list_entry_p->field_name_p,
                            name_p
                           );
                    fprintf(f,
                            "#define %s_ADD_%s_%s_ENUM_CHILD_TAG(paramsPtr, child, valName, tag) \\\n"
                            "    do { \\\n"
                            "        %s __val = %s_%s_ ##valName; \\\n"
                            "        %s_ADD_BAG_FIELD_CHILD_TAG(paramsPtr, %s, %s, %s_PARAM_ID(child), &__val, tag); \\\n"
                            "    } while (0)\n",
                            snu,
                            name_p,
                            parameter_list_entry_p->field_name_p,
                            parameter_p->get_c_type(),
                            lnl,
                            parameter_p->name_p,
                            snu,
                            name_p,
                            parameter_list_entry_p->field_name_p,
                            snu
                           );
                }
            }

            /* In group *_GET */
            sprintf(group, "%s_FM_GET", snu);
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Get field %s from bag %s.\n"
                    "  * @param bag_p (in) The bag value pointer\n"
                    "  * @return The value of the field.\n"
                    "  */\n",
                    group,
                    parameter_list_entry_p->field_name_p,
                    name_p
                   );
            if (field_context_bag_p == this)
                fprintf(f,
                        "%s %s_GET_%s_%s(mpl_bag_t *bag_p);\n",
                        param_c_type(parameter_p),
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p
                       );
            else
                fprintf(f,
                        "#define %s_GET_%s_%s(bag_p) \\\n"
                        "        %s_GET_%s_%s(bag_p)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        csnu,
                        field_context_bag_p->name_p,
                        parameter_list_entry_p->field_name_p
                       );
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Get field %s from bag %s.\n"
                    "  * @param bag_p (in) The bag value pointer\n"
                    "  * @param tag (in) the tag that the field must match\n"
                    "  * @return The value of the field.\n"
                    "  */\n",
                    group,
                    parameter_list_entry_p->field_name_p,
                    name_p
                   );
            if (field_context_bag_p == this)
                fprintf(f,
                        "%s %s_GET_%s_%s_TAG(mpl_bag_t *bag_p, int tag);\n",
                        param_c_type(parameter_p),
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p
                       );
            else
                fprintf(f,
                        "#define %s_GET_%s_%s_TAG(bag_p,tag) \\\n"
                        "        %s_GET_%s_%s_TAG((bag_p),(tag))\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        csnu,
                        field_context_bag_p->name_p,
                        parameter_list_entry_p->field_name_p
                       );
        }
        else if (parameter_p->is_string() ||
                 parameter_p->is_bag()) {
            /* In group *_ADD */
            sprintf(group, "%s_FM_ADD", snu);
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Add pointer of type %s to field '%s' of the bag\n"
                    "  * '%s'.\n"
                    "  * Allocates list memory.\n"
                    "  *\n"
                    "  * @param bag_p (in/out) The bag to add to\n"
                    "  * @param ptr (in) The %s to add\n"
                    "  */\n",
                    group,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    name_p,
                    param_c_type(parameter_p)
                   );
            fprintf(f,
                    "#define %s_ADD_%s_%s(bag_p, ptr) \\\n"
                    "    do { \\\n"
                    "        %s_ADD_BAG_FIELD(bag_p, %s, %s, ptr); \\\n"
                    "    } while (0)\n",
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p
                   );
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Add pointer of type %s to field '%s' of the bag\n"
                    "  * '%s'.\n"
                    "  * Allocates list memory.\n"
                    "  *\n"
                    "  * @param bag_p (in/out) The bag to add to\n"
                    "  * @param ptr (in) The %s to add\n"
                    "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                    "  */\n",
                    group,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    name_p,
                    param_c_type(parameter_p)
                   );
            fprintf(f,
                    "#define %s_ADD_%s_%s_TAG(bag_p, ptr, tag) \\\n"
                    "    do { \\\n"
                    "        %s_ADD_BAG_FIELD_TAG(bag_p, %s, %s, ptr, tag); \\\n"
                    "    } while (0)\n",
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p
                   );
            if (parameter_p->has_children()) {
                fprintf(f,
                        "/**\n"
                        "  * @ingroup %s\n"
                        "  * Add pointer of type %s to field '%s' of the bag\n"
                        "  * '%s', where the value belongs to a child parameter.\n"
                        "  * Note that to use this macro, the child and parent parameters\n"
                        "  * must belong to the same parameter set.\n"
                        "  * Allocates list memory.\n"
                        "  *\n"
                        "  * @param bag_p (in/out) The bag to add to\n"
                        "  * @param child (in) The child parameter name\n"
                        "  * @param ptr (in) The %s to add\n"
                        "  */\n",
                        group,
                        param_c_type(parameter_p),
                        parameter_list_entry_p->field_name_p,
                        name_p,
                        param_c_type(parameter_p)
                       );
                fprintf(f,
                        "#define %s_ADD_%s_%s_CHILD(bag_p, child, ptr) \\\n"
                        "    do { \\\n"
                        "        %s_ADD_BAG_FIELD_CHILD(bag_p, %s, %s, %s_PARAM_ID(child), ptr); \\\n"
                        "    } while (0)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu
                       );
                fprintf(f,
                        "/**\n"
                        "  * @ingroup %s\n"
                        "  * Add pointer of type %s to field '%s' of the bag\n"
                        "  * '%s', where the value belongs to a child parameter.\n"
                        "  * Note that to use this macro, the child and parent parameters\n"
                        "  * must belong to the same parameter set.\n"
                        "  * Allocates list memory.\n"
                        "  *\n"
                        "  * @param bag_p (in/out) The bag to add to\n"
                        "  * @param child (in) The child parameter name\n"
                        "  * @param ptr (in) The %s to add\n"
                        "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                        "  */\n",
                        group,
                        param_c_type(parameter_p),
                        parameter_list_entry_p->field_name_p,
                        name_p,
                        param_c_type(parameter_p)
                       );
                fprintf(f,
                        "#define %s_ADD_%s_%s_CHILD_TAG(bag_p, child, ptr, tag) \\\n"
                        "    do { \\\n"
                        "        %s_ADD_BAG_FIELD_TAG(bag_p, %s, %s, %s_PARAM_ID(child), ptr, tag); \\\n"
                        "    } while (0)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu
                       );
            }
            /* In group *_GET */
            sprintf(group, "%s_FM_GET", snu);
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Get field %s from bag %s.\n"
                    "  * @param bag_p (in) The bag value pointer\n"
                    "  * @return Pointer to the value of the field.\n"
                    "  */\n",
                    group,
                    parameter_list_entry_p->field_name_p,
                    name_p
                   );
            if (field_context_bag_p == this)
                fprintf(f,
                        "%s %s_GET_%s_%s_PTR(mpl_bag_t *bag_p);\n",
                        param_c_type(parameter_p),
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p
                       );
            else
                fprintf(f,
                        "#define %s_GET_%s_%s_PTR(bag_p) \\\n"
                        "        %s_GET_%s_%s_PTR(bag_p)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        csnu,
                        field_context_bag_p->name_p,
                        parameter_list_entry_p->field_name_p
                       );
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Get field %s from bag %s.\n"
                    "  * @param bag_p (in) The bag value pointer\n"
                    "  * @param tag (in) the tag that the field must match\n"
                    "  * @return Pointer to the value of the field.\n"
                    "  */\n",
                    group,
                    parameter_list_entry_p->field_name_p,
                    name_p
                   );
            if (field_context_bag_p == this)
                fprintf(f,
                        "%s %s_GET_%s_%s_PTR_TAG(mpl_bag_t *bag_p, int tag);\n",
                        param_c_type(parameter_p),
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p
                       );
            else
                fprintf(f,
                        "#define %s_GET_%s_%s_PTR_TAG(bag_p,tag) \\\n"
                        "        %s_GET_%s_%s_PTR_TAG((bag_p),(tag))\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        csnu,
                        field_context_bag_p->name_p,
                        parameter_list_entry_p->field_name_p
                       );
       }
        else if (parameter_p->is_array()) {
            const char *arr_type_str;

            switch(get_type_of_array(parameter_p->get_type())) {
                case 8:
                    arr_type_str = "mpl_uint8_array_t";
                    break;
                case 16:
                    arr_type_str = "mpl_uint16_array_t";
                    break;
                case 32:
                    arr_type_str = "mpl_uint32_array_t";
                    break;
                default:
                    assert(0);
            }

            /* In group *_ADD */
            sprintf(group, "%s_FM_ADD", snu);
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Add array of type %s to field '%s' of the bag\n"
                    "  * '%s'.\n"
                    "  * Allocates list memory.\n"
                    "  *\n"
                    "  * @param bag_p (in/out) The bag to add to\n"
                    "  * @param _arr_p (in) Array to add (%s_array_t*)\n"
                    "  */\n",
                    group,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    name_p,
                    param_c_type(parameter_p)
                   );
            fprintf(f,
                    "#define %s_ADD_%s_%s(bag_p, _arr_p) \\\n"
                    "    do { \\\n"
                    "        %s_ADD_BAG_FIELD(bag_p, %s, %s, _arr_p); \\\n"
                    "    } while (0)\n",
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p
                   );
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Add array of type %s to field '%s' of the bag\n"
                    "  * '%s'.\n"
                    "  * Allocates list memory.\n"
                    "  *\n"
                    "  * @param bag_p (in/out) The bag to add to\n"
                    "  * @param _arr_p (in) Array to add (%s_array_t*)\n"
                    "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                    "  */\n",
                    group,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    name_p,
                    param_c_type(parameter_p)
                   );
            fprintf(f,
                    "#define %s_ADD_%s_%s_TAG(bag_p, _arr_p, tag) \\\n"
                    "    do { \\\n"
                    "        %s_ADD_BAG_FIELD_TAG(bag_p, %s, %s, _arr_p, tag); \\\n"
                    "    } while (0)\n",
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p
                   );
            if (parameter_p->has_children()) {
                fprintf(f,
                        "/**\n"
                        "  * @ingroup %s\n"
                        "  * Add array of type %s to field '%s' of the bag\n"
                        "  * '%s', where the value belongs to a child parameter.\n"
                        "  * Note that to use this macro, the child and parent parameters\n"
                        "  * must belong to the same parameter set.\n"
                        "  * Allocates list memory.\n"
                        "  *\n"
                        "  * @param bag_p (in/out) The bag to add to\n"
                        "  * @param child (in) The child parameter name\n"
                        "  * @param _arr_p (in) Array to add (%s_array_t*)\n"
                        "  */\n",
                        group,
                        param_c_type(parameter_p),
                        parameter_list_entry_p->field_name_p,
                        name_p,
                        param_c_type(parameter_p)
                       );
                fprintf(f,
                        "#define %s_ADD_%s_%s_CHILD(bag_p, child, _arr_p) \\\n"
                        "    do { \\\n"
                        "        %s_ADD_BAG_FIELD_CHILD(bag_p, %s, %s, %s_PARAM_ID(child), _arr_p); \\\n"
                        "    } while (0)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu
                       );
                fprintf(f,
                        "/**\n"
                        "  * @ingroup %s\n"
                        "  * Add array of type %s to field '%s' of the bag\n"
                        "  * '%s', where the value belongs to a child parameter.\n"
                        "  * Note that to use this macro, the child and parent parameters\n"
                        "  * must belong to the same parameter set.\n"
                        "  * Allocates list memory.\n"
                        "  *\n"
                        "  * @param bag_p (in/out) The bag to add to\n"
                        "  * @param child (in) The child parameter name\n"
                        "  * @param _arr_p (in) Array to add (%s_array_t*)\n"
                        "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                        "  */\n",
                        group,
                        param_c_type(parameter_p),
                        parameter_list_entry_p->field_name_p,
                        name_p,
                        param_c_type(parameter_p)
                       );
                fprintf(f,
                        "#define %s_ADD_%s_%s_CHILD_TAG(bag_p, child, _arr_p, tag) \\\n"
                        "    do { \\\n"
                        "        %s_ADD_BAG_FIELD_CHILD_TAG(bag_p, %s, %s, %s_PARAM_ID(child), _arr_p, tag); \\\n"
                        "    } while (0)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu
                       );
            }
            /* In group *_GET */
            sprintf(group, "%s_FM_GET", snu);
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Get pointer to the field %s from bag %s.\n"
                    "  * @param bag_p (in) The bag value pointer\n"
                    "  * @return Pointer to the array.\n"
                    "  */\n",
                    group,
                    parameter_list_entry_p->field_name_p,
                    name_p
                   );
            if (field_context_bag_p == this)
                fprintf(f,
                        "%s * %s_GET_%s_%s_PTR(mpl_bag_t *bag_p);\n",
                        arr_type_str,
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p
                       );
            else
                fprintf(f,
                        "#define %s_GET_%s_%s_PTR(bag_p) \\\n"
                        "        %s_GET_%s_%s_PTR(bag_p)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        csnu,
                        field_context_bag_p->name_p,
                        parameter_list_entry_p->field_name_p
                       );
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Get pointer to the field %s from bag %s.\n"
                    "  * @param bag_p (in) The bag value pointer\n"
                    "  * @param tag (in) the tag that the field must match\n"
                    "  * @return Pointer to the array.\n"
                    "  */\n",
                    group,
                    parameter_list_entry_p->field_name_p,
                    name_p
                   );
            if (field_context_bag_p == this)
                fprintf(f,
                        "%s * %s_GET_%s_%s_PTR_TAG(mpl_bag_t *bag_p, int tag);\n",
                        arr_type_str,
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p
                       );
            else
                fprintf(f,
                        "#define %s_GET_%s_%s_PTR_TAG(bag_p,tag) \\\n"
                        "        %s_GET_%s_%s_PTR_TAG((bag_p),(tag))\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        csnu,
                        field_context_bag_p->name_p,
                        parameter_list_entry_p->field_name_p
                       );
        }
        else if (parameter_p->is_tuple()) {
            /* In group *_ADD */
            sprintf(group, "%s_FM_ADD", snu);
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Add tuple of type %s to field '%s' of the bag\n"
                    "  * '%s'.\n"
                    "  * Allocates list memory.\n"
                    "  *\n"
                    "  * @param bag_p (in/out) The bag to add to\n"
                    "  * @param _tup (in) Tuple to add (%s *)\n"
                    "  */\n",
                    group,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    name_p,
                    param_c_type(parameter_p)
                   );
            fprintf(f,
                    "#define %s_ADD_%s_%s(bag_p, _tup) \\\n"
                    "    do { \\\n"
                    "        %s_ADD_BAG_FIELD(bag_p, %s, %s, _tup); \\\n"
                    "    } while (0)\n",
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p
                   );
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Add tuple of type %s to field '%s' of the bag\n"
                    "  * '%s'.\n"
                    "  * Allocates list memory.\n"
                    "  *\n"
                    "  * @param _bag_p (in/out) The bag to add to\n"
                    "  * @param _tup (in) Tuple to add (%s *)\n"
                    "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                    "  */\n",
                    group,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    name_p,
                    param_c_type(parameter_p)
                   );
            fprintf(f,
                    "#define %s_ADD_%s_%s_TAG(_bag_p, _tup, tag) \\\n"
                    "    do { \\\n"
                    "        %s_ADD_BAG_FIELD_TAG(_bag_p, %s, %s, _tup, tag); \\\n"
                    "    } while (0)\n",
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p
                   );
            if (parameter_p->has_children()) {
                fprintf(f,
                        "/**\n"
                        "  * @ingroup %s\n"
                        "  * Add tuple of type %s to field '%s' of the bag\n"
                        "  * '%s', where the value belongs to a child parameter.\n"
                        "  * Note that to use this macro, the child and parent parameters\n"
                        "  * must belong to the same parameter set.\n"
                        "  * Allocates list memory.\n"
                        "  *\n"
                        "  * @param _bag_p (in/out) The bag to add to\n"
                        "  * @param _child (in) The child parameter name\n"
                        "  * @param _tup (in) Tuple to add (%s *)\n"
                        "  */\n",
                        group,
                        param_c_type(parameter_p),
                        parameter_list_entry_p->field_name_p,
                        name_p,
                        param_c_type(parameter_p)
                       );
                fprintf(f,
                        "#define %s_ADD_%s_%s_CHILD(_bag_p, _child, _tup) \\\n"
                        "    do { \\\n"
                        "        %s_ADD_BAG_FIELD_CHILD(_bag_p, %s, %s, %s_PARAM_ID(_child), _tup); \\\n"
                        "    } while (0)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu
                       );
                fprintf(f,
                        "/**\n"
                        "  * @ingroup %s\n"
                        "  * Add tuple of type %s to field '%s' of the bag\n"
                        "  * '%s', where the value belongs to a child parameter.\n"
                        "  * Note that to use this macro, the child and parent parameters\n"
                        "  * must belong to the same parameter set.\n"
                        "  * Allocates list memory.\n"
                        "  *\n"
                        "  * @param _bag_p (in/out) The bag to add to\n"
                        "  * @param _child (in) The child parameter name\n"
                        "  * @param _tup (in) Tuple to add (%s *)\n"
                        "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                        "  */\n",
                        group,
                        param_c_type(parameter_p),
                        parameter_list_entry_p->field_name_p,
                        name_p,
                        param_c_type(parameter_p)
                       );
                fprintf(f,
                        "#define %s_ADD_%s_%s_CHILD_TAG(_bag_p, _child, _tup, tag) \\\n"
                        "    do { \\\n"
                        "        %s_ADD_BAG_FIELD_CHILD_TAG(_bag_p, %s, %s, %s_PARAM_ID(_child), _tup, tag); \\\n"
                        "    } while (0)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        snu
                       );
            }
            /* In group *_GET */
            sprintf(group, "%s_FM_GET", snu);
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Get the field %s from bag %s.\n"
                    "  * @param bag_p (in) The bag value pointer\n"
                    "  * @return Pointer to the tuple (%s *).\n"
                    "  */\n",
                    group,
                    parameter_list_entry_p->field_name_p,
                    name_p,
                    param_c_type(parameter_p)
                   );
            if (field_context_bag_p == this)
                fprintf(f,
                        "%s *%s_GET_%s_%s_PTR(mpl_bag_t *bag_p);\n",
                        param_c_type(parameter_p),
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p
                       );
            else
                fprintf(f,
                        "#define %s_GET_%s_%s_PTR(bag_p) \\\n"
                        "        %s_GET_%s_%s_PTR(bag_p)\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        csnu,
                        field_context_bag_p->name_p,
                        parameter_list_entry_p->field_name_p
                       );
            fprintf(f,
                    "/**\n"
                    "  * @ingroup %s\n"
                    "  * Get the field %s from bag %s.\n"
                    "  * @param bag_p (in) The bag value pointer\n"
                    "  * @param tag (in) the tag that the field must match\n"
                    "  * @return Pointer to the tuple (%s *).\n"
                    "  */\n",
                    group,
                    parameter_list_entry_p->field_name_p,
                    name_p,
                    param_c_type(parameter_p)
                   );
            if (field_context_bag_p == this)
                fprintf(f,
                        "%s *%s_GET_%s_%s_PTR_TAG(mpl_bag_t *bag_p, int tag);\n",
                        param_c_type(parameter_p),
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p
                       );
            else
                fprintf(f,
                        "#define %s_GET_%s_%s_PTR_TAG(bag_p,tag) \\\n"
                        "        %s_GET_%s_%s_PTR_TAG((bag_p),(tag))\n",
                        snu,
                        name_p,
                        parameter_list_entry_p->field_name_p,
                        csnu,
                        field_context_bag_p->name_p,
                        parameter_list_entry_p->field_name_p
                       );
        }
        /* In group *_GET */
        sprintf(group, "%s_FM_GET", snu);
        fprintf(f,
                "/**\n"
                "  * @ingroup %s\n"
                "  * Get pointer to the parameter element of field %s from bag %s.\n"
                "  * @param bag_p (in) The bag value pointer\n"
                "  * @return Pointer to the parameter element.\n"
                "  */\n",
                group,
                parameter_list_entry_p->field_name_p,
                name_p
               );
        fprintf(f,
                "#define %s_GET_%s_%s_ELEMENT_PTR(bag_p) \\\n"
                "    %s_GET_BAG_FIELD_ELEMENT_PTR(bag_p, %s, %s)\n",
                snu,
                name_p,
                parameter_list_entry_p->field_name_p,
                snu,
                name_p,
                parameter_list_entry_p->field_name_p
               );
        fprintf(f,
                "/**\n"
                "  * @ingroup %s\n"
                "  * Get pointer to the parameter element of field %s from bag %s.\n"
                "  * @param bag_p (in) The bag value pointer\n"
                "  * @param tag (in) the tag that the field must match\n"
                "  * @return Pointer to the parameter element.\n"
                "  */\n",
                group,
                parameter_list_entry_p->field_name_p,
                name_p
               );
        fprintf(f,
                "#define %s_GET_%s_%s_ELEMENT_PTR_TAG(bag_p, tag) \\\n"
                "    %s_GET_BAG_FIELD_ELEMENT_PTR_TAG(bag_p, %s, %s, tag)\n",
                snu,
                name_p,
                parameter_list_entry_p->field_name_p,
                snu,
                name_p,
                parameter_list_entry_p->field_name_p
               );
        fprintf(f,
                "/**\n"
                "  * @ingroup %s\n"
                "  * Check if the field %s is present (and has a value) in bag %s.\n"
                "  * @param bag_p (in) The bag value pointer\n"
                "  * @return boolean (true if the field is present and has a value)\n"
                "  */\n",
                group,
                parameter_list_entry_p->field_name_p,
                name_p
               );
        fprintf(f,
                "#define %s_%s_%s_EXISTS(bag_p) \\\n"
                "    (%s_GET_BAG_FIELD_ELEMENT_PTR(bag_p, %s, %s) != NULL)\n",
                snu,
                name_p,
                parameter_list_entry_p->field_name_p,
                snu,
                name_p,
                parameter_list_entry_p->field_name_p
               );
        fprintf(f,
                "/**\n"
                "  * @ingroup %s\n"
                "  * Check if the field %s is present (and has a value) in bag %s.\n"
                "  * @param bag_p (in) The bag value pointer\n"
                "  * @param tag (in) the tag that the field must match\n"
                "  * @return boolean (true if the field is present and has a value)\n"
                "  */\n",
                group,
                parameter_list_entry_p->field_name_p,
                name_p
               );
        fprintf(f,
                "#define %s_%s_%s_EXISTS_TAG(bag_p, tag) \\\n"
                "    (%s_GET_BAG_FIELD_ELEMENT_PTR_TAG(bag_p, %s, %s, tag) != NULL)\n",
                snu,
                name_p,
                parameter_list_entry_p->field_name_p,
                snu,
                name_p,
                parameter_list_entry_p->field_name_p
               );
        fprintf(f,
                "/**\n"
                "  * @ingroup %s\n"
                "  * Return number of %s fields in bag.\n"
                "  * @param bag_p (in) The bag value pointer\n"
                "  * @return Number of fields\n"
                "  */\n",
                group,
                parameter_list_entry_p->field_name_p
               );
        fprintf(f,
                "#define %s_%s_%s_FIELD_COUNT(bag_p) \\\n"
                "    mpl_param_list_field_count(%s_PARAM_ID(%s), \\\n"
                "                               %s_FIELD_INDEX(%s, %s), \\\n"
                "                               bag_p)\n",
                snu,
                name_p,
                parameter_list_entry_p->field_name_p,
                snu,
                name_p,
                snu,
                name_p,
                parameter_list_entry_p->field_name_p
               );
        fprintf(f,
                "/**\n"
                "  * @ingroup %s\n"
                "  * Return number of %s fields in bag with tag.\n"
                "  * @param bag_p (in) The bag value pointer\n"
                "  * @param tag (in) the tag that the field must match\n"
                "  * @return Number of fields\n"
                "  */\n",
                group,
                parameter_list_entry_p->field_name_p
               );
        fprintf(f,
                "#define %s_%s_%s_FIELD_COUNT_TAG(bag_p, tag) \\\n"
                "    mpl_param_list_field_count_tag(%s_PARAM_ID(%s), \\\n"
                "                               %s_FIELD_INDEX(%s, %s), \\\n"
                "                               tag, bag_p)\n",
                snu,
                name_p,
                parameter_list_entry_p->field_name_p,
                snu,
                name_p,
                snu,
                name_p,
                parameter_list_entry_p->field_name_p
               );
        free(csnu);
    }
    free(snu);
}


int bag_parameter::gc_c_check_parameter_list(FILE *f,
                                             const char *print_this_first_p)
{
    int has_ellipsis = (ellipsis_p != NULL);
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    int count = 0;
    char *cn = name_p;
    char *clnl = parameter_set_p->name_p;
    char *csnl = parameter_set_p->get_short_name();
    char *csnu = str_toupper(csnl);

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter *parameter_p;
        parameter_set *ps_p;
        char *bag_name_p;
        char *snl;
        char *snu;
        char *lnl;
        char *lnu;
        char *fn = NULL;
        char *pn;
        int m;
        int o;
        int a;

        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);

        if (parameter_list_entry_context(parameter_list_entry_p) != this)
            continue;

        ps_p = parameter_list_entry_p->parameter_set_p;
        snl = ps_p->get_short_name();
        if (snl == NULL) {
            fprintf(f,
                    "    /* Parameter set %s has no short name defined -> no parameter checking code\n"
                    "       for parameter %s\n"
                    "    */\n",
                    ps_p->name_p,
                    parameter_list_entry_p->parameter_name_p
                   );
            continue;
        }

        snu = str_toupper(snl);
        lnl = ps_p->name_p;
        lnu = str_toupper(lnl);
        pn = parameter_list_entry_p->parameter_name_p;
        fn = parameter_list_entry_p->field_name_p;
        m = parameter_list_entry_p->multiple;
        o = parameter_list_entry_p->optional;
        a = parameter_list_entry_p->array;

        parameter_p = ps_p->find_parameter(pn);

        if (parameter_p->is_bag())
            bag_name_p = parameter_p->name_p;
        else
            bag_name_p = NULL;

        if (print_this_first_p) {
            fprintf(f,
                    "%s\n",
                    print_this_first_p
                   );
            print_this_first_p = NULL;
        }

        if (!o || !m) {
            fprintf(f,
                    "    if ("
                   );

            if (fn)
                fprintf(f,
                        "%s_%s_%s_FIELD_COUNT(bag_p) %s 1",
                        csnu,
                        cn,
                        fn,
                        o ? ">" : (m ? "<" : "!=")
                       );
            else
                fprintf(f,
                        "%s_PARAM_COUNT(bag_p,%s) %s 1",
                        snu,
                        pn,
                        o ? ">" : (m ? "<" : "!=")
                       );

            fprintf(f,
                    ") {\n"
                   );

            fprintf(f,
                    "        mpl_param_element_t *param_elem_p;\n"
                    "        mpl_param_element_id_t id;\n"
                   );
            if (fn)
                fprintf(f,
                        "        id = mpl_param_get_bag_field_id(%s_PARAM_ID(%s), %s_FIELD_INDEX(%s,%s));\n"
                        "        param_elem_p = mpl_param_element_create_empty(MPL_PARAMID_VIRTUAL_CLEAR(id));\n"
                        "        if (param_elem_p == NULL)\n"
                        "            return %s_MEMORY_ERROR;\n"
                        "        MPL_PARAM_ELEMENT_SET_FIELD_INFO(param_elem_p,%s_PARAM_ID(%s),%s_FIELD_INDEX(%s,%s));\n",
                        csnu,
                        cn,
                        csnu,
                        cn,
                        fn,
                        lnu,
                        csnu,
                        cn,
                        csnu,
                        cn,
                        fn
                       );
            else
                fprintf(f,
                        "        id = %s_PARAM_ID(%s);\n"
                        "        param_elem_p = mpl_param_element_create_empty(MPL_PARAMID_VIRTUAL_CLEAR(id));\n"
                        "        if (param_elem_p == NULL)\n"
                        "            return %s_MEMORY_ERROR;\n",
                        snu,
                        pn,
                        lnu
                       );
            fprintf(f,
                    "        mpl_list_append(result_list_pp, &param_elem_p->list_entry);\n"
                   );
            fprintf(f,
                    "    }\n"
                   );
            count++;
        }

        if (m && a) {
            if (print_this_first_p) {
              fprintf(f,
                      "%s\n",
                      print_this_first_p
                     );
              print_this_first_p = NULL;
            }
            fprintf(f,
                    "    {\n"
                    "        mpl_list_t *array_p = NULL;\n"
                    "        size_t i;\n"
                   );
            if (fn)
                fprintf(f,
                        "        array_p = %s_GET_BAG_FIELD_ALL(bag_p,%s,%s);\n",
                        csnu,
                        cn,
                        fn
                       );
            else
                fprintf(f,
                        "        array_p = %s_GET_ALL(bag_p,%s);\n",
                        snu,
                        pn
                       );

            fprintf(f,
                    "        for (i = 1; i <= mpl_list_len(array_p); i++) {\n"
                    "            mpl_param_element_t *param_elem_p;\n"
                    "            mpl_param_element_id_t id;\n"
                    "            if ("
                   );

            if (fn)
                fprintf(f,
                        "%s_%s_%s_FIELD_COUNT_TAG(array_p,i) != 1",
                        csnu,
                        cn,
                        fn
                       );
            else
                fprintf(f,
                        "%s_PARAM_COUNT_TAG(array_p,%s,i) != 1",
                        snu,
                        pn
                       );

            fprintf(f,
                    ") {\n"
                   );
            if (fn)
                fprintf(f,
                        "                id = mpl_param_get_bag_field_id(%s_PARAM_ID(%s), %s_FIELD_INDEX(%s,%s));\n"
                        "                param_elem_p = mpl_param_element_create_empty_tag(MPL_PARAMID_VIRTUAL_CLEAR(id),i);\n"
                        "                if (param_elem_p == NULL)\n"
                        "                    return %s_MEMORY_ERROR;\n"
                        "                MPL_PARAM_ELEMENT_SET_FIELD_INFO(param_elem_p,%s_PARAM_ID(%s),%s_FIELD_INDEX(%s,%s));\n",
                        csnu,
                        cn,
                        csnu,
                        cn,
                        fn,
                        lnu,
                        csnu,
                        cn,
                        csnu,
                        cn,
                        fn
                       );
            else
                fprintf(f,
                        "                id = %s_PARAM_ID(%s);\n"
                        "                param_elem_p = mpl_param_element_create_empty_tag(MPL_PARAMID_VIRTUAL_CLEAR(id),i);\n"
                        "                if (param_elem_p == NULL)\n"
                        "                    return %s_MEMORY_ERROR;\n",
                        snu,
                        pn,
                        lnu
                       );
            fprintf(f,
                    "                mpl_list_append(result_list_pp, &param_elem_p->list_entry);\n"
                   );
            fprintf(f,
                    "            }\n"
                   );
            if (bag_name_p != NULL) {
                fprintf(f,
                        "            if("
                       );
                if (fn)
                    fprintf(f,
                            "%s_%s_%s_FIELD_COUNT_TAG(array_p,i) > 0",
                            csnu,
                            cn,
                            fn
                           );
                else
                    fprintf(f,
                            "%s_PARAM_COUNT_TAG(array_p,%s,i) > 0",
                            snu,
                            pn
                           );
                fprintf(f,
                        ")\n"
                       );
                if (fn)
                    fprintf(f,
                            "                errors += %s_checkBag_%s(%s_GET_%s_%s_ELEMENT_PTR_TAG(array_p,i), result_list_pp);\n",
                            lnl,
                            bag_name_p,
                            csnu,
                            cn,
                            fn
                           );
                else
                    fprintf(f,
                            "                errors += %s_checkBag_%s(mpl_param_list_find_tag(%s_PARAM_ID(%s),i,array_p), result_list_pp);\n",
                            lnl,
                            bag_name_p,
                            snu,
                            pn
                           );

                bag_name_p = NULL;
            }
            fprintf(f,
                    "        }\n"
                   );
            fprintf(f,
                    "        mpl_param_list_destroy(&array_p);\n"
                    "    }\n"
                   );
            count++;
        }
        if (bag_name_p != NULL) {
            if (print_this_first_p) {
              fprintf(f,
                      "%s\n",
                      print_this_first_p
                     );
              print_this_first_p = NULL;
            }

            fprintf(f,
                    "    if ("
                   );
            if (fn)
                fprintf(f,
                        "%s_%s_%s_FIELD_COUNT(bag_p) > 0",
                        csnu,
                        cn,
                        fn
                       );
            else
                fprintf(f,
                        "%s_PARAM_COUNT(bag_p,%s) > 0",
                        snu,
                        pn
                       );
            fprintf(f,
                    ")\n"
                   );
            if (fn)
                fprintf(f,
                        "        errors += %s_checkBag_%s(%s_GET_%s_%s_ELEMENT_PTR(bag_p), result_list_pp);\n",
                        lnl,
                        bag_name_p,
                        csnu,
                        cn,
                        fn
                       );
            else
                fprintf(f,
                        "        errors += %s_checkBag_%s(mpl_param_list_find(%s_PARAM_ID(%s),bag_p), result_list_pp);\n",
                        lnl,
                        bag_name_p,
                        snu,
                        pn
                       );

            bag_name_p = NULL;
            count++;
        }

        if (!has_ellipsis) {
            fprintf(f,
                    "    {\n"
                    "        mpl_param_element_t *param_elem_p;\n"
                    "        mpl_param_element_id_t id;\n"
                   );

            if (fn)
                fprintf(f,
                        "        id = mpl_param_get_bag_field_id(%s_PARAM_ID(%s), %s_FIELD_INDEX(%s,%s));\n"
                        "        param_elem_p = mpl_param_element_create_empty(MPL_PARAMID_VIRTUAL_CLEAR(id));\n"
                        "                if (param_elem_p == NULL)\n"
                        "                    return %s_MEMORY_ERROR;\n"
                        "        MPL_PARAM_ELEMENT_SET_FIELD_INFO(param_elem_p,%s_PARAM_ID(%s),%s_FIELD_INDEX(%s,%s));\n",
                        csnu,
                        cn,
                        csnu,
                        cn,
                        fn,
                        lnu,
                        csnu,
                        cn,
                        csnu,
                        cn,
                        fn
                       );
            else
                fprintf(f,
                        "        id = %s_PARAM_ID(%s);\n"
                        "        param_elem_p = mpl_param_element_create_empty(MPL_PARAMID_VIRTUAL_CLEAR(id));\n"
                        "                if (param_elem_p == NULL)\n"
                        "                    return %s_MEMORY_ERROR;\n",
                        snu,
                        pn,
                        lnu
                       );
            fprintf(f,
                    "        mpl_list_append(allowed_parameters_pp, &param_elem_p->list_entry);\n",
                    snu,
                    pn
                   );

            fprintf(f,
                    "    }\n"
                   );
        }
        free(snu);
        free(lnu);
    }
    free(csnu);
    return count;
}

void bag_parameter::gc_c_bag(FILE *f)
{
    char *min_p;
    char *max_p;
    char *short_name_p = parameter_set_p->get_short_name();
    char *snu = str_toupper(short_name_p);
    char *lnu = str_toupper(parameter_set_p->name_p);
    mpl_list_t *parameter_list_p = (mpl_list_t*)get_property("parameter_list");


    fprintf(f,
            "int %s_checkBag_%s(mpl_param_element_t *bag_elem_p, mpl_list_t **result_list_pp)\n"
            "{\n"
            "    int errors = 0;\n",
            parameter_set_p->name_p,
            name_p
           );

    if (!is_virtual) {
        fprintf(f,
                "    mpl_list_t *allowed_parameters_p = NULL;\n"
               );
        if (ellipsis_p == NULL) {
            fprintf(f,
                    "    mpl_bag_t *bag_p = bag_elem_p->value_p;\n"
                   );
        }
    }

#ifdef CHK_FTRACE
    fprintf(f,
            "    printf(\"%s_checkBag_%s\\n\");\n"
            "\n",
            parameter_set_p->name_p,
            name_p
           );
#endif

    mpl_list_t *tmp_p;
    mpl_list_t *fcl_p = get_flat_child_list();
    MPL_LIST_FOR_EACH(fcl_p, tmp_p) {
        object_container *container_p = LISTABLE_PTR(tmp_p, object_container);
        parameter *child_p = (parameter*) container_p->object_p;
        char *child_psu = str_toupper(child_p->parameter_set_p->get_short_name());
        fprintf(f,
                "    if (bag_elem_p->id == %s_PARAM_ID(%s))\n"
                "        return %s_checkBag_%s(bag_elem_p, result_list_pp);\n",
                child_psu,
                child_p->name_p,
                child_p->parameter_set_p->name_p,
                child_p->name_p
               );
        free(child_psu);
    }
    DELETE_LISTABLE_LIST(&fcl_p, object_container);

    if (is_virtual) {
        fprintf(f,
                "    /* This bag is virtual */\n"
                "    errors++;\n"
               );
    }
    else {
        fprintf(f,
                "    errors += %s_checkBag_%s_params(bag_elem_p, result_list_pp, &allowed_parameters_p);\n",
                parameter_set_p->name_p,
                name_p
               );
        if (ellipsis_p == NULL) {
            fprintf(f,
                    "    if (%s_checkAllowedParameters(&errors, result_list_pp, allowed_parameters_p, bag_p) < 0)\n"
                    "        errors = %s_MEMORY_ERROR;\n"
                    "    mpl_param_list_destroy(&allowed_parameters_p);\n",
                    parameter_set_p->name_p,
                    lnu
                   );
        }
    }

    fprintf(f,
            "    return errors;\n"
            "}\n\n"
           );


    fprintf(f,
            "int %s_checkBag_%s_params(mpl_param_element_t *bag_elem_p, mpl_list_t **result_list_pp, mpl_list_t **allowed_parameters_pp)\n"
            "{\n"
            "    int errors = 0;\n",
            parameter_set_p->name_p,
            name_p
           );

    if (have_own_parameters()) {
        fprintf(f,
                "    mpl_bag_t *bag_p = bag_elem_p->value_p;\n"
               );

        fprintf(f,
                "    size_t result_list_len;\n"
               );
    }

#ifdef CHK_FTRACE
    fprintf(f,
            "    printf(\"%s_checkBag_%s_params\\n\");\n"
            "\n",
            parameter_set_p->name_p,
            name_p
           );
#endif

    if (parent_p != NULL) {
        fprintf(f,
                "    errors += %s_checkBag_%s_params(bag_elem_p, result_list_pp, allowed_parameters_pp);\n",
                ((bag_parameter*)parent_p)->parameter_set_p->name_p,
                parent_p->name_p
               );
    }

    if (have_own_parameters()) {
        if (gc_c_check_parameter_list(f,
                                      "    result_list_len = mpl_list_len(*result_list_pp);\n"
                                     )) {
            fprintf(f,
                    "\n"
                    "    if (mpl_list_len(*result_list_pp) > result_list_len)\n"
                    "        errors++;\n\n"
                   );
        }
        min_p = (char *) get_property("min");
        max_p = (char *) get_property("max");
        if (min_p != NULL) {
            fprintf(f,
                    "    if (mpl_list_len(bag_p) < %s)\n",
                    min_p
                   );
            fprintf(f,
                    "        errors++;\n"
                   );
        }
        if (max_p != NULL) {
            fprintf(f,
                    "    if (mpl_list_len(bag_p) > %s)\n",
                    max_p
                   );
            fprintf(f,
                    "        errors++;\n"
                   );
        }

        fprintf(f,
                "    if (errors) {\n"
                "        mpl_param_element_t *param_elem_p;\n"
                "        param_elem_p = mpl_param_element_create_empty_tag(MPL_PARAMID_VIRTUAL_CLEAR(%s_paramid_%s), bag_elem_p->tag);\n"
                "        if (param_elem_p == NULL)\n"
                "            return %s_MEMORY_ERROR;\n"
                "        mpl_list_append(result_list_pp, &param_elem_p->list_entry);\n"
                "    }\n",
                parameter_set_p->name_p,
                name_p,
                lnu
               );
    }

    fprintf(f,
            "    return errors;\n"
            "}\n\n"
           );

    char *lnl = parameter_set_p->name_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    mpl_list_t *pl_p = (mpl_list_t*) get_property("parameter_list");
    parameter_set *ps_p;

    MPL_LIST_FOR_EACH(pl_p, tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if ((parameter_list_entry_p->field_name_p == NULL)  ||
            (field_context(parameter_list_entry_p->field_name_p) != this))
            continue;

        ps_p = parameter_list_entry_p->parameter_set_p;
        parameter_p = ps_p->find_parameter(parameter_list_entry_p->parameter_name_p);

        if (parameter_p->is_int() ||
            parameter_p->is_bool() ||
            parameter_p->is_enum() ||
            parameter_p->is_addr()) {

            fprintf(f,
                    "%s %s_GET_%s_%s(mpl_bag_t *__bag_p)\n"
                    "{\n"
                    "    mpl_param_element_t *__elem_p;\n"
                    "    size_t __size;\n"
                    "    int __res;\n"
                    "    %s %s;\n"
                    "    __elem_p = %s_GET_%s_%s_ELEMENT_PTR(__bag_p);\n"
                    "    assert(__elem_p != NULL);\n"
                    "    __size = mpl_param_id_sizeof_param_value(mpl_param_get_bag_field_id(%s_PARAM_ID(%s), %s_FIELD_INDEX(%s, %s)));\n"
                    "    __res = mpl_param_value_copy_out(__elem_p, &%s, __size);\n"
                    "    assert(__res == (int)__size);\n"
                    "    return %s;\n"
                    "}\n",
                    param_c_type(parameter_p),
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p
                   );
            fprintf(f,
                    "%s %s_GET_%s_%s_TAG(mpl_bag_t *__bag_p, int __tag)\n"
                    "{\n"
                    "    mpl_param_element_t *__elem_p;\n"
                    "    size_t __size;\n"
                    "    int __res;\n"
                    "    %s %s;\n"
                    "    __elem_p = %s_GET_%s_%s_ELEMENT_PTR_TAG(__bag_p,__tag);\n"
                    "    assert(__elem_p != NULL);\n"
                    "    __size = mpl_param_id_sizeof_param_value(mpl_param_get_bag_field_id(%s_PARAM_ID(%s), %s_FIELD_INDEX(%s, %s)));\n"
                    "    __res = mpl_param_value_copy_out(__elem_p, &%s, __size);\n"
                    "    assert(__res == (int)__size);\n"
                    "    return %s;\n"
                    "}\n",
                    param_c_type(parameter_p),
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p
                   );
        }
        else if (parameter_p->is_string() ||
                 parameter_p->is_bag()) {
            fprintf(f,
                    "%s %s_GET_%s_%s_PTR(mpl_bag_t *__bag_p)\n"
                    "{\n"
                    "    mpl_param_element_t *__elem_p;\n"
                    "    __elem_p = %s_GET_%s_%s_ELEMENT_PTR(__bag_p);\n"
                    "    assert(__elem_p != NULL);\n"
                    "    return __elem_p->value_p;\n"
                    "}\n",
                    param_c_type(parameter_p),
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p
                   );
            fprintf(f,
                    "%s %s_GET_%s_%s_PTR_TAG(mpl_bag_t *__bag_p, int __tag)\n"
                    "{\n"
                    "    mpl_param_element_t *elem_p;\n"
                    "    elem_p = %s_GET_%s_%s_ELEMENT_PTR_TAG(__bag_p, __tag);\n"
                    "    assert(elem_p != NULL);\n"
                    "    return elem_p->value_p;\n"
                    "}\n",
                    param_c_type(parameter_p),
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p
                   );
        }
        else if (parameter_p->is_array()) {
            const char *arr_type_str;

            switch(get_type_of_array(parameter_p->get_type())) {
                case 8:
                    arr_type_str = "mpl_uint8_array_t";
                    break;
                case 16:
                    arr_type_str = "mpl_uint16_array_t";
                    break;
                case 32:
                    arr_type_str = "mpl_uint32_array_t";
                    break;
                default:
                    assert(0);
            }

            fprintf(f,
                    "%s * %s_GET_%s_%s_PTR(mpl_bag_t *__bag_p)\n"
                    "{\n"
                    "    mpl_param_element_t *elem_p;\n"
                    "    %s *%s;\n"
                    "    elem_p = %s_GET_%s_%s_ELEMENT_PTR(__bag_p);\n"
                    "    assert(elem_p != NULL);\n"
                    "    %s = elem_p->value_p;\n"
                    "    return %s;\n"
                    "}\n",
                    arr_type_str,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p
                   );
            fprintf(f,
                    "%s * %s_GET_%s_%s_PTR_TAG(mpl_bag_t *__bag_p, int __tag)\n"
                    "{\n"
                    "    mpl_param_element_t *elem_p;\n"
                    "    %s *%s;\n"
                    "    elem_p = %s_GET_%s_%s_ELEMENT_PTR_TAG(__bag_p, __tag);\n"
                    "    assert(elem_p != NULL);\n"
                    "    %s = elem_p->value_p;\n"
                    "    return %s;\n"
                    "}\n",
                    arr_type_str,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p
                   );
        }
        else if (parameter_p->is_tuple()) {
            fprintf(f,
                    "%s *%s_GET_%s_%s_PTR(mpl_bag_t *__bag_p)\n"
                    "{\n"
                    "    mpl_param_element_t *elem_p;\n"
                    "    %s *%s;\n"
                    "    elem_p = %s_GET_%s_%s_ELEMENT_PTR(__bag_p);\n"
                    "    assert(elem_p != NULL);\n"
                    "    %s = elem_p->value_p;\n"
                    "    return %s;\n"
                    "}\n",
                    param_c_type(parameter_p),
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p
                   );
            fprintf(f,
                    "%s *%s_GET_%s_%s_PTR_TAG(mpl_bag_t *__bag_p, int __tag)\n"
                    "{\n"
                    "    mpl_param_element_t *elem_p;\n"
                    "    %s *%s;\n"
                    "    elem_p = %s_GET_%s_%s_ELEMENT_PTR_TAG(__bag_p, __tag);\n"
                    "    assert(elem_p != NULL);\n"
                    "    %s = elem_p->value_p;\n"
                    "    return %s;\n"
                    "}\n",
                    param_c_type(parameter_p),
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    param_c_type(parameter_p),
                    parameter_list_entry_p->field_name_p,
                    snu,
                    name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p,
                    parameter_list_entry_p->field_name_p
                   );
        }
    }
    free(snu);
    free(lnu);
}

mpl_list_t *bag_parameter::get_possible_parameters()
{
    mpl_list_t *plist_p = NULL;
    mpl_list_t *tmp_p;
    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry *parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter *parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);
        object_container *container_p = new object_container(parameter_p);
        container_p->append_to(plist_p);
        mpl_list_append(&plist_p, parameter_p->get_flat_child_list());
    }
    return plist_p;
}

bool bag_parameter::param_is_field_or_subfield(parameter *param_p)
{
    mpl_list_t *tmp_p;
    mpl_list_t *plist_p = get_possible_parameters();
    MPL_LIST_FOR_EACH(plist_p, tmp_p) {
        object_container *container_p = LISTABLE_PTR(tmp_p, object_container);
        parameter *parameter_p = (parameter*) container_p->object_p;

        if (parameter_p == param_p) {
            DELETE_LISTABLE_LIST(&plist_p, object_container);
            return true;
        }

        if (parameter_p->is_bag() && (parameter_p != this)) {
            if (((bag_parameter*)parameter_p)->param_is_field_or_subfield(param_p)) {
                DELETE_LISTABLE_LIST(&plist_p, object_container);
                return true;
            }
        }
    }
    DELETE_LISTABLE_LIST(&plist_p, object_container);
    return false;
}

void int_parameter::cli_c_write_value_help(FILE *f)
{
    mpl_list_t *tmp_p;
    const char *sep = "";
    const char *rangesep = "";

    fprintf(f,
            "            sprintf(value_help, \"%s:%%s\", \"",
            get_type()
           );
    if (get_property("min")) {
        fprintf(f,
                "min=%s",
                (char*)get_property("min")
               );
        sep = ",";
    }
    else if (!get_property("number_ranges")) {
        fprintf(f,
                "min=%s",
                type_min()
               );
        sep = ",";
    }

    if (get_property("number_ranges")) {
        fprintf(f,
                "%srange(",
                sep
               );
        sep = ",";

        MPL_LIST_FOR_EACH((mpl_list_t*)get_property("number_ranges"), tmp_p) {
            number_range *number_range_p = LISTABLE_PTR(tmp_p, number_range);
            mpl_list_t *tmp_p;
            integer_range *integer_range_p;
            MPL_LIST_FOR_EACH(number_range_p->range_list_p, tmp_p) {
                integer_range_p = LISTABLE_PTR(tmp_p, integer_range);
                fprintf(f,
                        "%s%" PRIi64 "..%" PRIi64,
                        rangesep,
                        *integer_range_p->get_first_p(),
                        *integer_range_p->get_last_p()
                       );
                rangesep = ",";
            }
        }
        fprintf(f,
                ")"
               );
    }

    if (get_property("max")) {
        fprintf(f,
                "%smax=%s",
                sep,
                (char*)get_property("max")
               );
    }
    else if (!get_property("number_ranges")) {
        fprintf(f,
                "%smax=%s",
                sep,
                type_max()
               );
    }

    fprintf(f,
            "\");\n"
           );
}

void array_parameter::cli_c_write_value_help(FILE *f)
{
    switch(get_type_of_array(get_type())) {
        case 8:
            fprintf(f,
                    "            sprintf(value_help, \"%s:nnnnnnnnaabbcc.... (n=length(hex),aa/bb/cc=element value(hex))\");\n",
                    get_type()
                   );
            break;
        case 16:
            fprintf(f,
                    "            sprintf(value_help, \"%s:nnnnnnnnaaaabbbbcccc.... (n=length(hex),aaaa/bbbb/cccc=element value(hex))\");\n",
                    get_type()
                   );
            break;
        case 32:
            fprintf(f,
                    "            sprintf(value_help, \"%s:nnnnnnnnaaaaaaaabbbbbbbbcccccccc.... (n=length(hex),aaaaaaaa/bbbbbbbb/cccccccc=element value(hex))\");\n",
                    get_type()
                   );
            break;
        default:
            assert(0);
    }
}

void string_parameter::cli_c_write_value_help(FILE *f)
{
    switch(get_type_of_string(get_type())) {
        case 1: /* string */
            fprintf(f,
                    "            sprintf(value_help, \"%s:\\\"string value\\\"\");\n",
                    get_type()
                   );
            break;
        case 2: /* wstring */
            fprintf(f,
                    "            sprintf(value_help, \"%s:nnnnnnnnaaaaaaaabbbbbbbbcccccccc.... (n=length(hex),aaaaaaaa/bbbbbbbb/cccccccc=element value(hex))\");\n",
                    get_type()
                   );
            break;
        default:
            assert(0);
    }
}

void addr_parameter::cli_c_write_value_help(FILE *f)
{
    fprintf(f,
            "            sprintf(value_help, \"%s:0xaaaaaaaa\");\n",
            get_type()
           );
}

void tuple_parameter::cli_c_write_value_help(FILE *f)
{
    switch(get_type_of_tuple(get_type())) {
        case 1: /* string_tuple */
            fprintf(f,
                    "            sprintf(value_help, \"%s:key_string:value_string\");\n",
                    get_type()
                   );
            break;
        case -1: /* int_tuple */
            fprintf(f,
                    "            sprintf(value_help, \"%s:key_int:value_int\");\n",
                    get_type()
                   );
            break;
        case 2: /* strint_tuple */
            fprintf(f,
                    "            sprintf(value_help, \"%s:key_string:value_int\");\n",
                    get_type()
                   );
            break;
        case 8: /* struint8_tuple */
            fprintf(f,
                    "            sprintf(value_help, \"%s:key_string/value_uint8\");\n",
                    get_type()
                   );
            break;
        default:
            assert(0);
    }
}


void bag_parameter::cli_h_completions(FILE *f)
{
    fprintf(f,
            "int %s_%s_get_bag_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings);\n",
            parameter_set_p->name_p,
            name_p
           );
}

void bag_parameter::cli_c_completions(FILE *f)
{
    mpl_list_t *tmp_p;
    fprintf(f,
            "int %s_%s_get_bag_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings)\n"
            "{\n",
            parameter_set_p->name_p,
            name_p
           );
    int parameter_found = 0;
    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry *parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if ((parameter_list_entry_p->direction == direction_in) ||
            (parameter_list_entry_p->direction == direction_inout)) {
            parameter_found = 1;
            break;
        }
    }
    if (!parameter_found) {
        fprintf(f,
                "    return 0;\n"
                "}\n"
               );
        return;
    }
    fprintf(f,
            "    int numStrings = 0;\n"
            "    const char *pos;\n"
            "    const char *parend;\n"
            "    const char *p;\n"
           );
    fprintf(f,
            "    if(*posindex > strlen(line))\n"
            "        *posindex = strlen(line);\n"
            "    pos = &line[*posindex];\n"
           );
    fprintf(f,
            "    if (*(parend = param_end(pos)) == '}') {\n"
            "        completion_func_t func = pop_all_bag_ends(line, &pos, posindex, parend);\n"
            "        assert(func != NULL);\n"
            "        return func(line, posindex, completionStrings, maxStrings);\n"
            "    }\n"
           );
    fprintf(f,
            "    forward_to_param_start(&pos);\n"
            "    p = parend;\n"
            "    backward_to_param_start(pos, &p);\n"
            "    forward_to_param_start(&p);\n"
           );

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry *parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter *parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);
        if ((parameter_list_entry_p->direction != direction_in) &&
            (parameter_list_entry_p->direction != direction_inout))
            continue;
        int m = parameter_list_entry_p->multiple;

        mpl_list_t *plist_p = NULL;
        object_container *container_p = new object_container(parameter_p);
        container_p->append_to(plist_p);
        mpl_list_append(&plist_p, parameter_p->get_flat_child_list());
        mpl_list_t *tmp_p;
        MPL_LIST_FOR_EACH(plist_p, tmp_p) {
            object_container *container_p = LISTABLE_PTR(tmp_p, object_container);
            parameter *parameter_p = (parameter*) container_p->object_p;
            char *psn = parameter_p->parameter_set_p->name_p;
            char *nm = parameter_list_entry_p->field_name_p ? parameter_list_entry_p->field_name_p : parameter_p->name_p;
            char *pn = parameter_p->name_p;

            if (parameter_p->is_bag()) {
                fprintf(f,
                        "    if (!strncmp(\"%s\",pos,%zd)) {\n"
                        "        if ((strlen(pos) > %zd) && strstr(pos, \"={\")) {\n"
                        "            push_stack(%s_%s_get_bag_completions);\n"
                        "            *posindex = (strstr(pos, \"={\") + 1) - line;\n"
                        "            free_completion_strings(completionStrings, numStrings);\n"
                        "            return %s_%s_get_bag_completions(line,\n"
                        "                                            posindex,\n"
                        "                                            completionStrings,\n"
                        "                                            maxStrings);\n"
                        "        }\n"
                        "    }\n",
                        nm,
                        strlen(nm),
                        strlen(nm),
                        parameter_set_p->name_p,
                        name_p,
                        psn,
                        pn
                       );
            }
            else if (parameter_p->is_enum()) {
                fprintf(f,
                        "    if (!strncmp(\"%s\",pos,%zd)) {\n"
                        "        if ((strlen(pos) > %zd) && strstr(pos, \"=\")) {\n"
                        "            push_stack(%s_%s_get_bag_completions);\n"
                        "            *posindex = (strstr(pos, \"=\") + 1) - line;\n"
                        "            free_completion_strings(completionStrings, numStrings);\n"
                        "            return %s_%s_get_enum_completions(line,\n"
                        "                                            posindex,\n"
                        "                                            completionStrings,\n"
                        "                                            maxStrings);\n"
                        "        }\n"
                        "    }\n",
                        nm,
                        strlen(nm),
                        strlen(nm),
                        parameter_set_p->name_p,
                        name_p,
                        psn,
                        pn
                       );
            }
            else if (parameter_p->is_bool()) {
                fprintf(f,
                        "    if (!strncmp(\"%s\",pos,%zd)) {\n"
                        "        if ((strlen(pos) > %zd) && strstr(pos, \"=\")) {\n"
                        "            push_stack(%s_%s_get_bag_completions);\n"
                        "            *posindex = (strstr(pos, \"=\") + 1) - line;\n"
                        "            free_completion_strings(completionStrings, numStrings);\n"
                        "            return get_bool_completions(line,\n"
                        "                                        posindex,\n"
                        "                                        completionStrings,\n"
                        "                                        maxStrings);\n"
                        "        }\n"
                        "    }\n",
                        nm,
                        strlen(nm),
                        strlen(nm),
                        parameter_set_p->name_p,
                        name_p
                       );
            }
            else if (parameter_p->is_int() ||
                     parameter_p->is_array() ||
                     parameter_p->is_addr() ||
                     parameter_p->is_tuple() ||
                     parameter_p->is_string()) {
                fprintf(f,
                        "    if (!strncmp(\"%s\",pos,%zd)) {\n"
                        "        if ((strlen(pos) > %zd) && strstr(pos, \"=\")) {\n"
                        "            push_stack(%s_%s_get_bag_completions);\n"
                        "            *posindex = (strstr(pos, \"=\") + 1) - line;\n"
                        "            free_completion_strings(completionStrings, numStrings);\n",
                        nm,
                        strlen(nm),
                        strlen(nm),
                        parameter_set_p->name_p,
                        name_p
                       );
                parameter_p->cli_c_write_value_help(f);
                fprintf(f,
                        "            return get_%s_completions(line,\n"
                        "                                       posindex,\n"
                        "                                       completionStrings,\n"
                        "                                       maxStrings);\n"
                        "        }\n"
                        "    }\n",
                        parameter_p->is_int() ? "int" :
                        parameter_p->is_array() ? "array" :
                        parameter_p->is_addr() ? "addr" :
                        parameter_p->is_tuple() ? "tuple" :
                        parameter_p->is_string() ? parameter_p->get_type() : ""
                       );
            }
            else {
                fprintf(f,
                        "    if (!strncmp(\"%s\",pos,%zd)) {\n"
                        "        if (strlen(pos) > %zd) {\n"
                        "            *posindex = parend + 1 - line;\n"
                        "            free_completion_strings(completionStrings, numStrings);\n"
                        "            return %s_%s_get_bag_completions(line,\n"
                        "                                             posindex,\n"
                        "                                             completionStrings,\n"
                        "                                             maxStrings);\n"
                        "        }\n"
                        "    }\n",
                        nm,
                        strlen(nm),
                        strlen(nm),
                        parameter_set_p->name_p,
                        name_p
                       );
            }
            fprintf(f,
                    "    if (!strncmp(\"%s\",p,strlen(p))) {\n"
                    "        completionStrings[numStrings++] = create_completion_string(line, p, \"%s\", \"%s\", %s);\n"
                    "        if (numStrings >= maxStrings) goto do_exit;\n"
                    "    }\n",
                    nm,
                    nm,
                    parameter_p->get_type(),
                    m ? "1" : "0"
                   );
        }
        while ((tmp_p = mpl_list_remove(&plist_p, NULL))) {
            object_container *container_p = LISTABLE_PTR(tmp_p, object_container);
            delete container_p;
        }
    }
    fprintf(f,
            "do_exit:\n"
            "    *posindex = strlen(line);\n"
            "    return numStrings;\n"
           );
    fprintf(f,
            "}\n"
            "\n"
           );
}

void enum_parameter::cli_h_completions(FILE *f)
{
    fprintf(f,
            "int %s_%s_get_enum_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings);\n",
            parameter_set_p->name_p,
            name_p
           );
}

void enum_parameter::cli_c_completions(FILE *f)
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;

    fprintf(f,
            "int %s_%s_get_enum_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings)\n"
            "{\n",
            parameter_set_p->name_p,
            name_p
           );
    fprintf(f,
            "    int numStrings = 0;\n"
            "    const char *pos = &line[*posindex];\n"
            "    const char *p = pos;\n"
           );
    fprintf(f,
            "    assert(pos > line);\n"
            "    p = get_value_start(pos, p);\n"
           );

    MPL_LIST_FOR_EACH(enum_values_p, tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        char *nm = enum_value_p->name_p;
        char vs[24];
        sprintf(vs, "%" PRIi64, *enum_value_p->get_value_p());
        fprintf(f,
                "    if (!strncmp(\"%s \",p,%zd + 1) || !strncmp(\"%s \",p,%zd + 1)) {\n"
                "        completion_func_t func = pop_stack();\n"
                "        assert(func != NULL);\n"
                "        *posindex = param_end(pos) - line + 1;\n"
                "        return func(line, posindex, completionStrings, maxStrings);\n"
                "    }\n",
                nm,
                strlen(nm),
                vs,
                strlen(vs)
               );
        fprintf(f,
                "    if (!strncmp(\"%s\",p,strlen(p)) || !strncmp(\"%s\",p,strlen(p))) {\n"
                "        completionStrings[numStrings++] = create_completion_string(line, p, \"%s\", \"value\", 0);\n"
                "        if (numStrings >= maxStrings) goto do_exit;\n"
                "    }\n",
                nm,
                vs,
                nm
               );
    }
    fprintf(f,
            "do_exit:\n"
            "    *posindex = strlen(line);\n"
            "    return numStrings;\n"
           );
    fprintf(f,
            "}\n"
            "\n"
           );
}

void parameter::cli_h_help(FILE *f)
{
    fprintf(f,
            "int %s_%s_get_parameter_help(char **helptext);\n",
            parameter_set_p->get_short_name(),
            name_p
           );
}

void parameter::cli_c_help(FILE *f)
{
    fprintf(f,
            "int %s_%s_get_parameter_help(char **helptext)\n"
            "{\n",
            parameter_set_p->get_short_name(),
            name_p
           );
    ostringstream help_stream(ostringstream::out);
    string s;
    help(help_stream);
    s = help_stream.str();
    fprintf(f,
            "    *helptext = strdup(\"%s\");\n",
            s.c_str()
           );

    fprintf(f,
            "    return 0;\n"
            "}\n"
            "\n"
           );
}

void parameter::help(ostream &os)
{
    os << get_type() << ": " << name_p;
    os << "\\n";
    help_help_list(os, help_list_p, "  ");
}

void int_parameter::help(ostream &os)
{
    parameter::help(os);
    if (get_property("min")) {
      os << "min=" << (char*)get_property("min") << "\\n";
    }
    else if (!get_property("number_ranges")) {
      os << "min=" << type_min() << "\\n";
    }


    if (get_property("number_ranges")) {
      os << "range(";
      const char *rangesep = "";

      mpl_list_t *tmp_p;
      MPL_LIST_FOR_EACH((mpl_list_t*)get_property("number_ranges"), tmp_p) {
          number_range *number_range_p = LISTABLE_PTR(tmp_p, number_range);
          mpl_list_t *tmp_p;
          integer_range *integer_range_p;
          MPL_LIST_FOR_EACH(number_range_p->range_list_p, tmp_p) {
              integer_range_p = LISTABLE_PTR(tmp_p, integer_range);
              os << rangesep << *integer_range_p->get_first_p() << ".." << *integer_range_p->get_last_p();
              rangesep = ",";
        }
      }
      os << ")" << "\\n";
    }

    if (get_property("max")) {
      os << "max=" << (char*)get_property("max") << "\\n";
    }
    else if (!get_property("number_ranges")) {
      os << "max=" << type_max() << "\\n";
    }
}

void string_parameter::help(ostream &os)
{
    parameter::help(os);
    if (get_property("min")) {
      os << "min length=" << (char*)get_property("min") << "\\n";
    }

    if (get_property("max")) {
      os << "max length=" << (char*)get_property("max") << "\\n";
    }
}

void enum_parameter::help(ostream &os)
{
    parameter::help(os);

    os << "Values:\\n";

    mpl_list_t *tmp_p;
    enum_value *enum_value_p;
    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("enum_values"), tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        os << "  - ";
        os << enum_value_p->name_p << "=" << *enum_value_p->get_value_p();
        os << ": ";
        help_help_list(os, enum_value_p->help_list_p, "    ", "");
    }
}

void bag_parameter::cli_h_help(FILE *f)
{
    fprintf(f,
            "int %s_%s_get_bag_help(char **helptext);\n",
            parameter_set_p->get_short_name(),
            name_p
           );
}

void bag_parameter::cli_c_help(FILE *f)
{
    fprintf(f,
            "int %s_%s_get_bag_help(char **helptext)\n"
            "{\n",
            parameter_set_p->get_short_name(),
            name_p
           );
    ostringstream help_stream(ostringstream::out);
    string s;
    help(help_stream);
    s = help_stream.str();
    fprintf(f,
            "    *helptext = strdup(\"%s\");\n",
            s.c_str()
           );

    fprintf(f,
            "    return 0;\n"
            "}\n"
            "\n"
           );
}

void bag_parameter::help(ostream &os)
{
    os << "Bag: " << name_p;
    os << "\\n";
    help_help_list(os, help_list_p, "  ");
    help_parameter_list(os,
                        bag_parameter_list_p,
                        NULL,
                        direction_none,
                        parameter_set_p,
                        "Members"
                       );
}

void bag_parameter::api_hh(FILE *f, char *indent)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = parameter_set_p->get_short_name();
    char *snu = str_toupper(snl);

    if (method_ref_p) {
        if (method_ref_p->type == method_type_command) {
            command *cmd_p = (command*)method_ref_p;
            if (cmd_p->category_p->get_command_bag() == this->get_topmost_parent()) {
                if (cmd_p->cmd_dox_list_p) {
                    dox_dox_list_f(f, cmd_p->cmd_dox_list_p);
                }
            }
            else if (cmd_p->category_p->get_response_bag() == this->get_topmost_parent()) {
                if (cmd_p->resp_dox_list_p) {
                    dox_dox_list_f(f, cmd_p->resp_dox_list_p);
                }
            }
        }
        else if (method_ref_p->type == method_type_event) {
            event *evt_p = (event*)method_ref_p;
            if (evt_p->dox_list_p) {
                dox_dox_list_f(f, evt_p->dox_list_p);
            }
        }
    }

    fprintf(f,
            "%sclass %s%s%s%s %s{\n",
            indent,
            name_p,
            parent_p ? " : public " : " ",
            parent_p ? parent_p->name_p : "",
            parent_p ? " " : "",
            parent_p ? "" : ": public BAG "
           );
    fprintf(f,
            "%s    protected:\n"
            "%s        virtual void __set_id();\n",
            indent,
            indent
           );
    fprintf(f,
            "%s    public:\n",
            indent
           );
    api_hh_class_members_parameter_list(f,
                                        indent,
                                        bag_parameter_list_p,
                                        parent_p ? ((bag_parameter*)parent_p)->bag_parameter_list_p : NULL,
                                        parameter_set_p,
                                        direction_none);
    fprintf(f,
            "%s    public:\n",
            indent
           );

    fprintf(f,
            "%s        %s(mpl_bag_t *bag_p);\n",
            indent,
            name_p
           );
    fprintf(f,
            "%s        %s(",
            indent,
            name_p
           );

    api_parameter_list(f,
                       bag_parameter_list_p,
                       NULL,
                       direction_none
                      );
    fprintf(f,
            ");\n"
           );

    fprintf(f,
            "%s        %s(%s &obj);\n",
            indent,
            name_p,
            name_p
           );

    fprintf(f,
            "%s        %s &operator=(const %s &rhs);\n",
            indent,
            name_p,
            name_p
           );

    fprintf(f,
            "%s        bool operator==(%s &other);\n",
            indent,
            name_p,
            name_p
           );

    fprintf(f,
            "%s        virtual ~%s();\n",
            indent,
            name_p
           );

    fprintf(f,
            "%s        virtual mpl_bag_t *encode();\n",
            indent
           );


    fprintf(f,
            "%s        virtual void print();\n",
            indent
           );

    fprintf(f,
            "%s};\n",
            indent
           );
    free(snu);
    return;
}

void bag_parameter::api_cc(FILE *f, char *indent)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = parameter_set_p->get_short_name();
    char *snu = str_toupper(snl);

    /* Initialization constructor */
    fprintf(f,
            "%s%s::%s(",
            indent,
            name_p,
            name_p
           );

    api_parameter_list(f,
                       bag_parameter_list_p,
                       NULL,
                       direction_none
                      );
    fprintf(f,
            ")%s\n",
            parent_p || have_own_parameters() ? " :" : ""
           );
    if (parent_p) {
        fprintf(f,
                "%s    %s(",
                indent,
                parent_p->name_p
               );

        api_hh_constructor_parameter_list(f,
                                          indent,
                                          ((bag_parameter*)parent_p)->bag_parameter_list_p,
                                          parent_p->parent_p ? ((bag_parameter*)parent_p->parent_p)->bag_parameter_list_p : NULL,
                                          parent_p,
                                          parameter_set_p,
                                          direction_none);

        fprintf(f,
                ")"
               );
    }
    if (have_own_parameters()) {
        fprintf(f,
                "%s",
                parent_p ? ",\n" : ""
               );
        api_hh_constructor_parameter_list(f,
                                          indent,
                                          bag_parameter_list_p,
                                          parent_p ? ((bag_parameter*)parent_p)->bag_parameter_list_p : NULL,
                                          NULL,
                                          parameter_set_p,
                                          direction_none);
        fprintf(f,
                "\n"
               );
    }

    fprintf(f,
            "%s{\n",
            indent
           );

    fprintf(f,
            "%s    __set_id();\n",
            indent
           );

    api_cc_members_allocate_parameter_list(f,
                                           indent,
                                           bag_parameter_list_p,
                                           parent_p ? ((bag_parameter*)parent_p)->bag_parameter_list_p : NULL,
                                           parameter_set_p,
                                           direction_none);

    fprintf(f,
            "%s}\n",
            indent
           );

    /* Decoding constructor */
    fprintf(f,
            "%s%s::%s(mpl_bag_t *bag_p)",
            indent,
            name_p,
            name_p
           );
    if (parent_p != NULL) {
        fprintf(f,
                " :\n"
                "%s    %s(bag_p)\n",
                indent,
                parent_p->name_p
               );
    }
    else {
        fprintf(f,
                "\n"
               );
    }
    fprintf(f,
            "%s{\n",
            indent
           );

    fprintf(f,
            "%s    __set_id();\n",
            indent
           );
    if (have_own_parameters()) {
        api_decode_parameter_list(f,
                                  indent,
                                  bag_parameter_list_p,
                                  parent_p ? ((bag_parameter*)parent_p)->bag_parameter_list_p : NULL,
                                  parameter_set_p,
                                  name_p,
                                  (char*) "bag_p",
                                  direction_none);
    }

    fprintf(f,
            "%s}\n",
            indent
           );

    /* Copy constructor */
    fprintf(f,
            "%s%s::%s(%s &obj)",
            indent,
            name_p,
            name_p,
            name_p
           );
    fprintf(f,
            " :\n"
            "%s    %s(obj)\n",
            indent,
            parent_p ? parent_p->name_p : "BAG"
           );
    fprintf(f,
            "%s{\n",
            indent
           );

    api_cc_copy_constructor_parameter_list(f,
                                           indent,
                                           bag_parameter_list_p,
                                           parent_p ? ((bag_parameter*)parent_p)->bag_parameter_list_p : NULL,
                                           parameter_set_p,
                                           direction_none);

    fprintf(f,
            "%s}\n",
            indent
           );

    /* Destructor */
    fprintf(f,
            "%s%s::~%s()\n",
            indent,
            name_p,
            name_p
           );
    fprintf(f,
            "%s{\n",
            indent
           );

    api_free_decoded_parameter_list(f,
                                    indent,
                                    bag_parameter_list_p,
                                    parent_p ? ((bag_parameter*)parent_p)->bag_parameter_list_p : NULL,
                                    parameter_set_p,
                                    direction_none);

    fprintf(f,
            "%s}\n",
            indent
           );

    /* Assignment operator */
    fprintf(f,
            "%s%s &%s::operator=(const %s &obj)\n"
            "%s{\n",
            indent,
            name_p,
            name_p,
            name_p,
            indent
           );
    fprintf(f,
            "%s    if (this == &obj)\n"
            "%s        return *this;\n",
            indent,
            indent
           );

    if (parent_p) {
        fprintf(f,
                "%s    %s::operator=(obj);\n",
                indent,
                parent_p->name_p
               );
    }
    else {
        fprintf(f,
                "%s    BAG::operator=(obj);\n",
                indent
               );
    }

    api_free_decoded_parameter_list(f,
                                    indent,
                                    bag_parameter_list_p,
                                    parent_p ? ((bag_parameter*)parent_p)->bag_parameter_list_p : NULL,
                                    parameter_set_p,
                                    direction_none);

    api_cc_copy_constructor_parameter_list(f,
                                           indent,
                                           bag_parameter_list_p,
                                           parent_p ? ((bag_parameter*)parent_p)->bag_parameter_list_p : NULL,
                                           parameter_set_p,
                                           direction_none);

    fprintf(f,
            "%s    return *this;\n",
            indent
           );
    fprintf(f,
            "%s}\n",
            indent
           );

    /* Comparison operator */
    fprintf(f,
            "%sbool %s::operator==(%s &other)\n"
            "%s{\n",
            indent,
            name_p,
            name_p,
            indent
           );

    fprintf(f,
            "%s    mpl_param_element_t *this_elem = mpl_param_element_create_empty(this->id());\n"
            "%s    mpl_param_element_t *other_elem = mpl_param_element_create_empty(other.id());\n"
            "%s    this_elem->value_p = this->encode();\n"
            "%s    other_elem->value_p = other.encode();\n",
            indent,
            indent,
            indent,
            indent
           );

    fprintf(f,
            "%s    int diff = mpl_param_element_compare(this_elem, other_elem);\n"
            "%s    mpl_param_element_destroy(this_elem);\n"
            "%s    mpl_param_element_destroy(other_elem);\n"
            "%s    return (diff == 0);\n",
            indent,
            indent,
            indent,
            indent
           );
    fprintf(f,
            "%s}\n",
            indent
           );

    /* Encode method */
    fprintf(f,
            "%smpl_bag_t *%s::encode()\n",
            indent,
            name_p
           );
    fprintf(f,
            "%s{\n",
            indent
           );

    fprintf(f,
            "%s    mpl_bag_t *encoded_p = NULL;\n",
            indent
           );
    if (parent_p) {
        fprintf(f,
                "%s    encoded_p = %s::encode();\n",
                indent,
                parent_p->name_p
               );
    }

    api_cc_encode_parameter_list(f,
                                 indent,
                                 bag_parameter_list_p,
                                 parent_p ? ((bag_parameter*)parent_p)->bag_parameter_list_p : NULL,
                                 parameter_set_p,
                                 name_p,
                                 (char*)"encoded_p",
                                 direction_none
                                );
    fprintf(f,
            "%s    return encoded_p;\n",
            indent
           );

    fprintf(f,
            "%s}\n",
            indent
           );

    /* Print method */
    fprintf(f,
            "%svoid %s::print()\n",
            indent,
            name_p
           );
    fprintf(f,
            "%s{\n",
            indent
           );

    if (parent_p) {
        fprintf(f,
                "%s    %s::print();\n",
                indent,
                parent_p->name_p
               );
    } else {
        fprintf(f,
                "%s    BAG::print();\n",
                indent
               );
    }

    api_cc_print_parameter_list(f,
                                indent,
                                bag_parameter_list_p,
                                parent_p ? ((bag_parameter*)parent_p)->bag_parameter_list_p : NULL,
                                parameter_set_p,
                                direction_none
                                );
    fprintf(f,
            "%s}\n",
            indent
           );

    /* __set_id() method */
    fprintf(f,
            "%svoid %s::__set_id()\n"
            "%s{\n",
            indent,
            name_p,
            indent
           );
    fprintf(f,
            "%s    __id = %s_PARAM_ID(%s);\n",
            indent,
            snu,
            name_p
           );
    fprintf(f,
            "%s}\n",
            indent
           );

    free(snu);
}

void enum_parameter::dox_enum_values(FILE *f, int level)
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("enum_values"), tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        fprintf(f, "%s", spacing(level));
        dox_doc_f(f, enum_value_p->doc_list_p);
        fprintf(f, "%s", spacing(level));
        fprintf(f,
                "%s = %" PRIi64 "%s",
                enum_value_p->name_p,
                *enum_value_p->value_p,
                tmp_p->next_p ? "," : ""
               );
        dox_doc_b(f, enum_value_p->doc_list_p);
        fprintf(f, "\n");
    }
}


void enum_parameter::latex_enum_values(FILE *f)
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;

    lx_table_begin(f, 3);
    LX_HLINE;
    LX_TABH(3, "%s",
            "Enum Specification"
           );
    LX_COL_END;
    LX_HLINE;
    LX_COLH(Enumerator);
    LX_COL_SEP;
    LX_COLH(Value);
    LX_COL_SEP;
    LX_COLH(Description);
    LX_COL_END;
    LX_HLINE;

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("enum_values"), tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        fprintf_latex(f,
                     "%s",
                     enum_value_p->name_p
                    );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%" PRIi64,
                      *enum_value_p->value_p
                     );
        LX_COL_SEP;
        latex_latex_list(f, enum_value_p->latex_list_p);
        LX_COL_END;
    }

    LX_HLINE;
    LX_TAB_END;
}

void bag_parameter::dox_bag_parameter_list(FILE *f, int level)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        fprintf(f, "%s", spacing(level));
        if (parameter_list_entry_p->dox_list_p) {
            dox_dox_list_f(f, parameter_list_entry_p->dox_list_p);
        }
        else {
            dox_doc_f(f, parameter_list_entry_p->doc_list_p);
        }
        fprintf(f, "%s", spacing(level));
        if (parameter_list_entry_p->parameter_set_p != parameter_set_p) {
        fprintf(f,
                "%s::",
                parameter_list_entry_p->parameter_set_p->name_p
               );
        }
        fprintf(f,
                "%s_t",
                parameter_list_entry_p->parameter_name_p
               );
        fprintf(f,
                "%s",
                parameter_list_entry_p->optional ? "*" : ""
               );
        fprintf(f,
                " %s",
                parameter_list_entry_p->parameter_name_p
               );
        fprintf(f,
                "%s",
                parameter_list_entry_p->multiple ? "[]" : ""
               );
        fprintf(f,
                ";"
               );
        if (parameter_list_entry_p->dox_list_p) {
            dox_dox_list_b(f, parameter_list_entry_p->dox_list_p);
        }
        else {
            dox_doc_b(f, parameter_list_entry_p->doc_list_p);
        }
        fprintf(f, "\n");
    }
}

void bag_parameter::latex_bag_parameter_list(FILE *f, parameter_set *default_parameter_set_p)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    int o;
    int m;
    int psdef;

    lx_table_begin(f, 5);
    LX_HLINE;
    LX_TABH(4, "%s",
            "Bag Members"
           );
    LX_COL_END;
    LX_HLINE;
    LX_COLH(Parameter);
    LX_COL_SEP;
    LX_COLH(Field);
    LX_COL_SEP;
    LX_COLH(Options);
    LX_COL_SEP;
    LX_COLH(Type);
    LX_COL_SEP;
    LX_COLH(Description);
    LX_COL_END;
    LX_HLINE;


    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);
        o = parameter_list_entry_p->optional;
        m = parameter_list_entry_p->multiple;
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
                "%s%s%s",
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

    if (ellipsis_p) {
        fprintf_latex(f,
                      "..."
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "-"
                     );
        LX_COL_SEP;
        fprintf(f,
                "ellipsis"
               );
        LX_COL_SEP;
        LX_COL_SEP;
        if (ellipsis_p->latex_list_p)
            latex_latex_list(f, ellipsis_p->latex_list_p);
        else
            fprintf_latex(f,
                          "The bag may have additional (unspecified) parameters."
                         );
        LX_COL_END;
    }

    LX_HLINE;
    LX_TAB_END;
}

void parameter::dox(FILE *f, int level)
{
    fprintf(f, "%s", spacing(level));
    dox_doc_f(f, doc_list_p);
    fprintf(f, "%s", spacing(level));
    fprintf(f, "typedef %s %s_t;",
            type_p,
            name_p
           );
    dox_doc_b(f, doc_list_p);
    fprintf(f, "\n");
}



void parameter::latex(FILE *f, parameter_set *default_parameter_set_p)
{
    int psdef = (parameter_set_p == default_parameter_set_p);
    LX_SSS("%s%s%s",
           name_p,
           psdef ? "" : "parameter_set_p->name_p",
           psdef ? "" : "::"
          );
    LX_LABEL("%s:%s",
             parameter_set_p->name_p,
             name_p
            );

    latex_latex_list(f, latex_list_p);
    fprintf(f,
            "\n"
           );

    lx_table_begin(f, 3);
    LX_HLINE;
    LX_TABH(3, "%s",
            "Parameter Properties"
           );
    LX_COL_END;
    LX_HLINE;
    LX_COLH(Property);
    LX_COL_SEP;
    LX_COLH(Value);
    LX_COL_SEP;
    LX_COLH(Description);
    LX_COL_END;
    LX_HLINE;

    fprintf(f,
            "Type"
           );
    LX_COL_SEP;
    fprintf_latex(f,
                  "%s",
                  get_type()
                 );
    LX_COL_SEP;
    print_type_description(f);
    LX_COL_END;

    if (is_virtual) {
        fprintf(f,
                "Virtual"
               );
        LX_COL_SEP;
        fprintf(f,
                "%s",
                is_virtual ? "true" : "false"
               );
        LX_COL_SEP;
        if (is_virtual)
            fprintf_latex(f,
                          "This parameter is virtual, i.e. only child parameters can\n"
                          "be instansiated.\n"
                         );
        LX_COL_END;
    }
    if (parent_p) {
        fprintf(f,
                "Parents"
               );
        LX_COL_SEP;

        parameter *p_p = (parameter*)parent_p;

        while (p_p) {
            int ppsdef = (p_p->parameter_set_p == default_parameter_set_p);
            fprintf_latex(f,
                          "%s%s%s",
                          ppsdef ? "" : p_p->parameter_set_p->name_p,
                          ppsdef ? "" : "::",
                          p_p->name_p
                         );
            fprintf(f,
                    " \\ref{%s:%s}",
                    p_p->parameter_set_p->name_p,
                    p_p->name_p
                   );
            LX_NEWLINE;
            p_p = (parameter*)p_p->parent_p;
        }
        LX_COL_SEP;
        fprintf_latex(f,
                      "This parameter inherits another parameter, i.e. the parent(s) can represent\n"
                      "this parameter in parameter lists (command/response/event and bags).\n"
                     );
        LX_COL_END;
    }

    if (child_list_p) {
        mpl_list_t *tmp_p;

        fprintf_latex(f,
                      "Children"
                     );
        LX_COL_SEP;
        mpl_list_t *fcl_p = get_flat_child_list();
        MPL_LIST_FOR_EACH(fcl_p, tmp_p) {
            object_container *container_p = LISTABLE_PTR(tmp_p, object_container);
            parameter *child_p = (parameter*) container_p->object_p;
            int cpsdef = (child_p->parameter_set_p == default_parameter_set_p);
            fprintf_latex(f,
                          "%s%s%s",
                          cpsdef ? "" : child_p->parameter_set_p->name_p,
                          cpsdef ? "" : "::",
                          child_p->name_p
                         );
            fprintf(f,
                    " \\ref{%s:%s}",
                    child_p->parameter_set_p->name_p,
                    child_p->name_p
                   );
            LX_NEWLINE;
        }
        DELETE_LISTABLE_LIST(&fcl_p, object_container);
        LX_COL_SEP;
        fprintf_latex(f,
                      "This parameter is a parent, i.e. it can be referenced\n"
                      "in parameter lists to represent\n"
                      "child parameters.\n"
                     );
        LX_COL_END;
    }

    latex_options(f);

    LX_HLINE;
    LX_TAB_END;

    fprintf(f,
            "\n"
           );

}

void parameter::latex_options(FILE *f)
{
    if (get_property("set")) {
        fprintf_latex(f,
                      "Set"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      get_property("set") && *((int*)get_property("set")) ? "true" : "false"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "If the parameter is used in a parameter database, "
                      "this property can be used to provide access control."
                     );
        LX_COL_END;
    }

    if (get_property("get")) {
        fprintf_latex(f,
                      "Get"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s", get_property("get") && *((int*)get_property("get")) ? "true" : "false"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "If the parameter is used in a parameter database, "
                      "this property can be used to provide access control."
                     );
        LX_COL_END;
    }

    if (get_property("config")) {
        fprintf_latex(f,
                      "Config"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s", get_property("config") && *((int*)get_property("config")) ? "true" : "false"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "This property controls if the parameter can be used in a configuration database."
                     );
        LX_COL_END;
    }
}

void int_parameter::latex_options(FILE *f)
{
    parameter::latex_options(f);
    if (get_property("min")) {
        fprintf_latex(f,
                      "Min"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("min")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The minimum legal value."
                     );
        LX_COL_END;
    }

    if (get_property("max")) {
        fprintf_latex(f,
                      "Max"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                "%s",
                (char*)get_property("max")
               );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The maximum legal value."
                     );
        LX_COL_END;
    }

    if (get_property("default")) {
        fprintf_latex(f,
                      "Default"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("default")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The default value (for use in configuration databases)."
                     );
        LX_COL_END;
    }
    mpl_list_t *tmp_p;

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("number_ranges"), tmp_p) {
        number_range *number_range_p = LISTABLE_PTR(tmp_p, number_range);
        int anonymous = (!strcmp(number_range_p->name_p, NUMBER_RANGE_NAME_ANONYMOUS));
        fprintf_latex(f,
                      "Range%s%s%s%s",
                      anonymous ? "" : " ",
                      anonymous ? "" : "''",
                      anonymous ? "" : number_range_p->name_p,
                      anonymous ? "" : "''"
                     );
        LX_COL_SEP;
        mpl_list_t *tmp_p;
        int first_round = 1;
        MPL_LIST_FOR_EACH(number_range_p->range_list_p, tmp_p) {
            integer_range *integer_range_p = LISTABLE_PTR(tmp_p, integer_range);
            if (!first_round)
                LX_NEWLINE;
            fprintf_latex(f,
                          "%" PRIi64 "..%" PRIi64,
                          *integer_range_p->get_first_p(),
                          *integer_range_p->get_last_p()
                     );
            first_round = 0;
        }
        LX_COL_SEP;
        latex_latex_list(f, number_range_p->latex_list_p);
        LX_COL_END;
    }
}

void string_parameter::latex_options(FILE *f)
{
    parameter::latex_options(f);
    if (get_property("min")) {
        fprintf_latex(f,
                      "Min"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("min")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The minimum legal string length."
                     );
        LX_COL_END;
    }

    if (get_property("max")) {
        fprintf_latex(f,
                      "Max"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("max")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The maximum legal string length."
                     );
        LX_COL_END;
    }

    if (get_property("default")) {
        fprintf_latex(f,
                      "Default"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("default")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The default value (for use in configuration databases)."
                     );
        LX_COL_END;
    }
}

void bool_parameter::latex_options(FILE *f)
{
    parameter::latex_options(f);
    if (get_property("default")) {
        fprintf_latex(f,
                      "Default"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("default")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The default value (for use in configuration databases)."
                     );
        LX_COL_END;
    }
}

void array_parameter::latex_options(FILE *f)
{
    parameter::latex_options(f);
    if (get_property("min")) {
        fprintf_latex(f,
                      "Min"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("min")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The minimum legal array length."
                     );
        LX_COL_END;
    }

    if (get_property("max")) {
        fprintf_latex(f,
                      "Max"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("max")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The maximum legal array length."
                     );
        LX_COL_END;
    }
}

void enum_parameter::latex_options(FILE *f)
{
    parameter::latex_options(f);
    if (get_property("default")) {
        fprintf_latex(f,
                      "Default"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("default")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The default value (for use in configuration databases)."
                     );
        LX_COL_END;
    }
}

void bag_parameter::latex_options(FILE *f)
{
    parameter::latex_options(f);
    if (get_property("min")) {
        fprintf_latex(f,
                      "Min"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("min")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The minimum number of fields present."
                     );
        LX_COL_END;
    }

    if (get_property("max")) {
        fprintf_latex(f,
                      "Max"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("max")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The maximum number of fields present."
                     );
        LX_COL_END;
    }

    if (field_table_parameter_p) {
        fprintf_latex(f,
                      "Field table enum"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      field_table_parameter_p->name_p
                     );
        fprintf(f,
                " \\ref{%s:%s}",
                field_table_parameter_p->parameter_set_p->name_p,
                field_table_parameter_p->name_p
            );
        LX_COL_SEP;
        fprintf_latex(f,
                      "Enum mapping field names to numeric field identities."
                     );
        LX_COL_END;
    }
}

void tuple_parameter::latex_options(FILE *f)
{
    parameter::latex_options(f);
    if (get_property("min")) {
        fprintf_latex(f,
                      "Min"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("min")
                     );
        LX_COL_SEP;
        switch (get_type_of_tuple(get_type())) {
            case 1:
                fprintf_latex(f,
                              "The minimum legal string length for both key and value fields."
                             );
                break;
            case -1:
                fprintf_latex(f,
                              "The minimum legal value for both key and value fields."
                             );
                break;
            case 2:
            case 8:
                fprintf_latex(f,
                              "The minimum legal value for value field."
                             );
                break;
            default:
                assert(0);
        }
        LX_COL_END;
    }

    if (get_property("max")) {
        fprintf_latex(f,
                      "Max"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      (char*)get_property("max")
                     );
        LX_COL_SEP;
        switch (get_type_of_tuple(get_type())) {
            case 1:
                fprintf_latex(f,
                              "The maximum legal string length for both key and value fields."
                             );
                break;
            case -1:
                fprintf_latex(f,
                              "The maximum legal value for both key and value fields."
                             );
                break;
            case 2:
            case 8:
                fprintf_latex(f,
                              "The maximum legal value for value field."
                             );
                break;
            default:
                assert(0);
        }
        LX_COL_END;
    }

    if (get_property("default")) {
        fprintf_latex(f,
                      "Default"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "(%s, %s)",
                      (char*)get_property("default"),
                      (char*)get_property("default2")
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "The default value for both fields (for use in configuration databases)."
                     );
        LX_COL_END;
    }
}

void bag_parameter::dox(FILE *f, int level)
{
    fprintf(f, "%s", spacing(level));
    dox_doc_f(f, doc_list_p);
    fprintf(f, "%s", spacing(level));
    fprintf(f,
            "struct %s_t {\n",
            name_p
           );

    gc_h_bag(f);
    dox_bag_parameter_list(f, level);

    fprintf(f, "%s", spacing(level));
    fprintf(f,
            "};\n"
           );
}

void bag_parameter::latex(FILE *f, parameter_set *default_parameter_set_p)
{
    parameter::latex(f, default_parameter_set_p);
    latex_bag_parameter_list(f, default_parameter_set_p);
}

void enum_parameter::dox(FILE *f, int level)
{
    if (is_virtual)
        return;

    fprintf(f, "%s", spacing(level));
    dox_doc_f(f, doc_list_p);
    fprintf(f, "%s", spacing(level));
    fprintf(f,
            "enum %s_t {\n",
            name_p
           );

    dox_enum_values(f, level);

    fprintf(f, "%s", spacing(level));
    fprintf(f,
            "};\n"
           );
}

void enum_parameter::latex(FILE *f, parameter_set *default_parameter_set_p)
{
    parameter::latex(f, default_parameter_set_p);
    latex_enum_values(f);
}

void parameter::convert_doc()
{
    listable_object::convert_doc(get_type(), name_p);
}

void int_parameter::convert_doc()
{
    mpl_list_t *tmp_p;
    number_range *number_range_p;

    parameter::convert_doc();

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("number_ranges"), tmp_p) {
        number_range_p = LISTABLE_PTR(tmp_p, number_range);
        number_range_p->convert_doc("number_range",
                                    number_range_p->name_p);
    }
}

void bag_parameter::convert_doc()
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    parameter::convert_doc();

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_list_entry_p->convert_doc("parameter_list_entry",
                                            parameter_list_entry_p->parameter_name_p);
    }
}

void enum_parameter::convert_doc()
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;

    parameter::convert_doc();

    MPL_LIST_FOR_EACH(enum_values_p, tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        enum_value_p->convert_doc("enum_value",
                                  enum_value_p->name_p);
    }
}

int enum_parameter::num_entries_with_value(int64_t value)
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;
    int num_entries = 0;

    MPL_LIST_FOR_EACH(enum_values_p, tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        if (enum_value_p->value_p != NULL)
            if (*enum_value_p->value_p == value)
                num_entries++;
    }
    return num_entries;
}

int enum_parameter::num_entries_with_name(char *nm_p)
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;
    int num_entries = 0;

    MPL_LIST_FOR_EACH(enum_values_p, tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        if (!strcmp(enum_value_p->name_p, nm_p))
            num_entries++;
    }
    return num_entries;
}


int64_t enum_parameter::min_numeric_value()
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;
    int min_value_set = 0;
    int64_t min_value = 0;

    MPL_LIST_FOR_EACH(enum_values_p, tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        if (enum_value_p->value_p != NULL) {
            if (!min_value_set) {
                min_value = *enum_value_p->value_p;
                min_value_set = 1;
                continue;
            }
            min_value = (*enum_value_p->value_p < min_value) ? *enum_value_p->value_p : min_value;
        }
    }
    return min_value;
}

int64_t enum_parameter::max_numeric_value()
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;
    int max_value_set = 0;
    int64_t max_value = 0;

    MPL_LIST_FOR_EACH(enum_values_p, tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        if (enum_value_p->value_p != NULL) {
            if (!max_value_set) {
                max_value = *enum_value_p->value_p;
                max_value_set = 1;
                continue;
            }
            max_value = (*enum_value_p->value_p > max_value) ? *enum_value_p->value_p : max_value;
        }
    }
    return max_value;
}

void enum_parameter::wrap_up_definition()
{
    if (parent_p) {
        if (enum_values_p) {
            if (((parameter*)parent_p)->external_parent_parameter_set_name_p) {
                fprintf(stderr, "%s:%d: cannot add enum values when inheriting from another parameter set\n",
                        compiler_p->peek_filename(), compiler_p->lineno()
                       );
                exit(-1);
            }

            /* Prepend parent value list to this list */
            mpl_list_t *my_list_p = enum_values_p;
            enum_values_p = listable_object_list_clone((mpl_list_t *) parent_p->get_property("enum_values"));
            mpl_list_append(&enum_values_p, my_list_p);
        }
    }

    mpl_list_t *tmp_p;
    enum_value *enum_value_p;
    int64_t current_value = 0;

    MPL_LIST_FOR_EACH(enum_values_p, tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        if (enum_value_p->get_value_p() != NULL)
            current_value = *enum_value_p->get_value_p();
        else
            enum_value_p->set_value(current_value);

        current_value++;
    }

    MPL_LIST_FOR_EACH(enum_values_p, tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        if (num_entries_with_value(*enum_value_p->get_value_p()) != 1) {
            fprintf(stderr, "%s:%d: enum '%s': numeric value %" PRIi64 " not unique\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    name_p,
                    *enum_value_p->get_value_p()
                   );
            exit(-1);
        }
        if (num_entries_with_name(enum_value_p->get_name_p()) != 1) {
            fprintf(stderr, "%s:%d: enum '%s': value name '%s' not unique\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    name_p,
                    enum_value_p->get_name_p()
                   );
            exit(-1);
        }
    }
}

void bag_parameter::wrap_up_definition()
{
    if (parent_p) {
        /* Prepend parent parameter list to this list */
        mpl_list_t *my_list_p = bag_parameter_list_p;
        bag_parameter_list_p = listable_object_list_clone((mpl_list_t *) parent_p->get_property("parameter_list"));
        mpl_list_append(&bag_parameter_list_p, my_list_p);
    }

    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter *parameter_p = parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);
        if (parameter_list_entry_p->field_name_p != NULL) {
            if (field_table_parameter_p == NULL) {
                field_table_parameter_p = parameter_set_p->create_field_table_parameter(name_p);
                field_table_parameter_p->add_enum_value(parameter_list_entry_p->field_name_p,
                                                       (char *)"1");
            }
            else {
                field_table_parameter_p->add_enum_value(parameter_list_entry_p->field_name_p,
                                                       NULL);
            }
        }
        if ((parameter_p == this) && !parameter_list_entry_p->optional) {
            fprintf(stderr, "%s:%d: recursive field '%s' must be optional\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    parameter_list_entry_p->field_name_p ? parameter_list_entry_p->field_name_p : name_p
                   );
            exit(-1);
        }
    }
    if (field_table_parameter_p != NULL)
        field_table_parameter_p->wrap_up_definition();
}

void bag_parameter::force_parameter_list_direction_unless_inout(direction_t direction)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        if (parameter_list_entry_p->direction != direction_inout)
            parameter_list_entry_p->direction = direction;
    }
}

const char *parameter::tcl_param_id()
{
    if (!tcl_param_id_p) {
        tcl_param_id_p = (char*) calloc(1, 100);
        if (compiler_p->num_parameter_sets() > 1) {
            sprintf(tcl_param_id_p,
                    "%s.%s",
                    parameter_set_p->name_p,
                    name_p
                   );
        }
        else {
            sprintf(tcl_param_id_p,
                    "%s",
                    name_p
                   );
        }
    }
    return tcl_param_id_p;
}

void parameter::deja(FILE *f)
{
    fprintf(f, "    # %s parameter %s\n",
            get_type(),
            name_p);

    fprintf(f,
            "    namespace eval pack {\n"
           );
    fprintf(f,
            "        ## %s pack function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # value - the value to be packed\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The input formatted as \"packed mpl string\"\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set value $%sValue\n"
            "        #\n",
            is_addr() ? "addr" : "number"
           );
    fprintf(f,
            "        proc %s { value {field \"\"} {tag \"\"} } {\n",
            name_p
           );

    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"\\[$tag\\]\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    fprintf(f,
            "            return \"$name$tagstr=$value\""
           );
    fprintf(f,
            "\n"
            "        }\n\n"
           );
    fprintf(f,
            "    }\n\n"
           );

    fprintf(f,
            "    namespace eval check {\n"
           );

    fprintf(f,
            "        ## %s check function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # value - the value to check (if \"\": don't check but return actual value)\n"
            "        # msg - the message containing \"packed mpl string\" formatted value\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The actual value\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set value $%sValue\n"
            "        #\n",
            is_addr() ? "addr" : "number"
           );
    fprintf(f,
            "        proc %s { value msg {field \"\"} {tag \"\"} } {\n",
            name_p
           );
    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"$tag\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    fprintf(f,
            "            if {$value == \"\"} {\n"
            "                return [get_Value $name $msg $tagstr]\n"
            "            }\n"
           );
    fprintf(f,
            "            return [check_Value $name $value $msg $tagstr]\n"
           );

    fprintf(f,
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );
}

void string_parameter::deja(FILE *f)
{
    bool is_string = (!strcmp(get_type(), "string"));
    fprintf(f, "    # %s parameter %s\n",
            get_type(),
            name_p);

    fprintf(f,
            "    namespace eval pack {\n"
           );
    fprintf(f,
            "        ## %s pack function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # value - the value to be packed\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The input formatted as \"packed mpl string\"\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set value $%sValue\n"
            "        #\n",
            is_string ? "string" : "uint32_array"
           );
    fprintf(f,
            "        proc %s { value {field \"\"} {tag \"\"} } {\n",
            name_p
           );

    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"\\[$tag\\]\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    if (is_string) {
        fprintf(f,
                "            set value [fill_escape $value]\n"
               );
        fprintf(f,
                "            return \"$name$tagstr=$value\"\n"
               );
    }
    else {
        fprintf(f,
                "            set value [pack_uint32_array $value]\n"
               );
        fprintf(f,
                "            return \"$name$tagstr=$value\""
               );
    }
    fprintf(f,
            "\n"
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );

    fprintf(f,
            "    namespace eval check {\n"
           );
    fprintf(f,
            "        ## %s check function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # value - the value to check (if \"\": don't check but return actual value)\n"
            "        # msg - the message containing \"packed mpl string\" formatted value\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The actual value\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set value $%sValue\n"
            "        #\n",
            is_string ? "string" : "uint32_array"
           );
    fprintf(f,
            "        proc %s { value msg {field \"\"} {tag \"\"} } {\n",
            name_p
           );
    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"$tag\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            name_p
           );
    if (is_string) {
        fprintf(f,
                "            if {$value == \"\"} {\n"
                "                return [get_Value $name $msg $tagstr]\n"
                "            }\n"
               );
        fprintf(f,
                "            return [check_Value $name $value $msg $tagstr]\n"
               );
    }
    else {
        fprintf(f,
                "            if {$value == \"\"} {\n"
                "                return [get_uint32_array $name $msg $tagstr]\n"
                "            }\n"
               );
        fprintf(f,
                "            return [check_uint32_array $name $value $msg $tagstr]\n"
               );
    }

    fprintf(f,
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );
}

void bool_parameter::deja(FILE *f)
{
    fprintf(f, "    # %s parameter %s\n",
            get_type(),
            name_p);

    fprintf(f,
            "    namespace eval pack {\n"
           );
    fprintf(f,
            "        ## %s pack function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # value - the value to be packed\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The input formatted as \"packed mpl string\"\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set value $booleanValue\n"
            "        #\n"
           );
    fprintf(f,
            "        proc %s { value {field \"\"} {tag \"\"} } {\n",
            name_p
           );

    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"\\[$tag\\]\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    fprintf(f,
            "            if { $value } {\n"
            "                set value \"true\"\n"
            "            } else {\n"
            "                set value \"false\"\n"
            "            }\n"
           );
    fprintf(f,
            "            return \"$name$tagstr=$value\""
           );
    fprintf(f,
            "\n"
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );

    fprintf(f,
            "    namespace eval check {\n"
           );
    fprintf(f,
            "        ## %s check function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # value - the value to check (if \"\": don't check but return actual value)\n"
            "        # msg - the message containing \"packed mpl string\" formatted value\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The actual value\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set value $booleanValue\n"
            "        #\n"
           );
    fprintf(f,
            "        proc %s { value msg {field \"\"} {tag \"\"} } {\n",
            name_p
           );
    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"$tag\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    fprintf(f,
            "            if { $value } {\n"
            "                set value \"true\"\n"
            "            } else {\n"
            "                set value \"false\"\n"
            "            }\n"
           );
    fprintf(f,
            "            if {$value == \"\"} {\n"
            "                return [get_Value $name $msg $tagstr]\n"
            "            }\n"
           );
    fprintf(f,
            "            return [check_Value $name $value $msg $tagstr]\n"
           );

    fprintf(f,
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );
}

void array_parameter::deja(FILE *f)
{
    fprintf(f, "    # %s parameter %s\n",
            get_type(),
            name_p);
    fprintf(f,
            "    namespace eval pack {\n"
           );
    fprintf(f,
            "        ## %s pack function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # value - the value to be packed\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The input formatted as \"packed mpl string\"\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set value $uint%d_arrayValue\n"
            "        #\n",
            get_type_of_array(get_type())
           );
    fprintf(f,
            "        proc %s { value {field \"\"} {tag \"\"} } {\n",
            name_p
           );

    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"\\[$tag\\]\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    fprintf(f,
            "            set value [pack_uint%d_array $value]\n",
            get_type_of_array(get_type())
           );
    fprintf(f,
            "            return \"$name$tagstr=$value\""
           );
    fprintf(f,
            "\n"
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );

    fprintf(f,
            "    namespace eval check {\n"
           );
    fprintf(f,
            "        ## %s check function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # value - the value to check (if \"\": don't check but return actual value)\n"
            "        # msg - the message containing \"packed mpl string\" formatted value\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The actual value\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set value $uint%d_arrayValue\n"
            "        #\n",
            get_type_of_array(get_type())
           );
    fprintf(f,
            "        proc %s { value msg {field \"\"} {tag \"\"} } {\n",
            name_p
           );
    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"$tag\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    fprintf(f,
            "            if {$value == \"\"} {\n"
            "                return [get_uint%d_array $name $msg $tagstr]\n"
            "            }\n",
            get_type_of_array(get_type())
           );
    fprintf(f,
            "            return [check_uint%d_array $name $value $msg $tagstr]\n",
            get_type_of_array(get_type())
           );

    fprintf(f,
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );
}

void enum_parameter::deja(FILE *f)
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;

    fprintf(f, "    # %s parameter %s\n",
            get_type(),
            name_p);

    fprintf(f,
            "    set %s_enum [dict create]\n",
            name_p
           );

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("enum_values"), tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        fprintf(f,
                "    dict set %s_enum %" PRIi64 " %s\n",
                name_p,
                *enum_value_p->value_p,
                enum_value_p->name_p
               );
        fprintf(f,
                "    dict set %s_enum %s %" PRIi64 "\n",
                name_p,
                enum_value_p->name_p,
                *enum_value_p->value_p
               );
    }

    fprintf(f,
            "\n"
           );
    fprintf(f,
            "    namespace eval pack {\n"
           );
    fprintf(f,
            "        ## %s pack function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # value - the value to be packed\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The input formatted as \"packed mpl string\"\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set value $numericEnumValue\n"
            "        #   or\n"
            "        # set value $symbolicEnumValue\n"
            "        #\n"
           );
    fprintf(f,
            "        proc %s { value {field \"\"} {tag \"\"} } {\n",
            name_p
           );
    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"\\[$tag\\]\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    fprintf(f,
            "            if { [regexp \"^\\[0-9\\]+$|^0\\[xX+]\\[0-9a-fA-F\\]+$\" $value] } {\n"
           );
    int show_psn = (compiler_p->num_parameter_sets() > 1);
    fprintf(f,
            "                set value %s%s[dict get $%s::%s_enum $value]\n",
            show_psn ? parameter_set_p->name_p : "",
            show_psn ? "." : "",
            parameter_set_p->name_p,
            name_p
           );
    fprintf(f,
            "            }\n"
           );
    fprintf(f,
            "            return \"$name$tagstr=$value\""
           );
    fprintf(f,
            "\n"
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );

    fprintf(f,
            "    namespace eval check {\n"
           );
    fprintf(f,
            "        ## %s check function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # value - the value to check (if \"\": don't check but return actual value)\n"
            "        # msg - the message containing \"packed mpl string\" formatted value\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The actual value\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set value $numericEnumValue\n"
            "        #   or\n"
            "        # set value $symbolicEnumValue\n"
            "        #\n"
           );
    fprintf(f,
            "        proc %s { value msg {field \"\"} {tag \"\"} } {\n",
            name_p
           );
    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"$tag\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    fprintf(f,
            "            if {$value == \"\"} {\n"
            "                return [get_Value $name $msg $tagstr]\n"
            "            }\n"
           );
    fprintf(f,
            "            if { [regexp \"^\\[0-9\\]+$|^0\\[xX+]\\[0-9a-fA-F\\]+$\" $value] } {\n"
           );
    show_psn = (compiler_p->num_parameter_sets() > 1);
    fprintf(f,
            "                set value %s%s[dict get $%s::%s_enum $value]\n",
            show_psn ? parameter_set_p->name_p : "",
            show_psn ? "." : "",
            parameter_set_p->name_p,
            name_p
           );
    fprintf(f,
            "            }\n"
           );
    fprintf(f,
            "            return [check_Value $name $value $msg $tagstr]\n"
           );

    fprintf(f,
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );
}


void bag_parameter::deja(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;

    fprintf(f, "    # %s parameter %s\n",
            get_type(),
            name_p);

    fprintf(f,
            "    namespace eval pack {\n"
           );
    fprintf(f,
            "        ## %s pack function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # inDict - dictionary representing the value\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The input formatted as \"packed mpl string\"\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## inDict format:\n"
            "        # set inDict [dict create]\n"
           );
    int has_array_elements = 0;
    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p =
            parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);
        int m = parameter_list_entry_p->multiple;
        const char *pn = parameter_p->name_p;
        const char *fn = parameter_list_entry_p->field_name_p;
        fprintf(f,
                "        # dict set inDict %s $%s\n",
                fn ? fn : pn,
                m ? "arrayDict" : "value"
               );
        if (m)
            has_array_elements = 1;
    }
    if (has_array_elements)
        fprintf(f,
                "        #\n"
                "        ## arrayDict format:\n"
                "        # set arrayDict [dict create]\n"
                "        # dict set arrayDict 1 $elementValue1\n"
                "        # dict set arrayDict 2 $elementValue2\n"
                "        # dict set arrayDict 3 $elementValue3\n"
                "        # .....\n"
                "        # ...\n"
                "        #\n"
               );
    else
        fprintf(f,
                "        #\n"
               );

    fprintf(f,
            "        proc %s { inDict {field \"\"} {tag \"\"} } {\n",
            name_p
           );

    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"\\[$tag\\]\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            set pstr \"\"\n"
            "            set sep \"\"\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );

    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p =
            parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);
        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;
        const char *pn = parameter_p->name_p;
        const char *psn = parameter_p->parameter_set_p->name_p;
        const char *fn = parameter_list_entry_p->field_name_p;
        fprintf(f,
                "            if { [dict exists $inDict %s] } {\n",
                fn ? fn : pn
               );
        if (!m) {
            fprintf(f,
                    "                set pstr \"$pstr$sep[%s::pack::%s [dict get $inDict %s]%s%s]\"\n",
                    psn,
                    pn,
                    fn ? fn : pn,
                    fn ? " " : "",
                    fn ? fn : ""
                   );
        }
        else {
            fprintf(f,
                    "                set pstr \"$pstr$sep[pack_multiple %s::pack::%s [dict get $inDict %s]%s%s]\"\n",
                    psn,
                    pn,
                    fn ? fn : pn,
                    fn ? " " : "",
                    fn ? fn : ""
                   );
        }
        fprintf(f,
                "                set sep \",\"\n"
                "            }\n"
               );
    }

    fprintf(f,
            "            return \"$name$tagstr={$pstr}\"\n"
           );
    fprintf(f,
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );

    fprintf(f,
            "    namespace eval check {\n"
           );
    fprintf(f,
            "        ## %s check function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # checkDict - dictionary representing the value to check\n"
            "        # msg - the message containing \"packed mpl string\" formatted value\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # A dictionary with the same members as \"checkDict\" with actual\n"
            "        # values filled in\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## checkDict format:\n"
            "        # set checkDict [dict create]\n"
           );
    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p =
            parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);
        int m = parameter_list_entry_p->multiple;
        const char *pn = parameter_p->name_p;
        const char *fn = parameter_list_entry_p->field_name_p;
        fprintf(f,
                "        # dict set checkDict %s $%s\n",
                fn ? fn : pn,
                m ? "arrayDict" : "value"
               );
    }
    if (has_array_elements) {
        fprintf(f,
                "        #\n"
                "        ## checkDict elements:\n"
                "        # - value/arrayDict == $value/$dict means \"check and fill in actual value\n"
                "        #                                          in returned dictionary\"\n"
                "        # - value/arrayDict == \"\" means \"don't check but fill in actual value\n"
                "        #                                in returned dictionary\"\n"
                "        # - element not present means \"ignore (don't care)\"\n"
               );
        fprintf(f,
                "        #\n"
                "        ## arrayDict \"exact element\" format:\n"
                "        # set arrayDict [dict create]\n"
                "        # dict set arrayDict 1 $elementValue1\n"
                "        # dict set arrayDict 2 $elementValue2\n"
                "        # dict set arrayDict 3 $elementValue3\n"
                "        # .....\n"
                "        # ...\n"
               );
        fprintf(f,
                "        #\n"
                "        ## arrayDict \"any element\" format:\n"
                "        # set arrayDict [dict create]\n"
                "        # dict set arrayDict -1 $anyElementValue1\n"
                "        # dict set arrayDict -2 $anyElementValue2\n"
                "        # dict set arrayDict -3 $anyElementValue3\n"
                "        # .....\n"
                "        # ...\n"
                "        #\n"
               );
    }
    else {
        fprintf(f,
                "        #\n"
                "        ## checkDict elements:\n"
                "        # - value == $value means \"check and fill in actual value\n"
                "        #                          in returned dictionary\"\n"
                "        # - value == \"\" means \"don't check but fill in actual value\n"
                "        #                      in returned dictionary\"\n"
                "        # - element not present means \"ignore (don't care)\"\n"
                "        #\n"
               );
    }
    fprintf(f,
            "        proc %s { checkDict msg {field \"\"} {tag \"\"} } {\n",
            name_p
           );

    fprintf(f,
            "            set elementsDict [dict create]\n"
           );
    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p =
            parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);
        const char *pn = parameter_p->name_p;
        const char *fn = parameter_list_entry_p->field_name_p;
        fprintf(f,
                "            dict set elementsDict %s \"\"\n",
                fn ? fn : pn
               );
    }
    fprintf(f,
            "            set illegal_key [validate_dict_elements $checkDict $elementsDict]\n"
            "            if {$illegal_key != \"\"} {\n"
            "                fail \"Illegal element \\\"$illegal_key\\\" in checkDict\"\n"
            "            }\n"
           );

    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"$tag\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    fprintf(f,
            "            set bag [get_CompoundValue $name $msg $tagstr]\n"
            "            if {$checkDict == \"\"} {\n"
            "                return $bag\n"
            "            }\n",
            name_p
           );
    fprintf(f,
            "            set resDict [dict create]\n"
           );
    MPL_LIST_FOR_EACH((mpl_list_t*)get_property("parameter_list"), tmp_p) {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        parameter_p =
            parameter_list_entry_p->parameter_set_p->find_parameter(parameter_list_entry_p->parameter_name_p);
        int o = parameter_list_entry_p->optional;
        int m = parameter_list_entry_p->multiple;
        const char *pn = parameter_p->name_p;
        const char *psn = parameter_p->parameter_set_p->name_p;
        const char *fn = parameter_list_entry_p->field_name_p;
        fprintf(f,
                "            if { [dict exists $checkDict %s] } {\n",
                fn ? fn : pn
               );
        if (!m) {
            fprintf(f,
                    "                dict set resDict %s [%s::check::%s [dict get $checkDict %s] $msg %s $tagstr]\n",
                    fn ? fn : pn,
                    psn,
                    pn,
                    fn ? fn : pn,
                    fn ? fn : "\"\""
                   );
        }
        else {
            fprintf(f,
                    "                dict set resDict %s [check_multiple %s::check::%s [dict get $checkDict %s] $msg %s %s]\n",
                    fn ? fn : pn,
                    psn,
                    pn,
                    fn ? fn : pn,
                    fn ? fn : "\"\"",
                    parameter_p->is_bag() ? "true" : "false"
                   );
        }
        fprintf(f,
                "            }\n"
               );
    }

    fprintf(f,
            "            return $resDict\n"
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );
}

void tuple_parameter::deja(FILE *f)
{
    fprintf(f, "    # %s parameter %s\n",
            get_type(),
            name_p);

    fprintf(f,
            "    namespace eval pack {\n"
           );
    fprintf(f,
            "        ## %s pack function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # valueList - the value to be packed\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The input formatted as \"packed mpl string\"\n",
            name_p
           );

    int key_is_string = 0;
    int value_is_string = 0;

    if (!strcmp(get_type(), "string_tuple") ||
        !strcmp(get_type(), "strint_tuple") ||
        !strcmp(get_type(), "struint8_tuple"))
        key_is_string = 1;
    if (!strcmp(get_type(), "string_tuple"))
        value_is_string = 1;
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set valueList [list $%sKey $%sValue]\n"
            "        #\n",
            key_is_string ? "string" : "number",
            value_is_string ? "string" : "number"
           );
    fprintf(f,
            "        proc %s { valueList {field \"\"} {tag \"\"} } {\n",
            name_p
           );

    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"\\[$tag\\]\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    if (!strcmp(get_type(), "string_tuple") ||
        !strcmp(get_type(), "strint_tuple") ||
        !strcmp(get_type(), "struint8_tuple")) {
        fprintf(f,
                "            set key [fill_escape [lindex $valueList 0]]\n"
               );
    }
    else {
        fprintf(f,
                "            set key [lindex $valueList 0]\n"
               );
    }

    if (!strcmp(get_type(), "string_tuple")) {
        fprintf(f,
                "            set value [fill_escape [lindex $valueList 1]]\n"
               );
    }
    else {
        fprintf(f,
                "            set value [lindex $valueList 1]\n"
               );
    }

    if (!strcmp(get_type(), "struint8_tuple")) {
        fprintf(f,
                "            set delimiter \"/\"\n"
               );
    }
    else {
        fprintf(f,
                "            set delimiter \":\"\n"
               );
    }

    fprintf(f,
            "            return \"$name$tagstr=$key$delimiter$value\"\n"
           );
    fprintf(f,
            "\n"
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );

    fprintf(f,
            "    namespace eval check {\n"
           );
    fprintf(f,
            "        ## %s check function\n"
            "        #\n"
            "        ## Parameters:\n"
            "        # valueList - the value to check (if \"\": don't check but return actual value)\n"
            "        # msg - the message containing \"packed mpl string\" formatted value\n"
            "        # field - name of field (if member of a \"fielded bag\")\n"
            "        # tag - tag value (e.g. if element in an array)\n"
            "        #\n"
            "        ## Return value:\n"
            "        # The actual value\n",
            name_p
           );
    fprintf(f,
            "        #\n"
            "        ## Value format:\n"
            "        # set valueList [list $%sKey $%sValue]\n"
            "        #\n",
            key_is_string ? "string" : "number",
            value_is_string ? "string" : "number"
           );
    fprintf(f,
            "        proc %s { valueList msg {field \"\"} {tag \"\"} } {\n",
            name_p
           );
    fprintf(f,
            "            if {$tag != \"\"} {\n"
            "                set tagstr \"$tag\"\n"
            "            } else {\n"
            "                set tagstr \"\"\n"
            "            }\n"
            "            if {$field != \"\"} {\n"
            "                set name \"$field\"\n"
            "            } else {\n"
            "                set name \"%s\"\n"
            "            }\n",
            tcl_param_id()
           );
    fprintf(f,
            "            if {$valueList == \"\"} {\n"
            "                return [get_Value $name $msg $tagstr]\n"
            "            }\n"
           );
    if (!strcmp(get_type(), "string_tuple") ||
        !strcmp(get_type(), "strint_tuple") ||
        !strcmp(get_type(), "struint8_tuple")) {
        fprintf(f,
                "            set key [fill_escape [lindex $valueList 0]]\n"
               );
    }
    else {
        fprintf(f,
                "            set key [lindex $valueList 0]\n"
               );
    }

    if (!strcmp(get_type(), "string_tuple")) {
        fprintf(f,
                "            set value [fill_escape [lindex $valueList 1]]\n"
               );
    }
    else {
        fprintf(f,
                "            set value [lindex $valueList 1]\n"
               );
    }

    if (!strcmp(get_type(), "struint8_tuple")) {
        fprintf(f,
                "            set delimiter \"/\"\n"
               );
    }
    else {
        fprintf(f,
                "            set delimiter \":\"\n"
               );
    }
    fprintf(f,
            "            return [check_Value $name $key$delimiter$value $msg $tagstr]\n"
           );

    fprintf(f,
            "        }\n"
           );
    fprintf(f,
            "    }\n\n"
           );
}

