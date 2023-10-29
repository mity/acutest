
#include "cute.h"

void
test_tutorial(void)
{
    void* mem;

    mem = malloc(10);
    CUTE_CHECK(mem != NULL);

    mem = realloc(mem, 20);
    CUTE_CHECK(mem != NULL);

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
    CUTE_CHECK(a + b == 5);

    /* Here is CUTE_CHECK_FORMAT in action. */
    CUTE_CHECK_FORMAT(a + b == 5, "%d + %d == 5", a, b);

    /* We may also show more information about the failure. */
    if(!CUTE_CHECK(a + b == 5)) {
        CUTE_MSG("a: %d", a);
        CUTE_MSG("b: %d", b);
    }

    /* The macro CUTE_MSG() only outputs something when the preceding
     * condition fails, so we can avoid the 'if' statement. */
    CUTE_CHECK(a + b == 3);
    CUTE_MSG("a: %d", a);
    CUTE_MSG("b: %d", b);
}

static void
helper(void)
{
    /* Kill the current test with a condition which is never true. */
    CUTE_ASSERT(1 == 2);

    /* This never happens because the test is aborted above. */
    CUTE_CHECK(1 + 2 == 2 + 1);
}

void
test_abort(void)
{
    helper();

    /* This test never happens because the test is aborted inside the helper()
     * function. */
    CUTE_CHECK(1 * 2 == 2 * 1);
}

void
test_crash(void)
{
    int* invalid = ((int*)NULL) + 0xdeadbeef;

    *invalid = 42;
    CUTE_CHECK_FORMAT(1 == 1, "This should never execute, due to a write into "
                        "an invalid address.");
}

CUTE_LIST = {
    { "tutorial", test_tutorial },
    { "fail",     test_fail },
    { "abort",    test_abort },
    { "crash",    test_crash },
    { NULL, NULL }
};
