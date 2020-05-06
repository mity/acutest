
#include "acutest.h"

void
test_tutorial(void)
{
    void* mem;

    mem = malloc(10);
    TEST_CHECK(mem != NULL);

    mem = realloc(mem, 20);
    TEST_CHECK(mem != NULL);

    free(mem);
}

void
test_fail(void)
{
    int a, b;

    /* This condition is designed to fail so you can see what the failed test
     * output looks like. */
    a = 1;
    b = 2;
    TEST_CHECK(a + b == 5);

    /* Here is TEST_CHECK_ in action. */
    TEST_CHECK_(a + b == 5, "%d + %d == 5", a, b);

    /* We may also show more information about the failure. */
    if(!TEST_CHECK(a + b == 5)) {
        TEST_MSG("a: %d", a);
        TEST_MSG("b: %d", b);
    }

    /* The macro TEST_MSG() only outputs something when the preceding
     * condition fails, so we can avoid the 'if' statement. */
    TEST_CHECK(a + b == 3);
    TEST_MSG("a: %d", a);
    TEST_MSG("b: %d", b);
}

static void
helper(void)
{
    /* Kill the current test with a condition which is never true. */
    TEST_ASSERT(1 == 2);

    /* This never happens because the test is aborted above. */
    TEST_CHECK(1 + 2 == 2 + 1);
}

void
test_abort(void)
{
    helper();

    /* This test never happens because the test is aborted inside the helper()
     * function. */
    TEST_CHECK(1 * 2 == 2 * 1);
}

void
test_crash(void)
{
    int* invalid = ((int*)NULL) + 0xdeadbeef;

    *invalid = 42;
    TEST_CHECK_(1 == 1, "This should never execute, due to a write into "
                        "an invalid address.");
}

TEST_LIST = {
    { "tutorial", test_tutorial },
    { "fail",     test_fail },
    { "abort",    test_abort },
    { "crash",    test_crash },
    { NULL, NULL }
};
