/* config.h.  Generated from config.h.in by configure.  */
/*
 * "$Id: config.h.in 387 2009-04-18 17:05:52Z mike $"
 *
 * Configuration file for Mini-XML, a small XML-like file parsing library.
 *
 * Copyright 2003-2009 by Michael Sweet.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * Include necessary headers...
 */

#include <common.h>
#include <vsprintf.h>
#include <stdarg.h>
#include "misc.h"


#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef EOF
#define  EOF     (-1)
#endif


extern void * mxml_malloc(size_t rq_size);
extern void * mxml_calloc(uint32_t n_elements, size_t elem_size);
extern void * mxml_realloc(void *p_old, size_t new_size);
extern void mxml_mfree(void *pbuf);


/*
 * Version number...
 */

#define MXML_VERSION "Mini-XML v2.6"


/*
 * Long long support...
 */

//#define HAVE_LONG_LONG 1


/*
 * Do we have the snprintf() and vsnprintf() functions?
 */

#define HAVE_SNPRINTF       1
#define HAVE_VSNPRINTF      CONFIG_SYS_VSNPRINTF


/*
 * Do we have the strXXX() functions?
 */
#undef HAVE_STRDUP
//#define HAVE_STRDUP         1  /* to use the internal mxml_malloc, DO NOT enable this macro. */


/*
 * Do we have threading support?
 */
#undef DEBUG
//#define DEBUG 2

#undef HAVE_PTHREAD_H
//#define HAVE_PTHREAD_H 1

/*
 * Define prototypes for string functions as needed...
 */

#ifndef HAVE_STRDUP
extern char	*_mxml_strdup(const char *);
#define strdup _mxml_strdup
#endif /* !HAVE_STRDUP */

extern char	*_mxml_strdupf(const char *, ...);
extern char	*_mxml_vstrdupf(const char *, va_list);

#ifndef HAVE_SNPRINTF
extern int	_mxml_snprintf(char *, size_t, const char *, ...);
#define snprintf _mxml_snprintf
#endif /* !HAVE_SNPRINTF */

#ifndef HAVE_VSNPRINTF
extern int	_mxml_vsnprintf(char *, size_t, const char *, va_list);
#define vsnprintf _mxml_vsnprintf
#endif /* !HAVE_VSNPRINTF */

/*
 * End of "$Id: config.h.in 387 2009-04-18 17:05:52Z mike $".
 */
