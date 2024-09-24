
#ifndef MIN_UNIT_H
#define MIN_UNIT_H

#define mu_assert(message, test) \
    do {                         \
        if (!(test))             \
            return message;      \
    } while (0)

#define mu_run_test(test)                                               \
    do {                                                                \
        char* message = test();                                         \
        tests_run++;                                                    \
        if (message) {                                                  \
            printf("error in test %d: %s \n ", tests_run + 1, message); \
            return message;                                             \
        }                                                               \
    } while (0)

extern int tests_run;

#endif
