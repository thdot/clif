#ifndef H_CONSOLE_PARSER
#define H_CONSOLE_PARSER 1

#include <stdlib.h>
#include <string.h>
#include <boost/preprocessor.hpp>
#include "tree.h"


#define CLI_MAX_NR_OF_PARAMETERS 5

struct parameter_descriptor
{
    const char name;
    const char helptext;
    size_t size;
    void * value;
    int (* parser)(char *, void *);
};

struct cmd_descriptor
{
    RB_ENTRY(cmd_descriptor) descriptor;
    const char* command;
    const char helptext;
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
    add_param_descriptor(cmd_descr, i, sizeof(type), BOOST_PP_CAT(cli_parse_,type));

#define _CLI_PARSE_CAST_VALUES(descriptor, argtypes)                                                \
    BOOST_PP_SEQ_FOR_EACH_I(_CLI_PARSE_CAST_VAL, _, argtypes)
#define _CLI_PARSE_CAST_VAL(r, _, i, type)                                                          \
    BOOST_PP_COMMA_IF(i) (*(type*)descriptor->params[i].value)

//#define _CLI_PARSE_ARGS_DECL()                                                                      \
//    BOOST_PP_REPEAT(CLI_MAX_NR_OF_PARAMETERS, _CLI_PARSE_ARG, _)
//#define _CLI_PARSE_ARG(z, n, _)                                                                     \
//    void * BOOST_PP_CAT(arg, n) = NULL;


#define _CLI_CALLER_FN(argtypes)   BOOST_PP_CAT(cli_call_cmd_,     BOOST_PP_SEQ_CAT(argtypes))
#define _CLI_REGISTER_FN(argtypes) BOOST_PP_CAT(cli_register_cmd_, BOOST_PP_SEQ_CAT(argtypes))


//#define _CLI_PARSE_VAR_DECL_LIST(argtypes) BOOST_PP_SEQ_FOR_EACH_I(_CLI_PARSE_VAR_DECL, p, argtypes)
//#define _CLI_PARSE_VAR_DECL(_, name, i, type) type BOOST_PP_CAT(name, i);
//#define _CLI_PARSE_FN(argtypes)    BOOST_PP_CAT(cli_parse_cmd_,    BOOST_PP_SEQ_CAT(argtypes))
//int _CLI_PARSE_FN(argtypes) (char* cl, void* cb)                                                \
//{                                                                                               \
//    _CLI_PARSE_VAR_DECL_LIST(argtypes)                                                          \
//    if (sscanf(cl, _CLI_PARSE_FORMAT_STR(argtypes), _CLI_PARSE_VAR_LIST(&p, argtypes)) !=       \
//            BOOST_PP_SEQ_SIZE(argtypes)) return -1;                                             \
//    return 0;                                                                                   \
//}

#define CLI_REGISTER_CMD_PROTOTYPE(argtypes)                                                        \
    void _CLI_CALLER_FN(argtypes) (struct cmd_descriptor * descriptor)                              \
    {                                                                                               \
        ((void (*)(_CLI_PARSE_TYPE_LIST(argtypes)))descriptor->functionToCall)(                     \
            _CLI_PARSE_CAST_VALUES(descriptor, argtypes));                                          \
    }                                                                                               \
    void _CLI_REGISTER_FN(argtypes) (                                                               \
        const char* command,                                                                        \
        void (* functionToCall)(_CLI_PARSE_TYPE_LIST(argtypes)) )                                   \
    {                                                                                               \
        struct cmd_descriptor* cmd_descr = add_cmd_descriptor(                                      \
            command, functionToCall, _CLI_CALLER_FN(argtypes), BOOST_PP_SEQ_SIZE(argtypes));        \
        _CLI_ADD_PARAM_DESCRIPTORS(cmd_descr, argtypes)                                             \
    }                                                                                               \


#define CLI_REGISTER_CMD(command, function, argtypes)          \
        _CLI_REGISTER_FN(argtypes) (command, function)


static struct cmd_descriptor* add_cmd_descriptor(const char* command, void* functionToCall,
        void (* caller)(struct cmd_descriptor *), short numberOfParameters)
{
    struct cmd_descriptor* new_descriptor = malloc(sizeof(struct cmd_descriptor));
    memset(new_descriptor, 0x00, sizeof(struct cmd_descriptor));
    new_descriptor->command = strdup(command);
    new_descriptor->functionToCall = functionToCall;
    new_descriptor->caller = caller;
    new_descriptor->numberOfParameters = numberOfParameters;
    RB_INSERT(cmdtree, &cmdtree_head, new_descriptor);
    return new_descriptor;
}

static void add_param_descriptor(struct cmd_descriptor* cmd_descr, short index, size_t size,
        int (* parser)(char *, void *))
{
    cmd_descr->params[index].size = size;
    cmd_descr->params[index].parser = parser;
}

int cli_parse(char * line)
{
    int i;
    struct cmd_descriptor find, *descr;

    char command[256];
    sscanf(line, "%s", command);

    find.command = command;
    descr = RB_FIND(cmdtree, &cmdtree_head, &find);

    if (! descr) {
        printf("command not found: %s\n", command);
        return 1;
    }

    line += strlen(command);
    for (i = 0; i < descr->numberOfParameters; i++) {
        descr->params[i].value = malloc(descr->params[i].size);
        (*descr->params[i].parser)(line, descr->params[i].value);
    }

out:
    for (i = 0; i < descr->numberOfParameters; i++) {
        free(descr->params[i].value);
        descr->params[i].value = NULL;
    }


    return 0;
}

static int cli_parse_int(char * buf, void * result)
{
    *(int*)result = 42;
    return 0;
}

#endif
