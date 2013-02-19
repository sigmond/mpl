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
#include <FlexLexer.h>
#include "mplcomp.hh"
#include "mplcomp_parameter.hh"
#include <unistd.h>
#include <stdlib.h>

static void add_line(mpl_compiler *compiler_p, mpl_list_t **lines_pp, char *line_p, code_segment_t segment);

static void add_line(mpl_compiler *compiler_p, mpl_list_t **lines_pp, char *line_p, code_segment_t segment)
{
    line *line_entry_p = new line(compiler_p, strdup(line_p), segment);
    line_entry_p->append_to(lines_pp);
}


mpl_compiler::~mpl_compiler()
{
    DELETE_LISTABLE_LIST(&blocks_p, block);
    DELETE_LISTABLE_LIST(&categories_p, category);
    DELETE_LISTABLE_LIST(&parameter_sets_p, parameter_set);
    DELETE_LISTABLE_LIST(&compiler_options_p, compiler_option);
    DELETE_LISTABLE_LIST(&files_p, file);
    DELETE_LISTABLE_LIST(&remembered_files_p, file);
    DELETE_LISTABLE_LIST(&compiler_defines_p, compiler_define);
    DELETE_LISTABLE_LIST(&hlines_p, line);
    DELETE_LISTABLE_LIST(&clines_p, line);
    if (include_dir_name_p)
        free(include_dir_name_p);
    DELETE_LISTABLE_LIST(&groups_p, group_entry);
    if (flags_p)
        free(flags_p);
}

int mpl_compiler::check_if_condition_internal(char *if_condition_text_p)
{
    mpl_list_t *tmp_p;
    compiler_define *compiler_define_p;
    char *text_p = NULL;

    if (if_condition_text_p)
        text_p = strdup(trimstring(if_condition_text_p, '\\'));

    if (!experimental) {
        fprintf(stderr, "%s:%d: Compiler defines not supported in this mode\n",
                peek_filename(), lineno()
               );
        exit(-1);
    }

    MPL_LIST_FOR_EACH(compiler_defines_p, tmp_p) {
        compiler_define_p = LISTABLE_PTR(tmp_p, compiler_define);
        if (text_p) {
            if (!strcmp(compiler_define_p->key_p, text_p)) {
                return 1;
            }
        }
        else {
            if (!strcmp(compiler_define_p->key_p, current_if_condition.key_p)) {
                if (current_if_condition.check(compiler_define_p)) {
                    return 1;
                }
            }
        }
    }
    free(text_p);
    return 0;
}

int mpl_compiler::check_ifdef_condition(char *if_condition_text_p)
{
    return check_if_condition_internal(if_condition_text_p);
}

int mpl_compiler::check_if_condition()
{
    return check_if_condition_internal(NULL);
}

void mpl_compiler::add_left_operand_to_if_condition(char *left_operand_p)
{
    if (current_if_condition.key_p)
        free(current_if_condition.key_p);
    current_if_condition.key_p = strdup(left_operand_p);
}

void mpl_compiler::add_right_operand_to_if_condition(char *right_operand_p)
{
    if (current_if_condition.value_p)
        free(current_if_condition.value_p);
    current_if_condition.value_p = strdup(right_operand_p);
}

void mpl_compiler::add_operator_to_if_condition(char operator_char)
{
    switch (operator_char) {
        case '<':
            current_if_condition.if_operator = if_operator_lt;
            break;
        case '=':
            current_if_condition.if_operator = if_operator_eq;
            break;
        case '>':
            current_if_condition.if_operator = if_operator_gt;
            break;
        default:
            fprintf(stderr, "%s:%d: Operator %c not supported\n",
                    peek_filename(), lineno(),
                    operator_char
                   );
            exit(-1);
    }
}



void mpl_compiler::reset_if_condition()
{
    current_if_condition.if_operator = if_operator_none;
}

static const char *operator_string(if_operator_t if_operator)
{
    switch (if_operator){
#define IF_OPERATOR_VALUE_ELEMENT(ELEMENT) \
        case if_operator_##ELEMENT: \
        return #ELEMENT; \

        IF_OPERATORS
    }
    return "<unknown>";
}


void mpl_compiler::print_compiler_defines()
{
    mpl_list_t *tmp_p;
    compiler_define *compiler_define_p;

    printf("compiler_defines\n");
    MPL_LIST_FOR_EACH(compiler_defines_p, tmp_p) {
        compiler_define_p = LISTABLE_PTR(tmp_p, compiler_define);
        printf("%s", spacing(1));
        printf("%s = %s\n",
               compiler_define_p->key_p,
               compiler_define_p->value_p
              );
    }
    printf("\n");
}

void mpl_compiler::print_groups()
{
    mpl_list_t *tmp_p;
    group_entry *group_p;

    printf("groups\n");
    MPL_LIST_FOR_EACH(groups_p, tmp_p) {
        group_p = LISTABLE_PTR(tmp_p, group_entry);
        group_p->print(1);
    }
    printf("\n");
}

void mpl_compiler::create_category( char *parent_name_p, char *name_p )
{
    category *category_p;
    category *parent_p;

    if (find_category(name_p) != NULL) {
        fprintf(stderr, "%s:%d: duplicate category '%s'\n",
                peek_filename(), lineno(),
                name_p);
        exit(-1);
    }

    if (parent_name_p == NULL) {
        parent_p = NULL;
    }
    else {
        parent_p = find_category(parent_name_p);
        if (parent_p == NULL){
            fprintf(stderr, "%s:%d: no category parent '%s'\n",
                    peek_filename(), lineno(),
                    parent_name_p);
            exit(-1);
        }
    }

    category_p = new category(this, strdup(name_p), parent_p);
    add_any_forward_doc(&category_p->doc_list_p);
    category_p->append_to(categories_p);
    current_category_p = category_p;
}

category *mpl_compiler::find_category(char *name_p)
{
    mpl_list_t *tmp_p;
    category *category_p;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);
        if (!strcmp(category_p->name_p, name_p))
            return category_p;
    }

    return NULL;
}

void mpl_compiler::print_categories()
{
    mpl_list_t *tmp_p;
    category *category_p;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);
        category_p->print(0);
    }
}

void mpl_compiler::add_parameter_set_to_current_category(char *parameter_set_name_p)
{
    parameter_set *parameter_set_p;

    if (current_category_p == NULL) {
        fprintf(stderr, "%s:%d: no current category\n",
                peek_filename(), lineno());
        exit(-1);
    }
    parameter_set_p = find_parameter_set(parameter_set_name_p);
    if (parameter_set_p == NULL) {
        fprintf(stderr, "%s:%d: parameter set %s does not exist\n",
                peek_filename(), lineno(),
               parameter_set_name_p
               );
        exit(-1);
    }
    current_category_p->set_parameter_set(parameter_set_p);
    set_current_parameter_set(parameter_set_name_p);
}

void mpl_compiler::add_command_bag_to_current_category(char *name_p)
{
    if (current_category_p == NULL) {
        fprintf(stderr, "%s:%d: no current category\n",
                peek_filename(), lineno());
        exit(-1);
    }
    current_parameter_p = current_category_p->add_command_bag(name_p);
    current_parameter_list_pp = &((bag_parameter*)current_parameter_p)->bag_parameter_list_p;
}

void mpl_compiler::add_response_bag_to_current_category(char *name_p)
{
    if (current_category_p == NULL) {
        fprintf(stderr, "%s:%d: no current category\n",
                peek_filename(), lineno());
        exit(-1);
    }
    current_parameter_p = current_category_p->add_response_bag(name_p);
    current_parameter_list_pp = &((bag_parameter*)current_parameter_p)->bag_parameter_list_p;
}

void mpl_compiler::add_event_bag_to_current_category(char *name_p)
{
    if (current_category_p == NULL) {
        fprintf(stderr, "%s:%d: no current category\n",
                peek_filename(), lineno());
        exit(-1);
    }
    current_parameter_p = current_category_p->add_event_bag(name_p);
    current_parameter_list_pp = &((bag_parameter*)current_parameter_p)->bag_parameter_list_p;
}

void mpl_compiler::create_parameter_in_current_parameter_set(const char *type_p, char *name_p)
{
    assert(peek_block());

    if (is_inside_block(block_type_parameters_in_file_scope)) {
        current_parameter_p = get_current_parameter_set()->create_parameter_in_current_group(type_p, name_p);
    }
    else {
        current_parameter_p = get_current_parameter_set()->create_parameter_in_default_group(type_p, name_p);
    }
    add_any_forward_doc(&current_parameter_p->doc_list_p);
    if (!strcmp(type_p, "bag")) {
        if (!current_parameter_p->is_bag()) {
            fprintf(stderr, "%s:%d: current parameter is not a bag\n",
                    peek_filename(), lineno());
            exit(-1);
        }
        current_parameter_list_pp = &((bag_parameter*)current_parameter_p)->bag_parameter_list_p;
    }
}

void mpl_compiler::add_parent_to_current_parameter(char *parent_parameter_set_name_p,
                                                   char *parent_name_p)
{
    if (current_parameter_p == NULL) {
        fprintf(stderr, "%s:%d: no current parameter\n",
                peek_filename(), lineno());
        exit(-1);
    }

    current_parameter_p->add_parent_parameter(parent_parameter_set_name_p, parent_name_p);
}

void mpl_compiler::add_command_to_current_category(char *name_p)
{
    if (current_category_p == NULL) {
        fprintf(stderr, "%s:%d: no current category\n",
                peek_filename(), lineno());
        exit(-1);
    }
    reset_current_parameter_list();
    current_command_p = current_category_p->add_command(name_p);
}

void mpl_compiler::move_current_parameter_list_to_current_command()
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    while ((tmp_p = mpl_list_remove(current_parameter_list_pp, NULL)) != NULL)
    {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        current_command_p->add_parameter_list_entry(parameter_list_entry_p);
    }

    reset_current_parameter_list();
}

void mpl_compiler::add_event_to_current_category(char *name_p)
{
    if (current_category_p == NULL) {
        fprintf(stderr, "%s:%d: no current category\n",
                peek_filename(), lineno());
        exit(-1);
    }
    current_event_p = current_category_p->add_event(name_p);
}

void mpl_compiler::move_current_parameter_list_to_current_event()
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;

    while ((tmp_p = mpl_list_remove(current_parameter_list_pp, NULL)) != NULL)
    {
        parameter_list_entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
        current_event_p->add_parameter_list_entry(parameter_list_entry_p);
    }

    reset_current_parameter_list();
}



void mpl_compiler::create_parameter_set( char *name_p )
{
    parameter_set *parameter_set_p;

    if (!name_p || (strlen(name_p) == 0)) {
        fprintf(stderr, "%s:%d: parameter_set name empty\n",
                peek_filename(), lineno());
        exit(-1);
    }

    parameter_set_p = new parameter_set(this, strdup(name_p));
    add_any_forward_doc(&parameter_set_p->doc_list_p);

    if (find_parameter_set(name_p) != NULL) {
        fprintf(stderr, "%s:%d: duplicate parameter_set '%s'\n",
                peek_filename(), lineno(),
                name_p);
        exit(-1);
    }
    parameter_set_p->append_to(parameter_sets_p);
    set_current_parameter_set(name_p);
}

parameter_set *mpl_compiler::find_parameter_set(char *name_p)
{
    mpl_list_t *tmp_p;
    parameter_set *parameter_set_p;

    MPL_LIST_FOR_EACH(parameter_sets_p, tmp_p) {
        parameter_set_p = LISTABLE_PTR(tmp_p, parameter_set);
        if (!strcmp(parameter_set_p->name_p, name_p))
            return parameter_set_p;
    }
    return NULL;
}

void mpl_compiler::set_current_parameter_set(char *name_p)
{
    current_parameter_set_p = find_parameter_set(name_p);
    if (current_parameter_set_p == NULL) {
        fprintf(stderr, "%s:%d: No parameter set '%s'\n",
                peek_filename(), lineno(),
                name_p);
        exit(-1);
    }
}

void mpl_compiler::set_current_parameter_group(char *category_name_p)
{
    category *category_p = NULL;

    if (category_name_p != NULL) {
        category_p = find_category(category_name_p);
        if (category_p == NULL) {
            fprintf(stderr, "%s:%d: No category '%s'\n",
                    peek_filename(), lineno(),
                    category_name_p);
            exit(-1);
        }
    }

    current_parameter_set_p->use_parameter_group(category_p);
}

parameter_set *mpl_compiler::get_current_parameter_set()
{
    if (current_parameter_set_p == NULL) {
        if (current_category_p == NULL) {
            fprintf(stderr, "%s:%d: no current parameter set or category\n",
                    peek_filename(), lineno());
            exit(-1);
        }
        current_parameter_set_p = current_category_p->get_parameter_set();

        if (current_parameter_set_p == NULL) {
            fprintf(stderr, "%s:%d: No current parameter set\n",
                    peek_filename(), lineno()
                   );
            exit(-1);
        }
    }
    return current_parameter_set_p;
}

void mpl_compiler::print_parameter_sets()
{
    mpl_list_t *tmp_p;
    parameter_set *parameter_set_p;

    MPL_LIST_FOR_EACH(parameter_sets_p, tmp_p) {
        parameter_set_p = LISTABLE_PTR(tmp_p, parameter_set);
        parameter_set_p->print(0);
    }
}

void mpl_compiler::add_prefix_to_current_parameter_set(char *prefix_p)
{
    get_current_parameter_set()->add_prefix(prefix_p);
}

void mpl_compiler::add_id_to_current_parameter_set(char *id_p)
{
    mpl_list_t *tmp_p;
    parameter_set *parameter_set_p;

    MPL_LIST_FOR_EACH(parameter_sets_p, tmp_p) {
        parameter_set_p = LISTABLE_PTR(tmp_p, parameter_set);
        if (parameter_set_p == get_current_parameter_set())
            continue;

        if (!strcmp(parameter_set_p->paramset_id_p->value_p, id_p)) {
            fprintf(stderr, "%s:%d: A parameter set with numeric id %s already exists\n",
                    peek_filename(), lineno(),
                    id_p);
            exit(-1);
        }
    }

    get_current_parameter_set()->add_id(id_p);
}

void mpl_compiler::add_range_id_to_current_parameter_set(char *range_id_p)
{
    get_current_parameter_set()->add_range_id(range_id_p);
}

void mpl_compiler::add_bag_field_table_suffix_to_current_parameter_set(char *suffix_p)
{
    get_current_parameter_set()->add_bag_field_table_suffix(suffix_p);
}

void mpl_compiler::create_ellipsis()
{
    ellipsis *ellipsis_p = new ellipsis(this, 1);
    add_any_forward_doc(&ellipsis_p->doc_list_p);
    if (!current_parameter_p->is_bag()) {
        fprintf(stderr, "%s:%d: current parameter is not a bag\n",
                peek_filename(), lineno());
        exit(-1);
    }
    ((bag_parameter *)current_parameter_p)->ellipsis_p = ellipsis_p;
}

parameter_list_entry *mpl_compiler::create_parameter_list_entry(char *parameter_set_name_p,
                                                                char *name_p,
                                                                char *field_name_p,
                                                                int optional,
                                                                int multiple)
{
    parameter_set *parameter_set_p = get_parameter_set_or_current(parameter_set_name_p);

    parameter_list_entry *parameter_list_entry_p =
        new parameter_list_entry(this, parameter_set_p, name_p ? strdup(name_p) : NULL, field_name_p ? strdup(field_name_p) : NULL, optional, multiple, direction);
    if (doc_mode == doc_mode_normal) {
        add_any_forward_doc(&parameter_list_entry_p->doc_list_p);
    }
    else {
        set_backward_doc_target(&parameter_list_entry_p->doc_list_p);
    }
    return parameter_list_entry_p;
}

void mpl_compiler::append_to_current_parameter_list(parameter_list_entry *parameter_list_entry_p)
{
    if (find_parameter_entry_in_list(*current_parameter_list_pp,
                                     parameter_list_entry_p->parameter_set_p->name_p,
                                     parameter_list_entry_p->parameter_name_p,
                                     parameter_list_entry_p->field_name_p)) {
        fprintf(stderr, "%s:%d: parameter '%s::%s%s%s' already exists in parameter list\n",
                peek_filename(), lineno(),
                parameter_list_entry_p->parameter_set_p->name_p,
                parameter_list_entry_p->parameter_name_p,
                parameter_list_entry_p->field_name_p ? " " : "",
                parameter_list_entry_p->field_name_p ? parameter_list_entry_p->field_name_p : "");
        exit(-1);
    }
    parameter_list_entry_p->append_to(current_parameter_list_pp);
}

void mpl_compiler::set_array_option(char *option_p)
{
    parameter_list_entry *entry_p = LISTABLE_PTR(mpl_list_last(*current_parameter_list_pp), parameter_list_entry);
    entry_p->set_option(option_p);
}

void mpl_compiler::add_enum_value_to_current_parameter(char *name_p, char *value_p)
{
    if (current_parameter_p == NULL) {
        fprintf(stderr, "%s:%d: no current parameter\n",
                peek_filename(), lineno());
        exit(-1);
    }
    if (!current_parameter_p->is_enum()) {
        fprintf(stderr, "%s:%d: current parameter is not an enum\n",
                peek_filename(), lineno());
        exit(-1);
    }
    ((enum_parameter*)current_parameter_p)->add_enum_value(name_p, value_p);
}

void mpl_compiler::add_option_to_current_parameter(const char *option_p,
                                                   const char *value_p,
                                                   value_type_t value_type)
{
    if (current_parameter_p == NULL) {
        fprintf(stderr, "%s:%d: no current parameter\n",
                peek_filename(), lineno());
        exit(-1);
    }
    current_parameter_p->add_option(option_p, value_p, value_type);
}

void mpl_compiler::add_option_to_current_parameter(const char *option_p,
                                                   char *value1_p,
                                                   value_type_t value_type1,
                                                   char *value2_p,
                                                   value_type_t value_type2)
{
    if (current_parameter_p == NULL) {
        fprintf(stderr, "%s:%d: no current parameter\n",
                peek_filename(), lineno());
        exit(-1);
    }
    current_parameter_p->add_option(option_p, value1_p, value_type1, value2_p, value_type2);
}

void mpl_compiler::reset_current_category()
{
    current_category_p = NULL;
}

void mpl_compiler::reset_current_parameter_set()
{
    current_parameter_set_p = NULL;
}

void mpl_compiler::reset_current_parameter_list()
{
    current_parameter_list_tmp_p = NULL;
    current_parameter_list_pp = &current_parameter_list_tmp_p;
}

void mpl_compiler::print_compiler_options()
{
    mpl_list_t *tmp_p;
    compiler_option *compiler_option_p;

    printf("compiler_options\n");
    MPL_LIST_FOR_EACH(compiler_options_p, tmp_p) {
        compiler_option_p = LISTABLE_PTR(tmp_p, compiler_option);
        compiler_option_p->print(1);
    }
    printf("\n");
}

int mpl_compiler::get_compiler_option(const char *block_type_p, char *block_name_p, const char *option_name_p, char **option_value_pp)
{
    mpl_list_t *tmp_p;
    compiler_option *compiler_option_p;

    MPL_LIST_FOR_EACH(compiler_options_p, tmp_p) {
        compiler_option_p = LISTABLE_PTR(tmp_p, compiler_option);
        if (!strcmp(compiler_option_p->block_type_p, block_type_p) &&
            !strcmp(compiler_option_p->block_name_p, block_name_p) &&
            !strcmp(compiler_option_p->option_name_p, option_name_p)) {
            if (compiler_option_p->option_value_p)
                *option_value_pp = compiler_option_p->option_value_p;
            else
                *option_value_pp = NULL;
            return 1;
        }
    }
    return 0;
}

void mpl_compiler::create_compiler_option()
{
    compiler_option *compiler_option_p = new compiler_option(this);
    compiler_option_p->add_to(compiler_options_p);
}

void mpl_compiler::add_block_type_to_compiler_option(const char *block_type_p)
{
    compiler_option *compiler_option_p = LISTABLE_PTR(compiler_options_p, compiler_option);
    if (!strcmp(block_type_p, "parameter_set")) {
        compiler_option_p->block_type_p = block_type_p;
        return;
    }
    if (!strcmp(block_type_p, "category")) {
        compiler_option_p->block_type_p = block_type_p;
        return;
    }

    fprintf(stderr, "%s:%d: Invalid compiler option block type %s\n",
            peek_filename(), lineno(),
            block_type_p
           );
    exit(-1);
}

void mpl_compiler::add_block_name_to_compiler_option(char *block_name_p)
{
    compiler_option *compiler_option_p = LISTABLE_PTR(compiler_options_p, compiler_option);
    compiler_option_p->block_name_p = strdup(block_name_p);
}

void mpl_compiler::add_value_to_compiler_option_and_create_new_if_not_first(char *option_name_p, char *option_value_p)
{
    compiler_option *compiler_option_p = LISTABLE_PTR(compiler_options_p, compiler_option);
    if (compiler_option_p->option_name_p) {
        /* Create a new one under the same block */
        create_compiler_option();
        add_block_type_to_compiler_option(compiler_option_p->block_type_p);
        add_block_name_to_compiler_option(compiler_option_p->block_name_p);
        compiler_option_p = LISTABLE_PTR(compiler_options_p, compiler_option);
    }

    compiler_option_p->option_name_p = strdup(option_name_p);
    compiler_option_p->option_value_p = strdup(option_value_p);

    if (compiler_option_p->check()) {
        fprintf(stderr, "%s:%d: Invalid compiler option %s=%s for %s,%s\n",
                peek_filename(), lineno(),
                compiler_option_p->option_name_p,
                compiler_option_p->option_value_p,
                compiler_option_p->block_type_p,
                compiler_option_p->block_name_p
               );
        exit(-1);
    }
}

void mpl_compiler::create_number_range_in_current_parameter_set(char *number_range_name_p)
{
    number_range *number_range_p = new number_range(this, strdup(number_range_name_p));
    add_any_forward_doc(&number_range_p->doc_list_p);
    get_current_parameter_set()->add_number_range(number_range_p);
}

void mpl_compiler::add_integer_range_to_current_number_range(char *first_p, char *last_p)
{
    integer_range *integer_range_p;
    number_range *number_range_p = LISTABLE_PTR(get_current_parameter_set()->number_range_list_p,
                                                       number_range);
    integer_range_p = new integer_range(this, first_p, last_p);
    number_range_p->add_integer_range(integer_range_p);
}


void mpl_compiler::create_enumerator_list_in_current_parameter_set(char *enumerator_list_name_p)
{
    enumerator_list *enumerator_list_p = new enumerator_list(this, strdup(enumerator_list_name_p));
    add_any_forward_doc(&enumerator_list_p->doc_list_p);
    get_current_parameter_set()->add_enumerator_list(enumerator_list_p);
}

void mpl_compiler::add_enumerator_list_value_to_current(char *name_p, char *value_p)
{
    int64_t value;
    enum_value *enum_value_p;

    enumerator_list *enumerator_list_p = LISTABLE_PTR(get_current_parameter_set()->enumerator_lists_p,
                                                  enumerator_list);
    if (value_p != NULL) {
        if (string2int64(value_p, &value) != 0) {
            fprintf(stderr, "%s:%d: Could not convert string '%s' to integer\n",
                    peek_filename(), lineno(),
                    value_p
                   );
            exit(-1);
        }

        enum_value_p = new enum_value(this, strdup(name_p), value);
    }
    else {
        enum_value_p = new enum_value(this, strdup(name_p));
    }

    add_any_forward_doc(&enum_value_p->doc_list_p);
    enum_value_p->append_to(enumerator_list_p->enumerator_list_values_p);
}

void mpl_compiler::append_enumerator_list_to_current(char *parameter_set_name_p,
                                                     char *enumerator_list_name_p)
{
    mpl_list_t *tmp_p;
    enum_value *src_value_p;
    enum_value *dst_value_p;
    parameter_set *parameter_set_p = get_parameter_set_or_current(parameter_set_name_p);    
    enumerator_list *enumerator_list_p = LISTABLE_PTR(get_current_parameter_set()->enumerator_lists_p,
                                                  enumerator_list);
    enumerator_list *src_enumerator_list_p = parameter_set_p->get_enumerator_list(enumerator_list_name_p);
    if (src_enumerator_list_p == NULL) {
        fprintf(stderr, "%s:%d: Enumerator list %s does not exist in parameter set %s\n",
                peek_filename(), lineno(),
                enumerator_list_name_p,
                parameter_set_p->name_p
               );
        exit(-1);
    }

    MPL_LIST_FOR_EACH(src_enumerator_list_p->enumerator_list_values_p, tmp_p) {
        src_value_p = LISTABLE_PTR(tmp_p, enum_value);
        dst_value_p = new enum_value(*src_value_p);
        dst_value_p->append_to(enumerator_list_p->enumerator_list_values_p);
    }
}

void mpl_compiler::append_enumerator_list_to_current_enum(char *parameter_set_name_p,
                                                          char *enumerator_list_name_p)
{
    parameter_set *parameter_set_p = get_parameter_set_or_current(parameter_set_name_p);    
    enumerator_list *src_enumerator_list_p = parameter_set_p->get_enumerator_list(enumerator_list_name_p);
    if (src_enumerator_list_p == NULL) {
        fprintf(stderr, "%s:%d: Enumerator list %s does not exist in parameter set %s\n",
                peek_filename(), lineno(),
                enumerator_list_name_p,
                parameter_set_p->name_p
               );
        exit(-1);
    }
    if (!current_parameter_p->is_enum()) {
        fprintf(stderr, "%s:%d: current parameter is not an enum\n",
                peek_filename(), lineno());
        exit(-1);
    }
    ((enum_parameter*)current_parameter_p)->add_enumerator_list_values(src_enumerator_list_p);
}

int mpl_compiler::is_enumerator_list(char *name_p)
{
    if (current_parameter_set_p == NULL)
        return 0;

    return(current_parameter_set_p->get_enumerator_list(name_p) != NULL);
}


void mpl_compiler::check_parameters()
{
    mpl_list_t *tmp_p;
    category *category_p;
    parameter_set *parameter_set_p;
    int ret = 0;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);
        ret += category_p->check_parameters();
    }

    MPL_LIST_FOR_EACH(parameter_sets_p, tmp_p) {
        parameter_set_p = LISTABLE_PTR(tmp_p, parameter_set);
        ret += parameter_set_p->check_parameters();
    }

    if (ret > 0) {
        fprintf(stderr, "%d undefined parameters found\n", ret);
        exit(-1);
    }
}

void mpl_compiler::push_file(const char *filename_p, int lineno_offset, ifstream *stream_p)
{
    file *file_p = new file(this, strdup(filename_p), lineno_offset, stream_p);
    file_p->add_to(files_p);
}

char *mpl_compiler::peek_filename()
{
    file *file_p;
    if (files_p != NULL)
        file_p = LISTABLE_PTR(files_p, file);
    else
        file_p = LISTABLE_PTR(remembered_files_p, file);

    return file_p->filename_p;
}

file *mpl_compiler::current_file()
{
    if (files_p)
        return (LISTABLE_PTR(files_p, file));
    else
        return NULL;
}

int mpl_compiler::current_lineno_offset()
{
    file *file_p = current_file();
    if (file_p != NULL)
        return file_p->lineno_offset;
    else
        return 1;
}

file *mpl_compiler::pop_file()
{
    mpl_list_t *tmp_p;
    file *file_p;
    file *curf_p;

    tmp_p = mpl_list_remove(&files_p, NULL);
    file_p = LISTABLE_PTR(tmp_p, file);
    curf_p = current_file();
    if (curf_p)
        curf_p->lineno_offset += lexer_p->lineno() - file_p->lineno_offset;
    return(file_p);
}

void mpl_compiler::remember_file(file *file_p, int on_top)
{
    if (on_top)
        file_p->add_to(remembered_files_p);
    else
        file_p->append_to(remembered_files_p);
}

void mpl_compiler::forget_files()
{
    DELETE_LISTABLE_LIST(&remembered_files_p, file);
    DELETE_LISTABLE_LIST(&files_p, file);
}

void mpl_compiler::add_hline(char *line_p)
{
    add_line(this, &hlines_p, line_p, current_code_segment);
}

void mpl_compiler::add_cline(char *line_p)
{
    add_line(this, &clines_p, line_p, current_code_segment);
}

void mpl_compiler::set_literal_code_option(const char *option_name_p, char *option_value_p)
{
    if (!strcmp(option_name_p, "segment")) {
        if (!strcmp(option_value_p, "top")) {
            current_code_segment = code_segment_top;
            return;
        }
        if (!strcmp(option_value_p, "prototypes")) {
            current_code_segment = code_segment_prototypes;
            return;
        }
    }

    fprintf(stderr,
            "%s:%d: Illegal code option %s = %s\n",
            peek_filename(), lineno(),
            option_name_p,
            option_value_p
           );
    exit(-1);
}

void mpl_compiler::reset_literal_code_options()
{
    current_code_segment = code_segment_top;
}

void mpl_compiler::set_doc_mode(doc_mode_t mode)
{
    doc_mode = mode;
}

void mpl_compiler::create_doc(char *text_p, int is_forward)
{
    doc *doc_p;

    if (!is_forward) {
        if (current_doc_list_target_pp == NULL) {
            fprintf(stderr,
                    "%s:%d: Backward documentation entry not allowed in file scope.\n",
                    peek_filename(), lineno()
                   );
            exit(-1);
        }

        if ((doc_mode == doc_mode_normal) && (*current_doc_list_target_pp != NULL)) {
            fprintf(stderr,
                    "%s:%d: Documentation entry already exists for this item:\n"
                    "Old:\n'%s'\n"
                    "New:\n'%s'\n",
                    peek_filename(), lineno(),
                    LISTABLE_PTR(*current_doc_list_target_pp, doc)->text_p,
                    text_p
                   );
            exit(-1);
        }

        doc_p = new doc(strdup(text_p), 0);
        doc_p->append_to(current_doc_list_target_pp);
        return;
    } else {
        if (peek_block() == NULL) {
            /* File scope */
            if (current_doc_list_p != NULL) {
                /* Add current (previous) entry to file */
                add_file_doc();
                current_doc_list_p = NULL;
            }
        }

        doc_p = new doc(strdup(text_p), 1);
        doc_p->append_to(current_doc_list_p);
    }
}

void mpl_compiler::set_backward_doc_target(mpl_list_t **doc_list_pp)
{
    current_doc_list_target_pp = doc_list_pp;
}


void mpl_compiler::add_any_forward_doc(mpl_list_t **doc_list_pp)
{
    if (current_doc_list_p) {
        mpl_list_append(doc_list_pp, current_doc_list_p);
        current_doc_list_p = NULL;
    } else {
        set_backward_doc_target(doc_list_pp);
    }
}

void mpl_compiler::add_file_doc()
{
    file *file_p = current_file();
    if (file_p && current_doc_list_p)
        mpl_list_append(&file_p->doc_list_p, current_doc_list_p);
}

void mpl_compiler::reset_doc_target()
{
    current_doc_list_target_pp = NULL;
    current_doc_list_p = NULL;
}

void mpl_compiler::add_x_entry(const char *label_p, char *value_p, char *text_p)
{
    x_entry *x_entry_p;

    assert((parser_mode == parser_mode_dox) ||
           (parser_mode == parser_mode_latex) ||
           (parser_mode == parser_mode_help));

    if (parser_mode == parser_mode_dox) {
        if (current_dox_list_p == NULL){
            fprintf(stderr,
                    "%s:%d: No current dox list\n",
                    peek_filename(), lineno()
                   );
            exit(-1);
        }
        x_entry_p = LISTABLE_PTR(mpl_list_last(current_dox_list_p), dox_entry);
    }
    else if (parser_mode == parser_mode_latex) {
        if (current_latex_list_p == NULL){
            fprintf(stderr,
                    "%s:%d: No current latex list\n",
                    peek_filename(), lineno()
                   );
            exit(-1);
        }
        x_entry_p = LISTABLE_PTR(mpl_list_last(current_latex_list_p), latex_entry);
    }
    else {
        if (current_help_list_p == NULL){
            fprintf(stderr,
                    "%s:%d: No current help list\n",
                    peek_filename(), lineno()
                   );
            exit(-1);
        }
        x_entry_p = LISTABLE_PTR(mpl_list_last(current_help_list_p), help_entry);
    }


    if (x_entry_p->label_p) {
        x_entry *new_x_entry_p;
        if (parser_mode == parser_mode_dox) {
            new_x_entry_p = new dox_entry(*((dox_entry*)x_entry_p));
            new_x_entry_p->clear_info();
            new_x_entry_p->append_to(current_dox_list_p);
            x_entry_p = new_x_entry_p;
        }
        else if (parser_mode == parser_mode_latex) {
            new_x_entry_p = new latex_entry(*((latex_entry*)x_entry_p));
            new_x_entry_p->clear_info();
            new_x_entry_p->append_to(current_latex_list_p);
            x_entry_p = new_x_entry_p;
        }
        else {
            new_x_entry_p = new help_entry(*((help_entry*)x_entry_p));
            new_x_entry_p->clear_info();
            new_x_entry_p->append_to(current_help_list_p);
            x_entry_p = new_x_entry_p;
        }
    }

    x_entry_p->add_info(parser_mode, label_p, value_p, text_p);
}

void mpl_compiler::push_block(block_type_t block_type)
{
    block *block_p = new block(this, block_type);
    block_p->add_to(blocks_p);
}

block *mpl_compiler::pop_block()
{
    mpl_list_t *tmp_p;

    if (blocks_p == NULL)
        return NULL;

    tmp_p = mpl_list_remove(&blocks_p, NULL);
    return (LISTABLE_PTR(tmp_p, block));
}

block *mpl_compiler::peek_block()
{
    if (blocks_p == NULL)
        return NULL;

    return (LISTABLE_PTR(blocks_p, block));
}

int mpl_compiler::is_inside_block(block_type_t block_type)
{
    if (blocks_p == NULL)
        return 0;

    mpl_list_t *tmp_p;
    block *block_p;

    MPL_LIST_FOR_EACH(blocks_p, tmp_p) {
        block_p = LISTABLE_PTR(tmp_p, block);
        if (block_p->block_type == block_type)
            return 1;
    }
    return 0;
}

void mpl_compiler::do_block_start(block_type_t block_type)
{
    push_block(block_type);

    switch (block_type) {
        case block_type_category:
            break;
        case block_type_parameters_in_file_scope:
            add_any_forward_doc(&current_parameter_set_p->current_parameter_group_p->doc_list_p);
            break;
        case block_type_parameters_in_block_scope:
            current_parameter_set_p->wrap_up_definition();
            current_parameter_set_p->use_parameter_group(NULL);
            add_any_forward_doc(&current_parameter_set_p->current_parameter_group_p->doc_list_p);
            break;
        case block_type_commands:
            add_any_forward_doc(&current_category_p->commands.doc_list_p);
            set_doc_mode(doc_mode_commands_or_events);
            break;
        case block_type_events:
            add_any_forward_doc(&current_category_p->events.doc_list_p);
            set_doc_mode(doc_mode_commands_or_events);
            break;
        case block_type_parameter_set:
            break;
        case block_type_command_bag:
            break;
        case block_type_response_bag:
            break;
        case block_type_event_bag:
            break;
        case block_type_enum:
            break;
        case block_type_bag:
            break;
        case block_type_enumerator_list:
            break;
        default:
            break;
    }
}

void mpl_compiler::do_block_end()
{
    block *last_block_p = pop_block();

    if (last_block_p == NULL){
        return;
    }

    switch (last_block_p->block_type) {
        case block_type_category:
            current_category_p->wrap_up_definition();
            reset_doc_target();
            reset_current_category();
            reset_current_parameter_set();
            break;
        case block_type_parameters_in_file_scope:
            current_parameter_set_p->wrap_up_definition();
            reset_doc_target();
            reset_current_parameter_list();
            reset_current_parameter_set();
            break;
        case block_type_parameters_in_block_scope:
            current_parameter_set_p->wrap_up_definition();
            reset_doc_target();
            reset_current_parameter_list();
            break;
        case block_type_commands:
            reset_doc_target();
            set_doc_mode(doc_mode_normal);
            break;
        case block_type_events:
            reset_doc_target();
            set_doc_mode(doc_mode_normal);
            break;
        case block_type_parameter_set:
            current_parameter_set_p->wrap_up_definition();
            reset_current_parameter_set();
            break;
        case block_type_command_bag:
            current_category_p->command_spec_p->message_bag_p->wrap_up_definition();
            break;
        case block_type_response_bag:
            current_category_p->response_spec_p->message_bag_p->wrap_up_definition();
            break;
        case block_type_event_bag:
            current_category_p->event_spec_p->message_bag_p->wrap_up_definition();
            break;
        case block_type_enum:
            current_parameter_p->wrap_up_definition();
            break;
        case block_type_bag:
            current_parameter_p->wrap_up_definition();
            break;
        case block_type_enumerator_list:
            break;
        default:
            break;
    }

    delete last_block_p;
}

void mpl_compiler::gc_h_header(FILE *f, char *out_name_p)
{
    char *on_p = fix_for_header_define(out_name_p);
    if (codegen_mode == codegen_mode_mpl) {
        fprintf(f, "#ifndef mplcomp_%s_h\n", on_p);
        fprintf(f, "#define mplcomp_%s_h\n", on_p);
    }
    else if (codegen_mode == codegen_mode_api) {
        fprintf(f, "#ifndef mplcomp_%s_hh\n", on_p);
        fprintf(f, "#define mplcomp_%s_hh\n", on_p);
    }
    else {
        fprintf(f, "#ifndef mplcomp_%s_cli_h\n", on_p);
        fprintf(f, "#define mplcomp_%s_cli_h\n", on_p);
    }
    free(on_p);

    fprintf(f, "\n");
    fprintf(f, "/* NOTE: THIS IS A GENERATED FILE - DO NOT EDIT */\n\n");
    fprintf(f, "/* For copyright and license information, see separate file(s). */\n\n");

    fprintf(f,
            "#include <assert.h>\n"
           );
    fprintf(f,
            "#include \"mpl_param.h\"\n"
           );
    if (codegen_mode != codegen_mode_api) {
        fprintf(f,
                "#ifdef  __cplusplus\n"
                "extern \"C\" {\n"
                "#endif\n"
               );
    }
    fprintf(f, "\n");

    if (codegen_mode == codegen_mode_cli) {
        fprintf(f, "#include \"%s.h\"\n", out_name_p);
        fprintf(f,
                "typedef int (*completion_func_t)(const char  *line, int *posindex, char *completionStrings[], int maxStrings);\n"
                "typedef struct {\n"
                "    completion_func_t func;\n"
                "    mpl_list_t list_entry;\n"
                "} completion_callstack_entry_t;\n"
                "extern mpl_list_t *completionCallstack;\n"
                "char *formatCmdLine(const char *line, char *cmd_suffix);\n"
               );
    }
    else if (codegen_mode == codegen_mode_api) {
        fprintf(f, "#include \"%s.h\"\n", out_name_p);
        fprintf(f, "#include <wchar.h>\n");
    }
    else {
        gc_lines(f, hlines_p, code_segment_top, codegen_mode);
    }
}

void mpl_compiler::gc_c_header(FILE *f, char *out_name_p)
{
    fprintf(f, "\n");
    fprintf(f, "/* NOTE: THIS IS A GENERATED FILE - DO NOT EDIT */\n\n");
    fprintf(f, "/* For copyright and license information, see separate file(s). */\n\n");
    fprintf(f, "\n");
    fprintf(f, "#include <stdlib.h>\n");
    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include \"mpl_param.h\"\n");
    if (codegen_mode == codegen_mode_api) {
        fprintf(f, "#include \"%s.hh\"\n", out_name_p);
    }
    else {
        fprintf(f, "#include \"%s.h\"\n", out_name_p);
    }
    if (codegen_mode == codegen_mode_cli) {
        fprintf(f, "#include \"%s_cli.h\"\n", out_name_p);
        fprintf(f,
                "#include <ctype.h>\n"
                "mpl_list_t *completionCallstack;\n"
                "static char value_help[256];\n"
               );
        cli_c_common(f);
    }

    fprintf(f, "\n");

    if (codegen_mode == codegen_mode_mpl) {
        gc_lines(f, clines_p, code_segment_top, codegen_mode);
    }
}

void mpl_compiler::cli_c_common(FILE *f)
{
    fprintf(f,
            "static const char *strchr_escape(const char *s, char c, char escape)\n"
            "{\n"
            "    const char *p = s;\n"
            "    const char *e;\n"
            "\n"
            "    while (p && strlen(p)) {\n"
            "    p = strchr(p, c);\n"
            "    // did we find an escaped character?\n"
            "    e = p;\n"
            "    while (e && (e > s) && (*(e-1) == escape))\n"
            "        e--;\n"
            "    if (1 == ((p - e) %% 2)) {\n"
            "        // An odd number of escapes in a row: yes\n"
            "        p++;\n"
            "        continue;\n"
            "    }\n"
            "    else\n"
            "        break;\n"
            "    }\n"
            "    return p;\n"
            "}\n"
           );

    fprintf(f,
            "static const char *get_matching_close_bracket(char open_bracket, char close_bracket, const char *str_p, char escape)\n"
            "{\n"
            "    int num_open = 0;\n"
            "    const char *p;\n"
            "    p = str_p;\n"
            "    if (*p != open_bracket)\n"
            "        return NULL;\n"
            "    while (*p) {\n"
            "        if (*p == escape) {\n"
            "            p++;\n"
            "            continue;\n"
            "        }\n"
            "        if (*p == open_bracket)\n"
            "            num_open++;\n"
            "        if (*p == close_bracket)\n"
            "            num_open--;\n"
            "        if (num_open == 0)\n"
            "            return p;\n"
            "        p++;\n"
            "    }\n"
            "    return NULL;\n"
            "}\n"
           );

    fprintf(f,
            "static const char* param_end(const char *s)\n"
            "{\n"
            "    const char *bracket_start_pos = NULL;\n"
            "    const char *bracket_end_pos = NULL;\n"
            "    const char *spacepos = strchr_escape(s, ' ', '\\\\');\n"
            "    if (*s == '\\0')\n"
            "        return s;\n"
            "    if (*s == '{')\n"
            "        bracket_start_pos = s;\n"
            "    if (bracket_start_pos != NULL)\n"
            "        bracket_end_pos = get_matching_close_bracket('{', '}', bracket_start_pos, '\\\\');\n"
            "    if (bracket_end_pos != NULL)\n"
            "        return bracket_end_pos;\n"
            "    if (*s == '\\\"')\n"
            "        bracket_end_pos = strchr_escape(s+1, '\\\"', '\\\\');\n"
            "    if (bracket_end_pos != NULL)\n"
            "        spacepos = strchr_escape(bracket_end_pos, ' ', '\\\\');\n"
            "    if (spacepos == NULL)\n"
            "        return (s + strlen(s) + 1);\n"
            "    return spacepos;\n"
            "}\n"
           );

    fprintf(f,
            "static void push_stack(completion_func_t func)\n"
            "{\n"
            "    completion_callstack_entry_t *e = calloc(1, sizeof(*e));\n"
            "    e->func = func;\n"
            "    mpl_list_add(&completionCallstack, &e->list_entry);\n"
            "}\n"
           );
    fprintf(f,
            "static completion_func_t pop_stack()\n"
            "{\n"
            "    completion_func_t func;\n"
            "    mpl_list_t *tmp;\n"
            "    completion_callstack_entry_t *e;\n"
            "    tmp = mpl_list_remove(&completionCallstack, NULL);\n"
            "    if (tmp == NULL)\n"
            "        return NULL;\n"
            "    e = MPL_LIST_CONTAINER(tmp, completion_callstack_entry_t, list_entry);\n"
            "    func = e->func;\n"
            "    free(e);\n"
            "    return func;\n"
            "}\n"
           );

    fprintf(f,
            "static const char *get_word_start(const char *start_limit, const char *pos)\n"
            "{\n"
            "    while ((pos > start_limit) && !isspace(*pos))\n"
            "        pos--;\n"
            "    if (isspace(*pos))\n"
            "        pos++;\n"
            "    return pos;\n"
            "}\n"
           );

    fprintf(f,
            "static const char *get_value_start(const char *start_limit, const char *pos)\n"
            "{\n"
            "    while ((pos > start_limit) && (*pos != '='))\n"
            "        pos--;\n"
            "    if (isspace(*pos) || (*pos == '='))\n"
            "        pos++;\n"
            "    return pos;\n"
            "}\n"
           );

    fprintf(f,
            "static char *create_completion_string(const char *line, const char *pos, const char *name, const char *type, int multiple)\n"
            "{\n"
            "    char *str;\n"
            "    int newlen = strlen(line) + strlen(name) + (multiple ? 4 : 1);\n"
            "    if (!strcmp(type, \"bag\"))\n"
            "        newlen++;\n"
            "    else if (!strcmp(type, \"string\"))\n"
            "        newlen++;\n"
            "    else if (!strcmp(type, \"value\"))\n"
            "        newlen--;\n"
            "    str = calloc(1,newlen+1);\n"
            "    if (line[strlen(line)-1] == ' ' && ((line[strlen(line)-2] == '{') || (!strcmp(type,\"value\") && (line[strlen(line)-2] == '='))))\n"
            "        strncat(str, line, (pos - line) - 1);\n"
            "    else\n"
            "        strncat(str, line, pos - line);\n"
            "    strcat(str, name);\n"
            "    if (multiple) {\n"
            "        int index = 1;\n"
            "        const char *haystack;\n"
            "        char *needle = calloc(1, strlen(name) + 2);\n"
            "        sprintf(needle, \"%%s[\", name);\n"
            "        haystack = line;\n"
            "        while ((haystack = strstr(haystack, needle)) != NULL) {\n"
            "            index++;\n"
            "            haystack += strlen(needle);\n"
            "        }\n"
            "        free(needle);\n"
            "        sprintf(str + strlen(str), \"[%%d]\", index);\n"
            "    }\n"
            "    if (strcmp(type, \"value\"))\n"
            "        strcat(str, \"=\");\n"
            "    if (!strcmp(type, \"bag\"))\n"
            "        strcat(str, \"{\");\n"
            "    return str;\n"
            "}\n"
           );

    fprintf(f,
            "static void free_completion_strings(char *completionStrings[], int numStrings)\n"
            "{\n"
            "    int i;\n"
            "    for (i = 0; i < numStrings; i++) {\n"
            "        free(completionStrings[i]);\n"
            "        completionStrings[i] = NULL;\n"
            "    }\n"
            "}\n"
           );
    fprintf(f,
            "static completion_func_t pop_all_bag_ends(const char *line, const char **pos, int *posindex, const char *parend)\n"
            "{\n"
            "    completion_func_t func;\n"
            "    func = pop_stack();\n"
            "    *pos = parend + 1;\n"
            "    while (isspace(**pos) || (**pos == '}')) {\n"
            "        if (**pos == '}')\n"
            "            func = pop_stack();\n"
            "        (*pos)++;\n"
            "    }\n"
            "    *posindex = *pos - line;\n"
            "    return func;\n"
            "}\n"
           );
    fprintf(f,
            "static void forward_to_param_start(const char **pos)\n"
            "{\n"
            "    if (isspace(**pos) || (**pos == '{'))\n"
            "        (*pos)++;\n"
            "}\n"
           );
    fprintf(f,
            "static void backward_to_param_start(const char *start_limit, const char **pos)\n"
            "{\n"
            "    if (!isspace(**pos)) {\n"
            "        while ((*pos > start_limit) && !isspace(**pos))\n"
            "            (*pos)--;\n"
            "    }\n"
            "}\n"
           );

    fprintf(f,
            "char *formatCmdLine(const char *line, char *cmd_suffix)\n"
            "{\n"
            "    const char *p;\n"
            "    const char *s;\n"
            "    char *cmd = calloc(1, strlen(line) * 2);\n"
            "    p = strchr(line, ' ');\n"
            "    if (p) {\n"
            "        strncpy(cmd, line, p - line);\n"
            "        strcat(cmd, \"_\");\n"
            "        strcat(cmd, cmd_suffix);\n"
            "        strcat(cmd, \"={\");\n"
            "        while (*p == ' ')\n"
            "            p++;\n"
            "        do {\n"
            "            s = strchr(p,'=');\n"
            "            if (s == NULL)\n"
            "                break;\n"
            "            s++;\n"
            "            strncat(cmd,p,s-p);\n"
            "            while (*s == ' ')\n"
            "                s++;\n"
            "            p = s;\n"
            "            if (*s == '{')\n"
            "                continue;\n"
            "            if (s == NULL)\n"
            "                break;\n"
            "            s = strchr_escape(s,' ', '\\\\');\n"
            "            if (s == NULL)\n"
            "                break;\n"
            "            strncat(cmd,p,s-p);\n"
            "            s++;\n"
            "            while (*s == ' ')\n"
            "                s++;\n"
            "            if (*(s+1) == '}') {\n"
            "                p = s;\n"
            "                continue;\n"
            "            }\n"
            "            strcat(cmd,\",\");\n"
            "            p = s;\n"
            "        } while (*p);\n"
            "        strncat(cmd,p,strlen(p));\n"
            "        strcat(cmd, \"}\");\n"
            "    }\n"
            "    else {\n"
            "        strcpy(cmd, line);\n"
            "        strcat(cmd, \"_\");\n"
            "        strcat(cmd, cmd_suffix);\n"
            "    }\n"
            "    return cmd;\n"
            "}\n"
           );

    fprintf(f,
            "static int get_bool_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings)\n"
            "{\n"
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

    fprintf(f,
            "    if (!strncmp(\"true \",p,%zd + 1) || !strncmp(\"1 \",p,2)) {\n"
            "        completion_func_t func = pop_stack();\n"
            "        assert(func != NULL);\n"
            "        *posindex = param_end(pos) - line + 1;\n"
            "        return func(line, posindex, completionStrings, maxStrings);\n"
            "    }\n",
            strlen("true")
           );
    fprintf(f,
            "    if (!strncmp(\"true\",p,strlen(p)) || !strncmp(\"1\",p,strlen(p))) {\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, \"true\", \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "    }\n"
           );
    fprintf(f,
            "    if (!strncmp(\"false \",p,%zd + 1) || !strncmp(\"0 \",p,2)) {\n"
            "        completion_func_t func = pop_stack();\n"
            "        assert(func != NULL);\n"
            "        *posindex = param_end(pos) - line + 1;\n"
            "        return func(line, posindex, completionStrings, maxStrings);\n"
            "    }\n",
            strlen("false")
           );
    fprintf(f,
            "    if (!strncmp(\"false\",p,strlen(p)) || !strncmp(\"0\",p,strlen(p))) {\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, \"false\", \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "    }\n"
           );

    fprintf(f,
            "do_exit:\n"
            "    *posindex = strlen(line);\n"
            "    return numStrings;\n"
           );
    fprintf(f,
            "}\n"
            "\n"
           );

    fprintf(f,
            "static int get_int_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings)\n"
            "{\n"
           );
    fprintf(f,
            "    int numStrings = 0;\n"
            "    const char *pos = &line[*posindex];\n"
            "    const char *p = pos;\n"
            "    const char *d;\n"
           );
    fprintf(f,
            "    assert(pos > line);\n"
            "    p = get_value_start(pos, p);\n"
           );

    fprintf(f,
            "    d = p;\n"
            "    if ((*d == '-') && isdigit(*(d+1)))\n"
            "        d++;"
            "    while (isdigit(*d))\n"
            "        d++;\n"
            "    if (*d == ' ') {\n"
            "        completion_func_t func = pop_stack();\n"
            "        assert(func != NULL);\n"
            "        *posindex = param_end(pos) - line + 1;\n"
            "        return func(line, posindex, completionStrings, maxStrings);\n"
            "    }\n",
            strlen("true")
           );
    fprintf(f,
            "    if (!strcmp(pos, \" \") && !strlen(p)) {\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, value_help, \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, \"\", \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "    }\n"
           );

    fprintf(f,
            "do_exit:\n"
            "    *posindex = strlen(line);\n"
            "    return numStrings;\n"
           );
    fprintf(f,
            "}\n"
            "\n"
           );

    fprintf(f,
            "static int get_array_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings)\n"
            "{\n"
           );
    fprintf(f,
            "    int numStrings = 0;\n"
            "    const char *pos = &line[*posindex];\n"
            "    const char *p = pos;\n"
            "    const char *d;\n"
           );
    fprintf(f,
            "    assert(pos > line);\n"
            "    p = get_value_start(pos, p);\n"
           );

    fprintf(f,
            "    d = p;\n"
            "    while (isxdigit(*d))\n"
            "        d++;\n"
            "    if (*d == ' ') {\n"
            "        completion_func_t func = pop_stack();\n"
            "        assert(func != NULL);\n"
            "        *posindex = param_end(pos) - line + 1;\n"
            "        return func(line, posindex, completionStrings, maxStrings);\n"
            "    }\n",
            strlen("true")
           );
    fprintf(f,
            "    if (!strcmp(pos, \" \") && !strlen(p)) {\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, value_help, \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, \"\", \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "    }\n"
           );

    fprintf(f,
            "do_exit:\n"
            "    *posindex = strlen(line);\n"
            "    return numStrings;\n"
           );
    fprintf(f,
            "}\n"
            "\n"
           );

    fprintf(f,
            "static int get_addr_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings)\n"
            "{\n"
           );
    fprintf(f,
            "    int numStrings = 0;\n"
            "    const char *pos = &line[*posindex];\n"
            "    const char *p = pos;\n"
            "    const char *d;\n"
           );
    fprintf(f,
            "    assert(pos > line);\n"
            "    p = get_value_start(pos, p);\n"
           );

    fprintf(f,
            "    d = p;\n"
            "    if (*d == '0')\n"
            "        d++;\n"
            "    if ((*d == 'x') || (*d == 'X'))\n"
            "        d++;\n"
            "    while (isxdigit(*d))\n"
            "        d++;\n"
            "    if (*d == ' ') {\n"
            "        completion_func_t func = pop_stack();\n"
            "        assert(func != NULL);\n"
            "        *posindex = param_end(pos) - line + 1;\n"
            "        return func(line, posindex, completionStrings, maxStrings);\n"
            "    }\n",
            strlen("true")
           );
    fprintf(f,
            "    if (!strcmp(pos, \" \") && !strlen(p)) {\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, value_help, \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, \"\", \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "    }\n"
           );

    fprintf(f,
            "do_exit:\n"
            "    *posindex = strlen(line);\n"
            "    return numStrings;\n"
           );
    fprintf(f,
            "}\n"
            "\n"
           );

    fprintf(f,
            "static int get_tuple_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings)\n"
            "{\n"
           );
    fprintf(f,
            "    int numStrings = 0;\n"
            "    const char *pos = &line[*posindex];\n"
            "    const char *p = pos;\n"
            "    const char *d;\n"
           );
    fprintf(f,
            "    assert(pos > line);\n"
            "    p = get_value_start(pos, p);\n"
           );

    fprintf(f,
            "    d = p;\n"
            "    if (!strncmp(value_help,\"string_tuple\",strlen(\"string_tuple\"))) {\n"
            "        while (*d && (*d != ':')) {\n"
            "            if (*d == '\\\\')\n"
            "                d++;\n"
            "            if (isprint(*d))\n"
            "                d++;\n"
            "        }\n"
            "        if (*d == ':') {\n"
            "            while (*d && (*d != ' ')) {\n"
            "                if (*d == '\\\\')\n"
            "                    d++;\n"
            "                if (isprint(*d))\n"
            "                    d++;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    else if (!strncmp(value_help,\"int_tuple\",strlen(\"int_tuple\"))) {\n"
            "        while (*d && (*d != ':')) {\n"
            "            if (isdigit(*d))\n"
            "                d++;\n"
            "        }\n"
            "        if (*d == ':') {\n"
            "            d++;\n"
            "            while (*d && (*d != ' ')) {\n"
            "                if (isdigit(*d))\n"
            "                    d++;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    else if (!strncmp(value_help,\"strint_tuple\",strlen(\"strint_tuple\"))) {\n"
            "        while (*d && (*d != ':')) {\n"
            "            if (*d == '\\\\')\n"
            "                d++;\n"
            "            if (isprint(*d))\n"
            "                d++;\n"
            "        }\n"
            "        if (*d == ':') {\n"
            "            d++;\n"
            "            while (*d && (*d != ' ')) {\n"
            "                if (isdigit(*d))\n"
            "                    d++;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    else if (!strncmp(value_help,\"struint8_tuple\",strlen(\"struint8_tuple\"))) {\n"
            "        while (*d && (*d != '/')) {\n"
            "            if (*d == '\\\\')\n"
            "                d++;\n"
            "            if (isprint(*d))\n"
            "                d++;\n"
            "        }\n"
            "        if (*d == '/') {\n"
            "            d++;\n"
            "            while (*d && (*d != ' ')) {\n"
            "                if (isdigit(*d))\n"
            "                    d++;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    if (*d == ' ') {\n"
            "        completion_func_t func = pop_stack();\n"
            "        assert(func != NULL);\n"
            "        *posindex = param_end(pos) - line + 1;\n"
            "        return func(line, posindex, completionStrings, maxStrings);\n"
            "    }\n",
            strlen("true")
           );
    fprintf(f,
            "    if (!strcmp(pos, \" \") && !strlen(p)) {\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, value_help, \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, \"\", \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "    }\n"
           );

    fprintf(f,
            "do_exit:\n"
            "    *posindex = strlen(line);\n"
            "    return numStrings;\n"
           );
    fprintf(f,
            "}\n"
            "\n"
           );

    fprintf(f,
            "static int get_string_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings)\n"
            "{\n"
           );
    fprintf(f,
            "    int numStrings = 0;\n"
            "    const char *pos = &line[*posindex];\n"
            "    const char *p = pos;\n"
            "    const char *d;\n"
           );
    fprintf(f,
            "    assert(pos > line);\n"
            "    p = get_value_start(pos, p);\n"
           );

    fprintf(f,
            "    d = p;\n"
            "    if (*d == '\"') {\n"
            "        d = strchr_escape(d+1, '\"', '\\\\');\n"
            "        if (d == NULL)\n"
            "            d = p;\n"
            "        else\n"
            "            d++;\n"
            "    }\n"
            "    else {\n"
            "        while (!isspace(*d)) {\n"
            "            if (*d == '\\\\')\n"
            "                d++;\n"
            "            d++;\n"
            "        }\n"
            "    }\n"
            "    if (*d == ' ') {\n"
            "        completion_func_t func = pop_stack();\n"
            "        assert(func != NULL);\n"
            "        *posindex = param_end(pos) - line + 1;\n"
            "        return func(line, posindex, completionStrings, maxStrings);\n"
            "    }\n",
            strlen("true")
           );
    fprintf(f,
            "    if (!strcmp(pos, \"\\\" \")) {\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, value_help, \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "        completionStrings[numStrings++] = create_completion_string(line, p, \"\\\"\", \"value\", 0);\n"
            "        if (numStrings >= maxStrings) goto do_exit;\n"
            "    }\n"
           );

    fprintf(f,
            "do_exit:\n"
            "    *posindex = strlen(line);\n"
            "    return numStrings;\n"
           );
    fprintf(f,
            "}\n"
            "\n"
           );

    fprintf(f,
            "\n"
            "#define get_wstring_completions get_array_completions\n"
            "\n"
           );

}

void mpl_compiler::gc_h_footer(FILE *f, char *out_name_p)
{
    if (codegen_mode == codegen_mode_mpl) {
        gc_lines(f, hlines_p, code_segment_prototypes, codegen_mode);
    }
    if (codegen_mode != codegen_mode_api) {
        fprintf(f,
                "#ifdef  __cplusplus\n"
                "}\n"
                "#endif\n"
               );
    }
    char *on_p = fix_for_header_define(out_name_p);
    if (codegen_mode == codegen_mode_mpl)
        fprintf(f, "\n#endif /* mplcomp_%s_h */\n\n", on_p);
    else if (codegen_mode == codegen_mode_api)
        fprintf(f, "\n#endif /* mplcomp_%s_hh */\n\n", on_p);
    else
        fprintf(f, "\n#endif /* mplcomp_%s_cli_h */\n\n", on_p);
    free(on_p);
}

void mpl_compiler::gc_parameter_sets(FILE *hfile_p, FILE *cfile_p)
{
    mpl_list_t *tmp_p;
    parameter_set *parameter_set_p;

    MPL_LIST_FOR_EACH(parameter_sets_p, tmp_p) {
        parameter_set_p = LISTABLE_PTR(tmp_p, parameter_set);
        parameter_set_p->gc_h(hfile_p);
        parameter_set_p->gc_c(cfile_p);
    }
}

void mpl_compiler::gc_categories(FILE *hfile_p, FILE *cfile_p)
{
    mpl_list_t *tmp_p;
    category *category_p;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);
        category_p->gc(hfile_p, cfile_p, categories_p);
    }

}

bool mpl_compiler::bag_is_command(bag_parameter *bag_p)
{
    mpl_list_t *tmp_p;
    category *category_p;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);

        mpl_list_t *tmp_p;
        command *command_p;
        MPL_LIST_FOR_EACH(category_p->commands.method_list_p, tmp_p) {
            command_p = LISTABLE_PTR(tmp_p, command);
            if (command_p->command_bag_p == bag_p)
                return true;
        }
    }
    return false;
}

bool mpl_compiler::param_is_command_parameter(parameter *param_p)
{
    mpl_list_t *tmp_p;
    category *category_p;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);

        mpl_list_t *tmp_p;
        command *command_p;
        MPL_LIST_FOR_EACH(category_p->commands.method_list_p, tmp_p) {
            command_p = LISTABLE_PTR(tmp_p, command);
            if (command_p->command_bag_p->param_is_field_or_subfield(param_p))
                return true;
        }
    }
    return false;
}

bool mpl_compiler::bag_is_response(bag_parameter *bag_p)
{
    mpl_list_t *tmp_p;
    category *category_p;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);

        mpl_list_t *tmp_p;
        command *command_p;
        MPL_LIST_FOR_EACH(category_p->commands.method_list_p, tmp_p) {
            command_p = LISTABLE_PTR(tmp_p, command);
            if (command_p->response_bag_p == bag_p)
                return true;
        }
    }
    return false;
}

bool mpl_compiler::bag_is_event(bag_parameter *bag_p)
{
    mpl_list_t *tmp_p;
    category *category_p;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);

        mpl_list_t *tmp_p;
        event *event_p;
        MPL_LIST_FOR_EACH(category_p->events.method_list_p, tmp_p) {
            event_p = LISTABLE_PTR(tmp_p, event);
            if (event_p->event_bag_p == bag_p)
                return true;
        }
    }
    return false;
}

void mpl_compiler::deja_common(FILE *f)
{
    fprintf(f,
            "# This is a generated file - DO NOT EDIT!\n"
           );
    fprintf(f,
            "#\n"
            "# The code here requires that a \"glue layer\" with\n"
            "# the following functions exists:\n"
            "#\n"
            "#   send_Cmd { msg } { ... }\n"
            "#   sync_Cmd { {syncPattern <defaultSyncPattern>} } { ... }\n"
            "#   get_Msg { msgName }\n"
            "#   check_Value { name value msg {tag \"\"} } { ... }\n"
            "#   get_Value { name msg {tag \"\"} } { ... }\n"
            "#   get_CompoundValue { name msg {tag \"\"} } { ... }\n"
            "#\n"
           );

    fprintf(f,
            "proc pack_multiple { pack_func mdict {field \"\"}} {\n"
            "    set pstr \"\"\n"
            "    set sep \"\"\n"
            "    dict for {tag val} $mdict {\n"
            "        set pstr \"$pstr$sep[$pack_func $val $field $tag]\"\n"
            "        set sep \",\"\n"
            "    }\n"
            "    return \"$pstr\"\n"
            "}\n"
           );

    fprintf(f,
            "proc validate_dict_elements { new_dict good_dict } {\n"
            "    dict for {key val} $new_dict {\n"
            "        if {![dict exists $good_dict $key]} {\n"
            "            return $key\n"
            "        }\n"
            "    }\n"
            "    return \"\"\n"
            "}\n"
           );

    fprintf(f,
            "proc check_multiple { check_func mdict msg {field \"\"} {is_bag false}} {\n"
            "    set resDict [dict create]\n"
            "    dict for {tag val} $mdict {\n"
            "        if {$tag >= 0} {\n"
            "            if {$is_bag} {\n"
            "                set entry [$check_func \"\" $msg $field $tag]\n"
            "                set match_tag \"\"\n"
            "            } else {\n"
            "                set entry $msg\n"
            "                set match_tag $tag\n"
            "            }\n"
            "        } else {\n"
            "            set entry $msg\n"
            "            set match_tag \"\"\n"
            "        }\n"
            "        dict set resDict $tag [$check_func $val $entry $field $match_tag]\n"
            "    }\n"
            "    return $resDict\n"
            "}\n"
           );

    fprintf(f,
            "proc fill_escape { value {char ,} {esc \\\\} } {\n"
            "    set len [string length $value]\n"
            "    set res \"\"\n"
            "    for {set i 0} {$i < $len} {incr i} {\n"
            "        set thischar [string range $value $i $i]\n"
            "            if {$thischar == $char} {\n"
            "                set res \"$res$esc\"\n"
            "            }\n"
            "        set res \"$res$thischar\"\n"
            "    }\n"
            "    return $res\n"
            "}\n"
           );

    fprintf(f,
            "proc pack_uint8_array { value } {\n"
            "    return $value\n"
            "}\n"
           );
    fprintf(f,
            "proc pack_uint16_array { value } {\n"
            "    return $value\n"
            "}\n"
           );
    fprintf(f,
            "proc pack_uint32_array { value } {\n"
            "    return $value\n"
            "}\n"
           );

    fprintf(f,
            "proc get_uint8_array { name msg {tag \"\"} } {\n"
            "    return [get_Value $name $msg $tag]\n"
            "}\n"
           );
    fprintf(f,
            "proc get_uint16_array { name msg {tag \"\"} } {\n"
            "    return [get_Value $name $msg $tag]\n"
            "}\n"
           );
    fprintf(f,
            "proc get_uint32_array { name msg {tag \"\"} } {\n"
            "    return [get_Value $name $msg $tag]\n"
            "}\n"
           );

    fprintf(f,
            "proc check_uint8_array { name value msg {tag \"\"} } {\n"
            "    return [check_Value $name $value $msg $tag]\n"
            "}\n"
           );
    fprintf(f,
            "proc check_uint16_array { name value msg {tag \"\"} } {\n"
            "    return [check_Value $name $value $msg $tag]\n"
            "}\n"
           );
    fprintf(f,
            "proc check_uint32_array { name value msg {tag \"\"} } {\n"
            "    return [check_Value $name $value $msg $tag]\n"
            "}\n"
           );
}

void mpl_compiler::deja_parameter_sets(FILE *expect_file_p)
{
    mpl_list_t *tmp_p;
    parameter_set *parameter_set_p;

    MPL_LIST_FOR_EACH(parameter_sets_p, tmp_p) {
        parameter_set_p = LISTABLE_PTR(tmp_p, parameter_set);
        parameter_set_p->deja(expect_file_p);
    }
}

void mpl_compiler::deja_categories(FILE *expect_file_p)
{
    mpl_list_t *tmp_p;
    category *category_p;
    char indent[255] = {0};

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);
        category *top_category_p = category_p->get_topmost_category();
        if (top_category_p != category_p) {
            fprintf(expect_file_p,
                    "namespace eval %s {\n",
                    top_category_p->name_p
                   );
            sprintf(indent,"    ");
        }
        category_p->deja(expect_file_p, indent);
        if (top_category_p != category_p) {
            fprintf(expect_file_p,
                    "}\n"
                   );
        }
    }
}

void mpl_compiler::dox_categories(FILE *f)
{
    mpl_list_t *tmp_p;
    category *category_p;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);
        category_p->dox(f);
    }
}

void mpl_compiler::dox_parameter_sets(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_set *parameter_set_p;

    MPL_LIST_FOR_EACH(parameter_sets_p, tmp_p) {
        parameter_set_p = LISTABLE_PTR(tmp_p, parameter_set);
        parameter_set_p->dox(f);
    }
}

void mpl_compiler::latex_categories(FILE *f)
{
    mpl_list_t *tmp_p;
    category *category_p;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);
        category_p->latex(f);
    }
}

void mpl_compiler::latex_parameter_sets(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter_set *parameter_set_p;

    MPL_LIST_FOR_EACH(parameter_sets_p, tmp_p) {
        parameter_set_p = LISTABLE_PTR(tmp_p, parameter_set);
        parameter_set_p->latex(f);
    }
}

void mpl_compiler::convert_doc_remembered_files()
{
    mpl_list_t *tmp_p;
    file *file_p;

    MPL_LIST_FOR_EACH(remembered_files_p, tmp_p) {
        file_p = LISTABLE_PTR(tmp_p, file);
        file_p->convert_doc("file", file_p->filename_p);
    }
}

void mpl_compiler::convert_doc_categories()
{
    mpl_list_t *tmp_p;
    category *category_p;

    MPL_LIST_FOR_EACH(categories_p, tmp_p) {
        category_p = LISTABLE_PTR(tmp_p, category);
        category_p->convert_doc();
    }
}

void mpl_compiler::convert_doc_parameter_sets()
{
    mpl_list_t *tmp_p;
    parameter_set *parameter_set_p;

    MPL_LIST_FOR_EACH(parameter_sets_p, tmp_p) {
        parameter_set_p = LISTABLE_PTR(tmp_p, parameter_set);
        parameter_set_p->convert_doc();
    }
}

parameter_set *mpl_compiler::get_parameter_set_or_current(char *parameter_set_name_p)
{
    parameter_set *parameter_set_p;

    if (parameter_set_name_p != NULL) {
        parameter_set_p = find_parameter_set(parameter_set_name_p);
        if (parameter_set_p == NULL) {
            fprintf(stderr, "%s:%d: unknown parameter set %s\n",
                    peek_filename(), lineno(),
                   parameter_set_name_p);
            exit(-1);
        }
    }
    else {
        parameter_set_p = get_current_parameter_set();
    }
    return parameter_set_p;
}

void mpl_compiler::add_group(char *name_p, char *text_p, char *parent_name_p)
{
    if (get_group(name_p) != NULL)
        return;

    group_entry *group_p = new group_entry(strdup(name_p),
                                           strdup(text_p),
                                           get_group(parent_name_p));
    group_p->append_to(groups_p);
}

void mpl_compiler::set_group_parent(char *name_p, char *parent_name_p)
{
    group_entry *g_p = get_group(name_p);
    if (g_p)
        g_p->set_parent(get_group(parent_name_p));
}

group_entry *mpl_compiler::get_group(const char *name_p)
{
    if ((name_p == NULL) || !strlen(name_p))
        return NULL;

    mpl_list_t *tmp_p;
    group_entry *group_entry_p;

    MPL_LIST_FOR_EACH(groups_p, tmp_p) {
        group_entry_p = LISTABLE_PTR(tmp_p, group_entry);
        if (!strcmp(group_entry_p->name_p, name_p))
            return group_entry_p;
    }
    return NULL;
}

char *mpl_compiler::get_group_text(char *name_p)
{
    group_entry *g_p = get_group(name_p);
    if (g_p)
        return g_p->text_p;

    return NULL;
}

group_entry *mpl_compiler::get_group_parent(char *name_p)
{
    group_entry *g_p = get_group(name_p);
    if (g_p)
        return g_p->parent_p;

    return NULL;
}

