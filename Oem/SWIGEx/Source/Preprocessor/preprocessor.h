/* -----------------------------------------------------------------------------
 * preprocessor.h
 *
 *     SWIG preprocessor module.
 *
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1999-2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.
 *
 * $Header: /cvsroot/SWIG/Source/Preprocessor/preprocessor.h,v 1.7 2002/11/30 22:10:13 beazley Exp $
 * ----------------------------------------------------------------------------- */

#ifndef _PREPROCESSOR_H
#define _PREPROCESSOR_H

#include "swig.h"
#include "swigwarn.h"

#ifdef __cplusplus
extern "C" {
#endif
extern int     Preprocessor_expr(String *s, int *error);
extern char   *Preprocessor_expr_error(void);
extern Hash   *Preprocessor_define(const String_or_char *str, int swigmacro);
extern void    Preprocessor_undef(String_or_char *name);
extern void    Preprocessor_init();
extern String *Preprocessor_parse(String *s);
extern void    Preprocessor_include_all(int);
extern void    Preprocessor_import_all(int);
extern void    Preprocessor_ignore_missing(int);
extern List   *Preprocessor_depend(void);

#ifdef __cplusplus
}
#endif

#endif




