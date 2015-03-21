
#include <stdio.h>

#include "console_parser.h"


CLI_REGISTER_CMD_PROTOTYPE((int)(int)(int))


void test_int_int(int i1, int i2, int i3)
{
    printf("test_int_int: %d %d %d\n", i1, i2, i3);
}

int main( int argc, const char* argv[] )
{
    struct cmd_descriptor* descriptor;

    CLI_REGISTER_CMD("test_int_int", test_int_int, (int)(int)(int),
            doc("help text for test_int_int"),
            cmd_group("testgroup"),
            arg("arg0", "help for arg0"),
            arg("arg1", "help for arg1") );

    RB_FOREACH(descriptor, cmdtree, &cmdtree_head) {
        printf("%s\n", descriptor->command);
    }

    cli_parse("invalid_cmd");
    cli_parse("test_int_int 13 42");

    return 0;
}
