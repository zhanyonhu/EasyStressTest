/* Copyright zhanyonhu@163.com, Inc. and other Node contributors. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#ifndef _COMMONDEF_H_
#define _COMMONDEF_H_

extern "C"
{
#include "libtcc.h"
};

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

//JEMALLOC
#ifdef _USE_JEMALLOC
#undef malloc
#undef free
#define malloc je_malloc
#define free je_free
#include "jemalloc.h"
#endif	/*_USE_JEMALLOC*/

//EASTL
#define _USE_EASTL
#ifdef _USE_EASTL

#ifdef _DEBUG
#define EASTL_DLL	0
#else	//_DEBUG
#define EASTL_DLL	1
#define EASTL_API
#define EASTL_TEMPLATE_API
#endif	//_DEBUG

#define STL				eastl
#include "EASTL/fixed_hash_map.h"
#include "EASTL/hash_map.h"
#include "EASTL/fixed_list.h"
#include "EASTL/string.h"
typedef eastl_size_t	stl_size_t;

#else	//_USE_EASTL

typedef size_t			stl_size_t;

#endif // _USE_EASTL

#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <process.h>
#if !defined(__MINGW32__)
# include <crtdbg.h>
#endif
#else /*WIN32*/
#include <stdint.h> /* uintptr_t */

#include <errno.h>
#include <unistd.h> /* usleep */
#include <string.h> /* strdup */
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <assert.h>

#include <sys/select.h>
#include <pthread.h>
#endif /*WIN32*/

#include "uv.h"
#include "define.h"

#if defined(_MSC_VER) && _MSC_VER < 1600
# include "stdint-msvc2008.h"
#else
# include <stdint.h>
#endif

#if !defined(_WIN32)
# include <sys/time.h>
# include <sys/resource.h>  /* setrlimit() */
#endif

#ifdef _WIN32
# include <io.h>
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr)-offsetof(type, member)))

#define LOG(...)                        \
do {\
\
	fprintf(stderr, "%s", __VA_ARGS__); \
	fflush(stderr);                     \
} while (0)

//#define _USE_LOGF_TASK_ERR
#ifdef _USE_LOGF_TASK_ERR
#define LOGF_TASK_ERR		LOGF
#else
#define LOGF_TASK_ERR		__noop
#endif // _DEBUG


#define LOGF(...)                       \
do {\
\
	fprintf(stderr, __VA_ARGS__);       \
	fflush(stderr);                     \
} while (0)

#define FATAL(msg)                                        \
do {\
\
	fprintf(stderr, \
	"Fatal error in %s on line %d: %s\n", \
	__FILE__, \
	__LINE__, \
	msg);                                         \
	fflush(stderr);                                       \
	getchar();                                              \
	abort();                                              \
} while (0)

#ifdef _DEBUG
#define ASSERT(expr)                                      \
do {\
\
if (!(expr)) {\
	\
	fprintf(stderr, \
	"Assertion failed in %s on line %d: %s\n", \
	__FILE__, \
	__LINE__, \
#expr);                                       \
	getchar();                                              \
	abort();                                              \
}                                                       \
} while (0)
#else	/* _DEBUG*/
#define ASSERT(expr)   
#endif	/* _DEBUG*/

#ifdef _DEBUG
#define VERIFY(expr)		ASSERT(expr)
#else	/* _DEBUG*/
#define VERIFY(expr)		expr
#endif	/* _DEBUG*/

/* This macro cleans up the main loop. This is used to avoid valgrind
* warnings about memory being "leaked" by the main event loop.
*/
#define MAKE_VALGRIND_HAPPY()           \
do {\
\
	close_loop(uv_default_loop());      \
	uv_loop_delete(uv_default_loop());  \
} while (0)

#if !defined(_WIN32)

# define SET_FILE_LIMIT(num)                                                 \
	do {\
	\
	struct rlimit lim;                                                      \
	lim.rlim_cur = (num);                                                   \
	lim.rlim_max = lim.rlim_cur;                                            \
	if (setrlimit(RLIMIT_NOFILE, &lim))                                     \
	RETURN_SKIP("File descriptor limit too low.");                        \
	} while (0)

#else  /* defined(_WIN32) */

# define SET_FILE_LIMIT(num) do {} while (0)

#endif


#if defined _WIN32 && ! defined __GNUC__

#include <stdarg.h>

	/* Emulate snprintf() on Windows, _snprintf() doesn't zero-terminate the buffer
	* on overflow...
	*/
	static int snprintf(char* buf, size_t len, const char* fmt, ...) {
		va_list ap;
		int n;

		va_start(ap, fmt);
		n = _vsprintf_p(buf, len, fmt, ap);
		va_end(ap);

		/* It's a sad fact of life that no one ever checks the return value of
		* snprintf(). Zero-terminating the buffer hopefully reduces the risk
		* of gaping security holes.
		*/
		if (n < 0)
		if (len > 0)
			buf[0] = '\0';

		return n;
	}

#endif

#if defined(__clang__) ||                                \
	defined(__GNUC__) || \
	defined(__INTEL_COMPILER) || \
	defined(__SUNPRO_C)
# define UNUSED __attribute__((unused))
#else
# define UNUSED
#endif

	/* Fully close a loop */
	static void close_walk_cb(uv_handle_t* handle, void* arg) {
		if (!uv_is_closing(handle))
			uv_close(handle, NULL);
	}

	UNUSED static void close_loop(uv_loop_t* loop) {
		uv_walk(loop, close_walk_cb, NULL);
		uv_run(loop, UV_RUN_DEFAULT);
	}


#endif /* _COMMONDEF_H_ */
