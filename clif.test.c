
#include <unity.h>
#include <unity_fixture.h>

#include "clif.h"

CLIF_REGISTER_CMD_PROTOTYPE((int)(int))

void test_int_int(int i1, int i2)
{
    printf("test_int_int: %d %d\n", i1, i2);
}

void test_int_int_ptr(int i1, int * i2)
{
    printf("test_int_int_ptr: %d %d\n", i1, *i2);
}

void Register_commands()
{
    CLIF_REGISTER_CMD("test_int_int", test_int_int, (int, int),
            doc("help text for test_int_int"),
            cmd_group("testgroup"),
            arg(0, "arg0", "help for arg0"),
            arg(1, "arg1", "help for arg1") );
}

TEST_GROUP(CLIF_Test);

TEST_SETUP(CLIF_Test) {}
TEST_TEAR_DOWN(CLIF_Test) {}


TEST(CLIF_Test, ParseInvalidCommand)
{
    TEST_ASSERT_EQUAL_INT(clif_parse("invalid_cmd"), 1);
}


TEST_GROUP_RUNNER(CLIF_Test)
{
    RUN_TEST_CASE(CLIF_Test, ParseInvalidCommand);
}

static void RunAllTests(void)
{
    Register_commands();

    RUN_TEST_GROUP(CLIF_Test);
}


int main( int argc, const char* argv[] )
{
    return UnityMain(argc, argv, RunAllTests);
}

//    struct clif_cmd_descriptor* descriptor;

//    CLIF_REGISTER_CMD("test_int_int_ptr", test_int_int_ptr, (int, ptr(int)),
//            doc("help text for test_int_int_ptr"),
//            cmd_group("testgroup"),
//            arg(0, "arg0", "help for arg0"),
//            arg(1, "arg1", "help for arg1") );

//    RB_FOREACH(descriptor, cmdtree, &cmdtree_head) {
//        printf("cmd_descriptor:\n"
//               "  command: %s\n"
//                "  helptext: %s\n"
//                "  numberOfParameters: %d\n",
//                descriptor->command, descriptor->helptext, descriptor->numberOfParameters);
//    }
//
//
//    clif_parse("test_int_int 13 42");
//
//    return 0;
//}
