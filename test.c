
#include <stdio.h>

#include "console_parser.h"

void test_int_int(int i1, int i2)
{
    printf("test_int_int: %d %d\n", i1, i2);
}

int main( int argc, const char* argv[] )
{
    struct cmd_descriptor* descriptor;

    CLI_REGISTER_CMD("test_int_int", test_int_int, (int)(int));

    RB_FOREACH(descriptor, cmdtree, &cmdtree_head) {
        printf("%s\n", descriptor->command);
    }

    return 0;
}
