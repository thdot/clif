#ifndef H_CLIF
#define H_CLIF 1

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


#define _CLIF_PARSE_TYPE_LIST(paramtypes) BOOST_PP_SEQ_FOR_EACH_I(_CLIF_PARSE_TYPE, _, paramtypes)
#define _CLIF_PARSE_TYPE(r, _, i, type) BOOST_PP_COMMA_IF(i) type

#define _CLIF_ADD_PARAM_DESCRIPTORS(cmd_descr, paramtypes)                                            \
    BOOST_PP_SEQ_FOR_EACH_I(_CLIF_ADD_PARAM, cmd_descr, paramtypes)
#define _CLIF_ADD_PARAM(r, cmd_descr, i, type)                                                      \
    add_param_descriptor(cmd_descr, i, sizeof(type), BOOST_PP_STRINGIZE(type),                      \
            (int (*)(char *, void *))BOOST_PP_CAT(clif_parse_,type));

#define _CLIF_PARSE_CAST_VALUES(descriptor, paramtypes)                                               \
    BOOST_PP_SEQ_FOR_EACH_I(_CLIF_PARSE_CAST_VAL, _, paramtypes)
#define _CLIF_PARSE_CAST_VAL(r, _, i, type)                                                         \
    BOOST_PP_COMMA_IF(i) (*(type*)descriptor->params[i].value)

#define _CLIF_CALLER_FN(paramtypes)   BOOST_PP_CAT(clif_call_cmd_,     BOOST_PP_SEQ_CAT(paramtypes))
#define _CLIF_REGISTER_FN(paramtypes) BOOST_PP_CAT(clif_register_cmd_, BOOST_PP_SEQ_CAT(paramtypes))

#define CLIF_REGISTER_CMD_PROTOTYPE(paramtypes)                                                       \
    void _CLIF_CALLER_FN(paramtypes) (struct cmd_descriptor * descriptor)                             \
    {                                                                                               \
        ((void (*)(_CLIF_PARSE_TYPE_LIST(paramtypes)))descriptor->functionToCall)(                    \
            _CLIF_PARSE_CAST_VALUES(descriptor, paramtypes));                                         \
    }                                                                                               \
    void _CLIF_REGISTER_FN(paramtypes) (                                                              \
        const char* command,                                                                        \
        void (* functionToCall)(_CLIF_PARSE_TYPE_LIST(paramtypes)),                                   \
        const char* doc, const char* cmdgroup);                                                      \
//    {                                                                                               \
//        struct cmd_descriptor* cmd_descr = add_cmd_descriptor(                                      \
//            command, functionToCall, _CLIF_CALLER_FN(argtypes), BOOST_PP_SEQ_SIZE(argtypes),        \
//            doc, cmdgroup);                                                                         \
//        _CLIF_ADD_PARAM_DESCRIPTORS(cmd_descr, argtypes)                                            \
    }                                                                                               \

#define _CLIF_ARG_TYPE_doc(unused...)        1
#define _CLIF_ARG_TYPE_cmd_group(unused...)  2
#define _CLIF_ARG_TYPE_arg(index, unused...) BOOST_PP_ADD(10, index)

#define _CLIF_ARG_doc(helptext) helptext
#define _CLIF_ARG_cmd_group(group) group
#define _CLIF_ARG_arg(index, name, helptext) name, helptext

#define _CLIF_ARG_TYPE_IS(arg, type)                                                               \
    BOOST_PP_EQUAL(_CLIF_ARG_TYPE_##arg, type)

#define _CLIF_ADD(s, state, elem) BOOST_PP_ADD(state, elem)

#define _CLIF_COUNT_ARG_I(r, type, arg) (_CLIF_ARG_TYPE_IS(arg, type))

#define _CLIF_COUNT_ARG(argtype, args)                                                              \
    BOOST_PP_SEQ_FOLD_LEFT(                                                                         \
        _CLIF_ADD, 0,                                                                               \
        BOOST_PP_SEQ_FOR_EACH(_CLIF_COUNT_ARG_I, argtype, args))

#define _CLIF_HAS_NOT_ARG(argtype, args)                                                            \
    BOOST_PP_EQUAL(_CLIF_COUNT_ARG(argtype, args), 0)

#define _CLIF_EXPR_IIF(cond) BOOST_PP_CAT(_CLIF_EXPR_IIF_, cond)
#define _CLIF_EXPR_IIF_0(...)
#define _CLIF_EXPR_IIF_1(...) __VA_ARGS__

#define _CLIF_GET_ARG_I(r, type, arg)                                                               \
    _CLIF_EXPR_IIF(_CLIF_ARG_TYPE_IS(arg, type))(BOOST_PP_CAT(_CLIF_ARG_, arg) BOOST_PP_COMMA())

#define _CLIF_GET_ARG(type, args, defaultvalue)                                                     \
    BOOST_PP_SEQ_FOR_EACH(_CLIF_GET_ARG_I, type, args)                                              \
    _CLIF_EXPR_IIF(_CLIF_HAS_NOT_ARG(type, args))(defaultvalue BOOST_PP_COMMA())

#define _CLIF_DEFAULT_PARAM_DESCRIPTION(index)                                                      \
    "arg" BOOST_PP_STRINGIZE(index), ""

#define _CLIF_STATIC_PARAM_DESCRIPTION_I(r, _, param)                                               \
    sizeof(param), BOOST_PP_STRINGIZE(param),                                                       \
    (int (*)(char *, void *))BOOST_PP_CAT(clif_parse_, param),

#define _CLIF_STATIC_PARAM_DESCRIPTION_NULL(r, _, unused) 0, NULL, NULL,

#define _CLIF_STATIC_PARAM_DESCRIPTION(paramtypes)                                                  \
    BOOST_PP_SEQ_FOR_EACH(_CLIF_STATIC_PARAM_DESCRIPTION_I, _, paramtypes)                          \
    BOOST_PP_REPEAT(                                                                                \
        BOOST_PP_SUB(CLIF_MAX_NR_OF_PARAMETERS, BOOST_PP_SEQ_SIZE(paramtypes)),                     \
        _CLIF_STATIC_PARAM_DESCRIPTION_NULL, _)

#define _CLIF_GET_ARG_ARG(z, index, args)     \
    _CLIF_GET_ARG(_CLIF_ARG_TYPE_arg(index), args, _CLIF_DEFAULT_PARAM_DESCRIPTION(index))

#define _CLIF_HANDLE_REGISTER_CMD_ARGS(paramtypes, args)                                            \
    _CLIF_GET_ARG(_CLIF_ARG_TYPE_doc(), args, "")                                                   \
    _CLIF_GET_ARG(_CLIF_ARG_TYPE_cmd_group(), args, "")                                             \
    BOOST_PP_REPEAT(CLIF_MAX_NR_OF_PARAMETERS, _CLIF_GET_ARG_ARG, args)                             \
    _CLIF_STATIC_PARAM_DESCRIPTION(paramtypes) NULL

#define _CLIF_ADD_CMD_PARAM_DECL()                                                                  \
    BOOST_PP_REPEAT(CLIF_MAX_NR_OF_PARAMETERS, _CLIF_PARAM_DECL_USER_I, _)                          \
    BOOST_PP_REPEAT(CLIF_MAX_NR_OF_PARAMETERS, _CLIF_PARAM_DECL_STATIC_I, _)
#define _CLIF_PARAM_DECL_USER_I(z, n, _)                                                            \
    , const char * _CLIF_PARAM_NAME(name, n)                                                        \
    , const char * _CLIF_PARAM_NAME(doc, n)
#define _CLIF_PARAM_DECL_STATIC_I(z, n, _)                                                          \
    , size_t _CLIF_PARAM_NAME(size, n)                                                              \
    , const char * _CLIF_PARAM_NAME(type, n)                                                        \
    , int (* _CLIF_PARAM_NAME(parser, n))(char *, void *)
#define _CLIF_PARAM_NAME(name, index)                                                               \
    BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_CAT(param, index), _), name)

#define _CLIF_ADD_PARAM_DESCR(r, index, new_descriptor)                                             \
    new_descriptor->params[index].size = _CLIF_PARAM_NAME(size, index);                             \
    new_descriptor->params[index].parser = _CLIF_PARAM_NAME(parser, index);                         \
    new_descriptor->params[index].type = _CLIF_PARAM_NAME(type, index);                             \
    new_descriptor->params[index].helptext = _CLIF_PARAM_NAME(doc, index);


#define CLIF_REGISTER_CMD(command, function, paramtypes, args...)                                   \
    clif_add_cmd_descriptor(command, function,                                                      \
        _CLIF_CALLER_FN(paramtypes), BOOST_PP_SEQ_SIZE(paramtypes),                                 \
        _CLIF_HANDLE_REGISTER_CMD_ARGS(paramtypes, BOOST_PP_VARIADIC_TO_SEQ(args)));


void clif_add_cmd_descriptor(const char* command, void* functionToCall,
        void (* caller)(struct cmd_descriptor *), short numberOfParameters,
        const char* doc, const char* cmdgroup
        _CLIF_ADD_CMD_PARAM_DECL(), void* end )
{
    struct cmd_descriptor* new_descriptor = malloc(sizeof(struct cmd_descriptor));
    memset(new_descriptor, 0x00, sizeof(struct cmd_descriptor));
    new_descriptor->command = strdup(command);
    new_descriptor->functionToCall = functionToCall;
    new_descriptor->caller = caller;
    new_descriptor->numberOfParameters = numberOfParameters;
    new_descriptor->helptext = strdup(doc);
    RB_INSERT(cmdtree, &cmdtree_head, new_descriptor);
    BOOST_PP_REPEAT(CLIF_MAX_NR_OF_PARAMETERS, _CLIF_ADD_PARAM_DESCR, new_descriptor);
}


void clif_add_param_descriptor(struct cmd_descriptor* cmd_descr, short index, size_t size,
        const char * type, int (* parser)(char *, void *))
{

}

int clif_parse(char * line)
{
    int i = 0, result = 0;
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
            printf("too many number of arguments\n");
            result = 1;
            goto out;
        }
        descr->params[i].value = malloc(descr->params[i].size);
        (*descr->params[i].parser)(token, descr->params[i].value);
        token = strtok(NULL, " ");
        i++;
    }
    if (i != descr->numberOfParameters) {
        printf("invalid number of arguments (%d given)\n", i);
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
