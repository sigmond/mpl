/*
*   Copyright 2012 ST-Ericsson SA
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
#include <FlexLexer.h>
#include "mplcomp.hh"
#include "mplcomp_parameter.hh"
#include <unistd.h>
#include <stdlib.h>
#include <sstream>

parameter_set::~parameter_set()
{
    if (name_p)
        free(name_p);
    if (prefix_p)
        delete prefix_p;
    if (paramset_id_p)
        delete paramset_id_p;
    if (range_id_p)
        delete range_id_p;
    if (bag_field_table_suffix_p)
        delete bag_field_table_suffix_p;
    DELETE_LISTABLE_LIST(&parameter_group_list_p, parameter_group);
    DELETE_LISTABLE_LIST(&enumerator_lists_p, enumerator_list);
    DELETE_LISTABLE_LIST(&number_range_list_p, number_range);
}

parameter* parameter_set::create_parameter_in_group(const char *type_p,
                                                    char *parameter_name_p,
                                                    parameter_group *parameter_group_p)
{
    parameter *parameter_p;

    if (find_parameter(parameter_name_p) != NULL) {
        fprintf(stderr, "%s:%d: parameter %s already exists in parameter set %s\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                parameter_name_p, name_p);
        exit(-1);
    }

    if (get_type_of_int(type_p) != 0)
        /* Integer type */
        parameter_p = new int_parameter(compiler_p, strdup(parameter_name_p), type_p, this);
    else if (get_type_of_enum(type_p) != 0)
        /* Enum type */
        parameter_p = new enum_parameter(compiler_p, strdup(parameter_name_p), type_p, this);
    else if (get_type_of_bool(type_p) != 0)
        /* Bool type */
        parameter_p = new bool_parameter(compiler_p, strdup(parameter_name_p), type_p, this);
    else if (get_type_of_string(type_p) != 0)
        /* String type */
        parameter_p = new string_parameter(compiler_p, strdup(parameter_name_p), type_p, this);
    else if (get_type_of_tuple(type_p) != 0)
        /* Tuple */
        parameter_p = new tuple_parameter(compiler_p, strdup(parameter_name_p), type_p, this);
    else if (get_type_of_array(type_p) != 0)
        /* Array */
        parameter_p = new array_parameter(compiler_p, strdup(parameter_name_p), type_p, this);
    else if (!strcmp(type_p, "bag"))
        /* Bag */
        parameter_p = new bag_parameter(compiler_p, strdup(parameter_name_p), type_p, this);
    else if (!strcmp(type_p, "addr"))
        /* Addr */
        parameter_p = new addr_parameter(compiler_p, strdup(parameter_name_p), type_p, this);
    else {
        /* Other */
        fprintf(stderr, "%s:%d: unsupported parameter type %s\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                type_p);
        exit(-1);
    }

    parameter_p->append_to(parameter_group_p->parameters_p);
    return parameter_p;
}

parameter* parameter_set::create_parameter_in_default_group(const char *type_p,
                                                            char *parameter_name_p)
{
    return create_parameter_in_group(type_p, parameter_name_p, default_parameter_group_p);
}

parameter* parameter_set::create_parameter_in_current_group(const char *type_p,
                                                            char *parameter_name_p)
{
    return create_parameter_in_group(type_p, parameter_name_p, current_parameter_group_p);
}

int parameter_set::check_parameters()
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;
    int ret = 0;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;

        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            if (parameter_p->is_bag())
                ret += check_paramlist_parameters((mpl_list_t*) parameter_p->get_property("parameter_list"));
        }
    }
    return ret;
}

void parameter_set::print(int level)
{
    printf("%s", spacing(level++));
    printf("parameter_set\n");
    printf("%s", spacing(level));
    printf("->name_p = %s\n", name_p);
    prefix_p->print(level, "prefix_p");
    paramset_id_p->print(level, "paramset_id_p");
    if (range_id_p)
        range_id_p->print(level, "range_id_p");
    if (bag_field_table_suffix_p)
        bag_field_table_suffix_p->print(level, "bag_field_table_suffix_p");
    listable_object::print(level);
    print_parameter_groups(level);
    print_enumerator_lists(level);
    print_number_ranges(level);
    return;
}

void parameter_set::add_prefix(char *pfx_p)
{
    prefix_p = new string_entry(compiler_p, strdup(pfx_p));
    compiler_p->add_any_forward_doc(&prefix_p->doc_list_p);
}

void parameter_set::add_id(char *id_p)
{
    paramset_id_p = new string_entry(compiler_p, strdup(id_p));
    compiler_p->add_any_forward_doc(&paramset_id_p->doc_list_p);
}

void parameter_set::add_range_id(char *rid_p)
{
    range_id_p = new string_entry(compiler_p, strdup(rid_p));
    compiler_p->add_any_forward_doc(&range_id_p->doc_list_p);
}

void parameter_set::add_bag_field_table_suffix(char *suffix_p)
{
    if (bag_field_table_suffix_p != NULL) {
        fprintf(stderr, "%s:%d: bag field table suffix already defined\n",
                compiler_p->peek_filename(), compiler_p->lineno());
        exit(-1);
    }

    bag_field_table_suffix_p = new string_entry(compiler_p, strdup(suffix_p));
    compiler_p->add_any_forward_doc(&bag_field_table_suffix_p->doc_list_p);
}

void parameter_set::print_parameter_groups(int level)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);
        parameter_group_p->print(level);
    }
}

char *parameter_set::get_short_name()
{
    char *value_p = NULL;
    (void) compiler_p->get_compiler_option("parameter_set", name_p, "short_name", &value_p);
    return value_p;
}

void parameter_set::add_enumerator_list(enumerator_list *elist_p)
{
    elist_p->add_to(enumerator_lists_p);
}

enumerator_list *parameter_set::get_enumerator_list(char *enumerator_list_name_p)
{
    mpl_list_t *tmp_p;
    enumerator_list *enumerator_list_p;

    MPL_LIST_FOR_EACH(enumerator_lists_p, tmp_p) {
        enumerator_list_p = LISTABLE_PTR(tmp_p, enumerator_list);
        if (!strcmp(enumerator_list_p->name_p, enumerator_list_name_p))
            return enumerator_list_p;
    }
    return NULL;
}

void parameter_set::add_number_range(number_range *number_range_p)
{
    doc *doc_p;

    assert(number_range_p->name_p);
    if (find_number_range(number_range_p->name_p) != NULL) {
        fprintf(stderr, "%s:%d: number range %s already exists in parameter set %s\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                number_range_p->name_p, name_p);
        exit(-1);
    }

    if (range_id_p == NULL) {
        add_range_id((char*)NUMBER_RANGE_ID_DEFAULT);
    }

    enum_parameter *number_range_parameter_p =
        (enum_parameter*) find_parameter(range_id_p->value_p);
    if (number_range_parameter_p == NULL) {
        create_parameter_in_default_group("enum", range_id_p->value_p);
        number_range_parameter_p =
            (enum_parameter*) find_parameter(range_id_p->value_p);
        assert(number_range_parameter_p);
        number_range_parameter_p->add_option("virtual", "true", value_type_bool);
        doc_p = new doc(strdup("/** The values of this enum is used to identify number ranges. */"), 1);
        doc_p->append_to(number_range_parameter_p->doc_list_p);
        number_range_parameter_p->add_enum_value((char*)NUMBER_RANGE_NAME_ANONYMOUS, NULL);
        enum_value *anon_value_p = number_range_parameter_p->get_enum_value(NUMBER_RANGE_NAME_ANONYMOUS);
        doc_p = new doc(strdup(
                               "/** Unnamed number range. All number ranges that are\n"
                               "  * defined directly as parameter options will get this id. */"
                              ), 1);
        doc_p->append_to(anon_value_p->doc_list_p);
    }
    number_range_parameter_p->add_enum_value(number_range_p->name_p, NULL);
    enum_value *named_value_p = number_range_parameter_p->get_enum_value(number_range_p->name_p);
    doc_p = new doc(strdup("/** Named number range */"), 1);
    doc_p->append_to(named_value_p->doc_list_p);
    number_range_p->add_to(number_range_list_p);
}

number_range *parameter_set::find_number_range(const char *number_range_name_p)
{
    mpl_list_t *tmp_p;
    number_range *number_range_p;

    MPL_LIST_FOR_EACH(number_range_list_p, tmp_p) {
        number_range_p = LISTABLE_PTR(tmp_p, number_range);
        if (!strcmp(number_range_p->name_p, number_range_name_p))
            return number_range_p;
    }
    return NULL;
}


void parameter_set::print_enumerator_lists(int level)
{
    mpl_list_t *tmp_p;
    enumerator_list *enumerator_list_p;

    printf("%s", spacing(level));
    printf("name lists\n");
    MPL_LIST_FOR_EACH(enumerator_lists_p, tmp_p) {
        enumerator_list_p = LISTABLE_PTR(tmp_p, enumerator_list);
        enumerator_list_p->print(level+1);
    }
}

void parameter_set::print_number_ranges(int level)
{
    mpl_list_t *tmp_p;
    number_range *number_range_p;

    printf("%s", spacing(level));
    printf("number_ranges\n");
    MPL_LIST_FOR_EACH(number_range_list_p, tmp_p) {
        number_range_p = LISTABLE_PTR(tmp_p, number_range);
        number_range_p->print(level+1);
    }
}


parameter *parameter_set::find_parameter(char *name_p)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            if (!strcmp(parameter_p->name_p, name_p))
                return parameter_p;
        }
    }
    return NULL;
}

parameter_group *parameter_set::find_parameter_group(char *category_name_p)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        if (parameter_group_p->category_p)
            if (!strcmp(parameter_group_p->category_p->name_p, category_name_p))
                return parameter_group_p;
    }
    return NULL;
}

void parameter_set::gc_h_enumerator_lists(FILE *f)
{
    mpl_list_t *tmp_p;
    enumerator_list *enumerator_list_p;

    MPL_LIST_FOR_EACH(enumerator_lists_p, tmp_p) {
        enumerator_list_p = LISTABLE_PTR(tmp_p, enumerator_list);
        enumerator_list_p->gc_h(f, name_p);
    }
}

void parameter_set::gc_h_enums(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;
    int ret = 0;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;

        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            if (parameter_p->is_enum()) {
                ((enum_parameter*)parameter_p)->gc_h_enum(f, name_p);
            }
        }
    }
}


void parameter_set::gc_h_paramids(FILE *f)
{
    char *psl = name_p;
    char *psu = str_toupper(psl);

    fprintf(f,
            "/* %s_paramid_t */\n",
            psl
           );
    fprintf(f,
            "#define %s_PARAMETER_IDS \\\n",
            psu
           );

    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;
    int i = 1;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            char *type_p;
            char *min_p;
            char *max_p;
            char *default_p;
            char *default2_p;
            int *set_p;
            int *get_p;
            int *config_p;
            char min_str[255];
            char max_str[255];
            char default_str[255];
            char enum_str[255];
            char *pn;
            int id;
            const char *virt_p;

            parameter_p = LISTABLE_PTR(tmp_p, parameter);

            pn = parameter_p->name_p;

            type_p = (char *) parameter_p->get_property("type");
            min_p = (char *) parameter_p->get_property("min");
            max_p = (char *) parameter_p->get_property("max");
            default_p = (char *) parameter_p->get_property("default");
            default2_p = (char *) parameter_p->get_property("default2");
            set_p = (int *) parameter_p->get_property("set");
            get_p = (int *) parameter_p->get_property("get");
            config_p = (int *) parameter_p->get_property("config");
            id = i | (parameter_p->is_virtual ? 0x20000000 : 0);

            if (min_p)
                sprintf(min_str, "vmin(%s)", min_p);
            else
                sprintf(min_str, "no_min");

            if (max_p)
                sprintf(max_str, "vmax(%s)", max_p);
            else
                sprintf(max_str, "no_max");

            if (default2_p) {
                sprintf(default_str, "default(%s,%s)", default_p, default2_p);
            } else if (default_p) {
                if (get_type_of_enum(type_p) != 0) {
                    sprintf(default_str, "default(%s_%s_%s)",
                            psl,
                            parameter_p->name_p,
                            default_p);
                } else {
                    sprintf(default_str, "default(%s)", default_p);
                }
            } else {
                sprintf(default_str, "no_default");
            }

            if (get_type_of_enum(type_p) != 0)
                sprintf(enum_str, "%s", parameter_p->name_p);
            else
                sprintf(enum_str, "no_enum");

            fprintf(f,
                    "%s_PARAM_ID_ELEM(%s, %s, %s, %s, %s, %s, %s, %s, %s, 0x%x) \\\n",
                    psu,
                    parameter_p->name_p,
                    type_p,
                    enum_str,
                    min_str,
                    max_str,
                    (set_p && *set_p) ? "true" : "false",
                    (get_p && *get_p) ? "true" : "false",
                    (config_p && *config_p) ? "true" : "false",
                    default_str,
                    id
                   );
            i++;
        }
    }

    fprintf(f,
            "\n\n"
           );

    fprintf(f,
            "#define %s_PARAM_ID_ELEM(ELEMENT, TYPE, EXTRA, MIN, MAX, SET, GET, CONFIG, DEFAULT, ID) \\\n",
            psu
           );
    fprintf(f,
            "  %s_paramid_##ELEMENT = MPL_PARAM_SET_ID_TO_PARAMID_BASE(%s_PARAM_SET_ID) + ID,\n",
            psl,
            psu
           );

    fprintf(f,
            "\n\n"
           );

    fprintf(f,
            "typedef enum\n"
            "{\n"
           );
    fprintf(f,
            "  %s_paramid_base = MPL_PARAM_SET_ID_TO_PARAMID_BASE(%s_PARAM_SET_ID),\n",
            psl,
            psu
           );
    fprintf(f,
            "  %s_PARAMETER_IDS\n",
            psu
           );
    fprintf(f,
            "} %s_paramid_t;\n",
            psl
           );
    fprintf(f,
            "#undef %s_PARAM_ID_ELEM\n",
            psu
           );

    fprintf(f,
            "#define %s_enum_size_paramids %d\n",
            psl,
            i
           );
    fprintf(f,
            "\n\n"
           );
    free(psu);
}

void parameter_set::gc_h_macros(FILE *f)
{
    char *snu;
    char *snl;
    char *lnl = name_p;
    char *lnu = str_toupper(lnl);
    char group[100];
    int i;
    char *short_name_p;

    const char *basic_mpl_type_lc[] = {
        "int",
        "uint8",
        "uint16",
        "uint32",
        "uint64",
        "sint8",
        "sint8",
        "sint16",
        "sint32",
        "sint64",
        "bool",
        NULL
    };

    const char *basic_mpl_type_uc[] = {
        "INT",
        "UINT8",
        "UINT16",
        "UINT32",
        "UINT64",
        "CHAR",
        "SINT8",
        "SINT16",
        "SINT32",
        "SINT64",
        "BOOL",
        NULL
    };

    const char *basic_c_type[] = {
        "int",
        "uint8_t",
        "uint16_t",
        "uint32_t",
        "uint64_t",
        "char",
        "sint8_t",
        "sint16_t",
        "sint32_t",
        "int64_t",
        "bool",
        NULL
    };


    fprintf(f,
            "/* Macros */\n"
           );

    short_name_p = get_short_name();
    if (!short_name_p || !strlen(short_name_p)) {
        fprintf(f,
                "/* No short_name defined -> no macros */\n"
               );
        free(lnu);
        return;
    }

    snu = str_toupper(short_name_p);
    snl = short_name_p;

    fprintf(f,
            "/** @defgroup %s_FM %s Functions and macros\n"
            "  * @ingroup %s\n"
            "  * The macros and functions listed here can be used to manipulate\n"
            "  * %s parameter lists in different ways. They are implemented on top\n"
            "  * of the MPL library.\n"
            "  *\n"
            "  * Many of the macros use MPL functions that allocate MPL lists.\n"
            "  * These lists must be deallocated using @ref mpl_param_list_destroy.\n"
            "  */\n",
            snu,
            snu,
            snu,
            snu
           );

    /* Group *_UTIL */
    sprintf(group, "%s_FM_UTIL", snu);
    fprintf(f,
            "/** @defgroup %s Utility functions and macros\n"
            "  *  @ingroup %s_FM\n"
            "  */\n",
            group,
            snu
           );

    /* Group *_ADD */
    sprintf(group, "%s_FM_ADD", snu);
    fprintf(f,
            "/** @defgroup %s List insertion functions and macros\n"
            "  *  @ingroup %s_FM\n"
            "  */\n",
            group,
            snu
           );

    /* Group *_GET */
    sprintf(group, "%s_FM_GET", snu);
    fprintf(f,
            "/** @defgroup %s List extraction functions and macros\n"
            "  *  @ingroup %s_FM\n"
            "  */\n",
            group,
            snu
           );

    /* Group *_QUERY */
    sprintf(group, "%s_FM_QUERY", snu);
    fprintf(f,
            "/** @defgroup %s List query functions and macros\n"
            "  *  @ingroup %s_FM\n"
            "  */\n",
            group,
            snu
           );


    /* In group *_UTIL */
    sprintf(group, "%s_FM_UTIL", snu);

    /* *_MEMORY_EXCEPTION_HANDLER */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Default exception handler for memory allocation error\n"
            "  * Default behaviour is to assert. If a specific handler\n"
            "  * is needed, this can be set by defining\n"
            "  * USER_MEMORY_EXCEPTION_HANDLER(fn)\n"
            "  * @param fn (in) the name of the function/macro that failed\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_MEMORY_EXCEPTION_HANDLER(fn) \\\n"
            "   assert(false);\n\n",
            snu
           );

    /* *_PARAM_ID */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Obtain the full enum-identifier of a parameter id\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return the enum identifier\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_PARAM_ID(name) \\\n"
            "    %s_paramid_\\\n"
            "       ##name\n\n",
            snu,
            lnl
           );

    /* *_FIELD_INDEX */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Obtain the numeric field index of a bag-field\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of field in the bag\n"
            "  * @return the numeric field id\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_FIELD_INDEX(bagName, fieldName) \\\n"
            "                      %s_ ##bagName ##_%s_ ##fieldName\n\n",
            snu,
            lnl,
            bag_field_table_suffix_p ? bag_field_table_suffix_p->value_p : BAG_FIELD_TABLE_SUFFIX_DEFAULT
           );

    /* *_REMOVE */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Remove (and free) a parameter on a list\n"
            "  * @param list (in/out) The list\n"
            "  * @param name (in) the parameter id\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_REMOVE(list, name) \\\n"
            "    MPL_DESTROY_PARAM_FROM_LIST(%s_PARAM_ID(name), list)\n\n",
            snu,
            snu
           );

    /* *_REMOVE_TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Remove (and free) a parameter on a list\n"
            "  * @param list (in/out) The list\n"
            "  * @param name (in) the parameter id\n"
            "  * @param tag (in) the tag that the parameter must match\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_REMOVE_TAG(list, name, tag) \\\n"
            "    MPL_DESTROY_PARAM_FROM_LIST_TAG(%s_PARAM_ID(name), (tag), (list))\n\n",
            snu,
            snu
           );

    /* *_REMOVE_FIELD */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Remove (and free) a parameter on a list\n"
            "  * @param list (in/out) The list\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of field in the bag\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_REMOVE_FIELD(list, bagName, fieldName) \\\n"
            "    MPL_DESTROY_FIELD_FROM_LIST(%s_paramid_ ##bagName, \\\n"
            "                                %s_FIELD_INDEX(bagName, fieldName),\\\n"
            "                                (list))\n\n",
            snu,
            lnl,
            snu
           );

    /* *_REMOVE_FIELD_TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Remove (and free) a parameter on a list\n"
            "  * @param list (in/out) The list\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of field in the bag\n"
            "  * @param tag (in) the tag that the parameter must match\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_REMOVE_FIELD_TAG(list, bagName, fieldName, tag) \\\n"
            "    MPL_DESTROY_FIELD_FROM_LIST_TAG(%s_paramid_ ##bagName, \\\n"
            "                                %s_FIELD_INDEX(bagName, fieldName),\\\n"
            "                                (tag), (list))\n\n",
            snu,
            lnl,
            snu
           );

    /* ENUM VAR TO STRING PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * get string representation of a enum value\n"
            "  * @param name (in) the enum name\n"
            "  * @param val (in) the enum variable\n"
            "  * @return reference to a statically allocated value string\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ENUM_VAR_TO_STRING_PTR(name,val)        \\\n"
            "mpl_param_value_get_string(%s_PARAM_ID(name),&val)\n",
            snu,
            snu
           );

    /* ENUM TYPE */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Obtain the full name of a enum type\n"
            "  * @param name (in) the name of the enum\n"
            "  * @return the full enum type name\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ENUM_TYPE(name)          \\\n"
            "%s_##name                  \\\n"
            "##_t\n",
            snu,
            lnl
           );

    /* ENUM VAR DECLARE */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * declare a enum-identifier variable\n"
            "  * @param enumType (in) the name of the enum type\n"
            "  * @param name (in) the name of variable\n"
            "  * @return the full name enum-identifier declared variable\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ENUM_VAR_DECLARE(enumType,name)         \\\n"
            "%s_##enumType ##_t         \\\n"
            "name\n",
            snu,
            lnl
           );

    /* ENUM VAR DECLARE INIT */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * declare and initialize a enum-identifier variable\n"
            "  * @param enumType (in) the name of the enum type\n"
            "  * @param name (in) the name of variable\n"
            "  * @param value (in) the value (name) of the enum\n"
            "  * @return the variable full name enum-identifier declared and initialized\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ENUM_VAR_DECLARE_INIT(enumType,name,value)  \\\n"
            "%s_##enumType ##_t name    \\\n"
            "= %s_##enumType ##_ ##value\n",
            snu,
            lnl,
            lnl
           );

    /* ENUM_VALUE */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * obtain the full enum value from a short enum value\n"
            "  * @param enumType (in) the name of the enum type\n"
            "  * @param value (in) the (short) name of the enum value\n"
            "  * @return the full name enum value\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ENUM_VALUE(enumType,value)          \\\n"
            "%s_##enumType              \\\n"
            "##_ ##value\n",
            snu,
            lnl
           );

    /* ENUM_MIN */
    fprintf(f,
            "/**\n"
            "* @ingroup %s\n"
            "* obtain the minimum numeric value of an enum type\n"
            "* @param enumType (in) the name of the enum type\n"
            "* @return the minimum numeric\n"
            "*/\n",
            group
           );
    fprintf(f,
            "#define %s_ENUM_MIN(enumType) \\\n"
            "    %s_min_value_##enumType\n",
            snu,
            lnl
           );

    /* ENUM_MAX */
    fprintf(f,
            "/**\n"
            "* @ingroup %s\n"
            "* obtain the maximum numeric value of an enum type\n"
            "* @param enumType (in) the name of the enum type\n"
            "* @return the maximum numeric value\n"
            "*/\n",
            group
           );
    fprintf(f,
            "#define %s_ENUM_MAX(enumType) \\\n"
            "    %s_max_value_##enumType\n",
            snu,
            lnl
           );

    if (range_id_p) {
        /* NUMBER_RANGE_ID */
        fprintf(f,
                "/**\n"
                "* @ingroup %s\n"
                "* obtain the identity value of a number range\n"
                "* @param rangeName (in) the range name\n"
                "* @return numeric identity\n"
                "*/\n",
                group
               );
        fprintf(f,
                "#define %s_NUMBER_RANGE_ID(rangeName) \\\n"
                "    %s_%s_##rangeName\n",
                snu,
                lnl,
                range_id_p->value_p
               );

        /* NUMBER_RANGE_NAME_PTR */
        fprintf(f,
                "/**\n"
                "* @ingroup %s\n"
                "* obtain the name of a number range (string representation)\n"
                "* @param rangeId (in) the range id value\n"
                "* @return pointer to const string\n"
                "*/\n",
                group
               );
        fprintf(f,
                "#define %s_NUMBER_RANGE_NAME_PTR(rangeId) \\\n"
                "    (((rangeId) >= %s_min_value_%s) && \\\n"
                "     ((rangeId) <= %s_max_value_%s)) ? \\\n"
                "    %s_names_%s[(rangeId)].name_p : \"<ERROR: UNDEFINED RANGE>\"\n",
                snu,
                lnl,
                range_id_p->value_p,
                lnl,
                range_id_p->value_p,
                lnl,
                range_id_p->value_p
               );
    }


    /* *_ADD/GET_<basic type> */

    for (i = 0; basic_mpl_type_lc[i]; i++) {

        /* In group *_ADD */
        sprintf(group, "%s_FM_ADD", snu);

        /* ADD */
        fprintf(f,
                "/**\n"
                "  * @ingroup %s\n"
                "  * Add value or variable of type %s to the command list\n"
                "  *\n"
                "  * Allocates list memory.\n"
                "  *\n"
                "  * @param listPtr (in/out) The list to add to\n"
                "  * @param name (in) the name of the parameter (must be type %s)\n"
                "  * @param src (in) The %s to add (value or variable)\n"
                "  */\n",
                group,
                basic_mpl_type_lc[i],
                basic_mpl_type_lc[i],
                basic_c_type[i]
               );
        fprintf(f,
                "#define %s_ADD_%s(listPtr, name, src) \\\n"
                "    do { \\\n"
                "        int __src = src; \\\n"
                "        assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_%s); \\\n"
                "        if (mpl_param_list_add_%s(listPtr, \\\n"
                "                                  %s_PARAM_ID(name), \\\n"
                "                                  __src) < 0) { \\\n"
                "           if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
                "               %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_list_add_%s\"); \\\n"
                "           } \\\n"
                "       } \\\n"
                "    } while(0)\n",
                snu,
                basic_mpl_type_uc[i],
                snu,
                basic_mpl_type_lc[i],
                basic_mpl_type_lc[i],
                snu,
                snu,
                basic_mpl_type_lc[i]
               );

        /* ADD TAG */
        fprintf(f,
                "/**\n"
                "  * @ingroup %s\n"
                "  * Add value or variable of type %s (with tag) to the command list\n"
                "  *\n"
                "  * Allocates list memory.\n"
                "  *\n"
                "  * @param listPtr (in/out) The list to add to\n"
                "  * @param name (in) the name of the parameter (must be type %s)\n"
                "  * @param src (in) The %s to add (value or variable)\n"
                "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
                "  */\n",
                group,
                basic_mpl_type_lc[i],
                basic_mpl_type_lc[i],
                basic_c_type[i]
               );
        fprintf(f,
                "#define %s_ADD_%s_TAG(listPtr, name, src, tag) \\\n"
                "    do { \\\n"
                "        int __src = src; \\\n"
                "        assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_%s); \\\n"
                "        if (mpl_param_list_add_%s_tag(listPtr, \\\n"
                "                                      %s_PARAM_ID(name), \\\n"
                "                                      tag, \\\n"
                "                                      __src) < 0) { \\\n"
                "           if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
                "               %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_list_add_%s_tag\"); \\\n"
                "           } \\\n"
                "       } \\\n"
                "    } while(0)\n",
                snu,
                basic_mpl_type_uc[i],
                snu,
                basic_mpl_type_lc[i],
                basic_mpl_type_lc[i],
                snu,
                snu,
                basic_mpl_type_lc[i]
               );

        /* In group *_GET */
        sprintf(group, "%s_FM_GET", snu);

        /* GET */
        fprintf(f,
                "/**\n"
                "  * @ingroup %s\n"
                "  * Return the value of a parameter of type %s from a command list\n"
                "  * @param list (in) The list to extract the value from\n"
                "  * @param name (in) the name of the parameter\n"
                "  * @return The %s value\n"
                "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
                "  */\n",
                group,
                basic_mpl_type_lc[i],
                basic_mpl_type_lc[i]
               );

        fprintf(f,
                "#define %s_GET_%s(list, name) \\\n"
                "MPL_GET_PARAM_VALUE_FROM_LIST(%s, \\\n"
                "                              %s_PARAM_ID(name), \\\n"
                "                              list)\n",
                snu,
                basic_mpl_type_uc[i],
                basic_c_type[i],
                snu
               );

        /* GET TAG */
        fprintf(f,
                "/**\n"
                "  * @ingroup %s\n"
                "  * Return the value of a parameter of type %s from a command list\n"
                "  * @param list (in) The list to extract the value from\n"
                "  * @param name (in) the name of the parameter\n"
                "  * @param tag (in) the tag that the parameter must match\n"
                "  * @return The %s value\n"
                "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
                "  */\n",
                group,
                basic_mpl_type_lc[i],
                basic_mpl_type_lc[i]
               );
        fprintf(f,
                "#define %s_GET_%s_TAG(list, name, tag) \\\n"
                "MPL_GET_PARAM_VALUE_FROM_LIST_TAG(%s, \\\n"
                "                                  %s_PARAM_ID(name), \\\n"
                "                                  tag, \\\n"
                "                                  list)\n",
                snu,
                basic_mpl_type_uc[i],
                basic_c_type[i],
                snu
               );

    }

    /* In group *_ADD */
    sprintf(group, "%s_FM_ADD", snu);

    /* ADD BAG FIELD */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a field to a bag\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of the field\n"
            "  * @param var_p pointer to the variable containing the value\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_BAG_FIELD(listPtr, bagName, fieldName, var_p) \\\n"
            "do { \\\n"
            "    mpl_param_element_t *elem_p; \\\n"
            "    elem_p = mpl_param_element_create(mpl_param_get_bag_field_id(%s_paramid_ ##bagName, %s_FIELD_INDEX(bagName, fieldName)), \\\n"
            "                                      var_p); \\\n"
            "    if (elem_p == NULL) { \\\n"
            "        if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_element_create\"); \\\n"
            "        } \\\n"
            "    } \\\n"
            "    MPL_PARAM_ELEMENT_SET_FIELD_INFO(elem_p,%s_paramid_ ##bagName,%s_FIELD_INDEX(bagName, fieldName)); \\\n"
            "    mpl_list_add((listPtr), &elem_p->list_entry); \\\n"
            "} while (0)\n",
            snu,
            lnl,
            snu,
            snu,
            lnl,
            snu
           );

    /* ADD BAG FIELD TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a field to a bag\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of the field\n"
            "  * @param var_p pointer to the variable containing the value\n"
            "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_BAG_FIELD_TAG(listPtr, bagName, fieldName, var_p, tag) \\\n"
            "do { \\\n"
            "    mpl_param_element_t *elem_p; \\\n"
            "    elem_p = mpl_param_element_create_tag(mpl_param_get_bag_field_id(%s_paramid_ ##bagName, %s_FIELD_INDEX(bagName, fieldName)), \\\n"
            "                                          tag, var_p); \\\n"
            "    if (elem_p == NULL) { \\\n"
            "        if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_element_create_tag\"); \\\n"
            "        } \\\n"
            "    } \\\n"
            "    MPL_PARAM_ELEMENT_SET_FIELD_INFO(elem_p,%s_paramid_ ##bagName,%s_FIELD_INDEX(bagName, fieldName)); \\\n"
            "    mpl_list_add((listPtr), &elem_p->list_entry); \\\n"
            "} while (0)\n",
            snu,
            lnl,
            snu,
            snu,
            lnl,
            snu
           );

    /* ADD BAG FIELD CHILD */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a field to a bag using a child parameter of the field\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of the field\n"
            "  * @param childParamId (in) the parameter id of the child\n"
            "  * @param var_p pointer to the variable containing the value\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_BAG_FIELD_CHILD(listPtr, bagName, fieldName, childParamId, var_p) \\\n"
            "do { \\\n"
            "    mpl_param_element_t *elem_p; \\\n"
            "    elem_p = mpl_param_element_create(childParamId, \\\n"
            "                                      var_p); \\\n"
            "    if (elem_p == NULL) { \\\n"
            "        if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_element_create\"); \\\n"
            "        } \\\n"
            "    } \\\n"
            "    MPL_PARAM_ELEMENT_SET_FIELD_INFO(elem_p,%s_paramid_ ##bagName,%s_FIELD_INDEX(bagName, fieldName)); \\\n"
            "    mpl_list_add((listPtr), &elem_p->list_entry); \\\n"
            "} while (0)\n",
            snu,
            snu,
            lnl,
            snu
           );

    /* ADD BAG FIELD CHILD TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a field to a bag using a child parameter of the field\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of the field\n"
            "  * @param childParamId (in) the parameter id of the child\n"
            "  * @param var_p pointer to the variable containing the value\n"
            "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_BAG_FIELD_CHILD_TAG(listPtr, bagName, fieldName, childParamId, var_p, tag) \\\n"
            "do { \\\n"
            "    mpl_param_element_t *elem_p; \\\n"
            "    elem_p = mpl_param_element_create_tag(childParamId, \\\n"
            "                                          tag, var_p); \\\n"
            "    if (elem_p == NULL) { \\\n"
            "        if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_element_create_tag\"); \\\n"
            "        } \\\n"
            "    } \\\n"
            "    MPL_PARAM_ELEMENT_SET_FIELD_INFO(elem_p,%s_paramid_ ##bagName,%s_FIELD_INDEX(bagName, fieldName)); \\\n"
            "    mpl_list_add((listPtr), &elem_p->list_entry); \\\n"
            "} while (0)\n",
            snu,
            snu,
            lnl,
            snu
           );

    /* ADD ENUM FROM VALUE */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an enum to the command list giving the *named* enum value\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter (must be type enum)\n"
            "  * @param src (in) The named enum value to add\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_ENUM_FROM_VALUE(listPtr, name, src) \\\n"
            "    do { \\\n"
            "        assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_enum); \\\n"
            "        if (mpl_param_list_add_enum(listPtr, \\\n"
            "                                    %s_PARAM_ID(name), \\\n"
            "                                    %s_ENUM_VALUE(name,src)) < 0) {\\\n"
            "           if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "               %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_list_add_enum\"); \\\n"
            "           } \\\n"
            "       } \\\n"
            "    } while(0)\n",
            snu,
            snu,
            snu,
            snu,
            snu
           );

    /* ADD ENUM FROM VAR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an enum to the command list giving the *numeric* enum value\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter (must be type enum)\n"
            "  * @param var (in) The numeric enum value to add (value or variable)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_ENUM_FROM_VAR(listPtr, name, var) \\\n"
            "    do { \\\n"
            "        assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_enum); \\\n"
            "        if (mpl_param_list_add_enum(listPtr, \\\n"
            "                                      %s_PARAM_ID(name), \\\n"
            "                                      var) < 0) {\\\n"
            "           if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "               %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_list_add_enum\"); \\\n"
            "           } \\\n"
            "       } \\\n"
            "    } while(0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD ENUM FROM VAR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an enum to the command list giving the *numeric* enum value\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter (must be type enum)\n"
            "  * @param var (in) The numeric enum value to add (value or variable)\n"
            "  * @param tag (in) tag to use\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_ENUM_FROM_VAR_TAG(listPtr, name, var, tag) \\\n"
            "    do { \\\n"
            "        assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_enum); \\\n"
            "        if (mpl_param_list_add_enum_tag(listPtr, \\\n"
            "                                      %s_PARAM_ID(name), \\\n"
            "                                      tag, \\\n"
            "                                      var) < 0) {\\\n"
            "           if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "               %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_list_add_enum\"); \\\n"
            "           } \\\n"
            "       } \\\n"
            "    } while(0)\n",
            snu,
            snu,
            snu,
            snu
           );


    /* ADD FROM STRING */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a parameter to the command list providing a *stringed* value\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param src (in) The parameter value formatted as a string\n"
            "  * @param error_handling (in) Statement(s) to be run on error, typically\n"
            "  * *goto my_error_label* or *break* (if no specific handling).\n"
            "  * @remark This macro will perform format- and range/length check\n"
            "  * of the provided string\n"
            "  */\n",
            group
           );

    fprintf(f,
            "#define %s_ADD_FROM_STRING(listPtr, name, src, error_handling) \\\n"
            "do { \\\n"
            "    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT; \\\n"
            "    options.param_set_id = %s_PARAM_SET_ID; \\\n"
            "    mpl_param_element_t *elem = NULL; \\\n"
            "    if (src != NULL) { \\\n"
            "        if (mpl_param_unpack_internal(#name, \\\n"
            "                                      src, \\\n"
            "                                      &elem, \\\n"
            "                                      &options, \\\n"
            "                                      MPL_PARAM_ID_UNDEFINED) != 0) { \\\n"
            "            if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "                %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_unpack_internal\"); \\\n"
            "            } \\\n"
            "            else \\\n"
            "                error_handling; \\\n"
            "        } \\\n"
            "        if (elem != NULL) \\\n"
            "            mpl_list_add(listPtr, &elem->list_entry); \\\n"
            "        else { \\\n"
            "            if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "                %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_unpack_internal\"); \\\n"
            "            } \\\n"
            "            else \\\n"
            "                error_handling; \\\n"
            "        } \\\n"
            "    } \\\n"
            "    else { \\\n"
            "        error_handling; \\\n"
            "    } \\\n"
            "} while (0) \\\n",
            snu,
            lnu,
            snu,
            snu
           );


    /* ADD BAG FIELD FROM STRING */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a field to the command list providing a *stringed* value\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of the field\n"
            "  * @param src (in) The parameter value formatted as a string\n"
            "  * @param error_handling (in) Statement(s) to be run on error, typically\n"
            "  * *goto my_error_label* or *break* (if no specific handling).\n"
            "  * @remark This macro will perform format- and range/length check\n"
            "  * of the provided string\n"
            "  */\n",
            group
           );

    fprintf(f,
            "#define %s_ADD_BAG_FIELD_FROM_STRING(listPtr, bagName, fieldName, src, error_handling) \\\n"
            "do { \\\n"
            "    mpl_pack_options_t options = MPL_PACK_OPTIONS_DEFAULT; \\\n"
            "    options.param_set_id = %s_PARAM_SET_ID; \\\n"
            "    mpl_param_element_t *elem = NULL; \\\n"
            "    if (src != NULL) { \\\n"
            "        if (mpl_param_unpack_internal(#bagName \"%%\" #fieldName, \\\n"
            "                                      src, \\\n"
            "                                      &elem, \\\n"
            "                                      &options, \\\n"
            "                                      MPL_PARAM_ID_UNDEFINED) != 0) { \\\n"
            "            if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "                %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_unpack_internal\"); \\\n"
            "            } \\\n"
            "            else \\\n"
            "                error_handling; \\\n"
            "        } \\\n"
            "        if (elem != NULL) \\\n"
            "            mpl_list_add(listPtr, &elem->list_entry); \\\n"
            "        else { \\\n"
            "            if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "                %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_unpack_internal\"); \\\n"
            "            } \\\n"
            "            else \\\n"
            "                error_handling; \\\n"
            "        } \\\n"
            "    } \\\n"
            "    else { \\\n"
            "        error_handling; \\\n"
            "    } \\\n"
            "} while (0) \\\n",
            snu,
            lnu,
            snu,
            snu
           );


    /* ADD STRING */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a string type parameter to the command list\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter (must be string type)\n"
            "  * @param src (in) The parameter value formatted as a string\n"
            "  * @remark This macro will NOT perform length check\n"
            "  * of the provided string\n"
            "  */\n",
            group
           );

    fprintf(f,
            "#define %s_ADD_STRING(listPtr, name, src) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_string); \\\n"
            "    if (mpl_add_param_to_list(listPtr, \\\n"
            "                             %s_PARAM_ID(name), \\\n"
            "                             src) < 0) {\\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while(0)\n",
            snu,
            snu,
            snu,
            snu
           );


    /* ADD STRINGN */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a string type parameter to the command list\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter (must be string type)\n"
            "  * @param src (in) The parameter value formatted as a string\n"
            "  * @param len (in) The max string length\n"
            "  * @remark This macro will NOT perform length check\n"
            "  * of the provided string\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_STRINGN(listPtr, name, src, len) \\\n"
            "do { \\\n"
            "    mpl_param_element_t* param_elem_p; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_string); \\\n"
            "    param_elem_p = mpl_param_element_create_stringn \\\n"
            "                            (%s_PARAM_ID(name), src, len); \\\n"
            "    if (param_elem_p == NULL) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_element_create_stringn\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "    mpl_list_add(listPtr, &param_elem_p->list_entry);\\\n"
            "} while(0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD STRING TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a string type parameter to the command list\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter (must be string type)\n"
            "  * @param src (in) The parameter value formatted as a string\n"
            "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
            "  * @remark This macro will NOT perform length check\n"
            "  * of the provided string\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_STRING_TAG(listPtr, name, src, tag) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_string); \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, \\\n"
            "                                 %s_PARAM_ID(name), \\\n"
            "                                 tag, \\\n"
            "                                 src) < 0) {\\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list_tag\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while(0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD BAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a bag type parameter to the command list\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter (must be list)\n"
            "  * @param src (in) The parameter value (mpl_list_t*)\n"
            "  * @note The source list (bag) is copied when adding to the destination list\n"
            "  * (i.e. it is still owned by the caller)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_BAG(listPtr, name, src) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_bag); \\\n"
            "    if (mpl_add_param_to_list(listPtr, \\\n"
            "                             %s_PARAM_ID(name), \\\n"
            "                             src) < 0 ) {\\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while(0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD BAG TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a bag type parameter to the command list\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter (must be list)\n"
            "  * @param src (in) The parameter value (mpl_list_t*)\n"
            "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
            "  * @note The source list is copied when adding to the destination list\n"
            "  * (i.e. it is still owned by the caller)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_BAG_TAG(listPtr, name, src, tag) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_bag); \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, \\\n"
            "                                  %s_PARAM_ID(name), \\\n"
            "                                  tag, \\\n"
            "                                  src) < 0) {\\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list_tag\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while(0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD ADDR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an addr type parameter to the command list\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter (must be addr)\n"
            "  * @param src (in) The parameter value (void *)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_ADDR(listPtr, name, src) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_addr); \\\n"
            "    if (mpl_add_param_to_list(listPtr, \\\n"
            "                             %s_PARAM_ID(name), \\\n"
            "                             &src) < 0) {\\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while(0)\n",
            snu,
            snu,
            snu,
            snu
           );
    /* ADD ADDR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an addr type parameter to the command list\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter (must be addr)\n"
            "  * @param tag (in) The tag (integer 0-99) to mark the parameter with\n"
            "  * @param src (in) The parameter value (void *)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_ADDR_TAG(listPtr, name, src, tag) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_addr); \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, \\\n"
            "                                  %s_PARAM_ID(name), \\\n"
            "                                  tag, \\\n"
            "                                  &src) < 0) {\\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while(0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD CHAR_ARRAY */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an uint8_array type parameter to the command list\n"
            "  * giving a char array and a length. The values (array and\n"
            "  * length) are copied into the parameter.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param src (in) The array to add (array of char)\n"
            "  * @param bytes (in) The number of bytes to copy\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_CHAR_ARRAY(listPtr, name, src, bytes) \\\n"
            "do { \\\n"
            "    mpl_uint8_array_t arr; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint8_array); \\\n"
            "    arr.len = bytes; \\\n"
            "    arr.arr_p = (uint8_t*)src; \\\n"
            "    if (mpl_add_param_to_list(listPtr, %s_PARAM_ID(name), &arr) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD CHAR_ARRAY TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an uint8_array type parameter to the command list\n"
            "  * giving a char array and a length. The values (array and\n"
            "  * length) are copied into the parameter.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param src (in) The array to add (array of char)\n"
            "  * @param bytes (in) The number of bytes to copy\n"
            "  * @param tag (in) Tag to use\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_CHAR_ARRAY_TAG(listPtr, name, src, bytes, tag) \\\n"
            "do { \\\n"
            "    mpl_uint8_array_t arr; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint8_array); \\\n"
            "    arr.len = bytes; \\\n"
            "    arr.arr_p = (uint8_t*)src; \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, %s_PARAM_ID(name), tag, &arr) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD UINT8_ARRAY */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an uint8_array type parameter to the command list\n"
            "  * giving a uint8 array. The values (array and\n"
            "  * length) are copied into the parameter.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param src (in) The array to add (mpl_uint8_array_t*)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_UINT8_ARRAY(listPtr, name, src) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint8_array); \\\n"
            "    if (mpl_add_param_to_list(listPtr, %s_PARAM_ID(name), src) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD UINT8_ARRAY TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an uint8_array type parameter to the command list\n"
            "  * giving a uint8 array. The values (array and\n"
            "  * length) are copied into the parameter.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param src (in) The array to add (mpl_uint8_array_t*)\n"
            "  * @param tag (in) The tag to use\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_UINT8_ARRAY_TAG(listPtr, name, src, tag) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint8_array); \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, %s_PARAM_ID(name), tag, src) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD UINT16_ARRAY */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an uint16_array type parameter to the command list\n"
            "  * giving a uint16 array. The values (array and\n"
            "  * length) are copied into the parameter.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param src (in) The array to add (mpl_uint16_array_t*)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_UINT16_ARRAY(listPtr, name, src) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint16_array); \\\n"
            "    if (mpl_add_param_to_list(listPtr, %s_PARAM_ID(name), src) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD UINT16_ARRAY TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an uint16_array type parameter to the command list\n"
            "  * giving a uint16 array. The values (array and\n"
            "  * length) are copied into the parameter.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param src (in) The array to add (mpl_uint16_array_t*)\n"
            "  * @param tag (in) The tag to use\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_UINT16_ARRAY_TAG(listPtr, name, src, tag) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint16_array); \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, %s_PARAM_ID(name), tag, src) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD UINT32_ARRAY */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an uint32_array type parameter to the command list\n"
            "  * giving a uint32 array. The values (array and\n"
            "  * length) are copied into the parameter.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param src (in) The array to add (mpl_uint32_array_t*)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_UINT32_ARRAY(listPtr, name, src) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint32_array); \\\n"
            "    if (mpl_add_param_to_list(listPtr, %s_PARAM_ID(name), src) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD UINT32_ARRAY TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an uint32_array type parameter to the command list\n"
            "  * giving a uint32 array. The values (array and\n"
            "  * length) are copied into the parameter.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param src (in) The array to add (mpl_uint32_array_t*)\n"
            "  * @param tag (in) The tag to use\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_UINT32_ARRAY_TAG(listPtr, name, src, tag) \\\n"
            "do { \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint32_array); \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, %s_PARAM_ID(name), tag, src) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );


    /* ADD STRING TUPLE */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a string tuple parameter to the command list\n"
            "  * giving key and value (copied into the parameter).\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param key (in) The key (string)\n"
            "  * @param val (in) The value (string)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_STRING_TUPLE(listPtr, name, key, val) \\\n"
            "do { \\\n"
            "    mpl_string_tuple_t tup; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_string_tuple); \\\n"
            "    tup.key_p = key; \\\n"
            "    tup.value_p = val; \\\n"
            "    if (mpl_add_param_to_list(listPtr, %s_PARAM_ID(name), &tup) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD STRING TUPLE TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a string tuple parameter to the command list\n"
            "  * giving key and value (copied into the parameter).\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param key (in) The key (string)\n"
            "  * @param val (in) The value (string)\n"
            "  * @param tag (in) The tag to use\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_STRING_TUPLE_TAG(listPtr, name, key, val, tag) \\\n"
            "do { \\\n"
            "    mpl_string_tuple_t tup; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_string_tuple); \\\n"
            "    tup.key_p = key; \\\n"
            "    tup.value_p = val; \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, %s_PARAM_ID(name), tag, &tup) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD INT TUPLE */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an int tuple parameter to the command list\n"
            "  * giving key and value.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param key (in) The key (int)\n"
            "  * @param val (in) The value (int)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_INT_TUPLE(listPtr, name, key, val) \\\n"
            "do { \\\n"
            "    mpl_int_tuple_t tup; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_int_tuple); \\\n"
            "    tup.key = key; \\\n"
            "    tup.value = val; \\\n"
            "    if (mpl_add_param_to_list(listPtr, %s_PARAM_ID(name), &tup) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD INT TUPLE TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add an int tuple parameter to the command list\n"
            "  * giving key and value.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param key (in) The key (int)\n"
            "  * @param val (in) The value (int)\n"
            "  * @param tag (in) The tag to use\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_INT_TUPLE_TAG(listPtr, name, key, val, tag) \\\n"
            "do { \\\n"
            "    mpl_int_tuple_t tup; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_int_tuple); \\\n"
            "    tup.key = key; \\\n"
            "    tup.value = val; \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, %s_PARAM_ID(name), tag, &tup) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD STRINT TUPLE */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a strint tuple parameter to the command list\n"
            "  * giving key and value.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param key (in) The key (string, copied in)\n"
            "  * @param val (in) The value (int)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_STRINT_TUPLE(listPtr, name, key, val) \\\n"
            "do { \\\n"
            "    mpl_strint_tuple_t tup; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_strint_tuple); \\\n"
            "    tup.key_p = key; \\\n"
            "    tup.value = val; \\\n"
            "    if (mpl_add_param_to_list(listPtr, %s_PARAM_ID(name), &tup) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD STRINT TUPLE TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a strint tuple parameter to the command list\n"
            "  * giving key and value.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param key (in) The key (string, copied in)\n"
            "  * @param val (in) The value (int)\n"
            "  * @param tag (in) The tag to use\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_STRINT_TUPLE_TAG(listPtr, name, key, val, tag) \\\n"
            "do { \\\n"
            "    mpl_strint_tuple_t tup; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_strint_tuple); \\\n"
            "    tup.key_p = key; \\\n"
            "    tup.value = val; \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, %s_PARAM_ID(name), tag, &tup) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD STRUINT8 TUPLE */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a struint8 tuple parameter to the command list\n"
            "  * giving key and value.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param key (in) The key (string, copied in)\n"
            "  * @param val (in) The value (uint8)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_STRUINT8_TUPLE(listPtr, name, key, val) \\\n"
            "do { \\\n"
            "    mpl_struint8_tuple_t tup; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_struint8_tuple); \\\n"
            "    tup.key_p = key; \\\n"
            "    tup.value = val; \\\n"
            "    if (mpl_add_param_to_list(listPtr, %s_PARAM_ID(name), &tup) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* ADD STRUINT8 TUPLE TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Add a struint8 tuple parameter to the command list\n"
            "  * giving key and value.\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param listPtr (in/out) The list to add to\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param key (in) The key (string, copied in)\n"
            "  * @param val (in) The value (uint8)\n"
            "  * @param tag (in) The tag to use\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_ADD_STRUINT8_TUPLE_TAG(listPtr, name, key, val, tag) \\\n"
            "do { \\\n"
            "    mpl_struint8_tuple_t tup; \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_struint8_tuple); \\\n"
            "    tup.key_p = key; \\\n"
            "    tup.value = val; \\\n"
            "    if (mpl_add_param_to_list_tag(listPtr, %s_PARAM_ID(name), tag, &tup) < 0) { \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_add_param_to_list\"); \\\n"
            "       } \\\n"
            "    } \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* CONCAT LIST */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Concatenate a list to the destination list\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param destPtr (in/out) The list to concatenate to\n"
            "  * @param src (in) The source list\n"
            "  * @note The source list is copied when concatenating to the destination list\n"
            "  * (i.e. it is still owned by the caller)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_CONCAT_LIST(destPtr, src) \\\n"
            "do {                                                   \\\n"
            "   mpl_list_t * __elem;                                \\\n"
            "   __elem = mpl_param_list_clone(src);                 \\\n"
            "   if (__elem == NULL) {                                \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_list_clone\"); \\\n"
            "       }                                               \\\n"
            "   }                                                   \\\n"
            "   mpl_list_append(destPtr,__elem);                      \\\n"
            "} while(0)\n",
            snu,
            snu
           );

    /* PARAM CLONE TO LIST */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Extract a parameter from one list and add a copy of it to another\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param dstPtr (in/out) Destination list\n"
            "  * @param src (in) Source list\n"
            "  * @param name (in) the name of the parameter\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_PARAM_CLONE_TO_LIST(dstPtr,src,name)             \\\n"
            "do {                                                     \\\n"
            "    mpl_param_element_t * __par;                         \\\n"
            "    __par = mpl_param_list_find(%s_PARAM_ID(name),src);  \\\n"
            "    if (__par != NULL) {                                 \\\n"
            "        __par = mpl_param_element_clone(__par);          \\\n"
            "        if (__par == NULL)                               \\\n"
            "           if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "               %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_element_clone\"); \\\n"
            "           }                                             \\\n"
            "        if (__par != NULL)                               \\\n"
            "            mpl_list_add(dstPtr,&__par->list_entry);       \\\n"
            "    }                                                    \\\n"
            "} while(0)\n",
            snu,
            snu,
            snu
           );

    /* BAG FIELD CLONE TO LIST */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Extract a field from one bag and add a copy of it to another\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param dstPtr (in/out) Destination list\n"
            "  * @param src (in) Source list\n"
            "  * @param dst_context (in) name of destination bag\n"
            "  * @param dst_field (in) the name of destination field\n"
            "  * @param src_context (in) name of source bag\n"
            "  * @param src_field (in) the name of source field\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_BAG_FIELD_CLONE_TO_LIST(dstPtr,src,dst_context,dst_field,src_context,src_field) \\\n"
            "do {                                                     \\\n"
            "    mpl_param_element_t * __par;                         \\\n"
            "    __par = mpl_param_list_find_field(%s_PARAM_ID(src_context),%s_FIELD_INDEX(src_context,src_field),src); \\\n"
            "    if (__par != NULL) {                                 \\\n"
            "        __par = mpl_param_element_clone(__par);          \\\n"
            "        if (__par == NULL)                               \\\n"
            "           if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "               %s_MEMORY_EXCEPTION_HANDLER(\"mpl_param_element_clone\"); \\\n"
            "           }                                             \\\n"
            "        if (__par != NULL) {                             \\\n"
            "            MPL_PARAM_ELEMENT_SET_FIELD_INFO(__par,%s_PARAM_ID(dst_context),%s_FIELD_INDEX(dst_context,dst_field));\\\n"
            "            mpl_list_add(dstPtr,&__par->list_entry);       \\\n"
            "        }                                                \\\n"
            "    }                                                    \\\n"
            "} while(0)\n",
            snu,
            snu,
            snu,
            snu,
            snu,
            snu
           );



    /* In group *_GET */
    sprintf(group, "%s_FM_GET", snu);

    /* GET ELEMENT PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return pointer to a parameter element from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return The parameter element ref (mpl_param_element_t*) or NULL if not found\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_ELEMENT_PTR(list, name) \\\n"
            "    mpl_param_list_find(%s_paramid_ ##name, \\\n"
            "                        (list))\n",
            snu,
            lnl
           );

    /* GET ELEMENT PTR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return pointer to a parameter element from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the tag that the parameter must match\n"
            "  * @return The parameter element ref (mpl_param_element_t*) or NULL if not found\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_ELEMENT_PTR_TAG(list, name, tag) \\\n"
            "    mpl_param_list_find(%s_paramid_ ##name, \\\n"
            "                        (tag), (list))\n",
            snu,
            lnl
           );

    /* GET ELEMENT ID */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return parameter id of a parameter element from a command list\n"
            "  * (useful if the parameter is an unknown child type)\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return The parameter id (or MPL_PARAM_ID_UNDEFINED of not found)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_ELEMENT_ID(list, name) \\\n"
            "    (mpl_param_list_find(%s_paramid_ ##name, \\\n"
            "                        (list)) ? \\\n"
            "     mpl_param_list_find(%s_paramid_ ##name, \\\n"
            "                        (list))->id : MPL_PARAM_ID_UNDEFINED)\n",
            snu,
            lnl,
            lnl
           );

    /* GET ELEMENT ID TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return parameter id of a parameter element from a command list\n"
            "  * (useful if the parameter is an unknown child type)\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the tag that the parameter must match\n"
            "  * @return The parameter id (or MPL_PARAM_ID_UNDEFINED of not found)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_ELEMENT_ID_TAG(list, name, tag) \\\n"
            "    (mpl_param_list_find(%s_paramid_ ##name, \\\n"
            "                        (tag), (list)) ? \\\n"
            "     mpl_param_list_find(%s_paramid_ ##name, \\\n"
            "                        (tag), (list))->id : MPL_PARAM_ID_UNDEFINED)\n",
            snu,
            lnl,
            lnl
           );

    /* GET BAG FIELD ELEMENT PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return pointer to the parameter element of a bag field parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of the field\n"
            "  * @return The parameter element ref (mpl_param_element_t*) or NULL if not found\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_BAG_FIELD_ELEMENT_PTR(list, bagName, fieldName) \\\n"
            "    mpl_param_list_find_field(%s_paramid_ ##bagName, \\\n"
            "        %s_FIELD_INDEX(bagName, fieldName), \\\n"
            "        (list))\n",
            snu,
            lnl,
            snu
           );

    /* GET BAG FIELD ELEMENT PTR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return pointer to the parameter element of a bag field parameter from a list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of the field\n"
            "  * @param tag (in) the tag that the parameter must match\n"
            "  * @return The parameter element ref (mpl_param_element_t*) or NULL if not found\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_BAG_FIELD_ELEMENT_PTR_TAG(list, bagName, fieldName, tag) \\\n"
            "    mpl_param_list_find_field_tag(%s_paramid_ ##bagName, \\\n"
            "        %s_FIELD_INDEX(bagName, fieldName), \\\n"
            "        tag, \\\n"
            "        (list))\n",
            snu,
            lnl,
            snu
           );

    /* GET BAG FIELD ALL */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return a list of parameters of the same bag field from a command list\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param list (in) The list to extract from\n"
            "  * @param bagName (in) the name of the bag\n"
            "  * @param fieldName (in) the name of the field\n"
            "  * @return Pointer to a list of parameters of the same fielf (a list copy)\n"
            "  * @note The returned list is a copy (must be freed with\n"
            "  * mpl_param_list_destroy())\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_BAG_FIELD_ALL(list, bagName, fieldName) \\\n"
            "    mpl_param_list_find_field_all(%s_paramid_ ##bagName, \\\n"
            "        %s_FIELD_INDEX(bagName, fieldName), \\\n"
            "        (list))\n",
            snu,
            lnl,
            snu
           );

    /* GET ENUM */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return the value of an enum parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return The enum value\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_ENUM(list, name) \\\n"
            "MPL_GET_PARAM_VALUE_FROM_LIST(%s_ENUM_TYPE(name), \\\n"
            "                              %s_PARAM_ID(name), \\\n"
            "                              list)\n",
            snu,
            snu,
            snu
           );

    /* GET ENUM TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return the value of an enum parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the tag to search for\n"
            "  * @return The enum value\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_ENUM_TAG(list, name, tag) \\\n"
            "MPL_GET_PARAM_VALUE_FROM_LIST(%s_ENUM_TYPE(name), \\\n"
            "                              %s_PARAM_ID(name), \\\n"
            "                              tag, \\\n"
            "                              list)\n",
            snu,
            snu,
            snu
           );


    /* GET STRING PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return the value of a string parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return Pointer to the string (char*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned string is NOT a copy (must not be freed or overwritten)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_STRING_PTR(list, name)                             \\\n"
            "MPL_GET_PARAM_VALUE_PTR_FROM_LIST(char*,               \\\n"
            "                                  %s_PARAM_ID(name),  \\\n"
            "                                  list)\n",
            snu,
            snu
           );

    /* GET STRING PTR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return the value of a string parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the parameter must match this tag (an integer)\n"
            "  * @return Pointer to the string (char*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned string is NOT a copy (must not be freed or overwritten)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_STRING_PTR_TAG(list, name, tag)                         \\\n"
            "MPL_GET_PARAM_VALUE_PTR_FROM_LIST_TAG(char*,                \\\n"
            "                                      %s_PARAM_ID(name),   \\\n"
            "                                      tag,                  \\\n"
            "                                      list)\n",
            snu,
            snu
           );

    /* DUP STRING */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Duplicate a stringed representation of a parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param destPtr (out) A copy of the stringed representation of the parameter\n"
            "  * value\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned string must be deallocated using free()\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_DUP_STRING(list, name, destPtr)                  \\\n"
            "do {                                                        \\\n"
            "    void *value;                                            \\\n"
            "                                                            \\\n"
            "    value = MPL_GET_PARAM_VALUE_PTR_FROM_LIST(void*,        \\\n"
            "                                              %s_PARAM_ID(name), \\\n"
            "                                              list);        \\\n"
            "    *destPtr = strdup(mpl_param_value_get_string(%s_PARAM_ID(name), \\\n"
            "                                             value));       \\\n"
            "    if (*destPtr == NULL) {                                     \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"strdup\");         \\\n"
            "       }                                                    \\\n"
            "    }                                                       \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* DUP STRING TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Duplicate a stringed representation of a parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the tag that the parameter must match\n"
            "  * @param destPtr (out) A copy of the stringed representation of the parameter\n"
            "  * value\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned string must be deallocated using free()\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_DUP_STRING_TAG(list, name, tag, destPtr)                       \\\n"
            "do {                                                        \\\n"
            "    void *value;                                            \\\n"
            "                                                            \\\n"
            "    value = MPL_GET_PARAM_VALUE_PTR_FROM_LIST_TAG(void*,    \\\n"
            "                                                  %s_PARAM_ID(name), \\\n"
            "                                                  tag,      \\\n"
            "                                                  list);    \\\n"
            "    *destPtr = strdup(mpl_param_value_get_string(%s_PARAM_ID(name), \\\n"
            "                                             value));       \\\n"
            "    if (*destPtr == NULL) {                                     \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"strdup\");         \\\n"
            "       }                                                    \\\n"
            "    }                                                       \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* DUP BAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Duplicate the value of a bag parameter (compound type) from a command list\n"
            "  * and deliver as a pointer to list (a copy).\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return A copy of the bag (mpl_list_t*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned list is a copy (must be freed with\n"
            "  * mpl_param_list_destroy())\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_DUP_BAG(list, name)                        \\\n"
            "    mpl_param_list_clone(%s_GET_BAG_PTR(list, \\\n"
            "                                      name))\n",
            snu,
            snu
           );

    /* GET ADDR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * @ingroup RCL_GET\n"
            "  * Return the value of a addr parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return The addr value (pointer)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_ADDR(list, name) \\\n"
            "MPL_GET_PARAM_VALUE_FROM_LIST(void*, \\\n"
            "                              %s_PARAM_ID(name), \\\n"
            "                              list)\n",
            snu,
            snu
           );

    /* GET ADDR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * @ingroup RCL_GET\n"
            "  * Return the value of a addr parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the parameter must match this tag (an integer)\n"
            "  * @return The addr value (pointer)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_ADDR_TAG(list, name, tag) \\\n"
            "MPL_GET_PARAM_VALUE_FROM_LIST_TAG(void*, \\\n"
            "                                  %s_PARAM_ID(name), \\\n"
            "                                  tag,                  \\\n"
            "                                  list)\n",
            snu,
            snu
           );

    /* GET CHAR_ARRAY PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of an uint8_array type parameter from a command list\n"
            "  * and deliver as a pointer to char array (*not* a copy).\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return reference to the array (array of char)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_CHAR_ARRAY_PTR(list, name) \\\n"
            "  ((mpl_uint8_array_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST \\\n"
            "   (mpl_uint8_array_t*, %s_PARAM_ID(name), list))->arr_p\n",
            snu,
            snu
           );

    /* GET CHAR_ARRAY LENGTH */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the length of an uint8_array type parameter from a command list\n"
            "  * (number of bytes in the array)\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return length of the array (int)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_CHAR_ARRAY_LENGTH(list, name)                     \\\n"
            "  ((mpl_uint8_array_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST \\\n"
            "   (mpl_uint8_array_t*, RCL_PARAM_ID(name), list))->len\n",
            snu,
            snu
           );

    /* DUP CHAR ARRAY */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Duplicate the value of an uint8_array type parameter from a command list\n"
            "  * and deliver as a pointer to char array (a copy).\n"
            "  * @param list (in) The list to extract- the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param destPtr (out) A copy of the array value (array of char)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned array must be deallocated using free()\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_DUP_CHAR_ARRAY(list, name, destPtr)                     \\\n"
            "do {                                                        \\\n"
            "    mpl_uint8_array_t *arr;                                 \\\n"
            "                                                           \\\n"
            "    arr = MPL_GET_PARAM_VALUE_PTR_FROM_LIST(mpl_uint8_array_t*, \\\n"
            "                                            %s_PARAM_ID(name), \\\n"
            "                                            list);          \\\n"
            "    *destPtr = malloc(arr->len);                                \\\n"
            "    if (*destPtr == NULL) {                                     \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"malloc\");         \\\n"
            "       }                                                    \\\n"
            "    }                                                       \\\n"
            "    memcpy(*destPtr, arr->arr_p, arr->len);                     \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu
           );

    /* DUP CHAR ARRAY N */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Duplicate the value of an uint8_array type parameter from a command list\n"
            "  * and deliver as a pointer to char array (a copy).\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param destPtr (out) A copy of the array value (array of char)\n"
            "  * @param bytes (in) The number of bytes to copy\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned array must be deallocated using free()\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_DUP_CHAR_ARRAY_N(list, name, destPtr, bytes)                     \\\n"
            "do {                                                        \\\n"
            "   mpl_uint8_array_t *arr;                                 \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint8_array); \\\n"
            "                                                           \\\n"
            "    arr = MPL_GET_PARAM_VALUE_PTR_FROM_LIST(mpl_uint8_array_t*, \\\n"
            "                                            %s_PARAM_ID(name), \\\n"
            "                                            list);          \\\n"
            "    assert(bytes == arr->len);                              \\\n"
            "    *destPtr = malloc(bytes);                                   \\\n"
            "    if (*destPtr == NULL) {                                     \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"malloc\");         \\\n"
            "       }                                                    \\\n"
            "    }                                                       \\\n"
            "    memcpy(*destPtr, arr->arr_p, bytes);                        \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* GET UINT8_ARRAY PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of an uint8_array type parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return reference to the array (mpl_uint8_array_t*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_UINT8_ARRAY_PTR(list, name) \\\n"
            "  ((mpl_uint8_array_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST \\\n"
            "   (mpl_uint8_array_t*, %s_PARAM_ID(name), list))\n",
            snu,
            snu
           );

    /* GET UINT8_ARRAY PTR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of an uint8_array type parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) tag to use \n"
            "  * @return reference to the array (mpl_uint8_array_t*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_UINT8_ARRAY_PTR_TAG(list, name, tag) \\\n"
            "  ((mpl_uint8_array_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST_TAG \\\n"
            "   (mpl_uint8_array_t*, %s_PARAM_ID(name), tag, list))\n",
            snu,
            snu
           );

    /* GET UINT16_ARRAY PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of an uint16_array type parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return reference to the array (mpl_uint16_array_t*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_UINT16_ARRAY_PTR(list, name) \\\n"
            "  ((mpl_uint16_array_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST \\\n"
            "   (mpl_uint16_array_t*, %s_PARAM_ID(name), list))\n",
            snu,
            snu
           );

    /* GET UINT16_ARRAY PTR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of an uint16_array type parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) tag to use \n"
            "  * @return reference to the array (mpl_uint16_array_t*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_UINT16_ARRAY_PTR_TAG(list, name, tag) \\\n"
            "  ((mpl_uint16_array_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST_TAG \\\n"
            "   (mpl_uint16_array_t*, %s_PARAM_ID(name), tag, list))\n",
            snu,
            snu
           );

    /* DUP UINT16 ARRAY */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Duplicate the value of an uint16_array type parameter from a command list\n"
            "  * and deliver as a pointer to uint16 array (a copy).\n"
            "  * @param list (in) The list to extract- the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param destPtr (out) A copy of the array value (array of uint16)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned array must be deallocated using free()\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_DUP_UINT16_ARRAY(list, name, destPtr)                     \\\n"
            "do {                                                        \\\n"
            "    mpl_uint16_array_t *arr;                                 \\\n"
            "                                                           \\\n"
            "    arr = MPL_GET_PARAM_VALUE_PTR_FROM_LIST(mpl_uint16_array_t*, \\\n"
            "                                            %s_PARAM_ID(name), \\\n"
            "                                            list);          \\\n"
            "    *destPtr = malloc(arr->len * sizeof(uint16_t));         \\\n"
            "    if (*destPtr == NULL) {                                     \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"malloc\");         \\\n"
            "       }                                                    \\\n"
            "    }                                                       \\\n"
            "    memcpy(*destPtr, arr->arr_p, arr->len * sizeof(uint16_t));  \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu
           );

    /* DUP UINT16 ARRAY N */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Duplicate the value of an uint16_array type parameter from a command list\n"
            "  * and deliver as a pointer to uint16 array (a copy).\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param destPtr (out) A copy of the array value (array of uint16)\n"
            "  * @param bytes (in) The number of bytes to copy\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned array must be deallocated using free()\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_DUP_UINT16_ARRAY_N(list, name, destPtr, bytes)                     \\\n"
            "do {                                                        \\\n"
            "   mpl_uint16_array_t *arr;                                 \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint16_array); \\\n"
            "                                                           \\\n"
            "    arr = MPL_GET_PARAM_VALUE_PTR_FROM_LIST(mpl_uint16_array_t*, \\\n"
            "                                            %s_PARAM_ID(name), \\\n"
            "                                            list);          \\\n"
            "    assert(bytes == arr->len);                              \\\n"
            "    *destPtr = malloc(bytes * sizeof(uint16_t));            \\\n"
            "    if (*destPtr == NULL) {                                     \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"malloc\");         \\\n"
            "       }                                                    \\\n"
            "    }                                                       \\\n"
            "    memcpy(*destPtr, arr->arr_p, bytes * sizeof(uint16_t)); \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* GET UINT32_ARRAY PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of an uint32_array type parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return reference to the array (mpl_uint32_array_t*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_UINT32_ARRAY_PTR(list, name) \\\n"
            "  ((mpl_uint32_array_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST \\\n"
            "   (mpl_uint32_array_t*, %s_PARAM_ID(name), list))\n",
            snu,
            snu
           );

    /* GET UINT32_ARRAY PTR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of an uint32_array type parameter from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) tag to use \n"
            "  * @return reference to the array (mpl_uint32_array_t*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_UINT32_ARRAY_PTR_TAG(list, name, tag) \\\n"
            "  ((mpl_uint32_array_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST_TAG \\\n"
            "   (mpl_uint32_array_t*, %s_PARAM_ID(name), tag, list))\n",
            snu,
            snu
           );

    /* DUP UINT32 ARRAY */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Duplicate the value of an uint32_array type parameter from a command list\n"
            "  * and deliver as a pointer to uint32 array (a copy).\n"
            "  * @param list (in) The list to extract- the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param destPtr (out) A copy of the array value (array of uint32)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned array must be deallocated using free()\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_DUP_UINT32_ARRAY(list, name, destPtr)                     \\\n"
            "do {                                                        \\\n"
            "    mpl_uint32_array_t *arr;                                 \\\n"
            "                                                           \\\n"
            "    arr = MPL_GET_PARAM_VALUE_PTR_FROM_LIST(mpl_uint32_array_t*, \\\n"
            "                                            %s_PARAM_ID(name), \\\n"
            "                                            list);          \\\n"
            "    *destPtr = malloc(arr->len * sizeof(uint32_t));         \\\n"
            "    if (*destPtr == NULL) {                                     \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"malloc\");         \\\n"
            "       }                                                    \\\n"
            "    }                                                       \\\n"
            "    memcpy(*destPtr, arr->arr_p, arr->len * sizeof(uint32_t));  \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu
           );

    /* DUP UINT32 ARRAY N */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Duplicate the value of an uint32_array type parameter from a command list\n"
            "  * and deliver as a pointer to uint32 array (a copy).\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param destPtr (out) A copy of the array value (array of uint32)\n"
            "  * @param bytes (in) The number of bytes to copy\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned array must be deallocated using free()\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_DUP_UINT32_ARRAY_N(list, name, destPtr, bytes)                     \\\n"
            "do {                                                        \\\n"
            "   mpl_uint32_array_t *arr;                                 \\\n"
            "    assert(mpl_param_id_get_type(%s_PARAM_ID(name)) == mpl_type_uint32_array); \\\n"
            "                                                           \\\n"
            "    arr = MPL_GET_PARAM_VALUE_PTR_FROM_LIST(mpl_uint32_array_t*, \\\n"
            "                                            %s_PARAM_ID(name), \\\n"
            "                                            list);          \\\n"
            "    assert(bytes == arr->len);                              \\\n"
            "    *destPtr = malloc(bytes * sizeof(uint32_t));            \\\n"
            "    if (*destPtr == NULL) {                                     \\\n"
            "       if (mpl_get_errno() == E_MPL_FAILED_ALLOCATING_MEMORY) { \\\n"
            "           %s_MEMORY_EXCEPTION_HANDLER(\"malloc\");         \\\n"
            "       }                                                    \\\n"
            "    }                                                       \\\n"
            "    memcpy(*destPtr, arr->arr_p, bytes * sizeof(uint32_t)); \\\n"
            "} while (0)\n",
            snu,
            snu,
            snu,
            snu
           );

    /* GET STRING TUPLE PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of a string tuple type parameter from a command list\n"
            "  * and deliver as a pointer to mpl_string_tuple_t (*not* a copy).\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return reference to the tuple\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_STRING_TUPLE_PTR(list, name) \\\n"
            "  (mpl_string_tuple_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST \\\n"
            "   (mpl_string_tuple_t*, %s_PARAM_ID(name), list)\n",
            snu,
            snu
           );

    /* GET INT TUPLE PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of an int tuple type parameter from a command list\n"
            "  * and deliver as a pointer to mpl_int_tuple_t (*not* a copy).\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return reference to the tuple\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_INT_TUPLE_PTR(list, name) \\\n"
            "  (mpl_int_tuple_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST \\\n"
            "   (mpl_int_tuple_t*, %s_PARAM_ID(name), list)\n",
            snu,
            snu
           );

    /* GET STRINT TUPLE PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of a strint tuple type parameter from a command list\n"
            "  * and deliver as a pointer to mpl_strint_tuple_t (*not* a copy).\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return reference to the tuple\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_STRINT_TUPLE_PTR(list, name) \\\n"
            "  (mpl_strint_tuple_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST \\\n"
            "   (mpl_strint_tuple_t*, %s_PARAM_ID(name), list)\n",
            snu,
            snu
           );

    /* GET STRUINT8 TUPLE PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of a struint8 tuple type parameter from a command list\n"
            "  * and deliver as a pointer to mpl_struint8_tuple_t (*not* a copy).\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return reference to the tuple\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_STRUINT8_TUPLE_PTR(list, name) \\\n"
            "  (mpl_struint8_tuple_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST \\\n"
            "   (mpl_struint8_tuple_t*, %s_PARAM_ID(name), list)\n",
            snu,
            snu
           );

    /* GET STRUINT8 TUPLE PTR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Get the reference of a struint8 tuple type parameter from a command list\n"
            "  * and deliver as a pointer to mpl_struint8_tuple_t (*not* a copy).\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the parameter must match this tag (an integer)\n"
            "  * @return reference to the tuple\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_STRUINT8_TUPLE_PTR_TAG(list, name, tag) \\\n"
            "  (mpl_struint8_tuple_t *)MPL_GET_PARAM_VALUE_PTR_FROM_LIST_TAG \\\n"
            "   (mpl_struint8_tuple_t*, %s_PARAM_ID(name), tag, list)\n",
            snu,
            snu
           );

    /* GET BAG PTR */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return the value of a bag parameter (compound type) from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return The list value (mpl_list_t*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned list still belongs to the original list (must not be\n"
            "  * changed).\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_BAG_PTR(list, name)                                    \\\n"
            "MPL_GET_PARAM_VALUE_PTR_FROM_LIST(mpl_list_t*,              \\\n"
            "                                  %s_PARAM_ID(name),       \\\n"
            "                                  list)\n",
            snu,
            snu
           );

    /* GET BAG PTR TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return the value of a bag parameter (compound type) from a command list\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the parameter must match this tag (an integer)\n"
            "  * @return The list value (mpl_list_t*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_BAG_PTR_TAG(list, name, tag)                           \\\n"
            "MPL_GET_PARAM_VALUE_PTR_FROM_LIST_TAG(mpl_list_t*,          \\\n"
            "                                      %s_PARAM_ID(name),   \\\n"
            "                                      tag,                  \\\n"
            "                                      list)\n",
            snu,
            snu
           );

    /* DUP BAG TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Duplicate the value of a bag parameter (compound type) from a command list\n"
            "  * and deliver as a pointer to list (a copy).\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param list (in) The list to extract the value from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the parameter must match this tag (an integer)\n"
            "  * @return A copy of the list (mpl_list_t*)\n"
            "  * @note The parameter must exist on the list (otherwise the call may crash)\n"
            "  * @note The returned list is a copy (must be freed with\n"
            "  * mpl_param_list_destroy())\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_DUP_BAG_TAG(list, name, tag)                   \\\n"
            "mpl_param_list_clone(%s_GET_BAG_PTR(list, \\\n"
            "                                      name, \\\n"
            "                                      tag))\n",
            snu,
            snu
           );

    /* GET ALL */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return a list of parameters of the same name from a command list\n"
            "  *\n"
            "  * Allocates list memory.\n"
            "  *\n"
            "  * @param list (in) The list to extract from\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return Pointer to a list of parameters of the same name (a list copy)\n"
            "  * @note The returned list is a copy (must be freed with\n"
            "  * mpl_param_list_destroy())\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_GET_ALL(list, name)                                 \\\n"
            "mpl_param_list_find_all(%s_PARAM_ID(name), \\\n"
            "                        list)\n",
            snu,
            snu
           );

    /* In group *_QUERY */
    sprintf(group, "%s_FM_QUERY", snu);

    /* EXISTS */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Check if a particular parameter is present in a list\n"
            "  * (command, response or event list), and that the value\n"
            "  * is set (not NULL).\n"
            "  * @param list (in) The list to search\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return boolean (true if a parameter of the named type is found\n"
            "  * on the list)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_EXISTS(list, name) \\\n"
            "(MPL_PARAM_PRESENT_IN_LIST(%s_PARAM_ID(name), \\\n"
            "                           list) && \\\n"
            " MPL_PARAM_VALUE_PRESENT_IN_LIST(%s_PARAM_ID(name), \\\n"
            "                                 list))\n",
            snu,
            snu,
            snu
           );

    /* EXISTS TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Check if a particular parameter *with a specified tag* is present in a list\n"
            "  * (command, response or event list)\n"
            "  * @param list (in) The list to search\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the tag that the parameter must match\n"
            "  * @return boolean (true if a parameter of the named type is found\n"
            "  * on the list)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_EXISTS_TAG(list, name, tag) \\\n"
            "MPL_PARAM_PRESENT_IN_LIST_TAG(%s_PARAM_ID(name), \\\n"
            "                              tag, \\\n"
            "                              list)\n",
            snu,
            snu
           );

    /* PARAM COUNT */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return number of parameters in a list\n"
            "  * @param list (in) The list to search\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @return Number of parameters\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_PARAM_COUNT(list, name) \\\n"
            "mpl_param_list_param_count(%s_PARAM_ID(name), \\\n"
            "                           list)\n",
            snu,
            snu
           );

    /* PARAM COUNT TAG */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Return number of parameters in a list\n"
            "  * @param list (in) The list to search\n"
            "  * @param name (in) the name of the parameter\n"
            "  * @param tag (in) the tag that the parameter must match\n"
            "  * @return Number of parameters\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_PARAM_COUNT_TAG(list, name, tag) \\\n"
            "mpl_param_list_param_count_tag(%s_PARAM_ID(name), \\\n"
            "                               tag, list)\n",
            snu,
            snu
           );
    /* IS_RESPONSE */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Check if a particular message list is a response.\n"
            "  * @param list (in) The list to search\n"
            "  * @return boolean (true if the message list is a response)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_IS_RESPONSE(list) \\\n"
            " MPL_PARAM_PRESENT_IN_LIST(%s_PARAM_ID(RESP), list)\n",
            snu,
            snu
           );
    /* IS_EVENT */
    fprintf(f,
            "/**\n"
            "  * @ingroup %s\n"
            "  * Check if a particular message list is a event.\n"
            "  * @param list (in) The list to search\n"
            "  * @return boolean (true if the message list is a event)\n"
            "  */\n",
            group
           );
    fprintf(f,
            "#define %s_IS_EVENT(list) \\\n"
            " MPL_PARAM_PRESENT_IN_LIST(%s_PARAM_ID(EVENT), list)\n",
            snu,
            snu
           );
    free(lnu);
    free(snu);
}

void parameter_set::gc_h_bags(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;
    int bags_found = 0;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            if (parameter_p->is_bag()) {
                if (!bags_found) {
                    fprintf(f,
                            "\n"
                            "/** Routines to check that contents of a bag is sane (mandatory parameters\n"
                            "  * present, arrays have correct tags etc).\n"
                            "  */\n"
                           );
                    fprintf(f,
                            "int %s_checkAllowedParameters(int *errors_p, mpl_list_t **result_list_pp, mpl_list_t *allowed_parameters_p, mpl_bag_t *bag_p);\n",
                            name_p
                           );
                    bags_found = 1;
                }
                ((bag_parameter*)parameter_p)->gc_h_bag(f);
            }
        }
    }
}

void parameter_set::cli_h(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;
    int bags_found = 0;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            if (parameter_p->is_bag()) {
                if (compiler_p->bag_is_command((bag_parameter*)parameter_p) ||
                    compiler_p->param_is_command_parameter(parameter_p))
                    ((bag_parameter*)parameter_p)->cli_h_completions(f);
            }
            if (parameter_p->is_enum()) {
                if (compiler_p->param_is_command_parameter(parameter_p))
                    ((enum_parameter*)parameter_p)->cli_h_completions(f);
            }
        }
    }
}

void parameter_set::gc_h_functions(FILE *f)
{
    char *short_name_p = get_short_name();
    if (short_name_p) {
        char *snu = str_toupper(short_name_p);
        char group[100];

        /* Group *_INIT */
        sprintf(group, "%s_FM_INIT", snu);
        fprintf(f,
                "/** @defgroup %s Initialization functions and macros\n"
                "  *  @ingroup %s_FM\n"
                "  */\n",
                group,
                snu
               );

        fprintf(f,
                "/**\n"
                "  * @ingroup %s_FM_INIT\n"
                "  * Initialize the MPL parameter set \"%s\". Must be called\n"
                "  * (once) before any operations on %s parameters.\n"
                "  * @return 0 on success\n"
                "  */\n",
                snu,
                name_p,
                name_p
               );
        free(snu);
    }

    fprintf(f,
            "int %s_param_init(void);\n\n",
            name_p
           );
}


void parameter_set::gc_h(FILE *f)
{
    char *short_name_p;

    if (prefix_p == NULL) {
        fprintf(stderr,
                "%s:%d: Parameter set '%s' has no prefix defined.\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                name_p
               );
        exit(-1);
    }

    if (paramset_id_p == NULL) {
        fprintf(stderr,
                "%s:%d: Parameter set '%s' has no numeric id defined.\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                name_p
               );
        exit(-1);
    }

    short_name_p = get_short_name();

    if (compiler_p->codegen_mode == codegen_mode_cli) {
        cli_h(f);
        return;
    }
    else if (compiler_p->codegen_mode == codegen_mode_api) {
        api_hh(f, (char *)"");
        return;
    }

    fprintf(f,
            "/* Parameter set %s */\n\n",
            name_p);

    char *un_p = str_toupper(name_p);
    fprintf(f,
            "#define %s_PARAM_SET_ID %d\n\n",
            un_p,
            atoi(paramset_id_p->value_p)
           );
    fprintf(f,
            "#define %s_PARAMID_PREFIX \"%s\"\n\n",
            un_p,
            prefix_p->value_p
           );
    fprintf(f, "#define %s_INVALID_CATEGORY -1\n", un_p);
    fprintf(f, "#define %s_MEMORY_ERROR -2\n\n", un_p);
    gc_h_enumerator_lists(f);
    gc_h_enums(f);
    gc_h_paramids(f);
    gc_h_functions(f);
    gc_h_macros(f);
    if (!short_name_p || !strlen(short_name_p)) {
        fprintf(f,
                "/* No short_name defined -> no bag-checking code generated */\n"
               );
    } else {
        gc_h_bags(f);
    }

    fprintf(f, "\n");
    fprintf(f,
            "/* End parameter set %s */\n\n",
            name_p);
    free(un_p);
}

void parameter_set::gc_c_enums(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            if (parameter_p->is_enum()) {
                ((enum_parameter*)parameter_p)->gc_c_enum(f, name_p);
            }
        }
    }
}

void parameter_set::gc_c_ranges(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            parameter_p->gc_c_ranges(f, name_p);
        }
    }
}

void parameter_set::gc_c_field_values(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            parameter_p->gc_c_field_values(f, name_p);
        }
    }
}

void parameter_set::gc_c_children(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            parameter_p->gc_c_children(f, name_p);
        }
    }
}

void parameter_set::gc_c_extra(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;
    int has_only_enums = 1;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            if (get_type_of_enum((const char *) parameter_p->get_property("type")) == 0) {
                has_only_enums = 0;
                break;
            }
        }
    }

    fprintf(f,
            "/* Extra magic */\n"
           );
    if (!has_only_enums) {
        fprintf(f,
                "static const mpl_enum_value_t %s_names_no_enum[] = {{\"\", 0}};\n",
                name_p
               );
        fprintf(f,
                "#define %s_enum_size_no_enum 0\n",
                name_p
               );
        fprintf(f,
                "#define %s_no_enum_t\n\n",
                name_p
               );
        fprintf(f,
                "#define %s_min_value_no_enum 0\n",
                name_p
               );
    }
}

void parameter_set::gc_c_defaults(FILE *f)
{
    char *psl = name_p;
    char *psu = str_toupper(psl);

    fprintf(f,
            "/* Default values */\n"
           );
    fprintf(f,
            "#define %s_PARAM_ID_ELEM(ELEMENT, TYPE, EXTRA, MIN, MAX, SET, GET, CONFIG, DEFAULT, ID) \\\n",
            psu
           );
    fprintf(f,
            "const mpl_paramset_pre_##TYPE %s_ ##EXTRA ##_t \\\n",
            psl
           );
    fprintf(f,
            "  %s_default_##ELEMENT mpl_paramset_post_##TYPE = \\\n",
            psl
           );
    fprintf(f,
            "  mpl_paramset_left_cast_paren_ ##TYPE %s_ ##EXTRA ##_t mpl_paramset_right_cast_paren_ ##TYPE mpl_paramset_set_##TYPE ##_ ##DEFAULT; \\\n",
            psl
           );
    fprintf(f,
            "const mpl_paramset_##TYPE ##_min_max_type %s_min_##ELEMENT = mpl_paramset_set_##MIN;\\\n",
            psl
           );
    fprintf(f,
            "const mpl_paramset_##TYPE ##_min_max_type %s_max_##ELEMENT = mpl_paramset_set_##MAX;\\\n",
            psl
           );
    fprintf(f,
            "\n  %s_PARAMETER_IDS\n",
            psu
           );
    fprintf(f,
            "#undef %s_PARAM_ID_ELEM\n",
            psu
           );
    fprintf(f,
            "\n"
           );
    free(psu);
}

void parameter_set::gc_c_param_descr(FILE *f)
{
    char *psl = name_p;
    char *psu = str_toupper(psl);

    fprintf(f,
            "/* Param descr */\n"
           );
    fprintf(f,
            "#define %s_PARAM_ID_ELEM(ELEMENT, TYPE, EXTRA, MIN, MAX, SET, GET, CONFIG, DEFAULT, ID) \\\n",
            psu
           );
    fprintf(f,
           "{ #ELEMENT, \\\n"
           "  mpl_type_##TYPE, \\\n"
           "  (SET), \\\n"
           "  (GET), \\\n"
           "  (CONFIG), \\\n"
          );
    fprintf(f,
            "  (mpl_paramset_is_##DEFAULT)?&%s_default_##ELEMENT : NULL, \\\n",
            psl
           );
    fprintf(f,
            "  NULL, \\\n", /* Deprecated max_p */
            psl
           );
    fprintf(f,
            "  mpl_pack_param_value_##TYPE, \\\n"
            "  mpl_unpack_param_value_##TYPE, \\\n"
            "  mpl_clone_param_value_##TYPE, \\\n"
            "  mpl_copy_param_value_##TYPE, \\\n"
            "  mpl_compare_param_value_##TYPE, \\\n"
            "  mpl_sizeof_param_value_##TYPE, \\\n"
            "  mpl_free_param_value_##TYPE, \\\n"
           );
    fprintf(f,
            "  NULL, \\\n", /* Deprecated stringarr */
            psl
           );
    fprintf(f,
            "  0 \\\n" /* Deprecated stringarr_size */
            "},\n",
            psl
           );
    fprintf(f,
            "\n"
           );
    fprintf(f,
            "static const mpl_param_descr_t %s_param_descr[] =\n"
            "{\n",
            psl
           );
    fprintf(f,
            "  %s_PARAMETER_IDS\n"
            "};\n",
            psu
           );
    fprintf(f,
            "#undef %s_PARAM_ID_ELEM\n",
            psu
           );
    fprintf(f,
            "\n"
           );


    fprintf(f,
            "/* Param descr 2 */\n"
           );
    fprintf(f,
            "#define %s_PARAM_ID_ELEM(ELEMENT, TYPE, EXTRA, MIN, MAX, SET, GET, CONFIG, DEFAULT, ID) \\\n",
            psu
           );
    fprintf(f,
            "{\\\n"
           );
    fprintf(f,
            "  MPL_PARAMID_IS_VIRTUAL(ID), \\\n"
           );
    fprintf(f,
            "  (mpl_paramset_is_##MIN)?&%s_min_##ELEMENT : NULL, \\\n",
            psl
           );
    fprintf(f,
            "  (mpl_paramset_is_##MAX)?&%s_max_##ELEMENT : NULL, \\\n",
            psl
           );
    fprintf(f,
            "  %s_names_##EXTRA, \\\n",
            psl
           );
    fprintf(f,
            "  %s_enum_size_##EXTRA, \\\n",
            psl
           );
    fprintf(f,
            "  sizeof mpl_paramset_left_sizeof_paren_ ##TYPE %s_ ##EXTRA ##_t mpl_paramset_right_sizeof_paren_ ##TYPE, \\\n",
            psl
           );
    fprintf(f,
            "  (%s_min_value_ ##EXTRA < 0), \\\n",
            psl
           );
    fprintf(f,
            "  %s_ranges_##ELEMENT, \\\n",
            psl
           );
    fprintf(f,
            "  %s_range_size_##ELEMENT, \\\n",
            psl
           );
    fprintf(f,
            "  %s_field_values_##ELEMENT, \\\n",
            psl
           );
    fprintf(f,
            "  %s_field_values_size_##ELEMENT, \\\n",
            psl
           );
    fprintf(f,
            "  %s_children_##ELEMENT, \\\n",
            psl
           );
    fprintf(f,
            "  %s_children_size_##ELEMENT \\\n",
            psl
           );
    fprintf(f,
            "},\n"
           );
    fprintf(f,
            "\n"
           );
    fprintf(f,
            "static const mpl_param_descr2_t %s_param_descr2[] =\n"
            "{\n",
            psl
           );
    fprintf(f,
            "  %s_PARAMETER_IDS\n"
            "};\n",
            psu
           );
    fprintf(f,
            "#undef %s_PARAM_ID_ELEM\n",
            psu
           );
    fprintf(f,
            "\n"
           );



    fprintf(f,
            "MPL_DEFINE_PARAM_DESCR_SET2(%s, %s);\n",
            psl,
            psu
           );
    fprintf(f,
            "\n"
           );
    free(psu);
}

void parameter_set::gc_c_param_init(FILE *f)
{
    fprintf(f,
            "/* Param init */\n"
           );
    fprintf(f,
            "int %s_param_init(void)\n"
            "{\n",
            name_p
           );
    fprintf(f,
            "  return mpl_param_init(&%s_param_descr_set);\n"
            "}\n",
            name_p
           );
    fprintf(f,
            "\n"
           );
}

void parameter_set::gc_c_bags(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;
    int bags_found = 0;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            if (parameter_p->is_bag()) {
                if (!bags_found) {
                    fprintf(f,
                            "/* Bag-check routines */\n"
                           );
                    fprintf(f,
                            "int %s_checkAllowedParameters(int *errors_p, mpl_list_t **result_list_pp, mpl_list_t *allowed_parameters_p, mpl_bag_t *bag_p)\n"
                            "{\n"
                            "    mpl_list_t *tmp_p;\n"
                            "    MPL_LIST_FOR_EACH(bag_p, tmp_p) {\n"
                            "        mpl_param_element_t *elem_p;\n"
                            "        mpl_param_element_t *elem_found_p;\n"
                            "        elem_p = MPL_LIST_CONTAINER(tmp_p, mpl_param_element_t, list_entry);\n"
                            "        if (MPL_PARAM_ELEMENT_IS_FIELD(elem_p))\n"
                            "            elem_found_p = mpl_param_list_find_field(elem_p->context,elem_p->id_in_context,allowed_parameters_p);\n"
                            "        else\n"
                            "            elem_found_p = mpl_param_list_find(elem_p->id, allowed_parameters_p);\n"
                            "        if (!elem_found_p) {\n"
                            "            mpl_param_element_t *elem2_p;\n"
                            "            elem2_p = mpl_param_element_clone(elem_p);\n"
                            "            if (elem2_p == NULL)\n"
                            "                return -1;\n"
                            "            mpl_list_append(result_list_pp, &elem2_p->list_entry);\n"
                            "            (*errors_p)++;\n"
                            "        }\n"
                            "    }\n"
                            "    return 0;\n"
                            "}\n",
                            name_p
                           );
                    bags_found = 1;
                }
                ((bag_parameter*)parameter_p)->gc_c_bag(f);
            }
        }
    }
}

void parameter_set::gc_c(FILE *f)
{
    char *short_name_p;

    if (compiler_p->codegen_mode == codegen_mode_cli) {
        cli_c(f);
        return;
    }
    else if (compiler_p->codegen_mode == codegen_mode_api) {
        api_cc(f, (char*)"");
        return;
    }

    fprintf(f,
            "/* Parameter set %s */\n\n",
            name_p);

    gc_c_enums(f);
    gc_c_ranges(f);
    gc_c_field_values(f);
    gc_c_children(f);
    gc_c_extra(f);
    gc_c_defaults(f);
    gc_c_param_descr(f);
    gc_c_param_init(f);
    short_name_p = get_short_name();
    if (!short_name_p || !strlen(short_name_p)) {
        fprintf(f,
                "/* No short_name defined -> no bag-checking code generated */\n"
               );
    } else {
        gc_c_bags(f);
    }

    fprintf(f,
            "/* End parameter set %s */\n\n",
            name_p
           );
}

void parameter_set::cli_c(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;
    int bags_found = 0;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            if (parameter_p->is_bag()) {
                if (compiler_p->bag_is_command((bag_parameter*)parameter_p) ||
                    compiler_p->param_is_command_parameter(parameter_p))
                    ((bag_parameter*)parameter_p)->cli_c_completions(f);
            }
            if (parameter_p->is_enum()) {
                if (compiler_p->param_is_command_parameter(parameter_p))
                    ((enum_parameter*)parameter_p)->cli_c_completions(f);
            }
        }
    }
}

void parameter_set::api_hh(FILE *f, char *indent)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;
    char *newindent = (char*)calloc(1, strlen(indent) + strlen("    ") + 1);
    char *snu = str_toupper(get_short_name());

    fprintf(f,
            "namespace %s {\n",
            name_p
           );
    sprintf(newindent, "%s    ", indent);

#define INDENT(str) fprintf(f, "%s%s", newindent, str)
    INDENT("class BAG {\n");
    INDENT("    protected:\n");
    INDENT("        virtual void __set_id() { __id = MPL_PARAM_ID_UNDEFINED; }\n");
    INDENT("        mpl_param_element_id_t __id;\n");
    INDENT("    public:\n");
    INDENT("        BAG() { __set_id(); }\n");
    INDENT("        BAG(mpl_param_element_id_t __id) : __id(__id) {}\n");
    INDENT("        BAG(BAG &obj) { __id = obj.__id; }\n");
    INDENT("        virtual ~BAG() {}\n");
    INDENT("        mpl_param_element_id_t id() { return __id; }\n");
    INDENT("        virtual mpl_bag_t *encode() = 0;\n");
    INDENT("        virtual void print() {\n");
    INDENT("            printf(\"%s\\n\", mpl_param_id_get_string(__id));\n");
    INDENT("        }\n");
    INDENT("};\n");

    INDENT("class uint8_array {\n");
    INDENT("    public:\n");
    INDENT("        mpl_uint8_array_t array;\n");
    INDENT("        uint8_array(mpl_uint8_array_t *array);\n");
    INDENT("        uint8_array(char *arr_p, int len);\n");
    INDENT("        uint8_array(uint8_array &a);\n");
    INDENT("        virtual ~uint8_array();\n");
    INDENT("};\n");

    INDENT("class uint16_array {\n");
    INDENT("    public:\n");
    INDENT("        mpl_uint16_array_t array;\n");
    INDENT("        uint16_array(mpl_uint16_array_t *array);\n");
    INDENT("        uint16_array(char *arr_p, int len);\n");
    INDENT("        uint16_array(uint16_array &a);\n");
    INDENT("        virtual ~uint16_array();\n");
    INDENT("};\n");

    INDENT("class uint32_array {\n");
    INDENT("    public:\n");
    INDENT("        mpl_uint32_array_t array;\n");
    INDENT("        uint32_array(mpl_uint32_array_t *array);\n");
    INDENT("        uint32_array(char *arr_p, int len);\n");
    INDENT("        uint32_array(uint32_array &a);\n");
    INDENT("        virtual ~uint32_array();\n");
    INDENT("};\n");

    INDENT("class string_tuple {\n");
    INDENT("    public:\n");
    INDENT("        mpl_string_tuple_t tuple;\n");
    INDENT("        string_tuple(mpl_string_tuple_t *tuple);\n");
    INDENT("        string_tuple(char *key_p, char *value_p);\n");
    INDENT("        string_tuple(string_tuple &t);\n");
    INDENT("        virtual ~string_tuple();\n");
    INDENT("};\n");

    INDENT("class int_tuple {\n");
    INDENT("    public:\n");
    INDENT("        mpl_int_tuple_t tuple;\n");
    INDENT("        int_tuple(mpl_int_tuple_t *tuple);\n");
    INDENT("        int_tuple(int key, int value);\n");
    INDENT("        int_tuple(int_tuple &t);\n");
    INDENT("        virtual ~int_tuple();\n");
    INDENT("};\n");

    INDENT("class strint_tuple {\n");
    INDENT("    public:\n");
    INDENT("        mpl_strint_tuple_t tuple;\n");
    INDENT("        strint_tuple(mpl_strint_tuple_t *tuple);\n");
    INDENT("        strint_tuple(char *key_p, int value);\n");
    INDENT("        strint_tuple(strint_tuple &t);\n");
    INDENT("        virtual ~strint_tuple();\n");
    INDENT("};\n");

    INDENT("class struint8_tuple {\n");
    INDENT("    public:\n");
    INDENT("        mpl_struint8_tuple_t tuple;\n");
    INDENT("        struint8_tuple(mpl_struint8_tuple_t *tuple);\n");
    INDENT("        struint8_tuple(char *key_p, uint8_t value);\n");
    INDENT("        struint8_tuple(struint8_tuple &t);\n");
    INDENT("        virtual ~struint8_tuple();\n");
    INDENT("};\n");
#undef INDENT

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            if (parameter_p->is_bag()) {
                fprintf(f,
                        "%sclass %s;\n",
                        newindent,
                        parameter_p->name_p
                       );
            }
        }
    }

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            parameter_p->api_hh(f, newindent);
        }
    }

    fprintf(f,
            "}\n"
           );
    free(newindent);
    free(snu);
}

void parameter_set::api_cc(FILE *f, char *indent)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;
    char *newindent = (char *)calloc(1, strlen(indent) + strlen("    ") + 1);

    sprintf(newindent, "%s    ", indent);

    fprintf(f,
            "namespace %s {\n",
            name_p
           );

#define INDENT(str) fprintf(f, "%s%s", newindent, str)
    INDENT("uint8_array::uint8_array(mpl_uint8_array_t *array)\n");
    INDENT("{\n");
    INDENT("    this->array.arr_p = (uint8_t *)calloc(array->len, sizeof(uint8_t));\n");
    INDENT("    memcpy(this->array.arr_p, array->arr_p, array->len * sizeof(uint8_t));\n");
    INDENT("}\n");
    INDENT("uint8_array::uint8_array(char *arr_p, int len)\n");
    INDENT("{\n");
    INDENT("    this->array.arr_p = (uint8_t *)calloc(len, sizeof(uint8_t));\n");
    INDENT("    memcpy(this->array.arr_p, arr_p, len * sizeof(uint8_t));\n");
    INDENT("}\n");
    INDENT("uint8_array::uint8_array(uint8_array &a)\n");
    INDENT("{\n");
    INDENT("    this->array.arr_p = (uint8_t*)calloc(a.array.len, sizeof(uint8_t));\n");
    INDENT("    memcpy(this->array.arr_p, a.array.arr_p, a.array.len * sizeof(uint8_t));\n");
    INDENT("    this->array.len = a.array.len;\n");
    INDENT("}\n");
    INDENT("uint8_array::~uint8_array()\n");
    INDENT("{\n");
    INDENT("    free(array.arr_p);\n");
    INDENT("}\n");

    INDENT("uint16_array::uint16_array(mpl_uint16_array_t *array)\n");
    INDENT("{\n");
    INDENT("    this->array.arr_p = (uint16_t *)calloc(array->len, sizeof(uint16_t));\n");
    INDENT("    memcpy(this->array.arr_p, array->arr_p, array->len * sizeof(uint16_t));\n");
    INDENT("}\n");
    INDENT("uint16_array::uint16_array(char *arr_p, int len)\n");
    INDENT("{\n");
    INDENT("    this->array.arr_p = (uint16_t *)calloc(len, sizeof(uint16_t));\n");
    INDENT("    memcpy(this->array.arr_p, arr_p, len * sizeof(uint16_t));\n");
    INDENT("}\n");
    INDENT("uint16_array::uint16_array(uint16_array &a)\n");
    INDENT("{\n");
    INDENT("    this->array.arr_p = (uint16_t*)calloc(a.array.len, sizeof(uint16_t));\n");
    INDENT("    memcpy(this->array.arr_p, a.array.arr_p, a.array.len * sizeof(uint16_t));\n");
    INDENT("    this->array.len = a.array.len;\n");
    INDENT("}\n");
    INDENT("uint16_array::~uint16_array()\n");
    INDENT("{\n");
    INDENT("    free(array.arr_p);\n");
    INDENT("}\n");

    INDENT("uint32_array::uint32_array(mpl_uint32_array_t *array)\n");
    INDENT("{\n");
    INDENT("    this->array.arr_p = (uint32_t *)calloc(array->len, sizeof(uint32_t));\n");
    INDENT("    memcpy(this->array.arr_p, array->arr_p, array->len * sizeof(uint32_t));\n");
    INDENT("}\n");
    INDENT("uint32_array::uint32_array(char *arr_p, int len)\n");
    INDENT("{\n");
    INDENT("    this->array.arr_p = (uint32_t *)calloc(len, sizeof(uint32_t));\n");
    INDENT("    memcpy(this->array.arr_p, arr_p, len * sizeof(uint32_t));\n");
    INDENT("}\n");
    INDENT("uint32_array::uint32_array(uint32_array &a)\n");
    INDENT("{\n");
    INDENT("    this->array.arr_p = (uint32_t*)calloc(a.array.len, sizeof(uint32_t));\n");
    INDENT("    memcpy(this->array.arr_p, a.array.arr_p, a.array.len * sizeof(uint32_t));\n");
    INDENT("    this->array.len = a.array.len;\n");
    INDENT("}\n");
    INDENT("uint32_array::~uint32_array()\n");
    INDENT("{\n");
    INDENT("    free(array.arr_p);\n");
    INDENT("}\n");

    INDENT("string_tuple::string_tuple(mpl_string_tuple_t *tuple)\n");
    INDENT("{\n");
    INDENT("    this->tuple.key_p = strdup(tuple->key_p);\n");
    INDENT("    this->tuple.value_p = strdup(tuple->value_p);\n");
    INDENT("}\n");
    INDENT("string_tuple::string_tuple(char *key_p, char *value_p)\n");
    INDENT("{\n");
    INDENT("    this->tuple.key_p = strdup(key_p);\n");
    INDENT("    this->tuple.value_p = strdup(value_p);\n");
    INDENT("}\n");
    INDENT("string_tuple::string_tuple(string_tuple &t)\n");
    INDENT("{\n");
    INDENT("    this->tuple.key_p = strdup(t.tuple.key_p);\n");
    INDENT("    this->tuple.value_p = strdup(t.tuple.value_p);\n");
    INDENT("}\n");
    INDENT("string_tuple::~string_tuple()\n");
    INDENT("{\n");
    INDENT("    free(tuple.key_p);\n");
    INDENT("    free(tuple.value_p);\n");
    INDENT("}\n");

    INDENT("strint_tuple::strint_tuple(mpl_strint_tuple_t *tuple)\n");
    INDENT("{\n");
    INDENT("    this->tuple.key_p = strdup(tuple->key_p);\n");
    INDENT("    this->tuple.value = tuple->value;\n");
    INDENT("}\n");
    INDENT("strint_tuple::strint_tuple(char *key_p, int value)\n");
    INDENT("{\n");
    INDENT("    this->tuple.key_p = strdup(key_p);\n");
    INDENT("    this->tuple.value = value;\n");
    INDENT("}\n");
    INDENT("strint_tuple::strint_tuple(strint_tuple &t)\n");
    INDENT("{\n");
    INDENT("    this->tuple.key_p = strdup(t.tuple.key_p);\n");
    INDENT("    this->tuple.value = t.tuple.value;\n");
    INDENT("}\n");
    INDENT("strint_tuple::~strint_tuple()\n");
    INDENT("{\n");
    INDENT("    free(tuple.key_p);\n");
    INDENT("}\n");

    INDENT("int_tuple::int_tuple(mpl_int_tuple_t *tuple)\n");
    INDENT("{\n");
    INDENT("    this->tuple = *tuple;\n");
    INDENT("}\n");
    INDENT("int_tuple::int_tuple(int key, int value)\n");
    INDENT("{\n");
    INDENT("    this->tuple.key = key;\n");
    INDENT("    this->tuple.value = value;\n");
    INDENT("}\n");
    INDENT("int_tuple::int_tuple(int_tuple &t)\n");
    INDENT("{\n");
    INDENT("    this->tuple = t.tuple;\n");
    INDENT("}\n");
    INDENT("int_tuple::~int_tuple()\n");
    INDENT("{\n");
    INDENT("}\n");

    INDENT("struint8_tuple::struint8_tuple(mpl_struint8_tuple_t *tuple)\n");
    INDENT("{\n");
    INDENT("    this->tuple.key_p = strdup(tuple->key_p);\n");
    INDENT("    this->tuple.value = tuple->value;\n");
    INDENT("}\n");
    INDENT("struint8_tuple::struint8_tuple(char *key_p, uint8_t value)\n");
    INDENT("{\n");
    INDENT("    this->tuple.key_p = strdup(key_p);\n");
    INDENT("    this->tuple.value = value;\n");
    INDENT("}\n");
    INDENT("struint8_tuple::struint8_tuple(struint8_tuple &t)\n");
    INDENT("{\n");
    INDENT("    this->tuple.key_p = strdup(t.tuple.key_p);\n");
    INDENT("    this->tuple.value = t.tuple.value;\n");
    INDENT("}\n");
    INDENT("struint8_tuple::~struint8_tuple()\n");
    INDENT("{\n");
    INDENT("    free(tuple.key_p);\n");
    INDENT("}\n");
#undef INDENT

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            parameter_p->api_cc(f, newindent);
        }
    }

    fprintf(f,
            "}\n"
           );
    free(newindent);
}

void parameter_set::convert_doc()
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;

    doc_to_dox("parameter_set", name_p);
    doc_to_latex("parameter_set", name_p);
    doc_to_help("parameter_set", name_p);
    prefix_p->convert_doc("string_entry", "prefix");
    paramset_id_p->convert_doc("string_entry", "id");
    if (range_id_p)
        range_id_p->convert_doc("string_entry", "range_id");
    if (bag_field_table_suffix_p)
        bag_field_table_suffix_p->convert_doc("string_entry", "bag_field_table_suffix");
    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);
        parameter_group_p->convert_doc();
    }
}

void parameter_set::dox_parameters(FILE *f, int level)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            parameter_p->dox(f, level);
            fprintf(f, "\n");
        }
    }
}

void parameter_set::dox(FILE *f)
{
    dox_doc_f(f, doc_list_p);
    fprintf(f,
            "namespace %s\n"
            "{\n",
            name_p
           );

    if (prefix_p) {
        fprintf(f, "%s", spacing(1));
        dox_doc_f(f, prefix_p->doc_list_p);
        fprintf(f, "%s", spacing(1));
        fprintf(f,
                "const string prefix = \"%s\";",
                prefix_p->value_p
               );
        dox_doc_b(f, prefix_p->doc_list_p);
        fprintf(f, "\n");
    }

    dox_parameters(f, 1);
    gc_h_functions(f);
    gc_h_macros(f);

    fprintf(f,
            "}\n"
           );
    fprintf(f, "\n");
}

void parameter_set::add_dox_info(dox_entry *dox_entry_p,
                                 const char *label_p,
                                 char *value_p,
                                 char *text_p)
{
    dox_entry_p->label_p = (char *) calloc(1, strlen(label_p) + 2);
    dox_entry_p->value_p = (char *) calloc(1, strlen(value_p) +
                                           100);

    if (!strcmp(label_p, "ingroup")) {
        sprintf(dox_entry_p->label_p, "");
        sprintf(dox_entry_p->value_p, "");
    }
    else if (!strcmp(label_p, "see")) {
        sprintf(dox_entry_p->label_p, "");
        sprintf(dox_entry_p->value_p, "");
    }
    else if (!strcmp(label_p, "text")) {
        sprintf(dox_entry_p->label_p, "");
        sprintf(dox_entry_p->value_p, "%s", value_p);
    }
    else {
        fprintf(stderr,
                "%s:%d: Unsupported dox label '%s'\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                label_p
               );
        exit(-1);
    }
}

void parameter_set::latex(FILE *f)
{
    LX_S("%s",
         group_p ? group_p->text_p : name_p
        );
    LX_LABEL("pset:%s",
             name_p
            );

    latex_latex_list(f, latex_list_p);

    fprintf(f,
            "\n"
           );

    lx_table_begin(f, 3);
    LX_HLINE;
    LX_TABH(3, "%s",
            "Parameter Set Properties"
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

    fprintf_latex(f,
                  "Name"
                 );
    LX_COL_SEP;
    fprintf_latex(f,
                  "%s",
                  name_p
                 );
    LX_COL_SEP;
    fprintf_latex(f,
                  "The formal symbolic name of the parameter set.\n"
                 );
    LX_COL_END;

    fprintf_latex(f,
                  "Prefix"
                 );
    LX_COL_SEP;
    fprintf_latex(f,
                  "''%s''",
                  prefix_p->value_p
                 );
    LX_COL_SEP;
    if (prefix_p->latex_list_p)
        latex_latex_list(f, prefix_p->latex_list_p);
    else
        fprintf_latex(f,
                      "The textual identity of the parameter set. This is used in\n"
                      "the packed format of the parameter."
                     );
    LX_COL_END;

    fprintf_latex(f,
                  "Id"
                 );
    LX_COL_SEP;
    fprintf_latex(f,
                  "%d",
                  atoi(paramset_id_p->value_p)
                 );
    LX_COL_SEP;
    fprintf_latex(f,
                  "The numeric identity of the parameter set. This is used in\n"
                  "the internal format of the parameter."
                 );
    LX_COL_END;

    if (range_id_p) {
        fprintf_latex(f,
                      "Range Id"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      range_id_p->value_p
                     );
        fprintf(f,
                " \\ref{%s:%s}\n",
                name_p,
                range_id_p->value_p
               );
        LX_COL_SEP;
        if (range_id_p->latex_list_p)
            latex_latex_list(f, range_id_p->latex_list_p);
        else
            fprintf_latex(f,
                          "The name of a virtual enum parameter that is used to\n"
                          "identify number ranges in the parameter set."
                         );
        LX_COL_END;
    }

    if (bag_field_table_suffix_p) {
        fprintf_latex(f,
                      "Bag field table suffix"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      bag_field_table_suffix_p->value_p
                     );
        LX_COL_SEP;
        if (bag_field_table_suffix_p->latex_list_p)
            latex_latex_list(f, bag_field_table_suffix_p->latex_list_p);
        else
            fprintf_latex(f,
                          "Appended to bag name when creating names for bag-field-table enums.\n"
                         );
        LX_COL_END;
    }

    LX_HLINE;
    LX_TAB_END;

    if (default_parameter_group_p)
        default_parameter_group_p->latex(f);
}


void parameter_set::deja(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_group *parameter_group_p;

    fprintf(f,
            "# parameter set %s\n",
            name_p
           );

    if (get_short_name() == NULL) {
        fprintf(f,
                "# short_name not set for %s -> no code\n",
                name_p
               );
        return;
    }

    fprintf(f,
            "namespace eval %s {\n",
            name_p
           );

    MPL_LIST_FOR_EACH(parameter_group_list_p, tmp_p) {
        parameter_group_p = LISTABLE_PTR(tmp_p, parameter_group);

        mpl_list_t *tmp_p;
        parameter *parameter_p;
        MPL_LIST_FOR_EACH(parameter_group_p->parameters_p, tmp_p) {
            parameter_p = LISTABLE_PTR(tmp_p, parameter);
            parameter_p->deja(f);
            fprintf(f,
                    "\n"
                   );
        }
    }
    fprintf(f,
            "}\n"
            "# end parameter set %s\n",
            name_p
           );
}

enum_parameter *parameter_set::create_field_table_parameter(char *name_p)
{
    char *fnp_name_p = (char*) calloc(1, strlen(name_p) + 100);
    sprintf(fnp_name_p,
            "%s_%s",
            name_p,
            bag_field_table_suffix_p->value_p
           );
    if (find_parameter(fnp_name_p) != NULL) {
        fprintf(stderr,
                "%s:%d: Parameter '%s' already exists\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                fnp_name_p
               );
        exit(-1);
    }

    create_parameter_in_default_group("enum",
                                      fnp_name_p);
    enum_parameter* field_table_parameter_p = (enum_parameter*)find_parameter(fnp_name_p);
    assert(field_table_parameter_p);
    field_table_parameter_p->add_option("virtual", "true", value_type_bool);
    doc *doc_p = new doc(strdup("/** This enum is used to map between field names and numeric field ids. */"), 1);
    doc_p->append_to(field_table_parameter_p->doc_list_p);
    free(fnp_name_p);
    return field_table_parameter_p;
}

void parameter_set::wrap_up_definition()
{
    if (prefix_p == NULL) {
        fprintf(stderr,
                "%s:%d: No prefix defined\n",
                compiler_p->peek_filename(), compiler_p->lineno()
               );
        exit(-1);
    }

    if (paramset_id_p == NULL) {
        char *id_str_p = (char*) calloc(1, 100);
        sprintf(id_str_p, "%d", mpl_prefix2paramsetid(prefix_p->value_p));
        add_id(id_str_p);
        free(id_str_p);
    }

    if (range_id_p) {
        enum_parameter* range_id_parameter_p =
            (enum_parameter*) find_parameter(range_id_p->value_p);
        if (range_id_parameter_p)
            range_id_parameter_p->wrap_up_definition();
    }

    if (bag_field_table_suffix_p == NULL) {
        add_bag_field_table_suffix((char*) BAG_FIELD_TABLE_SUFFIX_DEFAULT);
    }
}

void parameter_set::use_parameter_group(category *category_p)
{
    if (category_p == NULL) {
        if (default_parameter_group_p == NULL) {
            default_parameter_group_p = new parameter_group(compiler_p,
                                                            this,
                                                            NULL);
            default_parameter_group_p->add_to(parameter_group_list_p);
        }
        current_parameter_group_p = default_parameter_group_p;
    }
    else {
        parameter_group *parameter_group_p = find_parameter_group(category_p->name_p);
        if (parameter_group_p == NULL) {
            parameter_group_p = new parameter_group(compiler_p,
                                                    this,
                                                    category_p);
            parameter_group_p->add_to(parameter_group_list_p);
        }
        current_parameter_group_p = parameter_group_p;
    }
}
