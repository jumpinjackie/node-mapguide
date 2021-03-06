/*
 * php5.swg
 *
 * PHP5 runtime library
 *
 */

#ifdef __cplusplus
extern "C" {
#endif
#include "zend.h"
#include "zend_API.h"
#include "php.h"
#include "zend_exceptions.h"

#ifdef _WIN32
#include <typeinfo.h>
#else
#include <typeinfo>
#endif

/* These TSRMLS_ stuff should already be defined now, but with older php under
   redhat are not... */
#ifndef TSRMLS_D
#define TSRMLS_D
#endif
#ifndef TSRMLS_DC
#define TSRMLS_DC
#endif
#ifndef TSRMLS_C
#define TSRMLS_C
#endif
#ifndef TSRMLS_CC
#define TSRMLS_CC
#endif

#ifdef __cplusplus
}
#endif

/* used to wrap returned objects in so we know whether they are newobject
   and need freeing, or not */
typedef struct _swig_object_wrapper {
  void * ptr;
  int newobject;
} swig_object_wrapper;

/* local scope self_constructors are set to 1 inside function wrappers
   which are also class constructors, so that the php5.swg output typemaps
   know whether or not to wrap returned objects in this_ptr or a new object */
int self_constructor=0;

/* empty zend destructor for types without one */
static ZEND_RSRC_DTOR_FUNC(SWIG_landfill) {};

/* This one makes old swig style string pointers but the php module doesn't
   use these any more.  This is just left here for old times sake and may go */
SWIGRUNTIME(void)
SWIG_MakePtr(char *c, void *ptr, swig_type_info *ty) {
  static char hex[17] = "0123456789abcdef";
  unsigned long p, s;
  char data[32], *r;

  r = data;
  p = (unsigned long) ptr;
  if (p > 0) {
    while (p > 0) {
      s = p & 0xf;
      *(r++) = hex[s];
      p = p >> 4;
    }
    *r = '_';
    while (r >= data) {
      *(c++) = *(r--);
    }
    strcpy (c, ty->name);
  } else {
    strcpy (c, "NULL");
  }
}

SWIGRUNTIME(void)
SWIG_SetPointerChar(char **c, void *ptr, swig_type_info *type) {
   char data[512];

   SWIG_MakePtr(data, ptr, type);
   *c = estrdup(data);
}

#define SWIG_SetPointerZval(a,b,c,d) SWIG_ZTS_SetPointerZval(a,b,c,d, SWIG_module_entry TSRMLS_CC)

SWIGRUNTIME(void)
SWIG_ZTS_SetPointerZval(zval *z, void *ptr, swig_type_info *type, int newobject, zend_module_entry* module_entry TSRMLS_DC) {
  swig_object_wrapper *value=NULL;
  /* No need to call SWIG_MakePtr here! */
  if (type->clientdata) {
    if (! (*(int *)(type->clientdata))) zend_error(E_ERROR, "Type: %s failed to register with zend",type->name);
    value=(swig_object_wrapper *)emalloc(sizeof(swig_object_wrapper));
    value->ptr=ptr;
    value->newobject=newobject;
    ZEND_REGISTER_RESOURCE(z, value, *(int *)(type->clientdata));
    return;
  } else { /* have to deal with old fashioned string pointer?
              but this should not get this far */
    zend_error(E_ERROR, "Type: %s not registered with zend",type->name);
  }
}

/* This old-style routine converts an old string-pointer c into a real pointer
   ptr calling making appropriate casting functions according to ty
   We don't use this any more */
SWIGRUNTIME(int)
SWIG_ConvertPtr_(char *c, void **ptr, swig_type_info *ty) {
   register int d;
   unsigned long p;
   swig_type_info *tc;

   if(c == NULL) {
   	*ptr = 0;
	return 0;
   }

   p = 0;
   if (*c != '_') {
    *ptr = (void *) 0;
    if (strcmp(c,"NULL") == 0) {
	return 0;
    } else {
	goto type_error;
    }
  }

    c++;
    /* Extract hex value from pointer */
    while ((d = *c)) {
      if ((d >= '0') && (d <= '9'))
        p = (p << 4) + (d - '0');
      else if ((d >= 'a') && (d <= 'f'))
        p = (p << 4) + (d - ('a'-10));
      else
        break;
      c++;
    }
    *ptr = (void *) p;
	
    if(ty) {
	tc = SWIG_TypeCheck(c,ty);
	if(!tc) goto type_error;
	*ptr = SWIG_TypeCast(tc, (void*)p);
    }
    return 0;

type_error:

    return -1;
}

/* This is a new pointer conversion routine
   Taking the native pointer p (which would have been converted from the old
   string pointer) and it's php type id, and it's type name (which also would
   have come from the old string pointer) it converts it to ptr calling 
   appropriate casting functions according to ty
   Sadly PHP has no API to find a type name from a type id, only from an instance
   of a resource of the type id, so we have to pass type_name as well.
   The two functions which might call this are:
   SWIG_ZTS_ConvertResourcePtr which gets the type name from the resource
   and the registered zend destructors for which we have one per type each
   with the type name hard wired in. */
SWIGRUNTIME(int)
SWIG_ZTS_ConvertResourceData(void * p, int type, const char *type_name, void **ptr, swig_type_info *ty TSRMLS_DC) {
  swig_type_info *tc;

  if (ty) {
    if (! type_name) {  
      /* can't convert p to ptr type ty if we don't know what type p is */
      return -1;
    } else {
      /* convert and cast p from type_name to ptr as ty
         Need to sort out const-ness, can SWIG_TypeCast really not take a const? */
      tc = SWIG_TypeCheck((char *)type_name,ty);
      if (!tc) return -1;
      *ptr = SWIG_TypeCast(tc, (void*)p);
    }
  } else {
    /* They don't care about the target type, so just pass on the pointer! */
    *ptr = (void *) p;
  }
  return 0;
}

/* This function fills ptr with a pointer of type ty by extracting the pointer
   and type info from the resource in z.  z must be a resource
   It uses SWIG_ZTS_ConvertResourceData to do the real work. */
SWIGRUNTIME(int)
SWIG_ZTS_ConvertResourcePtr(zval *z, void **ptr, swig_type_info *ty TSRMLS_DC) {
  swig_object_wrapper *value;
  void *p;
  int type;
  char *type_name;

  value = (swig_object_wrapper *) zend_list_find(z->value.lval,&type);
  p = value->ptr;
  if (type==-1) return -1;

  type_name=zend_rsrc_list_get_rsrc_type(z->value.lval TSRMLS_CC);

  return SWIG_ZTS_ConvertResourceData(p,type,type_name,ptr,ty TSRMLS_CC);
}

/* But in fact SWIG_ConvertPtr is the native interface for getting typed
   pointer values out of zvals.  We need the TSRMLS_ macros for when we
   make PHP type calls later as we handle php resources */
#define SWIG_ConvertPtr(a,b,c) SWIG_ZTS_ConvertPtr(a,b,c TSRMLS_CC)

/* We allow passing of a STRING or RESOURCE pointing to the object
   or an OBJECT whose _cPtr is a string or resource pointing to the object
   STRING pointers are very depracated */
SWIGRUNTIME(int)
SWIG_ZTS_ConvertPtr(zval *z, void **ptr, swig_type_info *ty TSRMLS_DC) {
   char *c;
   zval *val;
   
   if(z == NULL || z->type == IS_NULL) {
	*ptr = 0;
	return 0;
   }

   if (z->type==IS_OBJECT) {
     zval ** _cPtr;
     if (zend_hash_find(HASH_OF(z),"_cPtr",sizeof("_cPtr"),(void**)&_cPtr)==SUCCESS) {
       /* Don't co-erce to string if it isn't */
       if ((*_cPtr)->type==IS_STRING) c = Z_STRVAL_PP(_cPtr);
       else if ((*_cPtr)->type==IS_RESOURCE) {
         return SWIG_ZTS_ConvertResourcePtr(*_cPtr,ptr,ty TSRMLS_CC);
       } else goto type_error; /* _cPtr was not string or resource property */
     } else goto type_error; /* can't find property _cPtr */
   } else if (z->type==IS_RESOURCE) {
     return SWIG_ZTS_ConvertResourcePtr(z,ptr,ty TSRMLS_CC);
   } else if (z->type==IS_STRING) {
     c = Z_STRVAL_P(z); 
     return SWIG_ConvertPtr_(c,ptr,ty);
   } else goto type_error;

type_error:

    return -1;
}

void aw_convert_to_long(zval **op) 
{
	if((*op)->type != IS_STRING)
	{
	    if((*op)->type == IS_OBJECT)
            zend_error(E_ERROR, "No suitable conversion from object to integer value");
        else if((*op)->type == IS_NULL)
            zend_error(E_ERROR, "No suitable conversion from null reference to integer value");
	    convert_to_long_ex(op);
	    return;
	}
	char* strval = (*op)->value.str.val;
	char* endptr;
	(*op)->type = IS_LONG;
    (*op)->value.lval = strtol(strval, &endptr, 10);
    if(*endptr)
        zend_error(E_ERROR, "'%s' is an invalid integer value", strval);
    
	STR_FREE(strval);
}

void aw_convert_to_double(zval **op) 
{
	if((*op)->type != IS_STRING)
	{
	    if((*op)->type == IS_OBJECT)
            zend_error(E_ERROR, "No suitable conversion from object to floating point value");
        else if((*op)->type == IS_NULL)
            zend_error(E_ERROR, "No suitable conversion from null reference to floating point value");
	    convert_to_double_ex(op);
	    return;
	}
	char* strval = (*op)->value.str.val;
	char* endptr;
    (*op)->value.dval = strtod(strval, &endptr);
    (*op)->type = IS_DOUBLE;
    if(*endptr)
        zend_error(E_ERROR, "'%s' is an invalid floating point value", strval);

	STR_FREE(strval);
}
