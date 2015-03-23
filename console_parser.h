#ifndef H_CONSOLE_PARSER
#define H_CONSOLE_PARSER 1

#define BOOST_PP_VARIADICS

#include <stdlib.h>
#include <string.h>
#include <boost/preprocessor.hpp>
#include <boost/preprocessor/facilities/is_empty.hpp>
#include <boost/preprocessor/control/expr_iif.hpp>
#include "tree.h"


#define CLIF_MAX_NR_OF_PARAMETERS 5

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
    struct parameter_descriptor params[CLIF_MAX_NR_OF_PARAMETERS];
};

static int cmp_cmd_descriptor(struct cmd_descriptor * d1, struct cmd_descriptor * d2)
{
    return strcmp(d1->command, d2->command);
}

RB_HEAD(cmdtree, cmd_descriptor) cmdtree_head = RB_INITIALIZER(&cmdtree_head);
RB_GENERATE(cmdtree, cmd_descriptor, descriptor, cmp_cmd_descriptor)


#define _CLIF_PARSE_TYPE_LIST(argtypes) BOOST_PP_SEQ_FOR_EACH_I(_CLIF_PARSE_TYPE, _, argtypes)
#define _CLIF_PARSE_TYPE(r, _, i, type) BOOST_PP_COMMA_IF(i) type

#define _CLIF_ADD_PARAM_DESCRIPTORS(cmd_descr, argtypes)                                            \
    BOOST_PP_SEQ_FOR_EACH_I(_CLIF_ADD_PARAM, cmd_descr, argtypes)
#define _CLIF_ADD_PARAM(r, cmd_descr, i, type)                                                      \
    add_param_descriptor(cmd_descr, i, sizeof(type), BOOST_PP_STRINGIZE(type),                      \
            (int (*)(char *, void *))BOOST_PP_CAT(clif_parse_,type));

#define _CLIF_PARSE_CAST_VALUES(descriptor, argtypes)                                               \
    BOOST_PP_SEQ_FOR_EACH_I(_CLIF_PARSE_CAST_VAL, _, argtypes)
#define _CLIF_PARSE_CAST_VAL(r, _, i, type)                                                         \
    BOOST_PP_COMMA_IF(i) (*(type*)descriptor->params[i].value)

#define _CLIF_CALLER_FN(argtypes)   BOOST_PP_CAT(clif_call_cmd_,     BOOST_PP_SEQ_CAT(argtypes))
#define _CLIF_REGISTER_FN(argtypes) BOOST_PP_CAT(clif_register_cmd_, BOOST_PP_SEQ_CAT(argtypes))

#define CLIF_REGISTER_CMD_PROTOTYPE(argtypes)                                                       \
    void _CLIF_CALLER_FN(argtypes) (struct cmd_descriptor * descriptor)                             \
    {                                                                                               \
        ((void (*)(_CLIF_PARSE_TYPE_LIST(argtypes)))descriptor->functionToCall)(                    \
            _CLIF_PARSE_CAST_VALUES(descriptor, argtypes));                                         \
    }                                                                                               \
    void _CLIF_REGISTER_FN(argtypes) (                                                              \
        const char* command,                                                                        \
        void (* functionToCall)(_CLIF_PARSE_TYPE_LIST(argtypes)),                                   \
        const char* doc, const char* cmdgroup)                                                      \
    {                                                                                               \
        struct cmd_descriptor* cmd_descr = add_cmd_descriptor(                                      \
            command, functionToCall, _CLIF_CALLER_FN(argtypes), BOOST_PP_SEQ_SIZE(argtypes),        \
            doc, cmdgroup);                                                                         \
        _CLIF_ADD_PARAM_DESCRIPTORS(cmd_descr, argtypes)                                            \
    }                                                                                               \

#define _CLIF_ARG_TYPE_doc(unused...)        1
#define _CLIF_ARG_TYPE_arg(unused...)        2
#define _CLIF_ARG_TYPE_cmd_group(unused...)  3

#define _CLIF_ARG_doc(helptext) helptext
#define _CLIF_ARG_cmd_group(group) group
#define _CLIF_ARG_arg(name, helptext) name, helptext

#define _CLIF_ARG_TYPE_IS(arg, type)                                                                \
    BOOST_PP_EQUAL(BOOST_PP_CAT(_CLIF_ARG_TYPE_, arg), type)

#define _CLIF_ADD(s, state, elem) BOOST_PP_ADD(state, elem)

#define _CLIF_COUNT_ARG_I(r, type, arg) (_CLIF_ARG_TYPE_IS(arg, type))

#define _CLIF_COUNT_ARG(argtype, args)                                                              \
    BOOST_PP_SEQ_FOLD_LEFT(                                                                         \
        _CLIF_ADD, 0,                                                                               \
        BOOST_PP_SEQ_FOR_EACH(_CLIF_COUNT_ARG_I, argtype, args))

#define _CLIF_HAS_NOT_ARG(argtype, args)                                                            \
    BOOST_PP_EQUAL(_CLIF_COUNT_ARG(argtype, args), 0)

#define _CLIF_EXPR_IF(cond) BOOST_PP_CAT(_CLIF_EXPR_IF_, cond)
#define _CLIF_EXPR_IF_0(...)
#define _CLIF_EXPR_IF_1(...) __VA_ARGS__

#define _CLIF_GET_ARG_II(r, type, arg)                                                              \
    _CLIF_EXPR_IF(_CLIF_ARG_TYPE_IS(arg, type))(BOOST_PP_CAT(_CLIF_ARG_, arg))                    \
    BOOST_PP_COMMA_IF(_CLIF_ARG_TYPE_IS(arg, type))

#define _CLIF_GET_ARG_I(argtype, args)                                                              \
    BOOST_PP_SEQ_FOR_EACH(_CLIF_GET_ARG_II, argtype, args)

#define _CLIF_GET_ARG(argtype, args, defaultvalue)                                                  \
    _CLIF_GET_ARG_I(argtype, args)         \
    BOOST_PP_EXPR_IIF( _CLIF_HAS_NOT_ARG(argtype, args), defaultvalue) \
    BOOST_PP_COMMA_IF( _CLIF_HAS_NOT_ARG(argtype, args))

#define _CLIF_DEFAULT_PARAM_DESCRIPTION(z, index, argtypes)                                         \
    "arg" BOOST_PP_STRINGIZE(index), "",

#define _CLIF_ADD_MISSING_PARAM_DESCRIPTION(start, argtypes)                                        \
    BOOST_PP_REPEAT_FROM_TO(start, BOOST_PP_SEQ_SIZE(argtypes),                                     \
        _CLIF_DEFAULT_PARAM_DESCRIPTION, argtypes)

#define _CLIF_HANDLE_REGISTER_CMD_ARGS(argtypes, args)                                              \
    _CLIF_GET_ARG(_CLIF_ARG_TYPE_doc(), args, "")                                                   \
    _CLIF_GET_ARG(_CLIF_ARG_TYPE_cmd_group(), args, "")                                             \
    _CLIF_GET_ARG(_CLIF_ARG_TYPE_arg(), args, BOOST_PP_EMPTY)                                       \
    _CLIF_ADD_MISSING_PARAM_DESCRIPTION(                                                            \
        _CLIF_COUNT_ARG(_CLIF_ARG_TYPE_arg(), args), argtypes)                                      \
    NULL


#define CLIF_REGISTER_CMD(command, function, argtypes, args...)                                     \
    _CLIF_REGISTER_FN(argtypes) (command, function,                                                 \
        _CLIF_HANDLE_REGISTER_CMD_ARGS( argtypes, BOOST_PP_VARIADIC_TO_SEQ(args) ));


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

int clif_parse(char * line)
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

static int clif_parse_int(char * token, int * result)
{
    return sscanf(token, "%d", result) != 1;
}

#endif
