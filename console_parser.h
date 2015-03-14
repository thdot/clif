#ifndef H_CONSOLE_PARSER
#define H_CONSOLE_PARSER 1

#include <stdlib.h>
#include <string.h>
#include <boost/preprocessor.hpp>
#include "tree.h"

struct cmd_descriptor
{
    RB_ENTRY(cmd_descriptor) descriptor;
    const char* command;
    const char helptext;
    short expectedNumberOfParameters;
    void* functionToCall;
    int (* parser)(char*, void*);
};

static int cmp_cmd_descriptor(struct cmd_descriptor *d1, struct cmd_descriptor *d2)
{
    return strcmp(d1->command, d2->command);
}

RB_HEAD(cmdtree, cmd_descriptor) cmdtree_head = RB_INITIALIZER(&cmdtree_head);
RB_GENERATE(cmdtree, cmd_descriptor, descriptor, cmp_cmd_descriptor)

static void add_cmd_descriptor(const char* command, void* functionToCall)
{
    struct cmd_descriptor* new_descriptor = malloc(sizeof(struct cmd_descriptor));
    memset(new_descriptor, 0x00, sizeof(struct cmd_descriptor));
    new_descriptor->command = strdup(command);
    RB_INSERT(cmdtree, &cmdtree_head, new_descriptor);
}


#define _CLI_PARSE_VAR_DECL_LIST(argtypes) BOOST_PP_SEQ_FOR_EACH_I(_CLI_PARSE_VAR_DECL, p, argtypes)
#define _CLI_PARSE_VAR_DECL(_, name, i, type) type BOOST_PP_CAT(name, i);

#define _CLI_PARSE_VAR_LIST(name, argtypes) BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(argtypes), _CLI_PARSE_VAR, name)
#define _CLI_PARSE_VAR(_, n, name) BOOST_PP_COMMA_IF(n) BOOST_PP_CAT(name, n)

#define _CLI_PARSE_TYPE_LIST(argtypes) BOOST_PP_SEQ_FOR_EACH_I(_CLI_PARSE_TYPE, _, argtypes)
#define _CLI_PARSE_TYPE(r, _, i, type) BOOST_PP_COMMA_IF(i) type

#define _CLI_PARSE_FORMAT_STR(argtypes) BOOST_PP_SEQ_FOR_EACH(_CLI_PARSE_FORMAT, _, argtypes)
#define _CLI_PARSE_FORMAT(r, _, type) _CLI_PARSE_FORMAT2(type)
#define _CLI_PARSE_FORMAT2(type)  _CLI_FORMAT_SPECIFIER_##type

#define _CLI_PARSE_FN(argtypes) BOOST_PP_CAT(cli_parse_cmd_, BOOST_PP_SEQ_CAT(argtypes))

#define _CLI_REGISTER_FN(argtypes) BOOST_PP_CAT(cli_register_cmd_, BOOST_PP_SEQ_CAT(argtypes))

#define _CLI_FORMAT_SPECIFIER_int " %d"


#define CLI_REGISTER_CMD_PROTOTYPE(argtypes)                                                        \
    int _CLI_PARSE_FN(argtypes) (char* cl, void* cb)                                                \
    {                                                                                               \
        _CLI_PARSE_VAR_DECL_LIST(argtypes)                                                          \
        if (sscanf(cl, _CLI_PARSE_FORMAT_STR(argtypes), _CLI_PARSE_VAR_LIST(&p, argtypes)) !=       \
                BOOST_PP_SEQ_SIZE(argtypes)) return -1;                                             \
        ((void (*)(_CLI_PARSE_TYPE_LIST(argtypes)))cb)(_CLI_PARSE_VAR_LIST(p, argtypes));           \
        return 0;                                                                                   \
    }                                                                                               \
    void _CLI_REGISTER_FN(argtypes) (                                                               \
        const char* command,                                                                        \
        void (* functionToCall)(_CLI_PARSE_TYPE_LIST(argtypes)) )                                   \
    {                                                                                               \
        add_cmd_descriptor(command, functionToCall);                                                \
    }                                                                                               \


CLI_REGISTER_CMD_PROTOTYPE((int)(int))

#define CLI_REGISTER_CMD(command, function, argtypes)          \
        _CLI_REGISTER_FN(argtypes) (command, function)


#endif
