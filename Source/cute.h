/*
 * Cute -- C Unit Test Library
 * <https://github.com/mity/Cute>
 *
 * Copyright 2023 Alrick Grandison
 * Copyright 2013-2020 Martin Mitas
 * Copyright 2019 Garrett D'Amore
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef H_CUTE
#define H_CUTE


/************************
 *** Public interface ***
 ************************/

/* By default, "Cute.h" provides the main program entry point (function
 * main()). However, if the test suite is composed of multiple source files
 * which include "Cute.h", then this causes a problem of multiple main()
 * definitions. To avoid this problem, #define macro CUTE_NO_MAIN in all
 * compilation units but one or in all if you want to write your own custom main().
 */

/* Macro to specify list of unit tests in the suite.
 * The unit test implementation MUST provide list of unit tests it implements
 * with this macro:
 *
 *   CUTE_LIST = {
 *       { "test1_name", test1_func_ptr },
 *       { "test2_name", test2_func_ptr },
 *       ...
 *       { NULL, NULL }     // zeroed record marking the end of the list
 *   };
 *
 * The list specifies names of each test (must be unique) and pointer to
 * a function implementing it. The function does not take any arguments
 * and has no return values, i.e. every test function has to be compatible
 * with this prototype:
 *
 *   void test_func(void);
 *
 * Note the list has to be ended with a zeroed record.
 */
#define CUTE_LIST               const struct Cute_Test Cute_list_test[]


/* Macros for testing whether an unit test succeeds or fails. These macros
 * can be used arbitrarily in functions implementing the unit tests.
 *
 * If any condition fails throughout execution of a test, the test fails.
 *
 * CUTE_CHECK takes only one argument (the condition), CUTE_CHECK_FORMAT allows
 * also to specify an error message to print out if the condition fails.
 * (It expects printf-like format string and its parameters). The macros
 * return non-zero (condition passes) or 0 (condition fails).
 *
 * That can be useful when more conditions should be checked only if some
 * preceding condition passes, as illustrated in this code snippet:
 *
 *   SomeStruct* ptr = allocate_some_struct();
 *   if(CUTE_CHECK(ptr != NULL)) {
 *       CUTE_CHECK(ptr->member1 < 100);
 *       CUTE_CHECK(ptr->member2 > 200);
 *   }
 */
#define CUTE_CHECK_FORMAT(cond,...)   Cute_Check((cond), __FILE__, __LINE__, __VA_ARGS__)
#define CUTE_CHECK(cond)        Cute_Check((cond), __FILE__, __LINE__, "%s", #cond)


/* These macros are the same as CUTE_CHECK_FORMAT and CUTE_CHECK except that if the
 * condition fails, the currently executed unit test is immediately aborted.
 *
 * That is done either by calling abort() if the unit test is executed as a
 * child process; or via longjmp() if the unit test is executed within the
 * main Cute process.
 *
 * As a side effect of such abortion, your unit tests may cause memory leaks,
 * unflushed file descriptors, and other phenomena caused by the abortion.
 *
 * Therefore you should not use these as a general replacement for CUTE_CHECK.
 * Use it with some caution, especially if your test causes some other side
 * effects to the outside world (e.g. communicating with some server, inserting
 * into a database etc.).
 */
#define CUTE_ASSERT_FORMAT(cond,...)                                                 \
    do {                                                                       \
        if(!Cute_Check((cond), __FILE__, __LINE__, __VA_ARGS__))           \
            Cute_Abort();                                                  \
    } while(0)
#define CUTE_ASSERT(cond)                                                      \
    do {                                                                       \
        if(!Cute_Check((cond), __FILE__, __LINE__, "%s", #cond))           \
            Cute_Abort();                                                  \
    } while(0)

/* Sometimes it is useful to split execution of more complex unit tests to some
 * smaller parts and associate those parts with some names.
 *
 * This is especially handy if the given unit test is implemented as a loop
 * over some vector of multiple testing inputs. Using these macros allow to use
 * sort of subtitle for each iteration of the loop (e.g. outputting the input
 * itself or a name associated to it), so that if any CUTE_CHECK condition
 * fails in the loop, it can be easily seen which iteration triggers the
 * failure, without the need to manually output the iteration-specific data in
 * every single CUTE_CHECK inside the loop body.
 *
 * CUTE_CASE allows to specify only single string as the name of the case,
 * CUTE_CASE_FORMAT provides all the power of printf-like string formatting.
 *
 * Note that the test cases cannot be nested. Starting a new test case ends
 * implicitly the previous one. To end the test case explicitly (e.g. to end
 * the last test case after exiting the loop), you may use CUTE_CASE(NULL).
 */
#define CUTE_CASE_FORMAT(...)         Cute_Case(__VA_ARGS__)
#define CUTE_CASE(name)         Cute_Case("%s", name)


/* Maximal output per CUTE_CASE call. Longer messages are cut.
 * You may define another limit prior including "Cute.h"
 */
#ifndef CUTE_CASE_MAXSIZE
    #define CUTE_CASE_MAXSIZE    64
#endif


/* printf-like macro for outputting an extra information about a failure.
 *
 * Intended use is to output some computed output versus the expected value,
 * e.g. like this:
 *
 *   if(!CUTE_CHECK(produced == expected)) {
 *       CUTE_MSG("Expected: %d", expected);
 *       CUTE_MSG("Produced: %d", produced);
 *   }
 *
 * Note the message is only written down if the most recent use of any checking
 * macro (like e.g. CUTE_CHECK or CUTE_EXCEPTION) in the current test failed.
 * This means the above is equivalent to just this:
 *
 *   CUTE_CHECK(produced == expected);
 *   CUTE_MSG("Expected: %d", expected);
 *   CUTE_MSG("Produced: %d", produced);
 *
 * The macro can deal with multi-line output fairly well. It also automatically
 * adds a final new-line if there is none present.
 */
#define CUTE_MSG(...)           Cute_Message(__VA_ARGS__)


/* Maximal output per CUTE_MSG call. Longer messages are cut.
 * You may define another limit prior including "Cute.h"
 */
#ifndef CUTE_MSG_MAXSIZE
    #define CUTE_MSG_MAXSIZE    1024
#endif


/* Macro for dumping a block of memory.
 *
 * Its intended use is very similar to what CUTE_MSG is for, but instead of
 * generating any printf-like message, this is for dumping raw block of a
 * memory in a hexadecimal form:
 *
 *   CUTE_CHECK(size_produced == size_expected &&
 *              memcmp(addr_produced, addr_expected, size_produced) == 0);
 *   CUTE_DUMP("Expected:", addr_expected, size_expected);
 *   CUTE_DUMP("Produced:", addr_produced, size_produced);
 */
#define CUTE_DUMP(title, addr, size)    Cute_Dump(title, addr, size)

/* Maximal output per CUTE_DUMP call (in bytes to dump). Longer blocks are cut.
 * You may define another limit prior including "Cute.h"
 */
#ifndef CUTE_DUMP_MAXSIZE
    #define CUTE_DUMP_MAXSIZE   1024
#endif


/* Common test initialiation/clean-up
 *
 * In some test suites, it may be needed to perform some sort of the same
 * initialization and/or clean-up in all the tests.
 *
 * Such test suites may use macros CUTE_INIT and/or CUTE_FINI prior including
 * this header. The expansion of the macro is then used as a body of helper
 * function called just before executing every single (CUTE_INIT) or just after
 * it ends (CUTE_FINI).
 *
 * Examples of various ways how to use the macro CUTE_INIT:
 *
 *   #define CUTE_INIT      my_init_func();
 *   #define CUTE_INIT      my_init_func()      // Works even without the semicolon
 *   #define CUTE_INIT      setlocale(LC_ALL, NULL);
 *   #define CUTE_INIT      { setlocale(LC_ALL, NULL); my_init_func(); }
 *
 * CUTE_FINI is to be used in the same way.
 */


/**********************
 *** Implementation ***
 **********************/

/* The unit test files should not rely on anything below. */

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)
    #define CUTE_UNIX       1
    #include <errno.h>
    #include <libgen.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <signal.h>
    #include <time.h>

    #if defined CLOCK_PROCESS_CPUTIME_ID  &&  defined CLOCK_MONOTONIC
        #define CUTE_HAS_POSIX_TIMER    1
    #endif
#endif

#if defined(_gnu_linux_) || defined(__linux__)
    #define CUTE_LINUX      1
    #include <fcntl.h>
    #include <sys/stat.h>
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
    #define CUTE_WINDOWS        1
    #include <windows.h>
    #include <io.h>
#endif

#if defined(__APPLE__)
    #define CUTE_MAC
    #include <assert.h>
    #include <stdbool.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <sys/sysctl.h>
#endif

#ifdef __has_include
    #if __has_include(<valgrind.h>)
        #include <valgrind.h>
    #endif
#endif

/* Enable the use of the non-standard keyword __attribute__ to silence warnings under some compilers */
#if defined(__GNUC__) || defined(__clang__)
    #define CUTE_ATTRIBUTE(attr)    __attribute__((attr))
#else
    #define CUTE_ATTRIBUTE(attr)
#endif

/* Note our global private identifiers end with '_' to mitigate risk of clash
 * with the unit tests implementation. */

#ifdef __cplusplus
    extern "C" {
#endif

#ifdef _MSC_VER
    /* In the multi-platform code like ours, we cannot use the non-standard
     * "safe" functions from Microsoft C lib like e.g. sprintf_s() instead of
     * standard sprintf(). Hence, lets disable the warning C4996. */
    #pragma warning(push)
    #pragma warning(disable: 4996)
#endif


struct Cute_Test {
    const char* name;
    void (*func)(void);
};

struct Cute_TestData {
    unsigned char flags;
    double duration;
};

enum {
    CUTE_FLAG_RUN = 1 << 0,
    CUTE_FLAG_SUCCESS = 1 << 1,
    CUTE_FLAG_FAILURE = 1 << 2,
};

extern const struct Cute_Test Cute_list_test[];

int Cute_Check(int cond, const char* file, int line, const char* fmt, ...);
void Cute_Case(const char* fmt, ...);
void Cute_Message(const char* fmt, ...);
void Cute_Dump(const char* title, const void* addr, size_t size);
void Cute_Abort(void) CUTE_ATTRIBUTE(noreturn);


#ifndef CUTE_NO_MAIN

static char* Cute_argv0 = NULL;
static size_t Cute_size_list = 0;
static struct Cute_TestData* Cute_data_test = NULL;
static size_t Cute_count = 0;
static int Cute_noexec = -1;
static int Cute_nosummary = 0;
static int Cute_tap = 0;
static int Cute_mode_skip = 0;
static int Cute_worker = 0;
static int Cute_index_worker = 0;
static int Cute_failedcond = 0;
static int Cute_wasaborted = 0;
static FILE *Cute_output_xml = NULL;

static int Cute_stat_units_failed = 0;
static int Cute_stat_units_run = 0;

static const struct Cute_Test* Cute_currenttest = NULL;
static int Cute_currentindex = 0;
static char Cute_casename[CUTE_CASE_MAXSIZE] = "";
static int Cute_test_alreadylogged = 0;
static int Cute_case_alreadylogged = 0;
static int Cute_level_verbose = 2;
static int Cute_testfailures = 0;
static int Cute_colorize = 0;
static int Cute_timer = 0;

static int Cute_abort_has_jmpbuf = 0;
static jmp_buf Cute_jmpbuf_abort;


static void
Cute_Cleanup(void)
{
    free((void*) Cute_data_test);
}

static void CUTE_ATTRIBUTE(noreturn)
Cute_Exit(int exit_code)
{
    Cute_Cleanup();
    exit(exit_code);
}

#if defined CUTE_WINDOWS
    typedef LARGE_INTEGER Cute_type_timer;
    static LARGE_INTEGER acutest_timer_freq_;
    static Cute_type_timer Cute_start_timer;
    static Cute_type_timer Cute_end_timer;

    static void
    Cute_InitializeTimer(void)
    {
        QueryPerformanceFrequency(&acutest_timer_freq_);
    }

    static void
    Cute_GetTimerTime(LARGE_INTEGER* ts)
    {
        QueryPerformanceCounter(ts);
    }

    static double
    Cute_GetTimerDifference(LARGE_INTEGER start, LARGE_INTEGER end)
    {
        double duration = (double)(end.QuadPart - start.QuadPart);
        duration /= (double)acutest_timer_freq_.QuadPart;
        return duration;
    }

    static void
    Cute_PrintTimerDifference(void)
    {
        printf("%.6lf secs", Cute_GetTimerDifference(Cute_start_timer, Cute_end_timer));
    }
#elif defined CUTE_HAS_POSIX_TIMER
    static clockid_t Cute_id_timer;
    typedef struct timespec Cute_type_timer;
    static Cute_type_timer Cute_start_timer;
    static Cute_type_timer Cute_end_timer;

    static void
    Cute_InitializeTimer(void)
    {
        if(Cute_timer == 1)
            Cute_id_timer = CLOCK_MONOTONIC;
        else if(Cute_timer == 2)
            Cute_id_timer = CLOCK_PROCESS_CPUTIME_ID;
    }

    static void
    Cute_GetTimerTime(struct timespec* ts)
    {
        clock_gettime(Cute_id_timer, ts);
    }

    static double
    Cute_GetTimerDifference(struct timespec start, struct timespec end)
    {
        double endns;
        double startns;

        endns = end.tv_sec;
        endns *= 1e9;
        endns += end.tv_nsec;

        startns = start.tv_sec;
        startns *= 1e9;
        startns += start.tv_nsec;

        return ((endns - startns)/ 1e9);
    }

    static void
    Cute_PrintTimerDifference(void)
    {
        printf("%.6lf secs",
            Cute_GetTimerDifference(Cute_start_timer, Cute_end_timer));
    }
#else
    typedef int Cute_type_timer;
    static Cute_type_timer Cute_start_timer;
    static Cute_type_timer Cute_end_timer;

    void
    Cute_InitializeTimer(void)
    {}

    static void
    Cute_GetTimerTime(int* ts)
    {
        (void) ts;
    }

    static double
    Cute_GetTimerDifference(int start, int end)
    {
        (void) start;
        (void) end;
        return 0.0;
    }

    static void
    Cute_PrintTimerDifference(void)
    {}
#endif

#define CUTE_COLOR_DEFAULT              0
#define CUTE_COLOR_GREEN                1
#define CUTE_COLOR_RED                  2
#define CUTE_COLOR_DEFAULT_INTENSIVE    3
#define CUTE_COLOR_GREEN_INTENSIVE      4
#define CUTE_COLOR_RED_INTENSIVE        5

static int CUTE_ATTRIBUTE(format (printf, 2, 3))
Cute_PrintColored(int color, const char* fmt, ...)
{
    va_list args;
    char buffer[256];
    int n;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    buffer[sizeof(buffer)-1] = '\0';

    if(!Cute_colorize) {
        return printf("%s", buffer);
    }

#if defined CUTE_UNIX
    {
        const char* col_str;
        switch(color) {
            case CUTE_COLOR_GREEN:              col_str = "\033[0;32m"; break;
            case CUTE_COLOR_RED:                col_str = "\033[0;31m"; break;
            case CUTE_COLOR_GREEN_INTENSIVE:    col_str = "\033[1;32m"; break;
            case CUTE_COLOR_RED_INTENSIVE:      col_str = "\033[1;31m"; break;
            case CUTE_COLOR_DEFAULT_INTENSIVE:  col_str = "\033[1m"; break;
            default:                                col_str = "\033[0m"; break;
        }
        printf("%s", col_str);
        n = printf("%s", buffer);
        printf("\033[0m");
        return n;
    }
#elif defined CUTE_WINDOWS
    {
        HANDLE h;
        CONSOLE_SCREEN_BUFFER_INFO info;
        WORD attr;

        h = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(h, &info);

        switch(color) {
            case CUTE_COLOR_GREEN:              attr = FOREGROUND_GREEN; break;
            case CUTE_COLOR_RED:                attr = FOREGROUND_RED; break;
            case CUTE_COLOR_GREEN_INTENSIVE:    attr = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
            case CUTE_COLOR_RED_INTENSIVE:      attr = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
            case CUTE_COLOR_DEFAULT_INTENSIVE:  attr = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY; break;
            default:                                attr = 0; break;
        }
        if(attr != 0)
            SetConsoleTextAttribute(h, attr);
        n = printf("%s", buffer);
        SetConsoleTextAttribute(h, info.wAttributes);
        return n;
    }
#else
    n = printf("%s", buffer);
    return n;
#endif
}

static void
Cute_BeginTestLine(const struct Cute_Test* test)
{
    if(!Cute_tap) {
        if(Cute_level_verbose >= 3) {
            Cute_PrintColored(CUTE_COLOR_DEFAULT_INTENSIVE, "Test %s:\n", test->name);
            Cute_test_alreadylogged++;
        } else if(Cute_level_verbose >= 1) {
            int n;
            char spaces[48];

            n = Cute_PrintColored(CUTE_COLOR_DEFAULT_INTENSIVE, "Test %s... ", test->name);
            memset(spaces, ' ', sizeof(spaces));
            if(n < (int) sizeof(spaces))
                printf("%.*s", (int) sizeof(spaces) - n, spaces);
        } else {
            Cute_test_alreadylogged = 1;
        }
    }
}

static void
Cute_FinishTestLine(int result)
{
    if(Cute_tap) {
        const char* str = (result == 0) ? "ok" : "not ok";

        printf("%s %d - %s\n", str, Cute_currentindex + 1, Cute_currenttest->name);

        if(result == 0  &&  Cute_timer) {
            printf("# Duration: ");
            Cute_PrintTimerDifference();
            printf("\n");
        }
    } else {
        int color = (result == 0) ? CUTE_COLOR_GREEN_INTENSIVE : CUTE_COLOR_RED_INTENSIVE;
        const char* str = (result == 0) ? "OK" : "FAILED";
        printf("[ ");
        Cute_PrintColored(color, "%s", str);
        printf(" ]");

        if(result == 0  &&  Cute_timer) {
            printf("  ");
            Cute_PrintTimerDifference();
        }

        printf("\n");
    }
}

static void
Cute_IndentLine(int level)
{
    static const char spaces[] = "                ";
    int n = level * 2;

    if(Cute_tap  &&  n > 0) {
        n--;
        printf("#");
    }

    while(n > 16) {
        printf("%s", spaces);
        n -= 16;
    }
    printf("%.*s", n, spaces);
}

int CUTE_ATTRIBUTE(format (printf, 4, 5))
Cute_Check(int cond, const char* file, int line, const char* fmt, ...)
{
    const char *result_str;
    int result_color;
    int verbose_level;

    if(cond) {
        result_str = "ok";
        result_color = CUTE_COLOR_GREEN;
        verbose_level = 3;
    } else {
        if(!Cute_test_alreadylogged  &&  Cute_currenttest != NULL)
            Cute_FinishTestLine(-1);

        result_str = "failed";
        result_color = CUTE_COLOR_RED;
        verbose_level = 2;
        Cute_testfailures++;
        Cute_test_alreadylogged++;
    }

    if(Cute_level_verbose >= verbose_level) {
        va_list args;

        if(!Cute_case_alreadylogged  &&  Cute_casename[0]) {
            Cute_IndentLine(1);
            Cute_PrintColored(CUTE_COLOR_DEFAULT_INTENSIVE, "Case %s:\n", Cute_casename);
            Cute_test_alreadylogged++;
            Cute_case_alreadylogged++;
        }

        Cute_IndentLine(Cute_casename[0] ? 2 : 1);
        if(file != NULL) {
#ifdef CUTE_WINDOWS
            const char* lastsep1 = strrchr(file, '\\');
            const char* lastsep2 = strrchr(file, '/');
            if(lastsep1 == NULL)
                lastsep1 = file-1;
            if(lastsep2 == NULL)
                lastsep2 = file-1;
            file = (lastsep1 > lastsep2 ? lastsep1 : lastsep2) + 1;
#else
            const char* lastsep = strrchr(file, '/');
            if(lastsep != NULL)
                file = lastsep+1;
#endif
            printf("%s:%d: Check ", file, line);
        }

        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);

        printf("... ");
        Cute_PrintColored(result_color, "%s", result_str);
        printf("\n");
        Cute_test_alreadylogged++;
    }

    Cute_failedcond = (cond == 0);
    return !Cute_failedcond;
}

void CUTE_ATTRIBUTE(format (printf, 1, 2))
Cute_Case(const char* fmt, ...)
{
    va_list args;

    if(Cute_level_verbose < 2)
        return;

    if(Cute_casename[0]) {
        Cute_case_alreadylogged = 0;
        Cute_casename[0] = '\0';
    }

    if(fmt == NULL)
        return;

    va_start(args, fmt);
    vsnprintf(Cute_casename, sizeof(Cute_casename) - 1, fmt, args);
    va_end(args);
    Cute_casename[sizeof(Cute_casename) - 1] = '\0';

    if(Cute_level_verbose >= 3) {
        Cute_IndentLine(1);
        Cute_PrintColored(CUTE_COLOR_DEFAULT_INTENSIVE, "Case %s:\n", Cute_casename);
        Cute_test_alreadylogged++;
        Cute_case_alreadylogged++;
    }
}

void CUTE_ATTRIBUTE(format (printf, 1, 2))
Cute_Message(const char* fmt, ...)
{
    char buffer[CUTE_MSG_MAXSIZE];
    char* line_beg;
    char* line_end;
    va_list args;

    if(Cute_level_verbose < 2)
        return;

    /* We allow extra message only when something is already wrong in the
     * current test. */
    if(Cute_currenttest == NULL  ||  !Cute_failedcond)
        return;

    va_start(args, fmt);
    vsnprintf(buffer, CUTE_MSG_MAXSIZE, fmt, args);
    va_end(args);
    buffer[CUTE_MSG_MAXSIZE-1] = '\0';

    line_beg = buffer;
    while(1) {
        line_end = strchr(line_beg, '\n');
        if(line_end == NULL)
            break;
        Cute_IndentLine(Cute_casename[0] ? 3 : 2);
        printf("%.*s\n", (int)(line_end - line_beg), line_beg);
        line_beg = line_end + 1;
    }
    if(line_beg[0] != '\0') {
        Cute_IndentLine(Cute_casename[0] ? 3 : 2);
        printf("%s\n", line_beg);
    }
}

void
Cute_Dump(const char* title, const void* addr, size_t size)
{
    static const size_t BYTES_PER_LINE = 16;
    size_t line_beg;
    size_t truncate = 0;

    if(Cute_level_verbose < 2)
        return;

    /* We allow extra message only when something is already wrong in the
     * current test. */
    if(Cute_currenttest == NULL  ||  !Cute_failedcond)
        return;

    if(size > CUTE_DUMP_MAXSIZE) {
        truncate = size - CUTE_DUMP_MAXSIZE;
        size = CUTE_DUMP_MAXSIZE;
    }

    Cute_IndentLine(Cute_casename[0] ? 3 : 2);
    printf((title[strlen(title)-1] == ':') ? "%s\n" : "%s:\n", title);

    for(line_beg = 0; line_beg < size; line_beg += BYTES_PER_LINE) {
        size_t line_end = line_beg + BYTES_PER_LINE;
        size_t off;

        Cute_IndentLine(Cute_casename[0] ? 4 : 3);
        printf("%08lx: ", (unsigned long)line_beg);
        for(off = line_beg; off < line_end; off++) {
            if(off < size)
                printf(" %02x", ((const unsigned char*)addr)[off]);
            else
                printf("   ");
        }

        printf("  ");
        for(off = line_beg; off < line_end; off++) {
            unsigned char byte = ((const unsigned char*)addr)[off];
            if(off < size)
                printf("%c", (iscntrl(byte) ? '.' : byte));
            else
                break;
        }

        printf("\n");
    }

    if(truncate > 0) {
        Cute_IndentLine(Cute_casename[0] ? 4 : 3);
        printf("           ... (and more %u bytes)\n", (unsigned) truncate);
    }
}

/* This is called just before each test */
static void
Cute_Initialize(const char *test_name)
{
#ifdef CUTE_INIT
    CUTE_INIT
    ; /* Allow for a single unterminated function call */
#endif

    /* Suppress any warnings about unused variable. */
    (void) test_name;
}

/* This is called after each test */
static void
Cute_Uninitialize(const char *test_name)
{
#ifdef CUTE_FINI
    CUTE_FINI
    ; /* Allow for a single unterminated function call */
#endif

    /* Suppress any warnings about unused variable. */
    (void) test_name;
}

void
Cute_Abort(void)
{
    if(Cute_abort_has_jmpbuf) {
        longjmp(Cute_jmpbuf_abort, 1);
    } else {
        if(Cute_currenttest != NULL)
            Cute_Uninitialize(Cute_currenttest->name);
        abort();
    }
}

static void
Cute_PrintNameList(void)
{
    const struct Cute_Test* test;

    printf("Unit tests:\n");
    for(test = &Cute_list_test[0]; test->func != NULL; test++)
        printf("  %s\n", test->name);
}

static void
Cute_Remember(int i)
{
    if(Cute_data_test[i].flags & CUTE_FLAG_RUN)
        return;

    Cute_data_test[i].flags |= CUTE_FLAG_RUN;
    Cute_count++;
}

static void
Cute_SetSuccess(int i, int success)
{
    Cute_data_test[i].flags |= success ? CUTE_FLAG_SUCCESS : CUTE_FLAG_FAILURE;
}

static void
Cute_SetDuration(int i, double duration)
{
    Cute_data_test[i].duration = duration;
}

static int
Cute_ConfirmNameContainsWord(const char* name, const char* pattern)
{
    static const char word_delim[] = " \t-_/.,:;";
    const char* substr;
    size_t pattern_len;

    pattern_len = strlen(pattern);

    substr = strstr(name, pattern);
    while(substr != NULL) {
        int starts_on_word_boundary = (substr == name || strchr(word_delim, substr[-1]) != NULL);
        int ends_on_word_boundary = (substr[pattern_len] == '\0' || strchr(word_delim, substr[pattern_len]) != NULL);

        if(starts_on_word_boundary && ends_on_word_boundary)
            return 1;

        substr = strstr(substr+1, pattern);
    }

    return 0;
}

static int
Cute_Lookup(const char* pattern)
{
    int i;
    int n = 0;

    /* Try exact match. */
    for(i = 0; i < (int) Cute_size_list; i++) {
        if(strcmp(Cute_list_test[i].name, pattern) == 0) {
            Cute_Remember(i);
            n++;
            break;
        }
    }
    if(n > 0)
        return n;

    /* Try word match. */
    for(i = 0; i < (int) Cute_size_list; i++) {
        if(Cute_ConfirmNameContainsWord(Cute_list_test[i].name, pattern)) {
            Cute_Remember(i);
            n++;
        }
    }
    if(n > 0)
        return n;

    /* Try relaxed match. */
    for(i = 0; i < (int) Cute_size_list; i++) {
        if(strstr(Cute_list_test[i].name, pattern) != NULL) {
            Cute_Remember(i);
            n++;
        }
    }

    return n;
}


/* Called if anything goes bad in Cute, or if the unit test ends in other
 * way then by normal returning from its function (e.g. exception or some
 * abnormal child process termination). */
static void CUTE_ATTRIBUTE(format (printf, 1, 2))
Cute_PrintError(const char* fmt, ...)
{
    if(Cute_level_verbose == 0)
        return;

    if(Cute_level_verbose >= 2) {
        va_list args;

        Cute_IndentLine(1);
        if(Cute_level_verbose >= 3)
            Cute_PrintColored(CUTE_COLOR_RED_INTENSIVE, "ERROR: ");
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        printf("\n");
    }

    if(Cute_level_verbose >= 3) {
        printf("\n");
    }
}

/* Call directly the given test unit function. */
static int
Cute_RunTest(const struct Cute_Test* test, int index)
{
    int status = -1;

    Cute_wasaborted = 0;
    Cute_currenttest = test;
    Cute_currentindex = index;
    Cute_testfailures = 0;
    Cute_test_alreadylogged = 0;
    Cute_failedcond = 0;

    Cute_Initialize(test->name);
    Cute_BeginTestLine(test);

    /* This is good to do in case the test unit crashes. */
    fflush(stdout);
    fflush(stderr);

    if(!Cute_worker) {
        Cute_abort_has_jmpbuf = 1;
        if(setjmp(Cute_jmpbuf_abort) != 0) {
            Cute_wasaborted = 1;
            goto aborted;
        }
    }

    Cute_GetTimerTime(&Cute_start_timer);
    test->func();
aborted:
    Cute_abort_has_jmpbuf = 0;
    Cute_GetTimerTime(&Cute_end_timer);

    if(Cute_level_verbose >= 3) {
        Cute_IndentLine(1);
        if(Cute_testfailures == 0) {
            Cute_PrintColored(CUTE_COLOR_GREEN_INTENSIVE, "SUCCESS: ");
            printf("All conditions have passed.\n");

            if(Cute_timer) {
                Cute_IndentLine(1);
                printf("Duration: ");
                Cute_PrintTimerDifference();
                printf("\n");
            }
        } else {
            Cute_PrintColored(CUTE_COLOR_RED_INTENSIVE, "FAILED: ");
            if(!Cute_wasaborted) {
                printf("%d condition%s %s failed.\n",
                        Cute_testfailures,
                        (Cute_testfailures == 1) ? "" : "s",
                        (Cute_testfailures == 1) ? "has" : "have");
            } else {
                printf("Aborted.\n");
            }
        }
        printf("\n");
    } else if(Cute_level_verbose >= 1 && Cute_testfailures == 0) {
        Cute_FinishTestLine(0);
    }

    status = (Cute_testfailures == 0) ? 0 : -1;

    Cute_Uninitialize(test->name);
    Cute_Case(NULL);
    Cute_currenttest = NULL;

    return status;
}

/* Trigger the unit test. If possible (and not suppressed) it starts a child
 * process who calls Cute_RunTest(), otherwise it calls Cute_RunTest()
 * directly. */
static void
Cute_RunUnit(const struct Cute_Test* test, int index, int master_index)
{
    int failed = 1;
    Cute_type_timer start, end;

    Cute_currenttest = test;
    Cute_test_alreadylogged = 0;
    Cute_GetTimerTime(&start);

    if(!Cute_noexec) {

#if defined(CUTE_UNIX)

        pid_t pid;
        int exit_code;

        /* Make sure the child starts with empty I/O buffers. */
        fflush(stdout);
        fflush(stderr);

        pid = fork();
        if(pid == (pid_t)-1) {
            Cute_PrintError("Cannot fork. %s [%d]", strerror(errno), errno);
            failed = 1;
        } else if(pid == 0) {
            /* Child: Do the test. */
            Cute_worker = 1;
            failed = (Cute_RunTest(test, index) != 0);
            Cute_Exit(failed ? 1 : 0);
        } else {
            /* Parent: Wait until child terminates and analyze its exit code. */
            waitpid(pid, &exit_code, 0);
            if(WIFEXITED(exit_code)) {
                switch(WEXITSTATUS(exit_code)) {
                    case 0:   failed = 0; break;   /* test has passed. */
                    case 1:   /* noop */ break;    /* "normal" failure. */
                    default:  Cute_PrintError("Unexpected exit code [%d]", WEXITSTATUS(exit_code));
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
                Cute_PrintError("Test interrupted by %s.", signame);
            } else {
                Cute_PrintError("Test ended in an unexpected way [%d].", exit_code);
            }
        }

#elif defined(CUTE_WINDOWS)

        char buffer[512] = {0};
        STARTUPINFOA startupInfo;
        PROCESS_INFORMATION processInfo;
        DWORD exitCode;

        /* Windows has no fork(). So we propagate all info into the child
         * through a command line arguments. */
        _snprintf(buffer, sizeof(buffer)-1,
                 "%s --worker=%d %s --no-exec --no-summary %s --verbose=%d --color=%s -- \"%s\"",
                 Cute_argv0, index, Cute_timer ? "--time" : "",
                 Cute_tap ? "--tap" : "", Cute_level_verbose,
                 Cute_colorize ? "always" : "never",
                 test->name);
        memset(&startupInfo, 0, sizeof(startupInfo));
        startupInfo.cb = sizeof(STARTUPINFO);
        if(CreateProcessA(NULL, buffer, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            GetExitCodeProcess(processInfo.hProcess, &exitCode);
            CloseHandle(processInfo.hThread);
            CloseHandle(processInfo.hProcess);
            failed = (exitCode != 0);
            if(exitCode > 1) {
                switch(exitCode) {
                    case 3:             Cute_PrintError("Aborted."); break;
                    case 0xC0000005:    Cute_PrintError("Access violation."); break;
                    default:            Cute_PrintError("Test ended in an unexpected way [%lu].", exitCode); break;
                }
            }
        } else {
            Cute_PrintError("Cannot create unit test subprocess [%ld].", GetLastError());
            failed = 1;
        }

#else

        /* A platform where we don't know how to run child process. */
        failed = (Cute_RunTest(test, index) != 0);

#endif

    } else {
        /* Child processes suppressed through --no-exec. */
        failed = (Cute_RunTest(test, index) != 0);
    }
    Cute_GetTimerTime(&end);

    Cute_currenttest = NULL;

    Cute_stat_units_run++;
    if(failed)
        Cute_stat_units_failed++;

    Cute_SetSuccess(master_index, !failed);
    Cute_SetDuration(master_index, Cute_GetTimerDifference(start, end));
}

#if defined(CUTE_WINDOWS)
/* Callback for SEH events. */
static LONG CALLBACK
Cute_CallbackSehException(EXCEPTION_POINTERS *ptrs)
{
    Cute_Check(0, NULL, 0, "Unhandled SEH exception");
    Cute_Message("Exception code:    0x%08lx", ptrs->ExceptionRecord->ExceptionCode);
    Cute_Message("Exception address: 0x%p", ptrs->ExceptionRecord->ExceptionAddress);

    fflush(stdout);
    fflush(stderr);

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif


#define CUTE_CMDLINE_OPTFLAG_OPTIONALARG    0x0001
#define CUTE_CMDLINE_OPTFLAG_REQUIREDARG    0x0002

#define CUTE_CMDLINE_OPTID_NONE             0
#define CUTE_CMDLINE_OPTID_UNKNOWN          (-0x7fffffff + 0)
#define CUTE_CMDLINE_OPTID_MISSINGARG       (-0x7fffffff + 1)
#define CUTE_CMDLINE_OPTID_BOGUSARG         (-0x7fffffff + 2)

typedef struct acutest_test_CMDLINE_OPTION_ {
    char shortname;
    const char* longname;
    int id;
    unsigned flags;
} CUTE_CMDLINE_OPTION;

static int
Cute_HandleCmdlineShortOptGroup(const CUTE_CMDLINE_OPTION* options,
                    const char* arggroup,
                    int (*callback)(int /*optval*/, const char* /*arg*/))
{
    const CUTE_CMDLINE_OPTION* opt;
    int i;
    int ret = 0;

    for(i = 0; arggroup[i] != '\0'; i++) {
        for(opt = options; opt->id != 0; opt++) {
            if(arggroup[i] == opt->shortname)
                break;
        }

        if(opt->id != 0  &&  !(opt->flags & CUTE_CMDLINE_OPTFLAG_REQUIREDARG)) {
            ret = callback(opt->id, NULL);
        } else {
            /* Unknown option. */
            char badoptname[3];
            badoptname[0] = '-';
            badoptname[1] = arggroup[i];
            badoptname[2] = '\0';
            ret = callback((opt->id != 0 ? CUTE_CMDLINE_OPTID_MISSINGARG : CUTE_CMDLINE_OPTID_UNKNOWN),
                            badoptname);
        }

        if(ret != 0)
            break;
    }

    return ret;
}

#define CUTE_CMDLINE_AUXBUF_SIZE  32

static int
Cute_ReadCmdline(const CUTE_CMDLINE_OPTION* options, int argc, char** argv,
                      int (*callback)(int /*optval*/, const char* /*arg*/))
{

    const CUTE_CMDLINE_OPTION* opt;
    char auxbuf[CUTE_CMDLINE_AUXBUF_SIZE+1];
    int after_doubledash = 0;
    int i = 1;
    int ret = 0;

    auxbuf[CUTE_CMDLINE_AUXBUF_SIZE] = '\0';

    while(i < argc) {
        if(after_doubledash  ||  strcmp(argv[i], "-") == 0) {
            /* Non-option argument. */
            ret = callback(CUTE_CMDLINE_OPTID_NONE, argv[i]);
        } else if(strcmp(argv[i], "--") == 0) {
            /* End of options. All the remaining members are non-option arguments. */
            after_doubledash = 1;
        } else if(argv[i][0] != '-') {
            /* Non-option argument. */
            ret = callback(CUTE_CMDLINE_OPTID_NONE, argv[i]);
        } else {
            for(opt = options; opt->id != 0; opt++) {
                if(opt->longname != NULL  &&  strncmp(argv[i], "--", 2) == 0) {
                    size_t len = strlen(opt->longname);
                    if(strncmp(argv[i]+2, opt->longname, len) == 0) {
                        /* Regular long option. */
                        if(argv[i][2+len] == '\0') {
                            /* with no argument provided. */
                            if(!(opt->flags & CUTE_CMDLINE_OPTFLAG_REQUIREDARG))
                                ret = callback(opt->id, NULL);
                            else
                                ret = callback(CUTE_CMDLINE_OPTID_MISSINGARG, argv[i]);
                            break;
                        } else if(argv[i][2+len] == '=') {
                            /* with an argument provided. */
                            if(opt->flags & (CUTE_CMDLINE_OPTFLAG_OPTIONALARG | CUTE_CMDLINE_OPTFLAG_REQUIREDARG)) {
                                ret = callback(opt->id, argv[i]+2+len+1);
                            } else {
                                sprintf(auxbuf, "--%s", opt->longname);
                                ret = callback(CUTE_CMDLINE_OPTID_BOGUSARG, auxbuf);
                            }
                            break;
                        } else {
                            continue;
                        }
                    }
                } else if(opt->shortname != '\0'  &&  argv[i][0] == '-') {
                    if(argv[i][1] == opt->shortname) {
                        /* Regular short option. */
                        if(opt->flags & CUTE_CMDLINE_OPTFLAG_REQUIREDARG) {
                            if(argv[i][2] != '\0')
                                ret = callback(opt->id, argv[i]+2);
                            else if(i+1 < argc)
                                ret = callback(opt->id, argv[++i]);
                            else
                                ret = callback(CUTE_CMDLINE_OPTID_MISSINGARG, argv[i]);
                            break;
                        } else {
                            ret = callback(opt->id, NULL);

                            /* There might be more (argument-less) short options
                             * grouped together. */
                            if(ret == 0  &&  argv[i][2] != '\0')
                                ret = Cute_HandleCmdlineShortOptGroup(options, argv[i]+2, callback);
                            break;
                        }
                    }
                }
            }

            if(opt->id == 0) {  /* still not handled? */
                if(argv[i][0] != '-') {
                    /* Non-option argument. */
                    ret = callback(CUTE_CMDLINE_OPTID_NONE, argv[i]);
                } else {
                    /* Unknown option. */
                    char* badoptname = argv[i];

                    if(strncmp(badoptname, "--", 2) == 0) {
                        /* Strip any argument from the long option. */
                        char* assignment = strchr(badoptname, '=');
                        if(assignment != NULL) {
                            size_t len = assignment - badoptname;
                            if(len > CUTE_CMDLINE_AUXBUF_SIZE)
                                len = CUTE_CMDLINE_AUXBUF_SIZE;
                            strncpy(auxbuf, badoptname, len);
                            auxbuf[len] = '\0';
                            badoptname = auxbuf;
                        }
                    }

                    ret = callback(CUTE_CMDLINE_OPTID_UNKNOWN, badoptname);
                }
            }
        }

        if(ret != 0)
            return ret;
        i++;
    }

    return ret;
}

static void
Cute_PrintHelpMessage(void)
{
    printf("Usage: %s [options] [test...]\n", Cute_argv0);
    printf("\n");
    printf("Run the specified unit tests; or if the option '--skip' is used, run all\n");
    printf("tests in the suite but those listed.  By default, if no tests are specified\n");
    printf("on the command line, all unit tests in the suite are run.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -s, --skip            Execute all unit tests but the listed ones\n");
    printf("      --exec[=WHEN]     If supported, execute unit tests as child processes\n");
    printf("                          (WHEN is one of 'auto', 'always', 'never')\n");
    printf("  -E, --no-exec         Same as --exec=never\n");
#if defined CUTE_WINDOWS
    printf("  -t, --time            Measure test duration\n");
#elif defined CUTE_HAS_POSIX_TIMER
    printf("  -t, --time            Measure test duration (real time)\n");
    printf("      --time=TIMER      Measure test duration, using given timer\n");
    printf("                          (TIMER is one of 'real', 'cpu')\n");
#endif
    printf("      --no-summary      Suppress printing of test results summary\n");
    printf("      --tap             Produce TAP-compliant output\n");
    printf("                          (See https://testanything.org/)\n");
    printf("  -x, --xml-output=FILE Enable XUnit output to the given file\n");
    printf("  -l, --list            List unit tests in the suite and exit\n");
    printf("  -v, --verbose         Make output more verbose\n");
    printf("      --verbose=LEVEL   Set verbose level to LEVEL:\n");
    printf("                          0 ... Be silent\n");
    printf("                          1 ... Output one line per test (and summary)\n");
    printf("                          2 ... As 1 and failed conditions (this is default)\n");
    printf("                          3 ... As 1 and all conditions (and extended summary)\n");
    printf("  -q, --quiet           Same as --verbose=0\n");
    printf("      --color[=WHEN]    Enable colorized output\n");
    printf("                          (WHEN is one of 'auto', 'always', 'never')\n");
    printf("      --no-color        Same as --color=never\n");
    printf("  -h, --help            Display this help and exit\n");

    if(Cute_size_list < 16) {
        printf("\n");
        Cute_PrintNameList();
    }
}

static const CUTE_CMDLINE_OPTION acutest_cmdline_options_[] = {
    { 's',  "skip",         's', 0 },
    {  0,   "exec",         'e', CUTE_CMDLINE_OPTFLAG_OPTIONALARG },
    { 'E',  "no-exec",      'E', 0 },
#if defined CUTE_WINDOWS
    { 't',  "time",         't', 0 },
    {  0,   "timer",        't', 0 },   /* kept for compatibility */
#elif defined CUTE_HAS_POSIX_TIMER
    { 't',  "time",         't', CUTE_CMDLINE_OPTFLAG_OPTIONALARG },
    {  0,   "timer",        't', CUTE_CMDLINE_OPTFLAG_OPTIONALARG },  /* kept for compatibility */
#endif
    {  0,   "no-summary",   'S', 0 },
    {  0,   "tap",          'T', 0 },
    { 'l',  "list",         'l', 0 },
    { 'v',  "verbose",      'v', CUTE_CMDLINE_OPTFLAG_OPTIONALARG },
    { 'q',  "quiet",        'q', 0 },
    {  0,   "color",        'c', CUTE_CMDLINE_OPTFLAG_OPTIONALARG },
    {  0,   "no-color",     'C', 0 },
    { 'h',  "help",         'h', 0 },
    {  0,   "worker",       'w', CUTE_CMDLINE_OPTFLAG_REQUIREDARG },  /* internal */
    { 'x',  "xml-output",   'x', CUTE_CMDLINE_OPTFLAG_REQUIREDARG },
    {  0,   NULL,            0,  0 }
};

static int
Cute_CallbackCmdline(int id, const char* arg)
{
    switch(id) {
        case 's':
            Cute_mode_skip = 1;
            break;

        case 'e':
            if(arg == NULL || strcmp(arg, "always") == 0) {
                Cute_noexec = 0;
            } else if(strcmp(arg, "never") == 0) {
                Cute_noexec = 1;
            } else if(strcmp(arg, "auto") == 0) {
                /*noop*/
            } else {
                fprintf(stderr, "%s: Unrecognized argument '%s' for option --exec.\n", Cute_argv0, arg);
                fprintf(stderr, "Try '%s --help' for more information.\n", Cute_argv0);
                Cute_Exit(2);
            }
            break;

        case 'E':
            Cute_noexec = 1;
            break;

        case 't':
#if defined CUTE_WINDOWS  ||  defined CUTE_HAS_POSIX_TIMER
            if(arg == NULL || strcmp(arg, "real") == 0) {
                Cute_timer = 1;
    #ifndef CUTE_WINDOWS
            } else if(strcmp(arg, "cpu") == 0) {
                Cute_timer = 2;
    #endif
            } else {
                fprintf(stderr, "%s: Unrecognized argument '%s' for option --time.\n", Cute_argv0, arg);
                fprintf(stderr, "Try '%s --help' for more information.\n", Cute_argv0);
                Cute_Exit(2);
            }
#endif
            break;

        case 'S':
            Cute_nosummary = 1;
            break;

        case 'T':
            Cute_tap = 1;
            break;

        case 'l':
            Cute_PrintNameList();
            Cute_Exit(0);
            break;

        case 'v':
            Cute_level_verbose = (arg != NULL ? atoi(arg) : Cute_level_verbose+1);
            break;

        case 'q':
            Cute_level_verbose = 0;
            break;

        case 'c':
            if(arg == NULL || strcmp(arg, "always") == 0) {
                Cute_colorize = 1;
            } else if(strcmp(arg, "never") == 0) {
                Cute_colorize = 0;
            } else if(strcmp(arg, "auto") == 0) {
                /*noop*/
            } else {
                fprintf(stderr, "%s: Unrecognized argument '%s' for option --color.\n", Cute_argv0, arg);
                fprintf(stderr, "Try '%s --help' for more information.\n", Cute_argv0);
                Cute_Exit(2);
            }
            break;

        case 'C':
            Cute_colorize = 0;
            break;

        case 'h':
            Cute_PrintHelpMessage();
            Cute_Exit(0);
            break;

        case 'w':
            Cute_worker = 1;
            Cute_index_worker = atoi(arg);
            break;
        case 'x':
            Cute_output_xml = fopen(arg, "w");
            if (!Cute_output_xml) {
                fprintf(stderr, "Unable to open '%s': %s\n", arg, strerror(errno));
                Cute_Exit(2);
            }
            break;

        case 0:
            if(Cute_Lookup(arg) == 0) {
                fprintf(stderr, "%s: Unrecognized unit test '%s'\n", Cute_argv0, arg);
                fprintf(stderr, "Try '%s --list' for list of unit tests.\n", Cute_argv0);
                Cute_Exit(2);
            }
            break;

        case CUTE_CMDLINE_OPTID_UNKNOWN:
            fprintf(stderr, "Unrecognized command line option '%s'.\n", arg);
            fprintf(stderr, "Try '%s --help' for more information.\n", Cute_argv0);
            Cute_Exit(2);
            break;

        case CUTE_CMDLINE_OPTID_MISSINGARG:
            fprintf(stderr, "The command line option '%s' requires an argument.\n", arg);
            fprintf(stderr, "Try '%s --help' for more information.\n", Cute_argv0);
            Cute_Exit(2);
            break;

        case CUTE_CMDLINE_OPTID_BOGUSARG:
            fprintf(stderr, "The command line option '%s' does not expect an argument.\n", arg);
            fprintf(stderr, "Try '%s --help' for more information.\n", Cute_argv0);
            Cute_Exit(2);
            break;
    }

    return 0;
}


#ifdef CUTE_LINUX
static int
Cute_ConfirmTracerIsPresent(void)
{
    /* Must be large enough so the line 'TracerPid: ${PID}' can fit in. */
    static const int OVERLAP = 32;

    char buf[512];
    int tracer_present = 0;
    int fd;
    size_t n_read = 0;

    fd = open("/proc/self/status", O_RDONLY);
    if(fd == -1)
        return 0;

    while(1) {
        static const char pattern[] = "TracerPid:";
        const char* field;

        while(n_read < sizeof(buf) - 1) {
            ssize_t n;

            n = read(fd, buf + n_read, sizeof(buf) - 1 - n_read);
            if(n <= 0)
                break;
            n_read += n;
        }
        buf[n_read] = '\0';

        field = strstr(buf, pattern);
        if(field != NULL  &&  field < buf + sizeof(buf) - OVERLAP) {
            pid_t tracer_pid = (pid_t) atoi(field + sizeof(pattern) - 1);
            tracer_present = (tracer_pid != 0);
            break;
        }

        if(n_read == sizeof(buf) - 1) {
            /* Move the tail with the potentially incomplete line we're looking
             * for to the beginning of the buffer. */
            memmove(buf, buf + sizeof(buf) - 1 - OVERLAP, OVERLAP);
            n_read = OVERLAP;
        } else {
            break;
        }
    }

    close(fd);
    return tracer_present;
}
#endif

#ifdef CUTE_MAC
static bool
Cute_ConfirmDebug(void)
{
    int junk;
    int mib[4];
    struct kinfo_proc info;
    size_t size;

    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.
    info.kp_proc.p_flag = 0;

    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    // Call sysctl.
    size = sizeof(info);
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
    assert(junk == 0);

    // We're being debugged if the P_TRACED flag is set.
    return ( (info.kp_proc.p_flag & P_TRACED) != 0 );
}
#endif

int
main(int argc, char** argv)
{
    int i;

    Cute_argv0 = argv[0];

#if defined CUTE_UNIX
    Cute_colorize = isatty(STDOUT_FILENO);
#elif defined CUTE_WINDOWS
 #if defined _BORLANDC_
    Cute_colorize = isatty(_fileno(stdout));
 #else
    Cute_colorize = _isatty(_fileno(stdout));
 #endif
#else
    Cute_colorize = 0;
#endif

    /* Count all test units */
    Cute_size_list = 0;
    for(i = 0; Cute_list_test[i].func != NULL; i++)
        Cute_size_list++;

    Cute_data_test = (struct Cute_TestData*)calloc(Cute_size_list, sizeof(struct Cute_TestData));
    if(Cute_data_test == NULL) {
        fprintf(stderr, "Out of memory.\n");
        Cute_Exit(2);
    }

    /* Parse options */
    Cute_ReadCmdline(acutest_cmdline_options_, argc, argv, Cute_CallbackCmdline);

    /* Initialize the proper timer. */
    Cute_InitializeTimer();

#if defined(CUTE_WINDOWS)
    SetUnhandledExceptionFilter(Cute_CallbackSehException);
#ifdef _MSC_VER
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
#endif

    /* By default, we want to run all tests. */
    if(Cute_count == 0) {
        for(i = 0; Cute_list_test[i].func != NULL; i++)
            Cute_Remember(i);
    }

    /* Guess whether we want to run unit tests as child processes. */
    if(Cute_noexec < 0) {
        Cute_noexec = 0;

        if(Cute_count <= 1) {
            Cute_noexec = 1;
        } else {
#ifdef CUTE_WINDOWS
            if(IsDebuggerPresent())
                Cute_noexec = 1;
#endif
#ifdef CUTE_LINUX
            if(Cute_ConfirmTracerIsPresent())
                Cute_noexec = 1;
#endif
#ifdef CUTE_MAC
            if(Cute_ConfirmDebug())
                Cute_noexec = 1;
#endif
#ifdef RUNNING_ON_VALGRIND
            /* RUNNING_ON_VALGRIND is provided by optionally included <valgrind.h> */
            if(RUNNING_ON_VALGRIND)
                Cute_noexec = 1;
#endif
        }
    }

    if(Cute_tap) {
        /* TAP requires we know test result ("ok", "not ok") before we output
         * anything about the test, and this gets problematic for larger verbose
         * levels. */
        if(Cute_level_verbose > 2)
            Cute_level_verbose = 2;

        /* TAP harness should provide some summary. */
        Cute_nosummary = 1;

        if(!Cute_worker)
            printf("1..%d\n", (int) Cute_count);
    }

    int index = Cute_index_worker;
    for(i = 0; Cute_list_test[i].func != NULL; i++) {
        int run = (Cute_data_test[i].flags & CUTE_FLAG_RUN);
        if (Cute_mode_skip) /* Run all tests except those listed. */
            run = !run;
        if(run)
            Cute_RunUnit(&Cute_list_test[i], index++, i);
    }

    /* Write a summary */
    if(!Cute_nosummary && Cute_level_verbose >= 1) {
        if(Cute_level_verbose >= 3) {
            Cute_PrintColored(CUTE_COLOR_DEFAULT_INTENSIVE, "Summary:\n");

            printf("  Count of all unit tests:     %4d\n", (int) Cute_size_list);
            printf("  Count of run unit tests:     %4d\n", Cute_stat_units_run);
            printf("  Count of failed unit tests:  %4d\n", Cute_stat_units_failed);
            printf("  Count of skipped unit tests: %4d\n", (int) Cute_size_list - Cute_stat_units_run);
        }

        if(Cute_stat_units_failed == 0) {
            Cute_PrintColored(CUTE_COLOR_GREEN_INTENSIVE, "SUCCESS:");
            printf(" All unit tests have passed.\n");
        } else {
            Cute_PrintColored(CUTE_COLOR_RED_INTENSIVE, "FAILED:");
            printf(" %d of %d unit tests %s failed.\n",
                    Cute_stat_units_failed, Cute_stat_units_run,
                    (Cute_stat_units_failed == 1) ? "has" : "have");
        }

        if(Cute_level_verbose >= 3)
            printf("\n");
    }

    if (Cute_output_xml) {
#if defined CUTE_UNIX
        char *suite_name = basename(argv[0]);
#elif defined CUTE_WINDOWS
        char suite_name[_MAX_FNAME];
        _splitpath(argv[0], NULL, NULL, suite_name, NULL);
#else
        const char *suite_name = argv[0];
#endif
        fprintf(Cute_output_xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(Cute_output_xml, "<testsuite name=\"%s\" tests=\"%d\" errors=\"%d\" failures=\"%d\" skip=\"%d\">\n",
            suite_name, (int)Cute_size_list, Cute_stat_units_failed, Cute_stat_units_failed,
            (int)Cute_size_list - Cute_stat_units_run);
        for(i = 0; Cute_list_test[i].func != NULL; i++) {
            struct Cute_TestData *details = &Cute_data_test[i];
            fprintf(Cute_output_xml, "  <testcase name=\"%s\" time=\"%.2f\">\n", Cute_list_test[i].name, details->duration);
            if (details->flags & CUTE_FLAG_FAILURE)
                fprintf(Cute_output_xml, "    <failure />\n");
            if (!(details->flags & CUTE_FLAG_FAILURE) && !(details->flags & CUTE_FLAG_SUCCESS))
                fprintf(Cute_output_xml, "    <skipped />\n");
            fprintf(Cute_output_xml, "  </testcase>\n");
        }
        fprintf(Cute_output_xml, "</testsuite>\n");
        fclose(Cute_output_xml);
    }

    Cute_Cleanup();

    return (Cute_stat_units_failed == 0) ? 0 : 1;
}


#endif  // #ifndef CUTE_NO_MAIN

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#ifdef __cplusplus
    }  // extern "C"
#endif

#endif  // #ifndef H_CUTE
