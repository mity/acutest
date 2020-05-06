#include "acutest.h"

#include <exception>
#include <stdexcept>
#include <string>


class TestException : public std::exception
{
    std::string msg_;

public:
    virtual const char* what() const throw()
    { return msg_.c_str(); }

    explicit TestException(const std::string& msg) throw()
    : msg_(msg)
    {}

    virtual ~TestException() throw()
    {}
};


enum What {
    NO_THROW = 0,
    THROW_TEST_EXC,
    THROW_INVALID_ARG,
    THROW_CHAR_PTR,
    THROW_INT
};

/* This dummy function represents some code which we want to test.
 * As this is written in C++, they can throw exceptions.
 */
static void
some_function(What what, const char* msg = NULL)
{
    switch(what) {
        case NO_THROW:
            /* noop */
            break;

        case THROW_TEST_EXC:
            throw TestException(msg != NULL ? msg : "TestException");
            break;

        case THROW_INVALID_ARG:
            throw std::invalid_argument(msg != NULL ? msg : "std::invalid_argument");
            break;

        case THROW_CHAR_PTR:
            throw msg;
            break;

        case THROW_INT:
            throw 42;
            break;
    }
}


void test_exception_type(void)
{
    /* In C++, we may want to test whether some code throws an expected
     * exception type. */
    TEST_EXCEPTION(some_function(THROW_TEST_EXC), TestException);
    TEST_EXCEPTION(some_function(THROW_INVALID_ARG), std::invalid_argument);

    /* Naturally, testing for a type the actual exception is derived from,
     * is ok too. */
    TEST_EXCEPTION(some_function(THROW_TEST_EXC), std::exception);
    TEST_EXCEPTION(some_function(THROW_INVALID_ARG), std::exception);

    /* Fundemental types used as exceptions work too. */
    TEST_EXCEPTION(some_function(THROW_CHAR_PTR), const char*);
    TEST_EXCEPTION(some_function(THROW_INT), int);

    /* These checks fail because the given code does not throw an exception
     * at all, or throws a different type of exception. */
    TEST_EXCEPTION(some_function(NO_THROW), std::exception);
    TEST_EXCEPTION(some_function(THROW_INT), std::exception);
    TEST_EXCEPTION(some_function(THROW_INVALID_ARG), TestException);

    /* With TEST_CATCH_EXC_, we may use a custom message. */
    TEST_EXCEPTION_(some_function(THROW_INT), std::exception, "we may use a custom message");
}

void test_uncaught_std_exception(void)
{
    /* If the test throws unhandled exception, Acutest aborts the test unit
     * and considers it a failure. */
    some_function(THROW_TEST_EXC, "Acutest knows how to catch me :-)");
}

void test_uncaught_strange_exception(void)
{
    /* If the test throws unhandled exception, Acutest aborts the test unit
     * and considers it a failure even if it is not derived from std::exception.
     */
    some_function(THROW_CHAR_PTR, "Acutest knows how to catch me :-)");
}

void test_success(void)
{
    /* Do nothing */
}

TEST_LIST = {
    { "test_exception_type", test_exception_type },
    { "uncaught-std-exception", test_uncaught_std_exception },
    { "uncaught-strange-exception", test_uncaught_strange_exception },
    { "success", test_success },
    { NULL, NULL }
};
