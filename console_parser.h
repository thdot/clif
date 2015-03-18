#ifndef H_CONSOLE_PARSER
#define H_CONSOLE_PARSER 1

#define BOOST_PP_VARIADICS

#include <stdlib.h>
#include <string.h>
#include <boost/preprocessor.hpp>
#include <boost/preprocessor/facilities/is_empty.hpp>
#include <boost/preprocessor/control/expr_iif.hpp>
#include "tree.h"


#define CLI_MAX_NR_OF_PARAMETERS 5

struct parameter_descriptor
{
    const char * type;
    const char * name;
    const char * helptext;
    size_t size;
    void * value;
    int (* parser)(char *, void *);
};

struct cmd_descriptor
{
    RB_ENTRY(cmd_descriptor) descriptor;
    const char * command;
    const char * helptext;
    void* functionToCall;
    void (* caller)(struct cmd_descriptor *);
    short numberOfParameters;
    struct parameter_descriptor params[CLI_MAX_NR_OF_PARAMETERS];
};

static int cmp_cmd_descriptor(struct cmd_descriptor * d1, struct cmd_descriptor * d2)
{
    return strcmp(d1->command, d2->command);
}

RB_HEAD(cmdtree, cmd_descriptor) cmdtree_head = RB_INITIALIZER(&cmdtree_head);
RB_GENERATE(cmdtree, cmd_descriptor, descriptor, cmp_cmd_descriptor)


#define _CLI_PARSE_TYPE_LIST(argtypes) BOOST_PP_SEQ_FOR_EACH_I(_CLI_PARSE_TYPE, _, argtypes)
#define _CLI_PARSE_TYPE(r, _, i, type) BOOST_PP_COMMA_IF(i) type

#define _CLI_ADD_PARAM_DESCRIPTORS(cmd_descr, argtypes)                                             \
    BOOST_PP_SEQ_FOR_EACH_I(_CLI_ADD_PARAM, cmd_descr, argtypes)
#define _CLI_ADD_PARAM(r, cmd_descr, i, type)                                                       \
    add_param_descriptor(cmd_descr, i, sizeof(type), BOOST_PP_STRINGIZE(type),                      \
            (int (*)(char *, void *))BOOST_PP_CAT(cli_parse_,type));

#define _CLI_PARSE_CAST_VALUES(descriptor, argtypes)                                                \
    BOOST_PP_SEQ_FOR_EACH_I(_CLI_PARSE_CAST_VAL, _, argtypes)
#define _CLI_PARSE_CAST_VAL(r, _, i, type)                                                          \
    BOOST_PP_COMMA_IF(i) (*(type*)descriptor->params[i].value)

#define _CLI_CALLER_FN(argtypes)   BOOST_PP_CAT(cli_call_cmd_,     BOOST_PP_SEQ_CAT(argtypes))
#define _CLI_REGISTER_FN(argtypes) BOOST_PP_CAT(cli_register_cmd_, BOOST_PP_SEQ_CAT(argtypes))

#define CLI_REGISTER_CMD_PROTOTYPE(argtypes)                                                        \
    void _CLI_CALLER_FN(argtypes) (struct cmd_descriptor * descriptor)                              \
    {                                                                                               \
        ((void (*)(_CLI_PARSE_TYPE_LIST(argtypes)))descriptor->functionToCall)(                     \
            _CLI_PARSE_CAST_VALUES(descriptor, argtypes));                                          \
    }                                                                                               \
    void _CLI_REGISTER_FN(argtypes) (                                                               \
        const char* command,                                                                        \
        void (* functionToCall)(_CLI_PARSE_TYPE_LIST(argtypes)),                                    \
        const char* doc, const char* cmdgroup)                                                      \
    {                                                                                               \
        struct cmd_descriptor* cmd_descr = add_cmd_descriptor(                                      \
            command, functionToCall, _CLI_CALLER_FN(argtypes), BOOST_PP_SEQ_SIZE(argtypes),         \
            doc, cmdgroup);                                                                         \
        _CLI_ADD_PARAM_DESCRIPTORS(cmd_descr, argtypes)                                             \
    }                                                                                               \

#define _CLI_ARG_TYPE_doc(unused...)        1
#define _CLI_ARG_TYPE_arg(unused...)        2
#define _CLI_ARG_TYPE_cmd_group(unused...)  3

#define _CLI_ARG_doc(helptext) helptext
#define _CLI_ARG_cmd_group(group) group

#define _CLI_ARG_TYPE_IS(arg, type)                                                                 \
    BOOST_PP_EQUAL(BOOST_PP_CAT(_CLI_ARG_TYPE_, arg), type)

#define _CLI_HAS_ARG_I(r, type, arg)                                                                \
    BOOST_PP_EXPR_IIF(_CLI_ARG_TYPE_IS(arg, type), y)

#define _CLI_HAS_ARG(argtype, args)                                                                 \
    BOOST_PP_NOT(BOOST_PP_IS_EMPTY(BOOST_PP_SEQ_FOR_EACH(_CLI_HAS_ARG_I, argtype, args)))

#define _CLI_GET_ARG_I(r, type, arg)                                                                \
    BOOST_PP_EXPR_IIF(_CLI_ARG_TYPE_IS(arg, type), BOOST_PP_CAT(_CLI_ARG_, arg))

#define _CLI_GET_ARG(argtype, args, defaultvalue)                                                   \
    BOOST_PP_IIF(                                                                                   \
        _CLI_HAS_ARG(argtype, args),                                                                \
        BOOST_PP_SEQ_FOR_EACH(_CLI_GET_ARG_I, argtype, args),                                       \
        defaultvalue)

#define _CLI_HANDLE_REGISTER_CMD_ARGS(args)                                                         \
    _CLI_GET_ARG(_CLI_ARG_TYPE_doc(), args, ""),                                                    \
    _CLI_GET_ARG(_CLI_ARG_TYPE_cmd_group(), args, "")


#define CLI_REGISTER_CMD(command, function, argtypes, args...)                                      \
    _CLI_REGISTER_FN(argtypes) (command, function,                                                  \
            _CLI_HANDLE_REGISTER_CMD_ARGS( BOOST_PP_VARIADIC_TO_SEQ(args) ));


static struct cmd_descriptor* add_cmd_descriptor(const char* command, void* functionToCall,
        void (* caller)(struct cmd_descriptor *), short numberOfParameters,
        const char* doc, const char* cmdgroup)
{
    struct cmd_descriptor* new_descriptor = malloc(sizeof(struct cmd_descriptor));
    memset(new_descriptor, 0x00, sizeof(struct cmd_descriptor));
    new_descriptor->command = strdup(command);
    new_descriptor->functionToCall = functionToCall;
    new_descriptor->caller = caller;
    new_descriptor->numberOfParameters = numberOfParameters;
    new_descriptor->helptext = strdup(doc);
    RB_INSERT(cmdtree, &cmdtree_head, new_descriptor);
    return new_descriptor;
}

static void add_param_descriptor(struct cmd_descriptor* cmd_descr, short index, size_t size,
        const char * type, int (* parser)(char *, void *))
{
    cmd_descr->params[index].size = size;
    cmd_descr->params[index].parser = parser;
    cmd_descr->params[index].type = type;
}

int cli_parse(char * line)
{
    int i, result = 0;
    struct cmd_descriptor find, *descr;
    char * token;
    char * parseline = strdup(line);

    token = strtok(parseline, " ");
    if (token == NULL)
        return 0;

    find.command = token;
    descr = RB_FIND(cmdtree, &cmdtree_head, &find);

    if (! descr) {
        printf("command not found: %s\n", token);
        return 1;
    }

    token = strtok(NULL, " ");
    while (token != NULL) {
        if (i > descr->numberOfParameters) {
            printf("invalid number of arguments\n");
            result = 1;
            goto out;
        }
        descr->params[i].value = malloc(descr->params[i].size);
        (*descr->params[i].parser)(token, descr->params[i].value);
        token = strtok(NULL, " ");
        i++;
    }
    if (i != descr->numberOfParameters) {
        printf("invalid number of arguments\n");
        result = 1;
        goto out;
    }

    (*descr->caller)(descr);

out:
    for (i = 0; i < descr->numberOfParameters; i++) {
        free(descr->params[i].value);
        descr->params[i].value = NULL;
    }
    free(parseline);

    return result;
}

static int cli_parse_int(char * token, int * result)
{
    return sscanf(token, "%d", result) != 1;
}

#endif
