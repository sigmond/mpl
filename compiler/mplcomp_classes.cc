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
#include <FlexLexer.h>
#include "mplcomp.hh"
#include "mplcomp_parameter.hh"
#include <unistd.h>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <sstream>

listable_object::~listable_object()
{
    DELETE_LISTABLE_LIST(&doc_list_p, doc);
    DELETE_LISTABLE_LIST(&dox_list_p, dox_entry);
    DELETE_LISTABLE_LIST(&latex_list_p, latex_entry);
    DELETE_LISTABLE_LIST(&help_list_p, help_entry);
}

inheritable_object::~inheritable_object()
{
    DELETE_LISTABLE_LIST(&child_list_p, object_container);
    if (name_p)
        free(name_p);
}

enumerator_list::~enumerator_list()
{
    DELETE_LISTABLE_LIST(&enumerator_list_values_p, enum_value);
    if (name_p)
        free(name_p);
}

parameter_group::~parameter_group()
{
    DELETE_LISTABLE_LIST(&parameters_p, parameter);
}

number_range::~number_range()
{
    if (name_p)
        free(name_p);
    DELETE_LISTABLE_LIST(&range_list_p, integer_range);
}

command::~command()
{
    DELETE_LISTABLE_LIST(&cmd_dox_list_p, dox_entry);
    DELETE_LISTABLE_LIST(&resp_dox_list_p, dox_entry);
    DELETE_LISTABLE_LIST(&cmd_latex_list_p, latex_entry);
    DELETE_LISTABLE_LIST(&resp_latex_list_p, latex_entry);
    DELETE_LISTABLE_LIST(&cmd_help_list_p, help_entry);
}

methods::~methods()
{
    DELETE_LISTABLE_LIST(&method_list_p, method);
}

void listable_object::doc_to_x(parser_mode_t parser_mode,
                               const char *object_type_p,
                               const char *name_p,
                               mpl_list_t **local_x_list_pp,
                               mpl_list_t *local_doc_list_p)
{
    mpl_list_t *tmp_p;
    doc *doc_p;

    assert((parser_mode == parser_mode_dox) ||
           (parser_mode == parser_mode_latex) ||
           (parser_mode == parser_mode_help));

    if (local_doc_list_p == NULL)
        local_doc_list_p = doc_list_p;

    if (local_doc_list_p == NULL)
        return;

    if (parser_mode == parser_mode_dox)
        compiler_p->current_dox_list_p = NULL;
    else if (parser_mode == parser_mode_latex)
        compiler_p->current_latex_list_p = NULL;
    else
        compiler_p->current_help_list_p = NULL;

    MPL_LIST_FOR_EACH(local_doc_list_p, tmp_p) {
        doc_p = LISTABLE_PTR(tmp_p, doc);
        if (doc_p->text_p) {
            istringstream doc_stream(doc_p->text_p);
            x_entry *x_entry_p;
            if (parser_mode == parser_mode_dox) {
                x_entry_p = new dox_entry(compiler_p,
                                          object_type_p,
                                          this,
                                          doc_p->is_forward);
                if (compiler_p->current_dox_list_p == NULL)
                    compiler_p->current_dox_list_p = &x_entry_p->list_entry;
                else
                    x_entry_p->append_to(compiler_p->current_dox_list_p);

                if (local_x_list_pp)
                    *local_x_list_pp = compiler_p->current_dox_list_p;
                else
                    dox_list_p = compiler_p->current_dox_list_p;
            }
            else if (parser_mode == parser_mode_latex) {
                x_entry_p = new latex_entry(compiler_p,
                                            object_type_p,
                                            this);
                if (compiler_p->current_latex_list_p == NULL)
                    compiler_p->current_latex_list_p = &x_entry_p->list_entry;
                else
                    x_entry_p->append_to(compiler_p->current_latex_list_p);

                if (local_x_list_pp)
                    *local_x_list_pp = compiler_p->current_latex_list_p;
                else
                    latex_list_p = compiler_p->current_latex_list_p;
            }
            else {
                x_entry_p = new help_entry(compiler_p,
                                           object_type_p,
                                           this);
                if (compiler_p->current_help_list_p == NULL)
                    compiler_p->current_help_list_p = &x_entry_p->list_entry;
                else
                    x_entry_p->append_to(compiler_p->current_help_list_p);

                if (local_x_list_pp)
                    *local_x_list_pp = compiler_p->current_help_list_p;
                else
                    help_list_p = compiler_p->current_help_list_p;
            }
            compiler_p->set_lexer_stream(&doc_stream);
            compiler_p->parse(lexer_mode_doc, parser_mode, name_p);
        }
    }
}

void listable_object::doc_to_dox(const char *object_type_p,
                                 const char *name_p,
                                 mpl_list_t **local_dox_list_pp,
                                 mpl_list_t *local_doc_list_p)
{
    doc_to_x(parser_mode_dox,
             object_type_p,
             name_p,
             local_dox_list_pp,
             local_doc_list_p);
}

void listable_object::doc_to_latex(const char *object_type_p,
                                   const char *name_p,
                                   mpl_list_t **local_latex_list_pp,
                                   mpl_list_t *local_doc_list_p)
{
    doc_to_x(parser_mode_latex,
             object_type_p,
             name_p,
             local_latex_list_pp,
             local_doc_list_p);
}

void listable_object::doc_to_help(const char *object_type_p,
                                  const char *name_p,
                                  mpl_list_t **local_help_list_pp,
                                  mpl_list_t *local_doc_list_p)
{
    doc_to_x(parser_mode_help,
             object_type_p,
             name_p,
             local_help_list_pp,
             local_doc_list_p);
}

void listable_object::convert_doc(const char *type_p, const char *name_p)
{
    doc_to_dox(type_p, name_p);
    doc_to_latex(type_p, name_p);
    doc_to_help(type_p, name_p);
}

void listable_object::add_x_info(x_entry *x_entry_p,
                                 const char *label_p,
                                 char *value_p,
                                 char *text_p)
{
    x_entry_p->label_p = (char *) calloc(1, strlen(label_p) + 2);
    x_entry_p->value_p = (char *) calloc(1, strlen(value_p) +
                                           100);

    if (!strcmp(label_p, "text")) {
        sprintf(x_entry_p->label_p, "");
        sprintf(x_entry_p->value_p, "%s", value_p);
    }
    else {
        sprintf(x_entry_p->label_p, "");
        sprintf(x_entry_p->value_p, "");
    }
}

void listable_object::add_dox_info(dox_entry *dox_entry_p,
                                   const char *label_p,
                                   char *value_p,
                                   char *text_p)
{
    add_x_info(dox_entry_p, label_p, value_p, text_p);
}

void listable_object::add_latex_info(latex_entry *latex_entry_p,
                                     const char *label_p,
                                     char *value_p,
                                     char *text_p)
{
    if (!strcmp(label_p, "ingroup")) {
        latex_entry_p->label_p = strdup("");
        latex_entry_p->value_p = strdup("");
        group_p = compiler_p->get_group(value_p);
    }
    else {
        add_x_info(latex_entry_p, label_p, value_p, text_p);
    }
}

void listable_object::add_help_info(help_entry *help_entry_p,
                                    const char *label_p,
                                    char *value_p,
                                    char *text_p)
{
    if (!strcmp(label_p, "ingroup")) {
        help_entry_p->label_p = strdup("");
        help_entry_p->value_p = strdup("");
        group_p = compiler_p->get_group(value_p);
    }
    else {
        add_x_info(help_entry_p, label_p, value_p, text_p);
    }
}

void listable_object::print(int level)
{
    if (group_p) {
        printf("%s", spacing(level));
        printf("->group_p = %s\n", group_p->name_p);
    }
    print_doc_list(level, doc_list_p);
    print_dox_list(level, dox_list_p);
    print_latex_list(level, latex_list_p);
    print_help_list(level, help_list_p);
}


int if_condition::check(compiler_define *compiler_define_p)
{
    int left_integer;
    int right_integer;

    if (if_operator == if_operator_none) {
        fprintf(stderr, "%s:%d: Internal error: if-operator not defined\n",
                compiler_p->peek_filename(), compiler_p->lineno()
               );
        exit(-1);
    }

    if (convert_int(compiler_define_p->value_p, &left_integer) != 0) {
        fprintf(stderr, "%s:%d: Cannot convert %s to integer\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                compiler_define_p->value_p
               );
        exit(-1);
    }
    if (convert_int(value_p, &right_integer) != 0) {
        fprintf(stderr, "%s:%d: Cannot convert %s to integer\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                value_p
               );
        exit(-1);
    }

    switch (if_operator) {
        case if_operator_lt:
            if (left_integer < right_integer)
                return 1;
            break;
        case if_operator_eq:
            if (left_integer == right_integer)
                return 1;
            break;
        case if_operator_gt:
            if (left_integer > right_integer)
                return 1;
            break;
    }
    return 0;
}


void string_entry::print(int level, const char *name_p)
{
    printf("%s", spacing(level));
    printf("->%s = %s\n", name_p, value_p);

    listable_object::print(level);
}


void message_spec::add_to_enumerator(char *name_p)
{
    enum_value *enum_value_p;
    enum_value_p = new enum_value(compiler_p, strdup(name_p));
    enum_value_p->append_to(id_enumerator_list_p->enumerator_list_values_p);
}

void message_spec::convert_doc(const char *name_p)
{
    listable_object::convert_doc("message_spec", name_p);
}

void message_spec::wrap_up_definition(block_type_t block_type)
{
    const char *mtype_p = type == message_type_command ? "command" : type == message_type_response ? "response" : "event";
    const direction_t dir = type == message_type_command ? direction_in : direction_out;

    if (message_bag_p == NULL) {
        fprintf(stderr, "%s:%d: %s_bag parameter is missing\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                mtype_p
               );
        exit(-1);
    }
    message_bag_p->force_parameter_list_direction_unless_inout(dir);

}

void message_spec::print(int level)
{
    const char *mtype_p = type == message_type_command ? "command" : type == message_type_response ? "response" : "event";

    printf("%s", spacing(level));
    printf("->%s_spec\n", mtype_p);
    listable_object::print(level);
    printf("%s", spacing(level));
    printf("message_bag = %s\n", message_bag_p->name_p);
}

void command::wrap_up_definition()
{
    if (command_bag_p != NULL)
        command_bag_p->wrap_up_definition();
    if (response_bag_p != NULL) {
        response_bag_p->wrap_up_definition();
        response_bag_p->force_parameter_list_direction_unless_inout(direction_out);
    }
}

void event::wrap_up_definition()
{
    if (event_bag_p != NULL) {
        event_bag_p->wrap_up_definition();
        event_bag_p->force_parameter_list_direction_unless_inout(direction_out);
    }
}

void command::add_parameter_list_entry(parameter_list_entry *parameter_list_entry_p)
{
    parameter_list_entry *bag_entry_p;
    parameter_set *parameter_set_p = category_p->get_parameter_set();

    switch (parameter_list_entry_p->direction)
    {
        case direction_in:
            parameter_list_entry_p->append_to(command_bag_p->bag_parameter_list_p);
            break;
        case direction_inout:
        {
            parameter_list_entry *parameter_list_entry2_p =
                new parameter_list_entry(*parameter_list_entry_p);
            parameter_list_entry_p->append_to(command_bag_p->bag_parameter_list_p);
            parameter_list_entry2_p->append_to(response_bag_p->bag_parameter_list_p);
        }
        break;
        case direction_out:
            parameter_list_entry_p->append_to(response_bag_p->bag_parameter_list_p);
            break;
        default:
            assert(0);
            break;
    }
}


void command::print(int level)
{
    printf("%s", spacing(level));
    printf("name_p = %s\n", name_p);
    printf("%s", spacing(level));
    printf("parameter list\n");
    print_parameter_list(level+1, command_bag_p->bag_parameter_list_p);
    printf("%s", spacing(level));
    printf("response\n");
    print_parameter_list(level+1, response_bag_p->bag_parameter_list_p);
    listable_object::print(level);
    print_dox_list(level+1, cmd_dox_list_p, "cmd_dox_list");
    print_dox_list(level+1, resp_dox_list_p, "resp_dox_list");
    print_latex_list(level+1, cmd_latex_list_p, "cmd_latex_list");
    print_latex_list(level+1, resp_latex_list_p, "resp_latex_list");
    print_help_list(level+1, cmd_help_list_p, "cmd_help_list");
}

void command::convert_doc()
{
    last_parameter_list_entry_latex_target_p = NULL;
    listable_object::convert_doc("command", name_p);

    last_dox_entry_param_was_skipped = 0;
    doc_to_dox("command", name_p, &cmd_dox_list_p);
    last_dox_entry_param_was_skipped = 0;
    doc_to_dox("command", name_p, &resp_dox_list_p);

    last_latex_entry_param_was_skipped = 0;
    doc_to_latex("command", name_p, &cmd_latex_list_p);

    last_help_entry_param_was_skipped = 0;
    doc_to_help("command", name_p, &cmd_help_list_p);
}

void command::add_dox_info(dox_entry *dox_entry_p,
                           const char *label_p,
                           char *value_p,
                           char *text_p)
{
    int cmd_mode = (compiler_p->current_dox_list_p == cmd_dox_list_p);

    dox_entry_p->label_p = (char *) calloc(1, strlen(label_p) + 2);
    dox_entry_p->value_p = (char *) calloc(1, strlen(value_p) +
                                           100);

    if (!strcmp(label_p, "ingroup")) {
        sprintf(dox_entry_p->label_p, "@%s", label_p);
        sprintf(dox_entry_p->value_p, "%s", value_p);
    }
    else if (!strcmp(label_p, "param")) {
        int present_as_parameter = 0;
        int present_as_response = 0;
        direction_t dir = direction_in;
        mpl_list_t *list_p = NULL;

        if (parameter_property_in_list(value_p, "present", command_bag_p->bag_parameter_list_p))
            present_as_parameter = 1;

        if (parameter_property_in_list(value_p, "present", response_bag_p->bag_parameter_list_p))
            present_as_response = 1;

        if (!present_as_parameter && !present_as_response) {
            fprintf(stderr,
                    "%s:%d: Parameter '%s' not present in the command parameter list as %s or inout parameter\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    value_p,
                    cmd_mode ? "in" : "out"
                   );
            exit(-1);
        }

        if (cmd_mode && present_as_parameter) {
            list_p = command_bag_p->bag_parameter_list_p;
            dir = (direction_t) parameter_property_in_list(value_p, "direction", list_p);
        }
        else if (!cmd_mode && present_as_response) {
            list_p = response_bag_p->bag_parameter_list_p;
            dir = (direction_t) parameter_property_in_list(value_p, "direction", list_p);
        }

        if (list_p &&
            ((cmd_mode && (dir == direction_in)) ||
             (!cmd_mode && (dir == direction_out)) ||
             (dir == direction_inout))) {

            int optional = parameter_property_in_list(value_p, "optional", list_p);
            int multiple = parameter_property_in_list(value_p, "multiple", list_p);
            sprintf(dox_entry_p->label_p, "@%s", label_p);
            if (optional || dir || multiple) {
                sprintf(dox_entry_p->value_p,
                        "%s (%s%s%s%s%s)",
                        value_p,
                        dir ? (dir == direction_out) ? "out" : "inout" : "",
                        (dir && optional) ? "," : "",
                        optional ? "optional" : "",
                        ((dir || optional) && multiple) ? "," : "",
                        multiple ? "multiple" : ""
                       );
            }
            else {
                sprintf(dox_entry_p->value_p,
                        "%s",
                        value_p
                       );
            }
            last_dox_entry_param_was_skipped = 0;
        }
        else {
            last_dox_entry_param_was_skipped = 1;
        }
    }
    else if (!strcmp(label_p, "see")) {
        sprintf(dox_entry_p->label_p, "@%s", label_p);
        sprintf(dox_entry_p->value_p, "%s", value_p);
    }
    else if (!strcmp(label_p, "text")) {
        if (!last_dox_entry_param_was_skipped) {
            sprintf(dox_entry_p->label_p, "");
            sprintf(dox_entry_p->value_p, "%s", value_p);
        }
    }
    else {
        free(dox_entry_p->label_p);
        free(dox_entry_p->value_p);
        if (convert_locally_defined_dox_entry(label_p, value_p, dox_entry_p) != 0) {
            fprintf(stderr,
                    "%s:%d: Unsupported dox label '%s'\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    label_p
                   );
            exit(-1);
        }
    }
}

void command::add_latex_info(latex_entry *latex_entry_p,
                             const char *label_p,
                             char *value_p,
                             char *text_p)
{
    if (compiler_p->current_latex_list_p != latex_list_p)
        return;

    assert(latex_entry_p->label_p == NULL);
    latex_entry_p->label_p = (char *) calloc(1, strlen(label_p) + 2);
    assert(latex_entry_p->value_p == NULL);
    latex_entry_p->value_p = (char *) calloc(1, strlen(value_p) +
                                           100);

    if (!strcmp(label_p, "ingroup")) {
        sprintf(latex_entry_p->label_p, "");
        sprintf(latex_entry_p->value_p, "");
        group_p = compiler_p->get_group(value_p);
    }
    else if (!strcmp(label_p, "param")) {
        int present_as_parameter = 0;
        int present_as_response = 0;
        mpl_list_t *list_p = NULL;
        parameter_list_entry *parameter_list_entry_p;

        if (parameter_property_in_list(value_p, "present", command_bag_p->bag_parameter_list_p))
            present_as_parameter = 1;

        if (parameter_property_in_list(value_p, "present", response_bag_p->bag_parameter_list_p))
            present_as_response = 1;

        if (!present_as_parameter && !present_as_response) {
            fprintf(stderr,
                    "%s:%d: Parameter '%s' not present in the command parameter lists\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    value_p
                   );
            exit(-1);
        }

        if (present_as_parameter) {
            list_p = command_bag_p->bag_parameter_list_p;
            parameter_list_entry_p = find_parameter_entry_in_list(list_p,
                                                                  value_p);
            assert(parameter_list_entry_p != NULL);
        }
        else if (present_as_response) {
            list_p = response_bag_p->bag_parameter_list_p;
            parameter_list_entry_p = find_parameter_entry_in_list(list_p,
                                                                  value_p);
            assert(parameter_list_entry_p != NULL);
        }

        if (list_p) {
            sprintf(latex_entry_p->label_p, "");
            sprintf(latex_entry_p->value_p, "");
            last_latex_entry_param_was_skipped = 0;
            last_parameter_list_entry_latex_target_p = parameter_list_entry_p;
        }
        else {
            last_latex_entry_param_was_skipped = 1;
            last_parameter_list_entry_latex_target_p = NULL;
        }
    }
    else if (!strcmp(label_p, "see")) {
        sprintf(latex_entry_p->label_p, "");
        sprintf(latex_entry_p->value_p, "");
    }
    else if (!strcmp(label_p, "text")) {
        if (!last_latex_entry_param_was_skipped) {
            if (last_parameter_list_entry_latex_target_p) {
                latex_entry *l_e_p = new latex_entry(compiler_p,
                                                     "parameter_list_entry",
                                                     last_parameter_list_entry_latex_target_p);
                l_e_p->label_p = strdup("");
                l_e_p->value_p = strdup(value_p);
                l_e_p->append_to(last_parameter_list_entry_latex_target_p->latex_list_p);
            }
            else {
                sprintf(latex_entry_p->label_p, "");
                sprintf(latex_entry_p->value_p, "%s", value_p);
            }
        }
    }
    else { /* locally defined */
        if (convert_locally_defined_latex_entry("command", this, label_p, value_p, latex_entry_p) != 0) {
            sprintf(latex_entry_p->label_p, "");
            sprintf(latex_entry_p->value_p, "");
        }
    }
}

void command::add_help_info(help_entry *help_entry_p,
                            const char *label_p,
                            char *value_p,
                            char *text_p)
{
    if (compiler_p->current_help_list_p != help_list_p)
        return;

    help_entry_p->label_p = (char *) calloc(1, strlen(label_p) + 2);
    help_entry_p->value_p = (char *) calloc(1, strlen(value_p) +
                                           100);

    if (!strcmp(label_p, "ingroup")) {
        sprintf(help_entry_p->label_p, "");
        sprintf(help_entry_p->value_p, "");
        group_p = compiler_p->get_group(value_p);
    }
    else if (!strcmp(label_p, "param")) {
        int present_as_parameter = 0;
        int present_as_response = 0;
        mpl_list_t *list_p = NULL;
        parameter_list_entry *parameter_list_entry_p;

        if (parameter_property_in_list(value_p, "present", command_bag_p->bag_parameter_list_p))
            present_as_parameter = 1;

        if (parameter_property_in_list(value_p, "present", response_bag_p->bag_parameter_list_p))
            present_as_response = 1;

        if (!present_as_parameter && !present_as_response) {
            fprintf(stderr,
                    "%s:%d: Parameter '%s' not present in the command parameter lists\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    value_p
                   );
            exit(-1);
        }

        if (present_as_parameter) {
            list_p = command_bag_p->bag_parameter_list_p;
            parameter_list_entry_p = find_parameter_entry_in_list(list_p,
                                                                  value_p);
            assert(parameter_list_entry_p != NULL);
        }
        else if (present_as_response) {
            list_p = response_bag_p->bag_parameter_list_p;
            parameter_list_entry_p = find_parameter_entry_in_list(list_p,
                                                                  value_p);
            assert(parameter_list_entry_p != NULL);
        }

        if (list_p) {
            sprintf(help_entry_p->label_p, "");
            sprintf(help_entry_p->value_p, "");
            last_help_entry_param_was_skipped = 0;
            last_parameter_list_entry_help_target_p = parameter_list_entry_p;
        }
        else {
            last_help_entry_param_was_skipped = 1;
            last_parameter_list_entry_help_target_p = NULL;
        }
    }
    else if (!strcmp(label_p, "see")) {
        sprintf(help_entry_p->label_p, "");
        sprintf(help_entry_p->value_p, "");
    }
    else if (!strcmp(label_p, "text")) {
        if (!last_help_entry_param_was_skipped) {
            if (last_parameter_list_entry_help_target_p) {
                help_entry *l_e_p = new help_entry(compiler_p,
                                                   "parameter_list_entry",
                                                   last_parameter_list_entry_help_target_p);
                l_e_p->label_p = strdup("");
                l_e_p->value_p = strdup(value_p);
                l_e_p->append_to(last_parameter_list_entry_help_target_p->help_list_p);
            }
            else {
                sprintf(help_entry_p->label_p, "");
                sprintf(help_entry_p->value_p, "%s", value_p);
            }
        }
    }
    else { /* locally defined */
        if (convert_locally_defined_help_entry(label_p, value_p, help_entry_p) != 0) {
            sprintf(help_entry_p->label_p, "");
            sprintf(help_entry_p->value_p, "");
        }
    }
}

void command::dox(FILE *f, int level)
{
    char *sep_p;

    sep_p = strdup("");

    if (cmd_dox_list_p) {
        dox_dox_list_f(f, cmd_dox_list_p);
    }
    else {
        fprintf(f, "%s", spacing(level));
        dox_doc_f(f, doc_list_p);
    }

    fprintf(f,
            "void %s_Cmd(",
            name_p
           );
    if (command_bag_p->bag_parameter_list_p == NULL) {
        fprintf(f,
                "void"
               );
    } else {
        mpl_list_t *tmp_p;
        mpl_list_t *cmd_list_p = NULL;
        parameter_list_entry *entry_p;
        MPL_LIST_FOR_EACH(command_bag_p->bag_parameter_list_p, tmp_p) {
            entry_p = LISTABLE_PTR(tmp_p, parameter_list_entry);
            if (!parameter_property_in_list(entry_p->parameter_name_p,
                                           "present",
                                            (mpl_list_t *)category_p->get_command_bag()->get_property("parameter_list")
                                           )) {
                entry_p->clone()->append_to(cmd_list_p);
            }
        }
        dox_parameter_list(f, cmd_list_p, direction_in);
    }

    fprintf(f,
            ");\n"
           );


    if (resp_dox_list_p) {
        dox_dox_list_f(f, resp_dox_list_p);
    }
    else {
        fprintf(f, "%s", spacing(level));
        dox_doc_f(f, doc_list_p);
    }

    fprintf(f,
            "void %s_Resp(",
            name_p
           );
    if (response_bag_p->bag_parameter_list_p == NULL) {
        fprintf(f,
                "void"
               );
    } else {
        dox_parameter_list(f, response_bag_p->bag_parameter_list_p, direction_out);
    }

    fprintf(f,
            ");\n"
           );
}

void command::latex(FILE *f, parameter_set *default_parameter_set_p)
{
    LX_SSS("%s", name_p);
    latex_latex_list(f, latex_list_p);

    fprintf(f,
            "\n"
           );

    latex_parameter_list(f,
                         command_bag_p->bag_parameter_list_p,
                         (mpl_list_t *)category_p->get_command_bag()->get_property("parameter_list"),
                         direction_in,
                         default_parameter_set_p,
                         "Command parameters"
                        );

    fprintf(f,
            "\n"
           );

    latex_parameter_list(f,
                         response_bag_p->bag_parameter_list_p,
                         (mpl_list_t *)category_p->get_response_bag()->get_property("parameter_list"),
                         direction_out,
                         default_parameter_set_p,
                         "Response parameters"
                        );
}

void command::help(ostream &os, parameter_set *default_parameter_set_p)
{
    os << name_p;
    os << "\\n";
    help_help_list(os, help_list_p, "  ");
    help_parameter_list(os,
                        command_bag_p->bag_parameter_list_p,
                        (mpl_list_t *)category_p->get_command_bag()->get_property("parameter_list"),
                        direction_in,
                        default_parameter_set_p,
                        "Command parameters"
                       );
    help_parameter_list(os,
                        response_bag_p->bag_parameter_list_p,
                        (mpl_list_t *)category_p->get_response_bag()->get_property("parameter_list"),
                        direction_out,
                        default_parameter_set_p,
                        "Response parameters"
                       );
}

void command::deja(FILE *f, char *indent)
{
    fprintf(f,
            "%s# command %s\n",
            indent,
            name_p
           );

    deja_main_proc(f, indent);
    deja_send_proc(f, indent);
}

void command::deja_main_proc(FILE *f, char *indent)
{
    fprintf(f,
            "%s## Pack/send %s command and check response\n",
            indent,
            name_p
           );
#define INDENT(str) fprintf(f, "%s%s", indent, str)
    INDENT("#\n");
    INDENT("## Parameters:\n");
    INDENT("# inParamDict - dictionary representing the command parameters\n");
    INDENT("# checkParamDict - dictionary representing the response parameters\n");
    INDENT("#                  to check\n");
    INDENT("## Return value:\n");
    INDENT("# A dictionary with the same members as \"checkParamDict\" with actual\n");
    INDENT("# values filled in\n");
    INDENT("#\n");
    INDENT("## inParamDict format:\n");
    fprintf(f,
            "%s# See documentation of %s_CMD pack function\n",
            indent,
            name_p
           );
    INDENT("#\n");
    INDENT("## checkParamDict format:\n");
    fprintf(f,
            "%s# See documentation of %s_RESP check function\n",
            indent,
            name_p
           );
    INDENT("#\n");
#undef INDENT
    fprintf(f,
            "%snamespace export %s\n",
            indent,
            name_p
           );
    fprintf(f,
            "%sproc %s { inParamDict {checkParamDict \"\"} } {\n",
            indent,
            name_p
           );
    fprintf(f,
            "%s    send_%s $inParamDict\n",
            indent,
            name_p
           );
    fprintf(f,
            "%s    set msg [get_Msg %s]\n",
            indent,
            response_bag_p->tcl_param_id()
           );
    fprintf(f,
            "%s    return [%s::check::%s $checkParamDict $msg]\n",
            indent,
            response_bag_p->parameter_set_p->name_p,
            response_bag_p->name_p
           );
    fprintf(f,
            "%s}\n\n",
            indent
           );
}

void command::deja_send_proc(FILE *f, char *indent)
{
    fprintf(f,
            "%sproc send_%s { inParamDict } {\n",
            indent,
            name_p
           );
    fprintf(f,
            "%s    send_Cmd \"[%s::pack::%s $inParamDict]\"\n",
            indent,
            command_bag_p->parameter_set_p->name_p,
            command_bag_p->name_p
           );
    fprintf(f,
            "%s    sync_Cmd\n",
            indent
           );
    fprintf(f,
            "%s}\n\n",
            indent
           );
}

void event::add_parameter_list_entry(parameter_list_entry *parameter_list_entry_p)
{
    if (parameter_list_entry_p->direction == direction_out) {
        parameter_list_entry_p->append_to(event_bag_p->bag_parameter_list_p);
    }
    else {
        fprintf(stderr, "%s:%d: Event parameter '%s' is not of type 'out'\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                parameter_list_entry_p->parameter_name_p);
        exit(-1);
    }
}

void event::print(int level)
{
    printf("%s", spacing(level));
    printf("name_p = %s\n", name_p);
    printf("%s", spacing(level));
    printf("parameter list\n");
    print_parameter_list(level+1, event_bag_p->bag_parameter_list_p);
    listable_object::print(level);
}

void event::convert_doc()
{
    last_parameter_list_entry_latex_target_p = NULL;
    listable_object::convert_doc("event", name_p);
}

void event::add_dox_info(dox_entry *dox_entry_p,
                         const char *label_p,
                         char *value_p,
                         char *text_p)
{
    dox_entry_p->label_p = (char *) calloc(1, strlen(label_p) + 2);
    dox_entry_p->value_p = (char *) calloc(1, strlen(value_p) + 100);

    if (!strcmp(label_p, "ingroup")) {
        sprintf(dox_entry_p->label_p, "@%s", label_p);
        sprintf(dox_entry_p->value_p, "%s", value_p);
    }
    else if (!strcmp(label_p, "param")) {
        int present_as_response = parameter_property_in_list(value_p, "present", event_bag_p->bag_parameter_list_p);
        if (!present_as_response) {
            fprintf(stderr,
                    "%s:%d: Parameter '%s' not present in command parameter list\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    value_p
                   );
            exit(-1);
        }
        mpl_list_t *list_p = event_bag_p->bag_parameter_list_p;
        int optional = parameter_property_in_list(value_p, "optional", list_p);
        direction_t dir = (direction_t) parameter_property_in_list(value_p, "direction", list_p);
        int multiple = parameter_property_in_list(value_p, "multiple", list_p);
        sprintf(dox_entry_p->label_p, "@%s", label_p);
        if (optional || dir || multiple) {
            sprintf(dox_entry_p->value_p,
                    "%s (%s%s%s%s%s)",
                    value_p,
                    dir ? (dir == direction_out) ? "out" : "inout" : "",
                    (dir && optional) ? "," : "",
                    optional ? "optional" : "",
                    ((dir || optional) && multiple) ? "," : "",
                    multiple ? "multiple" : ""
                   );
        }
    }
    else if (!strcmp(label_p, "see")) {
        sprintf(dox_entry_p->label_p, "@%s", label_p);
        sprintf(dox_entry_p->value_p, "%s", value_p);
    }
    else if (!strcmp(label_p, "text")) {
        sprintf(dox_entry_p->label_p, "");
        sprintf(dox_entry_p->value_p, "%s", value_p);
    }
    else {
        free(dox_entry_p->label_p);
        free(dox_entry_p->value_p);
        if (convert_locally_defined_dox_entry(label_p, value_p, dox_entry_p) != 0) {
            fprintf(stderr,
                    "%s:%d: Unsupported dox label '%s'\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    label_p
                   );
            exit(-1);
        }
    }
}

void event::add_latex_info(latex_entry *latex_entry_p,
                           const char *label_p,
                           char *value_p,
                           char *text_p)
{
    if (compiler_p->current_latex_list_p != latex_list_p)
        return;

    assert(latex_entry_p->label_p == NULL);
    latex_entry_p->label_p = (char *) calloc(1, strlen(label_p) + 2);
    assert(latex_entry_p->value_p == NULL);
    latex_entry_p->value_p = (char *) calloc(1, strlen(value_p) +
                                           100);

    if (!strcmp(label_p, "ingroup")) {
        sprintf(latex_entry_p->label_p, "");
        sprintf(latex_entry_p->value_p, "");
        group_p = compiler_p->get_group(value_p);
    }
    else if (!strcmp(label_p, "param")) {
        int present_as_response = 0;
        mpl_list_t *list_p = NULL;
        parameter_list_entry *parameter_list_entry_p;

        if (parameter_property_in_list(value_p, "present", event_bag_p->bag_parameter_list_p))
            present_as_response = 1;

        if (!present_as_response) {
            fprintf(stderr,
                    "%s:%d: Parameter '%s' not present in the event response list\n",
                    compiler_p->peek_filename(), compiler_p->lineno(),
                    value_p
                   );
            exit(-1);
        }

        if (present_as_response) {
            list_p = event_bag_p->bag_parameter_list_p;
            parameter_list_entry_p = find_parameter_entry_in_list(list_p,
                                                                  value_p);
            assert(parameter_list_entry_p != NULL);
        }

        if (list_p) {
            sprintf(latex_entry_p->label_p, "");
            sprintf(latex_entry_p->value_p, "");
            last_latex_entry_param_was_skipped = 0;
            last_parameter_list_entry_latex_target_p = parameter_list_entry_p;
        }
        else {
            last_latex_entry_param_was_skipped = 1;
            last_parameter_list_entry_latex_target_p = NULL;
        }
    }
    else if (!strcmp(label_p, "see")) {
        sprintf(latex_entry_p->label_p, "");
        sprintf(latex_entry_p->value_p, "");
    }
    else if (!strcmp(label_p, "text")) {
        if (!last_latex_entry_param_was_skipped) {
            if (last_parameter_list_entry_latex_target_p) {
                latex_entry *l_e_p = new latex_entry(compiler_p,
                                                     "parameter_list_entry",
                                                     last_parameter_list_entry_latex_target_p);
                l_e_p->label_p = strdup("");
                l_e_p->value_p = strdup(value_p);
                l_e_p->append_to(last_parameter_list_entry_latex_target_p->latex_list_p);
            }
            else{
                sprintf(latex_entry_p->label_p, "");
                sprintf(latex_entry_p->value_p, "%s", value_p);
            }
        }
    }
    else { /* locally defined */
        if (convert_locally_defined_latex_entry("event", this, label_p, value_p, latex_entry_p) != 0) {
            sprintf(latex_entry_p->label_p, "");
            sprintf(latex_entry_p->value_p, "");
        }
    }
}

void event::dox(FILE *f, int level)
{
    if (dox_list_p) {
        dox_dox_list_f(f, dox_list_p);
    }
    else {
        fprintf(f, "%s", spacing(level));
        dox_doc_f(f, doc_list_p);
    }

    fprintf(f, "%s", spacing(level));
    fprintf(f,
            "void %s_Evt(",
            name_p
           );
    dox_parameter_list(f, event_bag_p->bag_parameter_list_p, direction_out);
    fprintf(f,
            ");\n"
           );
}

void event::latex(FILE *f, parameter_set *default_parameter_set_p)
{
    LX_SSS("%s", name_p);
    latex_latex_list(f, latex_list_p);

    fprintf(f,
            "\n"
           );

    latex_parameter_list(f,
                         event_bag_p->bag_parameter_list_p,
                         (mpl_list_t *)category_p->get_event_bag()->get_property("parameter_list"),
                         direction_out,
                         default_parameter_set_p,
                         "Parameters"
                        );
}

void event::deja(FILE *f, char *indent)
{
    fprintf(f,
            "%s# event %s\n",
            indent,
            name_p
           );
    fprintf(f,
            "%s## Check %s event\n",
            indent,
            name_p
           );
#define INDENT(str) fprintf(f, "%s%s", indent, str)
    INDENT("#\n");
    INDENT("## Parameters:\n");
    INDENT("# checkParamDict - dictionary representing the event parameters\n");
    INDENT("#                  to check\n");
    INDENT("## Return value:\n");
    INDENT("# A dictionary with the same members as \"checkParamDict\" with actual\n");
    INDENT("# values filled in\n");
    INDENT("#\n");
    INDENT("## checkParamDict format:\n");
    fprintf(f,
            "%s# See documentation of %s_EVENT check function\n",
            indent,
            name_p
           );
    INDENT("#\n");
#undef INDENT
    fprintf(f,
            "%snamespace export %s\n",
            indent,
            name_p
           );
    fprintf(f,
            "%sproc %s { {checkParamDict \"\"} } {\n",
            indent,
            name_p
           );
    fprintf(f,
            "%s    set msg [get_Msg %s]\n",
            indent,
            event_bag_p->tcl_param_id()
           );
    fprintf(f,
            "%s    return [%s::check::%s $checkParamDict $msg]\n",
            indent,
            event_bag_p->parameter_set_p->name_p,
            event_bag_p->name_p
           );
    fprintf(f,
            "%s}\n\n",
            indent
           );
}

void command::api_hh(FILE *f, char *indent)
{
    fprintf(f,
            "%s        virtual int %s(",
            indent,
            name_p);
    api_parameter_list(f,
                       command_bag_p->bag_parameter_list_p,
                       category_p->get_command_bag()->bag_parameter_list_p,
                       direction_in
                      );
    fprintf(f,
            ");\n"
           );
}

void command::api_hh_response_class(FILE *f, char *indent)
{
    fprintf(f,
            "%sclass %s_Response : %s_Response\n"
            "%s{\n",
            indent,
            name_p,
            category_p->name_p,
            indent
           );
    fprintf(f,
            "%s    public:\n",
            indent
           );
    fprintf(f,
            "%s        %s_Response(mpl_list_t *respMsg);\n",
            indent,
            name_p
           );
    fprintf(f,
            "%s    protected:\n",
            indent
           );

    api_hh_class_members_parameter_list(f,
                                        indent,
                                        response_bag_p->bag_parameter_list_p,
                                        category_p->get_response_bag()->bag_parameter_list_p,
                                        category_p->get_parameter_set(),
                                        direction_out);

    fprintf(f,
            "%s};\n",
            indent
           );
}

void command::api_cc_response_class(FILE *f, char *indent)
{
}

void event::api_hh_event_class(FILE *f, char *indent)
{
}

void event::api_cc_event_class(FILE *f, char *indent)
{
}

void command::api_cc(FILE *f, char *indent)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;
    char *snl = category_p->get_parameter_set()->get_short_name();
    char *snu = str_toupper(snl);

    /* Command method */
    fprintf(f,
            "%sint %s::%s(",
            indent,
            category_p->name_p,
            name_p);
    api_parameter_list(f,
                       command_bag_p->bag_parameter_list_p,
                       category_p->get_command_bag()->bag_parameter_list_p,
                       direction_in
                      );
    fprintf(f,
            ")\n"
            "%s{\n",
            indent
           );

    fprintf(f,
            "%s    mpl_list_t *cmdMsg = NULL;\n"
            "%s    mpl_bag_t *cmdParams = NULL;\n",
            indent,
            indent
           );
    fprintf(f,
            "\n"
           );

    api_cc_encode_parameter_list(f,
                                 indent,
                                 command_bag_p->bag_parameter_list_p,
                                 NULL,
                                 category_p->get_parameter_set(),
                                 name_p,
                                 (char*)"cmdParams",
                                 direction_in
                                );

    fprintf(f,
            "%s    {\n"
            "%s        mpl_param_element_t *__elem;\n"
            "%s        __elem = mpl_param_element_create_empty(%s_COMMAND_ID(%s));\n"
            "%s        __elem->value_p = cmdParams;\n"
            "%s        cmdMsg = &__elem->list_entry;\n"
            "%s    }\n",
            indent,
            indent,
            indent,
            snu,
            name_p,
            indent,
            indent,
            indent
           );

    fprintf(f,
            "%s    _send(cmdMsg);\n"
            "%s    mpl_param_list_destroy(&cmdMsg);\n"
            "%s    return(0);\n",
            indent,
            indent,
            indent
           );
    fprintf(f,
            "%s}\n",
            indent
           );


    /* Response class constructor */
    fprintf(f,
            "%s%s::%s_Response(mpl_list_t *inMsg) :\n"
            "%s    %s_Response(inMsg)\n"
            "%s{\n",
            indent,
            category_p->name_p,
            name_p,
            indent,
            category_p->name_p,
            indent);

    if (response_bag_p->have_own_parameters()) {
        fprintf(f,
                "%s    mpl_bag_t *respBag = %s_GET_RESPONSE_PARAMS_PTR(inMsg);\n",
                indent,
                snu
               );
        fprintf(f,
                "\n"
               );

        api_decode_parameter_list(f,
                                  indent,
                                  response_bag_p->bag_parameter_list_p,
                                  category_p->get_response_bag()->bag_parameter_list_p,
                                  category_p->get_parameter_set(),
                                  category_p->get_response_bag()->name_p,
                                  (char*)"respBag",
                                  direction_out);
    }

    fprintf(f,
            "%s}\n",
            indent
           );

    /* Destructor */
    fprintf(f,
            "%s%s::~%s_Response()\n",
            indent,
            category_p->name_p,
            name_p
           );
    fprintf(f,
            "%s{\n",
            indent
           );

    api_free_decoded_parameter_list(f,
                                    indent,
                                    response_bag_p->bag_parameter_list_p,
                                    category_p->get_response_bag()->bag_parameter_list_p,
                                    category_p->get_parameter_set(),
                                    direction_out);

    fprintf(f,
            "%s}\n",
            indent
           );

    free(snu);
}

void event::api_hh(FILE *f, char *indent)
{
    fprintf(f,
            "%s        virtual int %s_Event(",
            indent,
            name_p);
    api_parameter_list(f,
                       event_bag_p->bag_parameter_list_p,
                       category_p->get_event_bag()->bag_parameter_list_p,
                       direction_out
                      );
    fprintf(f,
            ");\n"
           );
}

void event::api_cc(FILE *f, char *indent)
{
}

void ellipsis::print(int level)
{
    printf("%s", spacing(level));
    printf("ellipsis\n");
    printf("%s", spacing(level+1));
    printf("->value = %d\n", value);
    listable_object::print(level);
}



void parameter_list_entry::add_dox_info(dox_entry *dox_entry_p,
                                        const char *label_p,
                                        char *value_p,
                                        char *text_p)
{
    dox_entry_p->label_p = (char *) calloc(1, strlen(label_p) + 2);
    dox_entry_p->value_p = (char *) calloc(1, strlen(value_p) +
                                           100);

    if (!strcmp(label_p, "see")) {
        sprintf(dox_entry_p->label_p, "@%s", label_p);
        sprintf(dox_entry_p->value_p, "%s", value_p);
    }
    else if (!strcmp(label_p, "text")) {
        sprintf(dox_entry_p->label_p, "");
        if (optional || multiple) {
            sprintf(dox_entry_p->value_p,
                    "(%s%s%s) %s",
                    optional ? "optional" : "",
                    (optional && multiple) ? "," : "",
                    multiple ? "multiple" : "",
                    value_p
                   );
        }
        else {
            sprintf(dox_entry_p->value_p, "%s", value_p);
        }
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

bool parameter_list_entry::is_same_as(parameter_list_entry *entry_p)
{
    if ((field_name_p == NULL) && (entry_p->field_name_p != NULL))
        return false;
    if (field_name_p != NULL) {
        if (entry_p->field_name_p == NULL)
            return false;
        if (strcmp(field_name_p,entry_p->field_name_p))
            return false;
    }
    if (strcmp(parameter_name_p,entry_p->parameter_name_p))
        return false;

    if (parameter_set_p != entry_p->parameter_set_p)
        return false;

    return true;
}

void parameter_list_entry::set_option(char *option_p)
{
    if (!strcmp("...", option_p)) {
        if (multiple)
            array = 0;
        return;
    }

    fprintf(stderr,
            "%s:%d: Illegal option '%s'\n",
            compiler_p->peek_filename(), compiler_p->lineno(),
            option_p
           );
    exit(-1);
}

void parameter_list_entry::print(int level)
{
    printf("%s", spacing(level));
    printf("->%s %s%s::%s%s\n",
           direction ? (direction == direction_out) ? "out" : "inout" : "",
           optional ? "*" : "",
           parameter_set_p->name_p,
           parameter_name_p,
           multiple ? "[]" : ""
          );
    listable_object::print(level);
}

void doc::print(int level)
{
    printf("%s", spacing(level));
    printf("->doc_p = '%s'\n\n", text_p);
}

void doc::dox(FILE *f)
{
    fprintf_dox(f,
                "%s\n",
                text_p
               );
}

void compiler_option::print(int level)
{
    printf("%s", spacing(level));
    printf("%s,%s: %s = %s\n",
           block_type_p,
           block_name_p,
           option_name_p,
           option_value_p
          );
    listable_object::print(level);
}

int compiler_option::check()
{
    if (!strcmp(block_type_p, "parameter_set")){
        if (!strcmp(option_name_p, "short_name")) {
            if (option_value_p != NULL) {
                return 0;
            }
        }
    }
    return 1;
}



void enumerator_list::print(int level)
{
    printf("%s", spacing(level));
    printf("%s:\n", name_p);
    listable_object::print(level);
    print_enum_values(level+1, enumerator_list_values_p);
}

void enumerator_list::gc_h(FILE *f, char *parameter_set_name_p)
{
    mpl_list_t *tmp_p;
    enum_value *enum_value_p;
    char *psl = parameter_set_name_p;
    char *psu = str_toupper(psl);
    char *pln = name_p;

    fprintf(f,
            "/* %s_%s */\n",
            psu,
            pln
           );

    fprintf(f,
            "#define %s_%s \\\n",
            psu,
            pln
           );

    MPL_LIST_FOR_EACH(enumerator_list_values_p, tmp_p) {
        enum_value_p = LISTABLE_PTR(tmp_p, enum_value);
        fprintf(f,
                "  %s_%s_VALUE_ELEMENT(%s) \\\n",
                psu,
                pln,
                enum_value_p->name_p
               );
        }
    fprintf(f,
            "\n"
           );
    free(psu);
}


void x_entry::add_info(parser_mode_t parser_mode,
                       const char *label_p,
                       char *value_p,
                       char *text_p)
{
    assert((parser_mode == parser_mode_dox) ||
           (parser_mode == parser_mode_latex) ||
           (parser_mode == parser_mode_help));
    if (!strcmp(target_object_type_p, "command")) {
        if (parser_mode == parser_mode_dox)
            ((command *) target_object_p)->add_dox_info((dox_entry*)this,
                                                        label_p,
                                                        value_p,
                                                        text_p);
        else if (parser_mode == parser_mode_latex)
            ((command *) target_object_p)->add_latex_info((latex_entry*)this,
                                                          label_p,
                                                          value_p,
                                                          text_p);
        else if (parser_mode == parser_mode_help)
            ((command *) target_object_p)->add_help_info((help_entry*)this,
                                                        label_p,
                                                        value_p,
                                                        text_p);
   }
    else if (!strcmp(target_object_type_p, "commands_class")) {
        if (parser_mode == parser_mode_dox)
            ((commands_class *) target_object_p)->add_dox_info((dox_entry*)this,
                                                               label_p,
                                                               value_p,
                                                               text_p);
        else if (parser_mode == parser_mode_latex)
            ((commands_class *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                 label_p,
                                                                 value_p,
                                                                 text_p);
    }
    else if (!strcmp(target_object_type_p, "event")) {
        if (parser_mode == parser_mode_dox)
            ((event *) target_object_p)->add_dox_info((dox_entry*)this,
                                                      label_p,
                                                      value_p,
                                                      text_p);
        else if (parser_mode == parser_mode_latex)
            ((event *) target_object_p)->add_latex_info((latex_entry*)this,
                                                        label_p,
                                                        value_p,
                                                        text_p);
    }
    else if (!strcmp(target_object_type_p, "events_class")) {
        if (parser_mode == parser_mode_dox)
            ((events_class *) target_object_p)->add_dox_info((dox_entry*)this,
                                                             label_p,
                                                             value_p,
                                                             text_p);
        else if (parser_mode == parser_mode_latex)
            ((events_class *) target_object_p)->add_latex_info((latex_entry*)this,
                                                               label_p,
                                                               value_p,
                                                               text_p);
    }
    else if (!strcmp(target_object_type_p, "category")) {
        if (parser_mode == parser_mode_dox)
            ((category *) target_object_p)->add_dox_info((dox_entry*)this,
                                                         label_p,
                                                         value_p,
                                                         text_p);
        else if (parser_mode == parser_mode_latex)
            ((category *) target_object_p)->add_latex_info((latex_entry*)this,
                                                           label_p,
                                                           value_p,
                                                           text_p);
    }
    else if (!strcmp(target_object_type_p, "parameter_list_entry")) {
        if (parser_mode == parser_mode_dox)
            ((parameter_list_entry *) target_object_p)->add_dox_info((dox_entry*)this,
                                                                     label_p,
                                                                     value_p,
                                                                     text_p);
        else if (parser_mode == parser_mode_latex)
            ((parameter_list_entry *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                       label_p,
                                                                       value_p,
                                                                       text_p);
    }
    else if (!strcmp(target_object_type_p, "parameter_set")) {
        if (parser_mode == parser_mode_dox)
            ((parameter_set *) target_object_p)->add_dox_info((dox_entry*)this,
                                                              label_p,
                                                              value_p,
                                                              text_p);
        else if (parser_mode == parser_mode_latex)
            ((parameter_set *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                label_p,
                                                                value_p,
                                                                text_p);
    }
    else if (!strcmp(target_object_type_p, "parameter_group")) {
        if (parser_mode == parser_mode_dox)
            ((parameter_group *) target_object_p)->add_dox_info((dox_entry*)this,
                                                                label_p,
                                                                value_p,
                                                                text_p);
        else if (parser_mode == parser_mode_latex)
            ((parameter_group *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                  label_p,
                                                                  value_p,
                                                                  text_p);
    }
    else if (!strcmp(target_object_type_p, "string_entry")) {
        if (parser_mode == parser_mode_dox)
            ((string_entry *) target_object_p)->add_dox_info((dox_entry*)this,
                                                             label_p,
                                                             value_p,
                                                             text_p);
        else if (parser_mode == parser_mode_latex)
            ((string_entry *) target_object_p)->add_latex_info((latex_entry*)this,
                                                               label_p,
                                                               value_p,
                                                               text_p);
    }
    else if (!strcmp(target_object_type_p, "ellipsis")) {
        if (parser_mode == parser_mode_dox)
            ((ellipsis *) target_object_p)->add_dox_info((dox_entry*)this,
                                                         label_p,
                                                         value_p,
                                                         text_p);
        else if (parser_mode == parser_mode_latex)
            ((ellipsis *) target_object_p)->add_latex_info((latex_entry*)this,
                                                           label_p,
                                                           value_p,
                                                           text_p);
    }
    else if (!strcmp(target_object_type_p, "enum_value")) {
        if (parser_mode == parser_mode_dox)
            ((enum_value *) target_object_p)->add_dox_info((dox_entry*)this,
                                                           label_p,
                                                           value_p,
                                                           text_p);
        else if (parser_mode == parser_mode_latex)
            ((enum_value *) target_object_p)->add_latex_info((latex_entry*)this,
                                                             label_p,
                                                             value_p,
                                                             text_p);
    }
    else if (!strcmp(target_object_type_p, "file")) {
        if (parser_mode == parser_mode_dox)
            ((file *) target_object_p)->add_dox_info((dox_entry*)this,
                                                     label_p,
                                                     value_p,
                                                     text_p);
        else if (parser_mode == parser_mode_latex)
            ((file *) target_object_p)->add_latex_info((latex_entry*)this,
                                                       label_p,
                                                       value_p,
                                                       text_p);
    }
    else if (!strcmp(target_object_type_p, "number_range")) {
        if (parser_mode == parser_mode_dox)
            ((number_range *) target_object_p)->add_dox_info((dox_entry*)this,
                                                             label_p,
                                                             value_p,
                                                             text_p);
        else if (parser_mode == parser_mode_latex)
            ((number_range *) target_object_p)->add_latex_info((latex_entry*)this,
                                                               label_p,
                                                               value_p,
                                                               text_p);
    }
    else if (get_type_of_int(target_object_type_p) != 0) {
        if (parser_mode == parser_mode_dox)
            ((int_parameter *) target_object_p)->add_dox_info((dox_entry*)this,
                                                              label_p,
                                                              value_p,
                                                              text_p);
        else if (parser_mode == parser_mode_latex)
            ((int_parameter *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                label_p,
                                                                value_p,
                                                                text_p);
    }
    else if (get_type_of_bool(target_object_type_p) != 0) {
        if (parser_mode == parser_mode_dox)
            ((bool_parameter *) target_object_p)->add_dox_info((dox_entry*)this,
                                                               label_p,
                                                               value_p,
                                                               text_p);
        else if (parser_mode == parser_mode_latex)
            ((bool_parameter *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                 label_p,
                                                                 value_p,
                                                                 text_p);
    }
    else if (get_type_of_enum(target_object_type_p) != 0) {
        if (parser_mode == parser_mode_dox)
            ((enum_parameter *) target_object_p)->add_dox_info((dox_entry*)this,
                                                               label_p,
                                                               value_p,
                                                               text_p);
        else if (parser_mode == parser_mode_latex)
            ((enum_parameter *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                 label_p,
                                                                 value_p,
                                                                 text_p);
    }
    else if (get_type_of_string(target_object_type_p) != 0) {
        if (parser_mode == parser_mode_dox)
            ((string_parameter *) target_object_p)->add_dox_info((dox_entry*)this,
                                                                 label_p,
                                                                 value_p,
                                                                 text_p);
        else if (parser_mode == parser_mode_latex)
            ((string_parameter *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                   label_p,
                                                                   value_p,
                                                                   text_p);
    }
    else if (get_type_of_tuple(target_object_type_p) != 0) {
        if (parser_mode == parser_mode_dox)
            ((tuple_parameter *) target_object_p)->add_dox_info((dox_entry*)this,
                                                                label_p,
                                                                value_p,
                                                                text_p);
        else if (parser_mode == parser_mode_latex)
            ((tuple_parameter *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                  label_p,
                                                                  value_p,
                                                                  text_p);
    }
    else if (get_type_of_array(target_object_type_p) != 0) {
        if (parser_mode == parser_mode_dox)
            ((array_parameter *) target_object_p)->add_dox_info((dox_entry*)this,
                                                                label_p,
                                                                value_p,
                                                                text_p);
        else if (parser_mode == parser_mode_latex)
            ((array_parameter *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                  label_p,
                                                                  value_p,
                                                                  text_p);
    }
    else if (get_type_of_addr(target_object_type_p) != 0) {
        if (parser_mode == parser_mode_dox)
            ((addr_parameter *) target_object_p)->add_dox_info((dox_entry*)this,
                                                               label_p,
                                                               value_p,
                                                               text_p);
        else if (parser_mode == parser_mode_latex)
            ((addr_parameter *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                 label_p,
                                                                 value_p,
                                                                 text_p);
    }
    else if (get_type_of_bag(target_object_type_p) != 0) {
        if (parser_mode == parser_mode_dox)
            ((bag_parameter *) target_object_p)->add_dox_info((dox_entry*)this,
                                                              label_p,
                                                              value_p,
                                                              text_p);
        else if (parser_mode == parser_mode_latex)
            ((bag_parameter *) target_object_p)->add_latex_info((latex_entry*)this,
                                                                label_p,
                                                                value_p,
                                                                text_p);
    }
    else {
        fprintf(stderr,
                "%s:%d: Unsupported dox_entry target object type '%s'\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                target_object_type_p
               );
        exit(-1);
    }
}


void file::add_latex_info(latex_entry *latex_entry_p,
                          const char *label_p,
                          char *value_p,
                          char *text_p)
{

    if (!strcmp(label_p, "defgroup")) {
        latex_entry_p->label_p = strdup("");
        latex_entry_p->value_p = strdup("");
        compiler_p->add_group(value_p,
                              text_p);
    }
    else {
        listable_object::add_latex_info(latex_entry_p, label_p, value_p, text_p);
    }
}

void group_entry::print(int level)
{
    printf("%s", spacing(level));
    printf("->name_p=%s\n",
           name_p
          );
    printf("%s", spacing(level));
    printf("->text_p=%s\n",
           text_p
          );
    if (parent_p) {
        printf("%s", spacing(level));
        printf("->parent_p=%s\n",
               parent_p->name_p
              );
    }
}


void commands_class::convert_doc()
{
    mpl_list_t *tmp_p;
    command *command_p;

    listable_object::convert_doc("commands_class", "commands");

    MPL_LIST_FOR_EACH(method_list_p, tmp_p) {
        command_p = LISTABLE_PTR(tmp_p, command);
        command_p->convert_doc();
    }
}

void events_class::convert_doc()
{
    mpl_list_t *tmp_p;
    event *event_p;

    listable_object::convert_doc("events_class", "events");

    MPL_LIST_FOR_EACH(method_list_p, tmp_p) {
        event_p = LISTABLE_PTR(tmp_p, event);
        event_p->convert_doc();
    }
}

void parameter_group::print(int level)
{
    mpl_list_t *tmp_p;
    parameter *parameter_p;

    printf("%s", spacing(level));
    printf("parameter group");
    if (category_p != NULL)
        printf(" in category '%s'\n",
              category_p->name_p);
    printf("\n");
    listable_object::print(level);
    MPL_LIST_FOR_EACH(parameters_p, tmp_p) {
        parameter_p = LISTABLE_PTR(tmp_p, parameter);
        parameter_p->print(level+1);
    }
}

void parameter_group::convert_doc()
{
    mpl_list_t *tmp_p;
    parameter *parameter_p;

    listable_object::convert_doc("parameter_group", parameter_set_p->name_p);

    MPL_LIST_FOR_EACH(parameters_p, tmp_p) {
        parameter_p = LISTABLE_PTR(tmp_p, parameter);
        parameter_p->convert_doc();
    }
}

void parameter_group::latex_parameters(FILE *f)
{
    mpl_list_t *tmp_p;
    parameter *parameter_p;

    MPL_LIST_FOR_EACH(parameters_p, tmp_p) {
        parameter_p = LISTABLE_PTR(tmp_p, parameter);
        parameter_p->latex(f, parameter_set_p);
    }
}

void parameter_group::latex(FILE *f)
{
    if (latex_list_p || parameters_p) {
        if (category_p) {
            if (group_p)
                LX_SS("%s",
                      group_p->text_p
                     );
            else
                LX_SS("%s Parameters",
                      category_p->name_p
                     );
        }
        else {
            LX_SS("%s",
                  "Parameters"
                 );
        }

        latex_latex_list(f, latex_list_p);
        latex_parameters(f);
    }
}


void inheritable_object::add_child(inheritable_object *object_p)
{
    object_container *child_p = new object_container(object_p);
    child_p->append_to(child_list_p);
}

mpl_list_t *inheritable_object::get_flat_child_list()
{
    mpl_list_t *cl_p = NULL;
    mpl_list_t *tmp_p;
    object_container *child_p;
    inheritable_object *object_p;

    cl_p = object_container_list_clone(child_list_p);

    MPL_LIST_FOR_EACH(child_list_p, tmp_p) {
        child_p = LISTABLE_PTR(tmp_p, object_container);
        object_p = (inheritable_object*) child_p->object_p;
        mpl_list_append(&cl_p, object_p->get_flat_child_list());
    }
    return cl_p;
}

mpl_list_t *inheritable_object::get_parent_list()
{
    mpl_list_t *pl_p = NULL;
    object_container *oc_p;
    inheritable_object *top_p = this;

    while (top_p->parent_p) {
        oc_p = new object_container(top_p->parent_p);
        oc_p->append_to(pl_p);
        top_p = top_p->parent_p;
    }
    return pl_p;
}

inheritable_object *inheritable_object::get_topmost_parent()
{
    inheritable_object *top_p = this;

    while (top_p->parent_p) {
        top_p = top_p->parent_p;
    }
    return top_p;
}

void object_container::print(int level)
{
    printf("%s", spacing(level));
    printf("object: %s\n",
           object_p->get_name()
          );
}

void number_range::print(int level)
{
    mpl_list_t *tmp_p;
    integer_range *ir_p;

    printf("%s", spacing(level));
    printf("->name_p = %s\n", name_p);

    MPL_LIST_FOR_EACH(range_list_p, tmp_p) {
        ir_p = LISTABLE_PTR(tmp_p, integer_range);
        ir_p->print(level+1);
    }

    listable_object::print(level);
}

integer_range::integer_range(mpl_compiler *compiler_p, char *first_p, char *last_p) :
    listable_object(compiler_p)
{
    int64_t first;
    int64_t last;

    if (string2int64(first_p, &first) != 0) {
        fprintf(stderr, "%s:%d: Could not convert string '%s' to integer\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                first_p
               );
        exit(-1);
    }
    if (string2int64(last_p, &last) != 0) {
        fprintf(stderr, "%s:%d: Could not convert string '%s' to integer\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                last_p
               );
        exit(-1);
    }
    this->first_p = new int64_t(first);
    this->last_p = new int64_t(last);
}

void integer_range::print(int level)
{
    printf("%s", spacing(level));
    printf("%" PRIi64 "..%" PRIi64 "\n",
           *first_p,
           *last_p
          );
}
