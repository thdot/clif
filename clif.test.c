
#include <unity.h>
#include <unity_fixture.h>

#include "clif.h"

static int test_data_int1, test_data_int2;

void test_int_int(int i1, int i2)
{
    test_data_int1 = i1;
    test_data_int2 = i2;
}

void test_int_ptr(int * i1)
{
    test_data_int1 = *i1;
}

CLIF_REGISTER_CMD_PROTOTYPE((int))
CLIF_REGISTER_CMD_PROTOTYPE((int)(int))

void Register_commands()
{
    CLIF_REGISTER_CMD("test_int_int", test_int_int, (int, int),
            doc("help text for test_int_int"),
            cmd_group("testgroup"),
            arg(0, "arg0", "help for arg0"),
            arg(1, "arg1", "help for arg1") );

    CLIF_REGISTER_CMD("test_int_ptr", test_int_ptr, (ptr(int)),
            doc("help text for test_int_ptr"),
            cmd_group("testgroup"),
            arg(0, "arg0", "help for arg0") );
}

TEST_GROUP(CLIF_Test);

TEST_SETUP(CLIF_Test)
{
    test_data_int1 = 0;
    test_data_int2 = 0;
}

TEST_TEAR_DOWN(CLIF_Test)
{}


TEST(CLIF_Test, InvalidCommand)
{
    TEST_ASSERT_EQUAL_INT(clif_parse("invalid_cmd"), 1);
}

TEST(CLIF_Test, ParseSomeInts)
{
    TEST_ASSERT_EQUAL_INT(clif_parse("test_int_int 13 42"), 0);
    TEST_ASSERT_EQUAL_INT(test_data_int1, 13);
    TEST_ASSERT_EQUAL_INT(test_data_int2, 42);

    TEST_ASSERT_EQUAL_INT(clif_parse("test_int_int -13 -42"), 0);
    TEST_ASSERT_EQUAL_INT(test_data_int1, -13);
    TEST_ASSERT_EQUAL_INT(test_data_int2, -42);

    TEST_ASSERT_EQUAL_INT(clif_parse("test_int_int 0 0"), 0);
    TEST_ASSERT_EQUAL_INT(test_data_int1, 0);
    TEST_ASSERT_EQUAL_INT(test_data_int2, 0);
}

TEST(CLIF_Test, MissingArguments)
{
    TEST_ASSERT_EQUAL_INT(clif_parse("test_int_int"), 3);
    TEST_ASSERT_EQUAL_INT(clif_parse("test_int_int 0"), 3);
    TEST_ASSERT_EQUAL_INT(clif_parse("test_int_int 9"), 3);
}

TEST(CLIF_Test, InvalidArguments)
{
    TEST_ASSERT_EQUAL_INT(clif_parse("test_int_int 7 a"), 2);
    TEST_ASSERT_EQUAL_INT(clif_parse("test_int_int a 7"), 2);
    TEST_ASSERT_EQUAL_INT(clif_parse("test_int_int a b"), 2);
}


TEST_GROUP_RUNNER(CLIF_Test)
{
    RUN_TEST_CASE(CLIF_Test, InvalidCommand);
    RUN_TEST_CASE(CLIF_Test, MissingArguments);
    RUN_TEST_CASE(CLIF_Test, InvalidArguments);
    RUN_TEST_CASE(CLIF_Test, ParseSomeInts);
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
