#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

extern int verbose;
extern int debug;

#define dbg_prnt_err(fmt, ...)                                                                      \
    do                                                                                              \
    {                                                                                               \
        if (verbose)                                                                                \
            fprintf(stderr, "{%d} [%s:%d] " fmt "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define dbg_prnt_inf(fmt, ...)                                                                      \
    do                                                                                              \
    {                                                                                               \
        if (verbose)                                                                                \
            fprintf(stderr, "{%d} [%s:%d] " fmt "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define dbg_prnt_dbg(fmt, ...)                                                                      \
    do                                                                                              \
    {                                                                                               \
        if (debug)                                                                                  \
            fprintf(stderr, "{%d} [%s:%d] " fmt "\n", getpid(), __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#endif /* _DEBUG_H_ */
