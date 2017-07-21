
#include "acutest.h"

#include <exception>
#include <string>


class TestException : public std::exception
{
    std::string msg_;

public:
    virtual const char* what() const throw()
    { return msg_.c_str(); }

    TestException(const std::string& msg) throw()
    : msg_(msg)
    {}

    virtual ~TestException() throw()
    {}
};


void test_std_exception(void)
{
    throw TestException("CUTest knows how to catch me :-)");
}


void test_strange_exception(void)
{
    throw "It deals also with exception not derived from std::exception";
}


TEST_LIST = {
    { "std-exception",     test_std_exception },
    { "strange-exception", test_strange_exception },
    { 0 }
};
