#ifndef __OS_DEP_H__
#define __OS_DEP_H__

#ifdef __linux
#include <linux/limits.h>
#elif __APPLE__
#include <limits.h>
#else
#error "OS not yet supported"
#endif

#endif
