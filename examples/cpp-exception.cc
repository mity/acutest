#include "acutest.h"

#include <exception>
#include <iostream>

class MyException : public std::invalid_argument {
public:
  explicit MyException(const std::string &msg) : invalid_argument(msg) {}
};

void test_my_exception(void) {
  // test pass
  TEST_CATCH_EXC(throw MyException("expected"), MyException);
}

void test_unexpected_exception(void) {
  // test failed
  TEST_CATCH_EXC(throw std::invalid_argument("unexpected"), MyException);
}

TEST_LIST = {{"my-exception", test_my_exception},
             {"unexpected-exception", test_unexpected_exception},
             {NULL, NULL}};
