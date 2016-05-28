#ifndef __LZ78_H__
#define __LZ78_H__

#include <stdio.h>

#define lz78_trace(format, ...) \
            printf("lz78: "format"\n", ##__VA_ARGS__)

#endif /* __LZ78_H__ */
