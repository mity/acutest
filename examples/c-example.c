
#include "acutest.h"


void test_tutorial(void)
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

    /* This condition is designed to fail so you can see its output. */
    a = 1;
    b = 2;
    TEST_CHECK(a + b == 5);

    /* Also show TEST_CHECK_ in action */
    TEST_CHECK_(a + b == 5, "%d + %d == 5", a, b);

    /* We may also show more information about the failure. */
    if(!TEST_CHECK(a + b == 4)) {
        TEST_MSG("a: %d", a);
        TEST_MSG("b: %d", b);
    }
}

void
test_crash(void)
{
    int* invalid = ((int*)NULL) + 0xdeadbeef;

    *invalid = 42;
    TEST_CHECK_(1 == 1, "We likely never get here, due to the crash above.");
}


TEST_LIST = {
    { "tutorial", test_tutorial },
    { "fail",     test_fail },
    { "crash",    test_crash },
    { NULL, NULL }
};
