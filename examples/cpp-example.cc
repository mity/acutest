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


void test_std_exception(void)
{
    throw TestException("Acutest knows how to catch me :-)");
}


void test_strange_exception(void)
{
    throw "Acutest can also catch exceptions not derived from std::exception.";
}

void test_catch_exception_by_type(void)
{
  TEST_CATCH_EXC(throw TestException("expected"), TestException);
}

void test_unexpected_exception(void)
{
  TEST_CATCH_EXC(throw std::invalid_argument("unexpected"), TestException);
}


TEST_LIST = {
    { "std-exception",     test_std_exception },
    { "strange-exception", test_strange_exception },
    { "expected-exception", test_catch_exception_by_type },
    { "unexpected-exception", test_unexpected_exception },
    { NULL, NULL }
};
