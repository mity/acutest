/*
 * CUTest -- C/C++ Unit Test facility (http://github.com/mity/cutest)
 * Copyright (c) 2013 Martin Mitas
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef CUTEST_H__
    #define CUTEST_H__
#else
    #error Header "cutest.h" already included. It can only be included once.
#endif


/************************
 *** Public interface ***
 ************************/

/* Macro to specify list of unit tests in the suite.
 * The unit test implementation MUST provide list of unit tests it implements
 * with this macro:
 *
 * TEST_LIST = {
 *     { "test1_name", test1_func_ptr },
 *     { "test2_name", test2_func_ptr },
 *     ...
 *     { 0 }
 * };
 *
 * The list specifies names of each tests (must be unique) and pointer to
 * a function implementing it. The function does not take any arguments
 * and have no return values.
 */
#define TEST_LIST              const struct test__ test_list__[]


/* Macros for testing whether an unit test succeeds or fails. These macros
 * can be used arbitrarily in functions implementing the unit tests.
 *
 * If condition fails throughout execution of a test, the test fails.
 *
 * TEST_CHECK takes only one argument (the condition), TEST_CHECK_ allows
 * also to specify an error message to print out if the condition fails.
 * (It expects printf-like format string and its parameters). The macros
 * return non-zero (condition passes) or 0 (condition fails).
 *
 * That can be useful when more conditions should be checked only if some
 * preceding condition passes, as illustrated here:
 *
 * SomeStruct* ptr = allocate_some_struct();
 * if(TEST_CHECK(ptr != NULL)) {
 *     TEST_CHECK(ptr->member1 < 100);
 *     TEST_CHECK(ptr->member2 > 200);
 * }
 */
#define TEST_CHECK_(cond,...)  test_check__((cond), __FILE__, __LINE__, #cond, __VA_ARGS__)
#define TEST_CHECK(cond)       test_check__((cond), __FILE__, __LINE__, #cond, NULL)


/**********************
 *** Implementation ***
 **********************/

/* The unit test files should not rely on anything below. */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)
    #define CUTEST_UNIX__    1
    #include <errno.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <signal.h>
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
    #define CUTEST_WIN__     1
    #include <windows.h>
#endif

#ifdef __cplusplus
    #include <exception>
#endif


/* Note our global private identifiers end with '__' to minimize risk of clash
 * with the unit tests implementation. */


#ifdef __cplusplus
    extern "C" {
#endif


struct test__ {
    const char* name;
    void (*func)(void);
};

extern const struct test__ test_list__[];

static char* test_argv0__ = NULL;
static int test_count__ = 0;
static int test_no_exec__ = 0;
static int test_no_summary__ = 0;
static int test_verbose_level__ = 2;
static int test_skip_mode__ = 0;

static int test_stat_failed_units__ = 0;
static int test_stat_run_units__ = 0;

static const struct test__* test_current_unit__ = NULL;
static int test_current_already_logged__ = 0;
static int test_current_failures__ = 0;


#ifdef __GNUC__
    void test_msg__(int verbose_level, const char* fmt, ...)
            __attribute__((format (printf, 2, 3)));
#endif


void
test_msg__(int verbose_level, const char* fmt, ...)
{
    va_list args;
    size_t len;

    if(verbose_level > test_verbose_level__)
        return;

    /* In low verbose levels, we haven't written what unit test we are running. */
    if(!test_current_already_logged__  &&  test_current_unit__ != NULL)
        printf("In unit test %s:\n", test_current_unit__->name);

    printf("  ");
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    len = strlen(fmt);
    if(len > 0  &&  fmt[len-1] != '\n')
        printf("\n");

    test_current_already_logged__++;
}

int
test_check__(int cond, const char* file, int line, const char* cond_str, const char* fmt, ...)
{
    const char *result_str;
    int verbose_level;
    char buffer[128] = "";

    if(cond) {
        result_str = "passed";
        verbose_level = 3;
    } else {
        result_str = "FAILED";
        verbose_level = 1;
        test_current_failures__++;

        if(fmt != NULL) {
            va_list args;

            buffer[0] = ' ';
            buffer[1] = ' ';
            va_start(args, fmt);
            vsnprintf(buffer+2, sizeof(buffer)-3, fmt, args);
            va_end(args);
            buffer[sizeof(buffer)-1] = '\0';
        }
    }

    test_msg__(verbose_level, "%s:%d: Condition '%s' has %s.%s",
               file, line, cond_str, result_str, buffer);
    return (cond != 0);
}

void
test_list_names__(void)
{
    const struct test__* test;

    printf("Unit tests:\n");
    for(test = &test_list__[0]; test->func != NULL; test++)
        printf("  %s\n", test->name);
}

const struct test__*
test_by_name__(const char* name)
{
    const struct test__* test;

    for(test = &test_list__[0]; test->func != NULL; test++) {
        if(strcmp(test->name, name) == 0)
            return test;
    }

    return NULL;
}

int
test_do_run__(const struct test__* test)
{
    test_current_unit__ = test;
    test_current_failures__ = 0;
    test_current_already_logged__ = 0;

    if(test_verbose_level__ >= 2) {
        printf("Starting test %s...\n", test->name);
        test_current_already_logged__++;
    }

#ifdef __cplusplus
    try {
#endif

        test->func();

#ifdef __cplusplus
    } catch(std::exception& e) {
        test_current_failures__++;
        const char* what = e.what();
        if(what == NULL)
            what = "<null>";
        test_msg__(1, "Caught C++ exception:  %s", what);
    } catch(...) {
        test_current_failures__++;
        test_msg__(1, "Caught C++ exception (not derived from std::exception)");
    }
#endif

    switch(test_current_failures__) {
        case 0:  test_msg__(2, "All conditions have passed."); break;
        case 1:  test_msg__(2, "One condition has FAILED."); break;
        default: test_msg__(2, "%d conditions have FAILED.", test_current_failures__); break;
    }

    test_current_unit__ = NULL;

    return (test_current_failures__ == 0) ? 0 : -1;
}

void
test_run__(const struct test__* test)
{
    int failed = 1;

    test_current_unit__ = test;
    test_current_already_logged__ = 0;

    if(!test_no_exec__) {

#if defined(CUTEST_UNIX__)

        pid_t pid;
        int exit_code;

        pid = fork();
        if(pid == (pid_t)-1) {
            test_msg__(1, "Cannot start the unit test subprocess. %s [%d]", strerror(errno), errno);
            failed = 1;
        } else if(pid == 0) {
            failed = (test_do_run__(test) != 0);
            exit(failed ? 1 : 0);
        } else {
            waitpid(pid, &exit_code, 0);
            if(WIFEXITED(exit_code)) {
                switch(WEXITSTATUS(exit_code)) {
                    case 0:   failed = 0; break;   /* test has passed. */
                    case 1:   /* noop */ break;    /* "normal" failure. */
                    default:  test_msg__(1, "Unexpected subprocess exit code [%d]", WEXITSTATUS(exit_code));
                }
            } else if(WIFSIGNALED(exit_code)) {
                char tmp[32];
                const char* signame;
                switch(WTERMSIG(exit_code)) {
                    case SIGINT:  signame = "SIGINT"; break;
                    case SIGHUP:  signame = "SIGHUP"; break;
                    case SIGQUIT: signame = "SIGQUIT"; break;
                    case SIGABRT: signame = "SIGABRT"; break;
                    case SIGKILL: signame = "SIGKILL"; break;
                    case SIGSEGV: signame = "SIGSEGV"; break;
                    case SIGILL:  signame = "SIGILL"; break;
                    case SIGTERM: signame = "SIGTERM"; break;
                    default:      sprintf(tmp, "signal %d", WTERMSIG(exit_code)); signame = tmp; break;
                }
                test_msg__(1, "Test interrupted by %s", signame);
            } else {
                test_msg__(1, "Test ended in an unexpected way [%d]", exit_code);
            }
        }

#elif defined(CUTEST_WIN__)

        char buffer[256] = {0};
        STARTUPINFOA startupInfo = {0};
        PROCESS_INFORMATION processInfo;
        DWORD exitCode;

        snprintf(buffer, sizeof(buffer)-1, "%s --no-exec --no-summary --verbose=%d \"%s\"",
                 test_argv0__, test_verbose_level__, test->name);
        startupInfo.cb = sizeof(STARTUPINFO);
        if(CreateProcessA(NULL, buffer, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            GetExitCodeProcess(processInfo.hProcess, &exitCode);
            CloseHandle(processInfo.hThread);
            CloseHandle(processInfo.hProcess);
            failed = (exitCode != 0);
        } else {
            test_msg__(1, "Cannot start the unit test subprocess [%ld].", GetLastError());
            failed = 1;
        }

#else

        failed = (test_do_run__(test) != 0);

#endif

    } else {
        failed = (test_do_run__(test) != 0);
    }

    test_current_unit__ = NULL;

    test_stat_run_units__++;
    if(failed)
        test_stat_failed_units__++;
}

#if defined(CUTEST_WIN__)
LONG CALLBACK
test_exception_filter__(EXCEPTION_POINTERS *ptrs)
{
    test_msg__(1, "Unhandled exception %08lx at %p. Unit test has crashed??",
               ptrs->ExceptionRecord->ExceptionCode, ptrs->ExceptionRecord->ExceptionAddress);
    fflush(stdout);
    fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void
test_help__(void)
{
    printf("Usage: %s [options] [test...]\n", test_argv0__);
    printf("Run the specified unit tests; or if the option '--skip' is used, run all\n");
    printf("tests in the suite but those listed.  By default, if no tests are specified\n");
    printf("on the command line, all unit tests in the suite are run.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -s, --skip            Execute all unit tests but the listed ones\n");
    printf("      --no-exec         Do not execute unit tests as child processes\n");
    printf("      --no-summary      Suppress printing of test results summary\n");
    printf("  -l, --list            List unit tests in the suite and exit\n");
    printf("  -v, --verbose         Enable more verbose output\n");
    printf("      --verbose=LEVEL   Set verbose level to LEVEL (small integer)\n");
    printf("  -h, --help            Display this help and exit\n");
    printf("\n");
    test_list_names__();
}

int
main(int argc, char** argv)
{
    const struct test__** tests = NULL;
    int i, j, n = 0;

    test_argv0__ = argv[0];

    /* Parse options */
    for(i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            test_help__();
            exit(0);
        } else if(strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            test_verbose_level__++;
        } else if(strncmp(argv[i], "--verbose=", 10) == 0) {
            test_verbose_level__ = atoi(argv[i] + 10);
        } else if(strcmp(argv[i], "--skip") == 0 || strcmp(argv[i], "-s") == 0) {
            test_skip_mode__ = 1;
        } else if(strcmp(argv[i], "--no-exec") == 0) {
            test_no_exec__ = 1;
        } else if(strcmp(argv[i], "--no-summary") == 0) {
            test_no_summary__ = 1;
        } else if(strcmp(argv[i], "--list") == 0 || strcmp(argv[i], "-l") == 0) {
            test_list_names__();
            exit(0);
        } else if(argv[i][0] != '-') {
            tests = (const struct test__**) realloc(tests, (n+1) * sizeof(struct test__));
            tests[n] = test_by_name__(argv[i]);
            if(tests[n] == NULL) {
                fprintf(stderr, "%s: Unrecognized unit test '%s'\n", argv[0], argv[i]);
                fprintf(stderr, "Try '%s --list' for list of unit tests.\n", argv[0]);
                exit(2);
            }
            n++;
        } else {
            fprintf(stderr, "%s: Unrecognized option '%s'\n", argv[0], argv[i]);
            fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
            exit(2);
        }
    }

#if defined(CUTEST_WIN__)
    SetUnhandledExceptionFilter(test_exception_filter__);
#endif

    /* Count all test units */
    test_count__ = 0;
    for(i = 0; test_list__[i].func != NULL; i++)
        test_count__++;

    /* Run the tests */
    if(n == 0) {
        /* Run all tests */
        for(i = 0; test_list__[i].func != NULL; i++)
            test_run__(&test_list__[i]);
    } else if(!test_skip_mode__) {
        /* Run the listed tests */
        for(i = 0; i < n; i++)
            test_run__(tests[i]);
    } else {
        /* Run all tests except those listed */
        int is_skipped;

        for(i = 0; test_list__[i].func != NULL; i++) {
            is_skipped = 0;
            for(j = 0; j < n; j++) {
                if(tests[j] == &test_list__[i]) {
                    is_skipped = 1;
                    break;
                }
            }
            if(!is_skipped)
                test_run__(&test_list__[i]);
        }
    }

    /* Write a summary */
    if(!test_no_summary__) {
        if(test_verbose_level__ >= 2) {
            printf("\nSummary:\n");
            printf("  Count of all unit tests:     %4d\n", test_count__);
            printf("  Count of run unit tests:     %4d\n", test_stat_run_units__);
            printf("  Count of failed unit tests:  %4d\n", test_stat_failed_units__);
            printf("  Count of skipped unit tests: %4d\n", test_count__ - test_stat_run_units__);
        }
        if(test_verbose_level__ >= 1) {
            printf("\n");
            if(test_stat_failed_units__ == 0)
                printf("SUCCESS: All unit tests have passed.\n");
            else
                printf("FAILED: %d of %d unit tests have failed.\n", test_stat_failed_units__, test_stat_run_units__);
        }
    }

    return (test_stat_failed_units__ == 0) ? 0 : 1;
}


#ifdef __cplusplus
    }  /* extern "C" */
#endif
