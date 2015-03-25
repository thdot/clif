
#include <stdio.h>

#include "clif.h"


CLIF_REGISTER_CMD_PROTOTYPE((int)(int))


void test_int_int(int i1, int i2)
{
    printf("test_int_int: %d %d\n", i1, i2);
}

int main( int argc, const char* argv[] )
{
    struct cmd_descriptor* descriptor;

    CLIF_REGISTER_CMD("test_int_int", test_int_int, (int)(int),
            doc("help text for test_int_int"),
            cmd_group("testgroup"),
            arg(0, "arg0", "help for arg0"),
            arg(1, "arg1", "help for arg1") );

    RB_FOREACH(descriptor, cmdtree, &cmdtree_head) {
        printf("cmd_descriptor:\n"
               "  command: %s\n"
                "  helptext: %s\n"
                "  numberOfParameters: %d\n",
                descriptor->command, descriptor->helptext, descriptor->numberOfParameters);
    }

    clif_parse("invalid_cmd");
    clif_parse("test_int_int 13 42");

    return 0;
}
