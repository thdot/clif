#ifndef H_CONSOLE_PARSER
#define H_CONSOLE_PARSER 1

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/cat.hpp>
#include <boost/preprocessor/cat.hpp>

struct cmd_descriptor
{
    const char* command;
    const char helptext;
    short expectedNumberOfParameters;
    void* functionToCall;
    int (* parser)(char*, void*);
};

int parse_cl_int(char* cl, void* cb)
{
    int p1;

    if (sscanf(cl, "%d", &p1) != 1)
        return -1;

    ((void (*)(int))cb)(p1);

    return 0;
}


#define _CLI_PAR_DECL(r, _, type) type BOOST_PP_CAT(p, r);
#define _CLI_PAR_LIST(r, size, type) BOOST_PP_COMMA_IF(BOOST_PP_SUB(r, size)) BOOST_PP_CAT(p, r)


#define CLI_REGISTER_CMD_PROTOTYPE(argtypes)         \
    int BOOST_PP_CAT(parse_cl_, BOOST_PP_SEQ_CAT(argtypes))(char* cl, void* cb) {       \
        BOOST_PP_SEQ_FOR_EACH(_CLI_PAR_DECL, _, argtypes)    \
        if (sscanf(cl, "%d", BOOST_PP_SEQ_FOR_EACH(_CLI_PAR_LIST, BOOST_PP_SEQ_SIZE(argtypes), argtypes))) != 1) \
    }


CLI_REGISTER_CMD_PROTOTYPE((int)(int))

#endif
