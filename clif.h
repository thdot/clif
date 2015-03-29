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


enum clif_ptr_type {
    CLIF_PTR_NONE,
    CLIF_PTR_INPUT,
    CLIF_PTR_RAW,
};

struct clif_param_descriptor
{
    const char * type;
    const char * name;
    const char * helptext;
    size_t size;
    void * value;
    enum clif_ptr_type ptr_type;
    int (* parser)(char *, void *);
};

struct clif_cmd_descriptor
{
    RB_ENTRY(clif_cmd_descriptor) descriptor;
    const char * command;
    const char * helptext;
    void* functionToCall;
    void (* caller)(struct clif_cmd_descriptor *);
    short numberOfParameters;
    struct clif_param_descriptor params[CLIF_MAX_NR_OF_PARAMETERS];
};

RB_HEAD(clif_cmdtree, clif_cmd_descriptor) clif_cmdtree_root;


#define _CLIF_PARSE_TYPE_LIST(paramtypes) BOOST_PP_SEQ_FOR_EACH_I(_CLIF_PARSE_TYPE, _, paramtypes)
#define _CLIF_PARSE_TYPE(r, _, i, type) BOOST_PP_COMMA_IF(i) type

#define _CLIF_ADD_PARAM_DESCRIPTORS(cmd_descr, paramtypes)                                          \
    BOOST_PP_SEQ_FOR_EACH_I(_CLIF_ADD_PARAM, cmd_descr, paramtypes)
#define _CLIF_ADD_PARAM(r, cmd_descr, i, type)                                                      \
    add_param_descriptor(cmd_descr, i, sizeof(type), BOOST_PP_STRINGIZE(type),                      \
            (int (*)(char *, void *))BOOST_PP_CAT(clif_parse_,type));

#define _CLIF_PARSE_CAST_VALUES(descriptor, paramtypes)                                             \
    BOOST_PP_SEQ_FOR_EACH_I(_CLIF_PARSE_CAST_VAL, _, paramtypes)
#define _CLIF_PARSE_CAST_VAL(r, _, i, type)                                                         \
    BOOST_PP_COMMA_IF(i) (*(type*)descriptor->params[i].value)

#define _CLIF_CALLER_FN(paramtypes)                                                                 \
    BOOST_PP_CAT(clif_call_cmd_, BOOST_PP_SEQ_CAT( \
            BOOST_PP_SEQ_TRANSFORM(_CLIF_ARG_PTR_TRANSFORM, _, paramtypes)))
#define _CLIF_REGISTER_FN(paramtypes)                                                               \
    BOOST_PP_CAT(clif_register_cmd_, BOOST_PP_SEQ_CAT(paramtypes))

#define CLIF_REGISTER_CMD_PROTOTYPE(paramtypes)                                                     \
    void _CLIF_CALLER_FN(paramtypes) (struct clif_cmd_descriptor * descriptor)                      \
    {                                                                                               \
        ((void (*)(_CLIF_PARSE_TYPE_LIST(paramtypes)))descriptor->functionToCall)(                  \
            _CLIF_PARSE_CAST_VALUES(descriptor, paramtypes));                                       \
    }                                                                                               \
    void _CLIF_REGISTER_FN(paramtypes) (                                                            \
        const char* command,                                                                        \
        void (* functionToCall)(_CLIF_PARSE_TYPE_LIST(paramtypes)),                                 \
        const char* doc, const char* cmdgroup);                                                     \

#define _CLIF_ARG_PTR_GOOBLE_ptr(type)
#define _CLIF_ARG_PTR_GOOBLE_raw_ptr(type)

#define _CLIF_ARG_PTR_TYPE_ptr(type) type
#define _CLIF_ARG_PTR_TYPE_raw_ptr(type) raw_ptr

#define _CLIF_ARG_PTR_TRANSFORM(s, _, type) \
    BOOST_PP_IIF( \
            BOOST_PP_IS_EMPTY(BOOST_PP_CAT(_CLIF_ARG_PTR_GOOBLE_, type)), \
            BOOST_PP_CAT(_CLIF_ARG_PTR_TYPE_, type), \
            type)


#define _CLIF_ARG_TYPE_doc(unused...)        1
#define _CLIF_ARG_TYPE_cmd_group(unused...)  2
#define _CLIF_ARG_TYPE_arg(index, unused...) BOOST_PP_ADD(10, index)

#define _CLIF_ARG_doc(helptext) helptext
#define _CLIF_ARG_cmd_group(group) group
#define _CLIF_ARG_arg(index, name, helptext) name, helptext

#define _CLIF_ARG_TYPE_IS(arg, type)                                                                \
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
    BOOST_PP_REPEAT(CLIF_MAX_NR_OF_PARAMETERS, _CLIF_PARAM_DECL_USER, _)                            \
    BOOST_PP_REPEAT(CLIF_MAX_NR_OF_PARAMETERS, _CLIF_PARAM_DECL_STATIC, _)
#define _CLIF_PARAM_DECL_USER(z, n, _)                                                              \
    , const char * _CLIF_PARAM_NAME(name, n)                                                        \
    , const char * _CLIF_PARAM_NAME(doc, n)
#define _CLIF_PARAM_DECL_STATIC(z, n, _)                                                            \
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
        _CLIF_CALLER_FN(BOOST_PP_TUPLE_TO_SEQ(paramtypes)), BOOST_PP_TUPLE_SIZE(paramtypes),        \
        _CLIF_HANDLE_REGISTER_CMD_ARGS(                                                             \
            BOOST_PP_TUPLE_TO_SEQ(paramtypes), BOOST_PP_VARIADIC_TO_SEQ(args)));


void clif_add_cmd_descriptor(const char* command, void* functionToCall,
        void (* caller)(struct clif_cmd_descriptor *), short numberOfParameters,
        const char* doc, const char* cmdgroup
        _CLIF_ADD_CMD_PARAM_DECL(), void* end );

int clif_parse(char * line);

int clif_parse_int(char * token, int * result);


#endif
