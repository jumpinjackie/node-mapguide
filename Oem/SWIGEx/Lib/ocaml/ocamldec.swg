/* -*-c-*-
 * -----------------------------------------------------------------------
 * ocaml/ocamldec.swg
 * Copyright (C) 2000, 2001 Matthias Koeppe
 *
 * Ocaml runtime code -- declarations
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
#define SWIGEXT extern "C"
SWIGEXT {
#else
#define SWIGEXT 
#endif
#define value caml_value_t
#define CAML_VALUE caml_value_t
#define alloc caml_alloc
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/misc.h>
#undef alloc

// Adapted from memory.h and mlvalues.h

#define SWIG_CAMLlocal1(x) \
  caml_value_t x = 0; \
  CAMLxparam1 (x)

#define SWIG_CAMLlocal2(x, y) \
  caml_value_t x = 0, y = 0; \
  CAMLxparam2 (x, y)

#define SWIG_CAMLlocal3(x, y, z) \
  caml_value_t x = 0, y = 0, z = 0; \
  CAMLxparam3 (x, y, z)

#define SWIG_CAMLlocal4(x, y, z, t) \
  caml_value_t x = 0, y = 0, z = 0, t = 0; \
  CAMLxparam4 (x, y, z, t)

#define SWIG_CAMLlocal5(x, y, z, t, u) \
  caml_value_t x = 0, y = 0, z = 0, t = 0, u = 0; \
  CAMLxparam5 (x, y, z, t, u)

#define SWIG_CAMLlocalN(x, size) \
  caml_value_t x [(size)] = { 0, /* 0, 0, ... */ }; \
  CAMLxparamN (x, (size))

#define SWIG_Field(x, i) (((caml_value_t *)(x)) [i])           /* Also an l-value. */
#define SWIG_Store_field(block, offset, val) do{ \
  mlsize_t caml__temp_offset = (offset); \
  caml_value_t caml__temp_val = (val); \
  modify (&SWIG_Field ((block), caml__temp_offset), caml__temp_val); \
}while(0)

#define SWIG_Data_custom_val(v) ((void *) &SWIG_Field((v), 1))
#ifdef ARCH_BIG_ENDIAN
#define SWIG_Tag_val(val) (((unsigned char *) (val)) [-1])
                                                 /* Also an l-value. */
#define SWIG_Tag_hp(hp) (((unsigned char *) (hp)) [sizeof(caml_value_t)-1])
                                                 /* Also an l-value. */
#else
#define SWIG_Tag_val(val) (((unsigned char *) (val)) [-sizeof(caml_value_t)])
                                                 /* Also an l-value. */
#define SWIG_Tag_hp(hp) (((unsigned char *) (hp)) [0])
                                                 /* Also an l-value. */
#endif

#ifndef ARCH_ALIGN_INT64
#define SWIG_Int64_val(v) (*((int64 *) SWIG_Data_custom_val(v)))
#else
CAMLextern int64 Int64_val(caml_value_t v);
#define SWIG_Int64_val(v) Int64_val(v)
#endif

#if defined(SWIG_NOINCLUDE)
#	define SWIGSTATIC
#elif defined(SWIG_GLOBAL)
#	define SWIGSTATIC
#else
#	define SWIGSTATIC static
#endif

#define SWIG_NewPointerObj(p,type,flags) caml_val_ptr(p,type)

#define SWIG_contract_assert(expr, msg) if(!(expr)) {failwith(msg);} else

    SWIGSTATIC int
    SWIG_GetPtr(void *source, void **result, swig_type_info *type, swig_type_info *result_type);
    
    SWIGSTATIC void *
    SWIG_MustGetPtr (CAML_VALUE v,  swig_type_info *type);
    
    static CAML_VALUE _wrap_delete_void( CAML_VALUE );
    
    static int enum_to_int( char *name, CAML_VALUE v );
    static CAML_VALUE int_to_enum( char *name, int v );

    static CAML_VALUE caml_list_nth( CAML_VALUE lst, int n );
    static CAML_VALUE caml_list_append( CAML_VALUE lst, CAML_VALUE elt );
    static int caml_list_length( CAML_VALUE lst );
    static CAML_VALUE caml_array_new( int n );
    static void caml_array_set( CAML_VALUE arr, int n, CAML_VALUE item );
    static CAML_VALUE caml_array_nth( CAML_VALUE arr, int n );
    static int caml_array_length( CAML_VALUE arr );

    static CAML_VALUE caml_val_char( char c );
    static CAML_VALUE caml_val_uchar( unsigned char c );

    static CAML_VALUE caml_val_short( short s );
    static CAML_VALUE caml_val_ushort( unsigned short s );
    
    static CAML_VALUE caml_val_int( int x );
    static CAML_VALUE caml_val_uint( unsigned int x );

    static CAML_VALUE caml_val_long( long x );
    static CAML_VALUE caml_val_ulong( unsigned long x );

    static CAML_VALUE caml_val_float( float f );
    static CAML_VALUE caml_val_double( double d );

    static CAML_VALUE caml_val_ptr( void *p, swig_type_info *descriptor );

    static CAML_VALUE caml_val_string( const char *str );
    static CAML_VALUE caml_val_string_len( const char *str, int len );

    static long caml_long_val( CAML_VALUE v );
    static double caml_double_val( CAML_VALUE v );

    static int caml_ptr_val_internal( CAML_VALUE v, void **out,
				      swig_type_info *descriptor );
    static void *caml_ptr_val( CAML_VALUE v, swig_type_info *descriptor );

    static char *caml_string_val( CAML_VALUE v );
    static int caml_string_len( CAML_VALUE v );

#ifdef __cplusplus
}
#endif

/* mzschemedec.swg ends here */
