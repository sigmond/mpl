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
#include <sstream>

void category::print(int level)
{
    printf("%s", spacing(level++));
    printf("category\n");
    printf("%s", spacing(level));
    printf("->name_p = %s\n", name_p);
    printf("%s", spacing(level));
    if (parent_p)
        printf("->parent_p->name_p = %s\n", parent_p->get_name());
    else
        printf("->parent_p is NULL\n");
    printf("%s", spacing(level));
    if (parameter_set_p)
        printf("->parameter_set_p->name_p = %s\n", parameter_set_p->name_p);
    if (command_spec_p)
        command_spec_p->print(level);
    if (response_spec_p)
        response_spec_p->print(level);
    if (event_spec_p)
        event_spec_p->print(level);
    print_commands(level);
    print_doc_list(level, commands.doc_list_p);
    print_events(level);
    print_doc_list(level, events.doc_list_p);
    listable_object::print(level);

    return;
}

void category::set_parameter_set(parameter_set *pset_p)
{
    const char *estr_p;

    if (parameter_set_p != NULL) {
        fprintf(stderr, "%s:%d: parameter set already defined for category %s\n",
                compiler_p->peek_filename(), compiler_p->lineno(),
                name_p
               );
        exit(-1);
    }

    parameter_set_p = pset_p;
}

bag_parameter *category::add_message_bag(message_spec *message_spec_p, char *bag_name_p)
{
    if (!get_parameter_set()) {
        fprintf(stderr, "%s:%d: no parameter set\n",
                compiler_p->peek_filename(), compiler_p->lineno());
        exit(-1);
    }

    message_spec_p->message_bag_p = (bag_parameter*) get_parameter_set()->create_parameter_in_current_group("bag", bag_name_p);
    message_spec_p->message_bag_p->add_option("virtual", "true", value_type_bool);
    compiler_p->add_any_forward_doc(&message_spec_p->message_bag_p->doc_list_p);
    char *enumerator_name_p = (char*) calloc(1, 100);
    sprintf(enumerator_name_p, "%s_%s",
            name_p,
            message_spec_p->type == message_type_command ? "commands" : message_spec_p->type == message_type_response ? "responses" : "events"
           );
    assert(get_parameter_set()->get_enumerator_list(enumerator_name_p) == NULL);
    message_spec_p->id_enumerator_list_p = new enumerator_list(compiler_p, enumerator_name_p);
    get_parameter_set()->add_enumerator_list(message_spec_p->id_enumerator_list_p);
    return message_spec_p->message_bag_p;
}

bag_parameter *category::add_command_bag(char *name_p)
{
    if (command_spec_p == NULL)
        command_spec_p = new message_spec(compiler_p, message_type_command);
    return add_message_bag(command_spec_p, name_p);
}

bag_parameter *category::add_response_bag(char *name_p)
{
    if (response_spec_p == NULL)
        response_spec_p = new message_spec(compiler_p, message_type_response);
    return add_message_bag(response_spec_p, name_p);
}

bag_parameter *category::add_event_bag(char *name_p)
{
    if (event_spec_p == NULL)
        event_spec_p = new message_spec(compiler_p, message_type_event);
    return add_message_bag(event_spec_p, name_p);
}

command *category::add_command(char *name_p)
{
    parameter_set *parameter_set_p;

    command *command_p = new command(compiler_p, this, strdup(name_p));
    compiler_p->add_any_forward_doc(&command_p->doc_list_p);
    parameter_set_p = get_parameter_set();
    command_p->append_to(commands.method_list_p);
    get_command_spec()->add_to_enumerator(name_p);
    get_response_spec()->add_to_enumerator(name_p);

    if (command_p->command_bag_p == NULL) {
        char *command_bag_name_p = (char*) calloc(1, 100);
        sprintf(command_bag_name_p,
                "%s_%s",
                name_p,
                get_command_bag()->name_p
               );
        command_p->command_bag_p = (bag_parameter*)parameter_set_p->create_parameter_in_default_group("bag", command_bag_name_p);
        assert(command_p->command_bag_p != NULL);
        command_p->command_bag_p->method_ref_p = command_p;
        command_p->command_bag_p->add_parent_parameter(parameter_set_p->name_p,
                                                       get_command_bag()->name_p);
        free(command_bag_name_p);
    }

    if (command_p->response_bag_p == NULL) {
        char *response_bag_name_p = (char*) calloc(1, 100);
        sprintf(response_bag_name_p,
                "%s_%s",
                name_p,
                get_response_bag()->name_p
               );
        command_p->response_bag_p = (bag_parameter*)parameter_set_p->create_parameter_in_default_group("bag", response_bag_name_p);
        assert(command_p->response_bag_p != NULL);
        command_p->response_bag_p->method_ref_p = command_p;
        command_p->response_bag_p->add_parent_parameter(parameter_set_p->name_p,
                                                        get_response_bag()->name_p);
        free(response_bag_name_p);
    }

    return command_p;
}

void category::print_commands(int level)
{
    mpl_list_t *tmp_p;
    command *command_p;

    printf("%s", spacing(level));
    printf("commands\n");
    MPL_LIST_FOR_EACH(commands.method_list_p, tmp_p) {
        command_p = LISTABLE_PTR(tmp_p, command);
        command_p->print(level+1);
    }
}

void category::print_events(int level)
{
    mpl_list_t *tmp_p;
    event *event_p;

    printf("%s", spacing(level));
    printf("events\n");
    MPL_LIST_FOR_EACH(events.method_list_p, tmp_p) {
        event_p = LISTABLE_PTR(tmp_p, event);
        event_p->print(level+1);
    }
}

event *category::add_event(char *name_p)
{
    parameter_set *parameter_set_p;

    event *event_p = new event(compiler_p, this, strdup(name_p));
    compiler_p->add_any_forward_doc(&event_p->doc_list_p);
    parameter_set_p = get_parameter_set();
    event_p->append_to(events.method_list_p);
    get_event_spec()->add_to_enumerator(name_p);

    if (event_p->event_bag_p == NULL) {
        char *event_bag_name_p = (char*) calloc(1, 100);
        sprintf(event_bag_name_p,
                "%s_%s",
                name_p,
                get_event_bag()->name_p
               );
        event_p->event_bag_p = (bag_parameter*)parameter_set_p->create_parameter_in_default_group("bag", event_bag_name_p);
        assert(event_p->event_bag_p != NULL);
        event_p->event_bag_p->method_ref_p = event_p;
        event_p->event_bag_p->add_parent_parameter(parameter_set_p->name_p,
                                                   get_event_bag()->name_p);
        free(event_bag_name_p);
    }

    return event_p;
}

category *category::get_topmost_category()
{
    category *category_p = this;

    while (category_p) {
        if (category_p->parent_p == NULL)
            return category_p;

        category_p = (category *)category_p->parent_p;
    }
    return NULL;
}

parameter_set *category::get_parameter_set()
{
    category *category_p = this;

    while (category_p && (category_p->parameter_set_p == NULL)) {
        category_p = (category *)category_p->parent_p;
    }

    if (category_p == NULL)
        return compiler_p->current_parameter_set_p;

    return category_p->parameter_set_p;
}

message_spec *category::get_command_spec()
{
    category *category_p = this;

    while (category_p && (category_p->command_spec_p == NULL)) {
        category_p = (category *)category_p->parent_p;
    }

    if (category_p == NULL)
        return NULL;

    return category_p->command_spec_p;
}

message_spec *category::get_response_spec()
{
    category *category_p = this;

    while (category_p && (category_p->response_spec_p == NULL)) {
        category_p = (category *)category_p->parent_p;
    }

    if (category_p == NULL)
        return NULL;

    return category_p->response_spec_p;
}

message_spec *category::get_event_spec()
{
    category *category_p = this;

    while (category_p && (category_p->event_spec_p == NULL)) {
        category_p = (category *)category_p->parent_p;
    }

    if (category_p == NULL)
        return NULL;

    return category_p->event_spec_p;
}

bag_parameter *category::get_command_bag()
{
    return get_command_spec()->message_bag_p;
}

bag_parameter *category::get_response_bag()
{
    return get_response_spec()->message_bag_p;
}

bag_parameter *category::get_event_bag()
{
    return get_event_spec()->message_bag_p;
}

int category::check_parameters()
{
    mpl_list_t *tmp_p;
    command *command_p;
    event *event_p;
    int ret = 0;

    if (command_spec_p)
        ret += check_paramlist_parameters((mpl_list_t*) command_spec_p->message_bag_p->get_property("parameter_list"));
    if (response_spec_p)
        ret += check_paramlist_parameters((mpl_list_t*) response_spec_p->message_bag_p->get_property("parameter_list"));
    if (event_spec_p)
        ret += check_paramlist_parameters((mpl_list_t*) event_spec_p->message_bag_p->get_property("parameter_list"));

    MPL_LIST_FOR_EACH(commands.method_list_p, tmp_p) {
        command_p = LISTABLE_PTR(tmp_p, command);
        ret += check_paramlist_parameters((mpl_list_t*) command_p->command_bag_p->get_property("parameter_list"));
        ret += check_paramlist_parameters((mpl_list_t*) command_p->response_bag_p->get_property("parameter_list"));
    }
    MPL_LIST_FOR_EACH(events.method_list_p, tmp_p) {
        event_p = LISTABLE_PTR(tmp_p, event);
        ret += check_paramlist_parameters((mpl_list_t*) event_p->event_bag_p->get_property("parameter_list"));
    }
    return ret;
}

void category::gc(FILE *hfile_p, FILE *cfile_p, mpl_list_t *categories_p)
{
    char *lnl = get_parameter_set()->name_p;
    char *lnu = str_toupper(lnl);
    char *snl = get_parameter_set()->get_short_name();
    if (!snl) {
        fprintf(hfile_p,
                "/* Category's parameter set has no short_name defined -> no code */\n"
               );
        fprintf(cfile_p,
                "/* Category's parameter set has no short_name defined -> no code */\n"
               );
        return;
    }

    char *snu = str_toupper(snl);
    char *cnl = name_p;
    char *cnu = str_toupper(cnl);

    if (command_spec_p) {
        if (compiler_p->codegen_mode == codegen_mode_mpl) {
            /* IS_COMMAND */
            fprintf(hfile_p,
                    "/**\n"
                    "  * Check if a particular message list is a command.\n"
                    "  * @param list (in) The list to search\n"
                    "  * @return boolean (true if the message list is a command)\n"
                    "  */\n"
                   );
            fprintf(hfile_p,
                    "#define %s_IS_COMMAND(list) \\\n"
                    " MPL_PARAM_PRESENT_IN_LIST(%s_PARAM_ID(%s), list)\n",
                    cnu,
                    snu,
                    get_command_bag()->name_p
                   );
            /* IS_RESPONSE */
            fprintf(hfile_p,
                    "/**\n"
                    "  * Check if a particular message list is a response.\n"
                    "  * @param list (in) The list to search\n"
                    "  * @return boolean (true if the message list is a response)\n"
                    "  */\n"
                   );
            fprintf(hfile_p,
                    "#define %s_IS_RESPONSE(list) \\\n"
                    " MPL_PARAM_PRESENT_IN_LIST(%s_PARAM_ID(%s), list)\n",
                    cnu,
                    snu,
                    get_response_bag()->name_p
                   );
            /* IS_EVENT */
            fprintf(hfile_p,
                    "/**\n"
                    "  * Check if a particular message list is a event.\n"
                    "  * @param list (in) The list to search\n"
                    "  * @return boolean (true if the message list is a event)\n"
                    "  */\n"
                   );
            fprintf(hfile_p,
                    "#define %s_IS_EVENT(list) \\\n"
                    " MPL_PARAM_PRESENT_IN_LIST(%s_PARAM_ID(%s), list)\n",
                    cnu,
                    snu,
                    get_event_bag()->name_p
                   );
            fprintf(hfile_p,
                    "#define %s_COMMAND_ID_TYPE \\\n"
                    "    mpl_param_element_id_t\n",
                    cnu
                   );
            fprintf(hfile_p,
                    "#define %s_COMMAND_ID_VAR_DECLARE(var) \\\n"
                    "    mpl_param_element_id_t var \n",
                    cnu
                   );
            fprintf(hfile_p,
                    "#define %s_COMMAND_ID_VAR_DECLARE_INIT(var,value)  \\\n"
                    "    mpl_param_element_id_t var      \\\n"
                    "        = (value)\n",
                    cnu
                   );
            fprintf(hfile_p,
                    "#define %s_COMMAND_ID_TO_STRING_PTR(command_id)    \\\n"
                    "    mpl_param_id_get_string(command_id)\n",
                    cnu
                   );
            fprintf(hfile_p,
                    "#define %s_COMMAND_ID(name)          \\\n"
                    "    %s_paramid_             \\\n"
                    "    ##name                           \\\n"
                    "    ##_%s\n",
                    cnu,
                    lnl,
                    command_spec_p->message_bag_p->name_p
                   );
            fprintf(hfile_p,
                    "#define %s_GET_COMMAND_ID(cmdList)             \\\n"
                    "    %s_GET_ELEMENT_ID(cmdList,%s)\n",
                    cnu,
                    snu,
                    command_spec_p->message_bag_p->name_p
                   );
            fprintf(hfile_p,
                    "#define %s_GET_COMMAND_PARAMS_PTR(cmd)                \\\n"
                    "    %s_GET_BAG_PTR(cmd,%s)\n",
                    cnu,
                    snu,
                    command_spec_p->message_bag_p->name_p
                   );

            fprintf(hfile_p,
                    "#define %s_RESPONSE_ID_TYPE \\\n"
                    "    mpl_param_element_id_t\n",
                    cnu
                   );
            fprintf(hfile_p,
                    "#define %s_RESPONSE_ID_VAR_DECLARE(var) \\\n"
                    "    mpl_param_element_id_t var \n",
                    cnu
                   );
            fprintf(hfile_p,
                    "#define %s_RESPONSE_ID_VAR_DECLARE_INIT(var,value)  \\\n"
                    "    mpl_param_element_id_t var      \\\n"
                    "        = (value)\n",
                    cnu
                   );
            fprintf(hfile_p,
                    "#define %s_RESPONSE_ID_TO_STRING_PTR(response_id)    \\\n"
                    "    mpl_param_id_get_string(response_id)\n",
                    cnu
                   );
            fprintf(hfile_p,
                    "#define %s_RESPONSE_ID(name)          \\\n"
                    "    %s_paramid_             \\\n"
                    "    ##name                           \\\n"
                    "    ##_%s\n",
                    cnu,
                    lnl,
                    response_spec_p->message_bag_p->name_p
                   );
            fprintf(hfile_p,
                    "#define %s_GET_RESPONSE_ID(cmdList)             \\\n"
                    "    %s_GET_ELEMENT_ID(cmdList,%s)\n",
                    cnu,
                    snu,
                    response_spec_p->message_bag_p->name_p
                   );
            fprintf(hfile_p,
                    "#define %s_GET_RESPONSE_PARAMS_PTR(cmd)                \\\n"
                    "    %s_GET_BAG_PTR(cmd,%s)\n",
                    cnu,
                    snu,
                    response_spec_p->message_bag_p->name_p
                   );

            fprintf(hfile_p,
                    "#define %s_COMMAND_ID_TO_RESPONSE_ID(command_id)   \\\n"
                    "    %s_command_bag_param_id_to_response_bag_param_id(command_id)\n",
                    cnu,
                    cnl
                   );
            fprintf(hfile_p,
                    "mpl_param_element_id_t %s_command_bag_param_id_to_response_bag_param_id(mpl_param_element_id_t command_param_id);\n",
                    name_p
                   );
            fprintf(cfile_p,
                    "mpl_param_element_id_t %s_command_bag_param_id_to_response_bag_param_id(mpl_param_element_id_t command_bag_param_id)\n"
                    "{\n"
                    "#define %s_%s_responses_VALUE_ELEMENT(NAME)   \\\n"
                    "    case %s_paramid_ ##NAME ##_%s:            \\\n"
                    "    return %s_paramid_ ##NAME ##_%s; \\\n"
                    "\n"
                    "    switch (command_bag_param_id) {\n"
                    "        %s_%s_responses\n"
                    "        default:\n"
                    "        break;\n"
                    "    }\n"
                    "    return MPL_PARAM_ID_UNDEFINED;\n"
                    "#undef %s_%s_responses_VALUE_ELEMENT\n"
                    "}\n",
                    name_p,
                    lnu,
                    name_p,
                    lnl,
                    command_spec_p->message_bag_p->name_p,
                    lnl,
                    response_spec_p->message_bag_p->name_p,
                    lnu,
                    name_p,
                    lnu,
                    name_p
                   );
        }
        else if (compiler_p->codegen_mode == codegen_mode_cli) {
            fprintf(hfile_p,
                    "int %s_get_command_completions(const char *line, int *posindex, char *completionStrings[], int maxStrings);\n",
                    name_p
                   );
            fprintf(hfile_p,
                    "int %s_get_command_help(const char *line, char **helptext);\n",
                    name_p
                   );
            cli_command_completions(cfile_p);
            cli_command_help(cfile_p);
        }
        else if (compiler_p->codegen_mode == codegen_mode_api) {
        }
    }

    if ((compiler_p->codegen_mode == codegen_mode_cli) ||
        (compiler_p->codegen_mode == codegen_mode_api)) {
        mpl_list_t *tmp_p;
        command *command_p;
        event *event_p;
        char *tcn = fix_for_header_define(get_topmost_category()->name_p);

        if (compiler_p->codegen_mode == codegen_mode_api) {
            if (parent_p == NULL) {
                char *indent = strdup("        ");
                fprintf(hfile_p,
                        "namespace %s {\n",
                        get_parameter_set()->name_p
                       );
                fprintf(hfile_p,
                        "    namespace %s {\n",
                        name_p
                       );
                fprintf(cfile_p,
                        "namespace %s {\n",
                        get_parameter_set()->name_p
                       );
                fprintf(cfile_p,
                        "    namespace %s {\n",
                        name_p
                       );
                api_hh(hfile_p, indent);
                api_cc(cfile_p, indent);
                fprintf(hfile_p,
                        "    }\n"
                       );
                fprintf(hfile_p,
                        "}\n"
                       );
                fprintf(cfile_p,
                        "    }\n"
                       );
                fprintf(cfile_p,
                        "}\n"
                       );
                free(indent);
            }
        }
        MPL_LIST_FOR_EACH(commands.method_list_p, tmp_p) {
            command_p = LISTABLE_PTR(tmp_p, command);
            if (compiler_p->codegen_mode == codegen_mode_cli) {
                fprintf(hfile_p,
                        "int %s_%s_get_parameter_completions(const char *line, int *posindex, char *completionStrings[], int maxStrings);\n",
                        tcn,
                        command_p->name_p
                       );
                fprintf(hfile_p,
                        "int %s_%s_get_command_parameter_help(char **helptext);\n",
                        tcn,
                        command_p->name_p
                       );
                cli_parameter_completions(cfile_p, command_p);
                cli_parameter_help(cfile_p, command_p);
            }
        }
        free(tcn);
    }

    if ((compiler_p->codegen_mode == codegen_mode_mpl) && event_spec_p) {
        fprintf(hfile_p,
                "#define %s_EVENT_ID_TYPE \\\n"
                "    mpl_param_element_id_t\n",
                cnu
               );
        fprintf(hfile_p,
                "#define %s_EVENT_ID_VAR_DECLARE(var) \\\n"
                "    mpl_param_element_id_t var \n",
                cnu
               );
        fprintf(hfile_p,
                "#define %s_EVENT_ID_VAR_DECLARE_INIT(var,value)  \\\n"
                "    mpl_param_element_id_t var      \\\n"
                "        = (value)\n",
                cnu
               );
        fprintf(hfile_p,
                "#define %s_EVENT_ID_TO_STRING_PTR(event_id)    \\\n"
                "    mpl_param_id_get_string(event_id)\n",
                cnu
               );
        fprintf(hfile_p,
                "#define %s_EVENT_ID(name)          \\\n"
                "    %s_paramid_             \\\n"
                "    ##name                           \\\n"
                "    ##_%s\n",
                cnu,
                lnl,
                event_spec_p->message_bag_p->name_p
               );
        fprintf(hfile_p,
                "#define %s_GET_EVENT_ID(cmdList)             \\\n"
                "    %s_GET_ELEMENT_ID(cmdList,%s)\n",
                cnu,
                snu,
                event_spec_p->message_bag_p->name_p
               );
        fprintf(hfile_p,
                "#define %s_GET_EVENT_PARAMS_PTR(cmd)                \\\n"
                "    %s_GET_BAG_PTR(cmd,%s)\n",
                cnu,
                snu,
                event_spec_p->message_bag_p->name_p
               );
    }
    free(lnu);
    free(cnu);
    free(snu);
}

void category::api_hh(FILE *f, char *indent)
{
    mpl_list_t *tmp_p;
    parameter_list_entry *parameter_list_entry_p;
    parameter *parameter_p;

    if (parent_p == NULL) {
        fprintf(f,
                "%s    int send(%s *command_p);\n",
                indent,
                get_command_bag()->name_p
               );
        fprintf(f,
                "%s    int __send(%s *command_p);\n",
                indent,
                get_command_bag()->name_p
               );
        fprintf(f,
                "%s    BAG *receive(mpl_list_t *inMsg);\n",
                indent
               );
        fprintf(f,
                "#define __SEND %s::__send\n",
                name_p
               );
    }
    return;
}

void category::api_cc(FILE *f, char *indent)
{
    char *snl = get_parameter_set()->get_short_name();
    char *snu = str_toupper(snl);
    char *cnu = str_toupper(name_p);

    if (parent_p == NULL) {


        fprintf(f,
                "%sint send(%s *command_p)\n"
                "%s{\n"
                "%s    return __SEND(command_p);\n"
                "%s}\n",
                indent,
                get_command_bag()->name_p,
                indent,
                indent,
                indent
               );
        fprintf(f,
                "%sint __send(%s *command_p)\n"
                "%s{\n",
                indent,
                get_command_bag()->name_p,
                indent
               );
#define INDENT(str) fprintf(f, "%s%s", indent, str)
        INDENT("    char *buf = NULL;\n");
        INDENT("    int len;\n");
        INDENT("    int ret = -1;\n");
        INDENT("    mpl_list_t *outMsg;\n");
        INDENT("    mpl_param_element_t *__elem;\n");
        INDENT("    __elem = mpl_param_element_create_empty(command_p->id());\n");
        INDENT("    __elem->value_p = command_p->encode();\n");
        INDENT("    outMsg = &__elem->list_entry;\n");
        INDENT("    len = mpl_param_list_pack(outMsg, NULL, 0);\n");
        INDENT("    if (len <= 0) {\n");
        INDENT("        printf(\"!!! FAILED PACKING MESSAGE !!!\\n\");\n");
        INDENT("        goto exit;\n");
        INDENT("    }\n");
        INDENT("    buf = (char*) malloc(len+1);\n");
        INDENT("    len = mpl_param_list_pack(outMsg, buf, len+1);\n");
        INDENT("    mpl_param_list_destroy(&outMsg);\n");
        INDENT("    if (len <= 0) {\n");
        INDENT("        printf(\"!!! FAILED PACKING MESSAGE !!!\\n\");\n");
        INDENT("        goto exit;\n");
        INDENT("    }\n");
        INDENT("    printf(\"Message sent:'%s'\\n\", buf);\n");
        INDENT("    ret = 0;\n");
        INDENT("exit:\n");
        INDENT("    free(buf);\n");
        INDENT("    return ret;\n");
        INDENT("}\n");
        fprintf(f,
                "\n"
               );
#undef INDENT
    }

    fprintf(f,
            "%sBAG *receive(mpl_list_t *inMsg)\n",
            indent
           );
    fprintf(f,
            "%s{\n",
            indent
           );
    fprintf(f,
            "%s    if (%s_IS_COMMAND(inMsg)) {\n"
            "%s        mpl_bag_t *params_p = %s_GET_COMMAND_PARAMS_PTR(inMsg);\n"
            "%s        switch (%s_GET_COMMAND_ID(inMsg)) {\n",
            indent,
            cnu,
            indent,
            cnu,
            indent,
            cnu
           );

    mpl_list_t *tmp_p;
    mpl_list_t *clist_p = NULL;
    object_container *container_p = new object_container(this);
    container_p->append_to(clist_p);
    mpl_list_append(&clist_p, get_flat_child_list());
    MPL_LIST_FOR_EACH(clist_p, tmp_p) {
        object_container *container_p = LISTABLE_PTR(tmp_p, object_container);
        category *child_p = (category*) container_p->object_p;
        mpl_list_t *tmp_p;

        MPL_LIST_FOR_EACH(child_p->commands.method_list_p, tmp_p) {
            command *command_p = LISTABLE_PTR(tmp_p, command);
            fprintf(f,
                    "%s            case %s_PARAM_ID(%s_%s):\n"
                    "%s                return new %s_%s(params_p);\n",
                    indent,
                    snu,
                    command_p->name_p,
                    child_p->get_command_bag()->name_p,
                    indent,
                    command_p->name_p,
                    child_p->get_command_bag()->name_p
                   );
        }
    }
    DELETE_LISTABLE_LIST(&clist_p, object_container);

    fprintf(f,
            "%s            default:\n"
            "%s                return NULL;\n",
            indent,
            indent
           );
    fprintf(f,
            "%s        }\n"
            "%s    }\n",
            indent,
            indent
           );
    fprintf(f,
            "%s    else if (%s_IS_RESPONSE(inMsg)) {\n"
            "%s        mpl_bag_t *params_p = %s_GET_RESPONSE_PARAMS_PTR(inMsg);\n"
            "%s        switch (%s_GET_RESPONSE_ID(inMsg)) {\n",
            indent,
            cnu,
            indent,
            cnu,
            indent,
            cnu
           );

    clist_p = NULL;
    container_p = new object_container(this);
    container_p->append_to(clist_p);
    mpl_list_append(&clist_p, get_flat_child_list());
    MPL_LIST_FOR_EACH(clist_p, tmp_p) {
        object_container *container_p = LISTABLE_PTR(tmp_p, object_container);
        category *child_p = (category*) container_p->object_p;
        mpl_list_t *tmp_p;

        MPL_LIST_FOR_EACH(child_p->commands.method_list_p, tmp_p) {
            command *command_p = LISTABLE_PTR(tmp_p, command);
            fprintf(f,
                    "%s            case %s_PARAM_ID(%s_%s):\n"
                    "%s                return new %s_%s(params_p);\n",
                    indent,
                    snu,
                    command_p->name_p,
                    child_p->get_response_bag()->name_p,
                    indent,
                    command_p->name_p,
                    child_p->get_response_bag()->name_p
                   );
        }
    }
    DELETE_LISTABLE_LIST(&clist_p, object_container);

    fprintf(f,
            "%s            default:\n"
            "%s                return NULL;\n",
            indent,
            indent
           );
    fprintf(f,
            "%s        }\n"
            "%s    }\n",
            indent,
            indent
           );
    fprintf(f,
            "%s    else if (%s_IS_EVENT(inMsg)) {\n"
            "%s        mpl_bag_t *params_p = %s_GET_EVENT_PARAMS_PTR(inMsg);\n"
            "%s        switch (%s_GET_EVENT_ID(inMsg)) {\n",
            indent,
            cnu,
            indent,
            cnu,
            indent,
            cnu
           );

    clist_p = NULL;
    container_p = new object_container(this);
    container_p->append_to(clist_p);
    mpl_list_append(&clist_p, get_flat_child_list());
    MPL_LIST_FOR_EACH(clist_p, tmp_p) {
        object_container *container_p = LISTABLE_PTR(tmp_p, object_container);
        category *child_p = (category*) container_p->object_p;
        mpl_list_t *tmp_p;

        MPL_LIST_FOR_EACH(child_p->events.method_list_p, tmp_p) {
            event *event_p = LISTABLE_PTR(tmp_p, event);
            fprintf(f,
                    "%s            case %s_PARAM_ID(%s_%s):\n"
                    "%s                return new %s_%s(params_p);\n",
                    indent,
                    snu,
                    event_p->name_p,
                    child_p->get_event_bag()->name_p,
                    indent,
                    event_p->name_p,
                    child_p->get_event_bag()->name_p
                   );
        }
    }
    DELETE_LISTABLE_LIST(&clist_p, object_container);

    fprintf(f,
            "%s            default:\n"
            "%s                return NULL;\n",
            indent,
            indent
           );

    fprintf(f,
            "%s        }\n"
            "%s    }\n",
            indent,
            indent
           );
    fprintf(f,
            "%s    return NULL;\n"
            "%s}\n",
            indent,
            indent
           );
    free(snu);
    free(cnu);
}

void category::dox_commands(FILE *f)
{
    mpl_list_t *tmp_p;
    command *command_p;

    fprintf(f, "%s", spacing(1));
    dox_doc_f(f, commands.doc_list_p);

    MPL_LIST_FOR_EACH(commands.method_list_p, tmp_p) {
        command_p = LISTABLE_PTR(tmp_p, command);
        command_p->dox(f, 1);
    }
    fprintf(f, "\n");
}

void category::dox_events(FILE *f)
{
    mpl_list_t *tmp_p;
    event *event_p;

    fprintf(f, "%s", spacing(1));
    dox_doc_f(f, events.doc_list_p);

    MPL_LIST_FOR_EACH(events.method_list_p, tmp_p) {
        event_p = LISTABLE_PTR(tmp_p, event);
        event_p->dox(f, 1);
    }
    fprintf(f, "\n");
}

void category::dox(FILE *f)
{
    parameter_set *parameter_set_p;

    parameter_set_p = get_parameter_set();
    fprintf(f,
            "using namespace %s;\n",
            parameter_set_p->name_p
           );
    fprintf(f, "\n");

    dox_commands(f);
    dox_events(f);

    fprintf(f, "\n");

}

void category::latex(FILE *f)
{
    LX_S("%s",
         group_p ? group_p->text_p : name_p
        );
    LX_LABEL("cat:%s",
             name_p
            );

    latex_latex_list(f, latex_list_p);

    fprintf(f,
            "\n"
           );

    lx_table_begin(f, 3);
    LX_HLINE;
    LX_TABH(3, "%s",
            "Category Properties"
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

    if (parameter_set_p) {
        fprintf_latex(f,
                      "Parameter set"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      parameter_set_p->group_p ? parameter_set_p->group_p->text_p : parameter_set_p->name_p
                     );
        fprintf(f,
                " \\ref{pset:%s}",
                parameter_set_p->name_p
               );
        LX_COL_SEP;
        fprintf_latex(f,
                      "Parameters defined in this category belongs to this parameter set."
                     );
        LX_COL_END;
    }


    if (command_spec_p) {
        fprintf_latex(f,
                      "Command bag"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      command_spec_p->message_bag_p->name_p
                     );
        fprintf(f,
                " \\ref{%s:%s}",
                command_spec_p->message_bag_p->parameter_set_p->name_p,
                command_spec_p->message_bag_p->name_p
               );
        LX_COL_SEP;
        fprintf_latex(f,
                      "Defines the parent bag of all commands in the category.\n"
                      "Each command defines a child bag\n"
                      "adding in-parameters of the command as members.\n"
                     );
        LX_COL_END;
    }

    if (response_spec_p) {
        fprintf_latex(f,
                      "Response bag"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      response_spec_p->message_bag_p->name_p
                     );
        fprintf(f,
                " \\ref{%s:%s}",
                response_spec_p->message_bag_p->parameter_set_p->name_p,
                response_spec_p->message_bag_p->name_p
               );
        LX_COL_SEP;
        fprintf_latex(f,
                      "Defines the parent bag of all command responses in the category.\n"
                      "Each command defines a child bag\n"
                      "adding out- and inout-parameters of the command as members.\n"
                     );
        LX_COL_END;
    }

    if (event_spec_p) {
        fprintf_latex(f,
                      "Event bag"
                     );
        LX_COL_SEP;
        fprintf_latex(f,
                      "%s",
                      event_spec_p->message_bag_p->name_p
                     );
        fprintf(f,
                " \\ref{%s:%s}",
                event_spec_p->message_bag_p->parameter_set_p->name_p,
                event_spec_p->message_bag_p->name_p
               );
        LX_COL_SEP;
        fprintf_latex(f,
                      "Defines the parent bag of all events in the category.\n"
                      "Each event defines a child bag\n"
                      "adding out-parameters of the event as members.\n"
                     );
        LX_COL_END;
    }

    if (parent_p) {
        fprintf(f,
                "Parents");
        LX_COL_SEP;

        category *p_p = (category *)parent_p;

        while (p_p) {
            fprintf_latex(f,
                          "%s",
                          p_p->group_p ? p_p->group_p->text_p : p_p->name_p
                         );
            fprintf(f,
                    " \\ref{cat:%s}",
                    p_p->name_p
                   );
            LX_NEWLINE;
            p_p = (category *)p_p->parent_p;
        }
        LX_COL_SEP;
        fprintf_latex(f,
                      "This category inherits another category, i.e. properties of the\n"
                      "parent category (if not overridden here) will also be valid for\n"
                      "this category."
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
            category *child_p = (category*) container_p->object_p;
            fprintf_latex(f,
                          "%s",
                          child_p->group_p ? child_p->group_p->text_p : child_p->name_p
                         );
            fprintf(f,
                    " \\ref{cat:%s}",
                    child_p->name_p
                   );
            LX_NEWLINE;
        }
        DELETE_LISTABLE_LIST(&fcl_p, object_container);
        LX_COL_SEP;
        fprintf_latex(f,
                      "This category is a parent, i.e. properties of this\n"
                      "category (if not overridden in the child) will also be valid for\n"
                      "the child."
                     );
        LX_COL_END;
    }

    LX_HLINE;
    LX_TAB_END;

    fprintf(f,
            "\n"
           );

    LX_SS("%s",
          commands.group_p ? commands.group_p->text_p : "Commands");
    if (commands.method_list_p) {
        latex_latex_list(f, commands.latex_list_p);
        latex_commands(f);
    }
    else {
        fprintf_latex(f,
                      "No commands"
                     );
    }

    LX_SS("%s",
          events.group_p ? events.group_p->text_p : "Events");

    if (events.method_list_p) {
        latex_latex_list(f, events.latex_list_p);
        latex_events(f);
    }
    else {
        fprintf_latex(f,
                      "No events"
                     );
    }

    parameter_set *parameter_set_p = get_parameter_set();
    parameter_group *parameter_group_p = parameter_set_p->find_parameter_group(name_p);
    if (parameter_group_p)
        parameter_group_p->latex(f);
}

void category::latex_commands(FILE *f)
{
    mpl_list_t *tmp_p;
    command *command_p;

    MPL_LIST_FOR_EACH(commands.method_list_p, tmp_p) {
        command_p = LISTABLE_PTR(tmp_p, command);
        command_p->latex(f, get_parameter_set());
    }
}

void category::latex_events(FILE *f)
{
    mpl_list_t *tmp_p;
    event *event_p;

    MPL_LIST_FOR_EACH(events.method_list_p, tmp_p) {
        event_p = LISTABLE_PTR(tmp_p, event);
        event_p->latex(f, get_parameter_set());
    }
}


void category::convert_doc()
{
    listable_object::convert_doc("category", name_p);
    if (command_spec_p)
        command_spec_p->convert_doc("command_spec");
    if (response_spec_p)
        response_spec_p->convert_doc("response_spec");
    if (event_spec_p)
        event_spec_p->convert_doc("event_spec");

    commands.convert_doc();
    events.convert_doc();
}


void category::deja(FILE *f, char *indent)
{
    char newindent[255] = {0};
    fprintf(f,
            "%s# category %s\n",
            indent,
            name_p
           );

    if (get_parameter_set()->get_short_name() == NULL) {
        fprintf(f,
                "%s# short_name not set for category parameter set -> no code\n",
                indent
               );
        return;
    }
    fprintf(f,
            "%snamespace eval %s {\n",
            indent,
            name_p
           );
    sprintf(newindent, "%s    ",
           indent);

    deja_commands(f, newindent);
    deja_events(f, newindent);
    fprintf(f,
            "%s}\n"
            "%s# end category %s\n\n",
            indent,
            indent,
            name_p
           );
}

void category::deja_commands(FILE *f, char *indent)
{
    mpl_list_t *tmp_p;
    command *command_p;

    MPL_LIST_FOR_EACH(commands.method_list_p, tmp_p) {
        command_p = LISTABLE_PTR(tmp_p, command);
        command_p->deja(f, indent);
    }
}

void category::deja_events(FILE *f, char *indent)
{
    mpl_list_t *tmp_p;
    event *event_p;

    MPL_LIST_FOR_EACH(events.method_list_p, tmp_p) {
        event_p = LISTABLE_PTR(tmp_p, event);
        event_p->deja(f, indent);
    }
}

void category::wrap_up_definition()
{
    if (!get_response_spec()) {
        fprintf(stderr, "%s:%d: no command specification\n",
                compiler_p->peek_filename(), compiler_p->lineno());
        exit(-1);
    }

    if (!get_command_spec()) {
        fprintf(stderr, "%s:%d: no command specification\n",
                compiler_p->peek_filename(), compiler_p->lineno());
        exit(-1);
    }

    if (commands.method_list_p) {
        get_command_spec()->wrap_up_definition(block_type_command_bag);
        get_response_spec()->wrap_up_definition(block_type_response_bag);

        mpl_list_t *tmp_p;
        command *command_p;

        MPL_LIST_FOR_EACH(commands.method_list_p, tmp_p) {
            command_p = LISTABLE_PTR(tmp_p, command);
            command_p->wrap_up_definition();
        }
    }
    if (events.method_list_p) {
        get_event_spec()->wrap_up_definition(block_type_event_bag);

        mpl_list_t *tmp_p;
        event *event_p;

        MPL_LIST_FOR_EACH(events.method_list_p, tmp_p) {
            event_p = LISTABLE_PTR(tmp_p, event);
            event_p->wrap_up_definition();
        }
    }
}

void category::cli_command_completions(FILE *f)
{
    char *lnl = get_parameter_set()->name_p;
    char *lnu = str_toupper(lnl);

    fprintf(f,
            "int %s_get_command_completions(const char *line, int *posindex, char *completionStrings[], int maxStrings)\n"
            "{\n"
            "    int numStrings = 0;\n"
            "    int len = strlen(line);\n"
            "    /* supress not-used warning: */\n"
            "    (void) get_matching_close_bracket;\n"
            "    (void) param_end;\n"
            "    (void) push_stack;\n"
            "    (void) pop_stack;\n"
            "    (void) get_word_start;\n"
            "    (void) get_value_start;\n"
            "    (void) create_completion_string;\n"
            "    (void) free_completion_strings;\n"
            "    (void) pop_all_bag_ends;\n"
            "    (void) forward_to_param_start;\n"
            "    (void) backward_to_param_start;\n"
            "    (void) formatCmdLine;\n"
            "    (void) get_bool_completions;\n"
            "    (void) get_int_completions;\n"
            "    (void) get_array_completions;\n"
            "    (void) get_addr_completions;\n"
            "    (void) get_tuple_completions;\n"
            "\n",
            name_p
           );
    fprintf(f,
            "#define %s_%s_commands_VALUE_ELEMENT(NAME)               \\\n"
            "    if ((len > 4) && !strncmp(\"help \" #NAME,line,len)) {                      \\\n"
            "        completionStrings[numStrings++] = strdup(\"help \" #NAME); \\\n"
            "        if (numStrings >= maxStrings) goto do_exit;      \\\n"
            "    }                                                    \\\n"
            "\n"
            "        %s_%s_commands\n"
            "\n"
            "#undef %s_%s_commands_VALUE_ELEMENT\n"
            "\n",
            lnu,
            name_p,
            lnu,
            name_p,
            lnu,
            name_p
           );
    get_parameter_set()->cli_parameter_help_completions(f);
    fprintf(f,
            "#define %s_%s_commands_VALUE_ELEMENT(NAME)               \\\n"
            "    if (!strncmp(#NAME,line,strlen(#NAME))) {            \\\n"
            "        int namelen = strlen(#NAME);                     \\\n"
            "        if ((len > namelen) && ((line)[namelen] == ' ')) { \\\n"
            "            completion_callstack_entry_t *e = calloc(1, sizeof(*e));\\\n"
            "            e->func = %s_ ##NAME ##_get_parameter_completions; \\\n"
            "            mpl_list_add(&completionCallstack, &e->list_entry);\\\n"
            "            *posindex = namelen + 1;                           \\\n"
            "            free_completion_strings(completionStrings, numStrings);\\\n"
            "            return %s_ ##NAME ##_get_parameter_completions(line, \\\n"
            "                                                           posindex,\\\n"
            "                                                           completionStrings, \\\n"
            "                                                           maxStrings);\\\n"
            "        }                                                \\\n"
            "    }                                                    \\\n"
            "    if (!strncmp(#NAME,line,len)) {                      \\\n"
            "        completionStrings[numStrings++] = strdup(#NAME); \\\n"
            "        if (numStrings >= maxStrings) goto do_exit;      \\\n"
            "    }                                                    \\\n"
            "\n"
            "        %s_%s_commands\n"
            "\n"
            "#undef %s_%s_commands_VALUE_ELEMENT\n"
            "\n"
            "do_exit:\n"
            "    return numStrings;\n",
            lnu,
            name_p,
            name_p,
            name_p,
            lnu,
            name_p,
            lnu,
            name_p
           );
    fprintf(f,
            "}\n"
            "\n"
           );
    free(lnu);
}

void category::cli_parameter_completions(FILE *f, command *command_p)
{
    mpl_list_t *tmp_p;
    char *tcn = fix_for_header_define(get_topmost_category()->name_p);
    fprintf(f,
            "int %s_%s_get_parameter_completions(const char  *line, int *posindex, char *completionStrings[], int maxStrings)\n"
            "{\n",
            tcn,
            command_p->name_p
           );
    free(tcn);
    int parameter_found = 0;
    MPL_LIST_FOR_EACH((mpl_list_t*)command_p->command_bag_p->get_property("parameter_list"), tmp_p) {
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
            "    const char *pos = &line[*posindex];\n"
            "    const char *p = pos + strlen(pos) - 1;\n"
           );
    fprintf(f,
            "    p = get_word_start(pos, p);\n"
           );
    MPL_LIST_FOR_EACH((mpl_list_t*)command_p->command_bag_p->get_property("parameter_list"), tmp_p) {
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
            char *tcn = fix_for_header_define(get_topmost_category()->name_p);

            if (parameter_p->is_bag()) {
                fprintf(f,
                        "    if (!strncmp(\"%s\",pos,%zd)) {\n"
                        "        if ((strlen(pos) > %zd) && strstr(pos, \"={\")) {\n"
                        "            push_stack(%s_%s_get_parameter_completions);\n"
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
                        tcn,
                        command_p->name_p,
                        psn,
                        pn
                       );
            }
            else if (parameter_p->is_enum()) {
                fprintf(f,
                        "    if (!strncmp(\"%s\",pos,%zd)) {\n"
                        "        if ((strlen(pos) > %zd) && strstr(pos, \"=\")) {\n"
                        "            push_stack(%s_%s_get_parameter_completions);\n"
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
                        tcn,
                        command_p->name_p,
                        psn,
                        pn
                       );
            }
            else if (parameter_p->is_bool()) {
                fprintf(f,
                        "    if (!strncmp(\"%s\",pos,%zd)) {\n"
                        "        if ((strlen(pos) > %zd) && strstr(pos, \"=\")) {\n"
                        "            push_stack(%s_%s_get_parameter_completions);\n"
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
                        tcn,
                        command_p->name_p
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
                        "            push_stack(%s_%s_get_parameter_completions);\n"
                        "            *posindex = (strstr(pos, \"=\") + 1) - line;\n"
                        "            free_completion_strings(completionStrings, numStrings);\n",
                        nm,
                        strlen(nm),
                        strlen(nm),
                        tcn,
                        command_p->name_p
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
                        "        if ((strlen(pos) > %zd) && strstr(pos, \" \")) {\n"
                        "            *posindex = (strstr(pos, \" \") + 1) - line;\n"
                        "            free_completion_strings(completionStrings, numStrings);\n"
                        "            return %s_%s_get_parameter_completions(line,\n"
                        "                                            posindex,\n"
                        "                                            completionStrings,\n"
                        "                                            maxStrings);\n"
                        "        }\n"
                        "    }\n",
                        nm,
                        strlen(nm),
                        strlen(nm),
                        tcn,
                        command_p->name_p
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
            free(tcn);
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


void category::cli_command_help(FILE *f)
{
    char *lnl = get_parameter_set()->name_p;
    char *lnu = str_toupper(lnl);

    fprintf(f,
            "int %s_get_command_help(const char *line, char **helptext)\n"
            "{\n",
            name_p
           );
    fprintf(f,
            "#define %s_%s_commands_VALUE_ELEMENT(NAME)               \\\n"
            "    if (!strcmp(\"help \" #NAME, line)) \\\n"
            "        return %s_ ##NAME ##_get_command_parameter_help(helptext); \\\n"
            "\n"
            "        %s_%s_commands\n"
            "\n"
            "#undef %s_%s_commands_VALUE_ELEMENT\n"
            "\n",
            lnu,
            name_p,
            name_p,
            lnu,
            name_p,
            lnu,
            name_p
           );
    get_parameter_set()->cli_call_parameter_help(f);
    fprintf(f,
            "    *helptext = NULL;\n"
            "    return -1;\n"
            "}\n"
            "\n"
           );
    free(lnu);
}


void category::cli_parameter_help(FILE *f, command *command_p)
{
    char *tcn = fix_for_header_define(get_topmost_category()->name_p);
    fprintf(f,
            "int %s_%s_get_command_parameter_help(char  **helptext)\n"
            "{\n",
            tcn,
            command_p->name_p
           );
    free(tcn);

    ostringstream help_stream(ostringstream::out);
    string s;
    command_p->help(help_stream, get_parameter_set());
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

