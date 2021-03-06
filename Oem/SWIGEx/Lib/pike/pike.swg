/* -----------------------------------------------------------------------------
 * pike.swg
 *
 * Pike configuration module.
 * ----------------------------------------------------------------------------- */

%insert(runtime) "precommon.swg"
%insert(runtime) "common.swg";          // Common type-checking code
%insert(runtime) "pikerun.swg";         // Pike run-time code

%insert(runtime) %{
#ifdef __cplusplus
extern "C" {
#endif
#include "global.h"
#include "module.h"
#include "interpret.h"
#ifdef __cplusplus
}
#endif
%}

/* -----------------------------------------------------------------------------
 *                          standard typemaps
 * ----------------------------------------------------------------------------- */

/* --- Input arguments --- */

/* Primitive datatypes. */

%typemap(in, pikedesc="tInt")
    int, unsigned int, short, unsigned short,
    long, unsigned long, char, signed char, unsigned char,
    bool, enum SWIGTYPE, long long, unsigned long long
{
    if ($input.type != T_INT)
        Pike_error("Bad argument: Expected an integer.\n");
    $1 = ($1_ltype) $input.u.integer;
}

%typemap(in, pikedesc="tFloat") float, double {
    if ($input.type != T_FLOAT)
        Pike_error("Bad argument: Expected a float.\n");
    $1 = ($1_ltype) $input.u.float_number;
}

%typemap(in, pikedesc="tStr") char *, char [ANY] {
    if ($input.type != T_STRING)
        Pike_error("Bad argument: Expected a string.\n");
    $1 = ($1_ltype) STR0($input.u.string);
}

/* Pointers, references and arrays */

%typemap(in) SWIGTYPE *,
             SWIGTYPE &,
             SWIGTYPE []
	"SWIG_ConvertPtr($input.u.object, (void **) &$1, $1_descriptor, 1);"
	
/* Void pointer.  Accepts any kind of pointer */
%typemap(in) void * "/* FIXME */";

/* Object passed by value. Convert to a pointer */
%typemap(in) SWIGTYPE ($&1_ltype argp) "/* FIXME */";

/* Pointer to a class member */
%typemap(in) SWIGTYPE (CLASS::*) "/* FIXME */";

/* Const primitive references.  Passed by value */

%typemap(in, pikedesc="tInt") const int & (int temp),
	     const short & (short temp),
             const long  & (long temp),
             const unsigned int & (unsigned int temp),
             const unsigned short & (unsigned short temp),
             const unsigned long & (unsigned long temp),
	     const char & (char temp),
             const signed char & (signed char temp),
             const unsigned char & (unsigned char temp),
             const bool & (bool temp),
	     const long long & ($*1_ltype temp),
	     const unsigned long long & ($*1_ltype temp)
{
  if ($input.type != T_INT)
    Pike_error("Bad argument: Expected an integer.\n");
    temp = ($*1_ltype) $input.u.integer;
    $1 = &temp;
}

%typemap(in, pikedesc="tFloat") const float & (float temp),
	     const double & (double temp)
{
  if ($input.type != T_FLOAT)
    Pike_error("Bad argument: Expected a float.\n");
    temp = ($*1_ltype) $input.u.float_number;
    $1 = &temp;
}

/************************ Output Typemaps *****************************/

%typemap(out, pikedesc="tInt")
    int, unsigned int,
    short, unsigned short,
    long, unsigned long,
    char, signed char, unsigned char, 
    bool, enum SWIGTYPE
	"push_int($1);";

%typemap(out, pikedesc="tInt") long long	"push_int64($1);";
%typemap(out, pikedesc="tInt") unsigned long long	"push_int64($1);";
%typemap(out, pikedesc="tFloat") float, double	"push_float($1);";
%typemap(out, pikedesc="tStr") char *		"push_text($1);";

/* Pointers, references, and arrays */
%typemap(out, pikedesc="tObj") SWIGTYPE*, SWIGTYPE &, SWIGTYPE [] "push_object(SWIG_NewPointerObj((void *) $1, $1_descriptor, $owner));";

/* Void return value; don't push anything */
%typemap(out, pikedesc="tVoid") void		"";

/* Dynamic casts */

%typemap(out) SWIGTYPE *DYNAMIC, SWIGTYPE &DYNAMIC "/* FIXME */";

/* Member pointer */
%typemap(out) SWIGTYPE (CLASS::*) "/* FIXME */";

/* Special typemap for character array return values */
%typemap(out, pikedesc="tStr") char [ANY], const char [ANY] "push_text($1);";

/* Primitive types--return by value */
%typemap(out, "tObj") SWIGTYPE 
#ifdef __cplusplus
{
  $&1_ltype resultptr;
  resultptr = new $1_ltype(($1_ltype &) $1);
  push_object(SWIG_NewPointerObj((void *) resultptr, $&1_descriptor, 1));
}
#else
{
  $&1_ltype resultptr;
  resultptr = ($&1_ltype) malloc(sizeof($1_type));
  memmove(resultptr, &$1, sizeof($1_type));
  push_object(SWIG_NewPointerObj((void *) resultptr, $&1_descriptor, 1));
}
#endif

/* References to primitive types.  Return by value */

%typemap(out, pikedesc="tInt") const int &, const unsigned int &,
              const short &, const unsigned short &,
              const long &, const unsigned long &,
              const char &, const signed char &, const unsigned char &,
              const bool &,
	      const long long &, const unsigned long long &
      "push_int(*($1));";

%typemap(out, pikedesc="tFloat") const float &, const double &  "push_float(*($1));";

/************************ Constant Typemaps *****************************/

%typemap(constant)
    int, unsigned int,
    short, unsigned short,
    long, unsigned long,
    signed char, unsigned char, 
    bool, enum SWIGTYPE,
    long long, unsigned long long
    	"add_integer_constant(\"$symname\", $1, 0);";

%typemap(constant) char
	"add_integer_constant(\"$symname\", '$1', 0);";

%typemap(constant) long long, unsigned long long
	"add_integer_constant(\"$symname\", $1, 0);";

%typemap(constant) float, double
	"add_float_constant(\"$symname\", $1, 0);";

%typemap(constant) char *
	"add_string_constant(\"$symname\", \"$1\", 0);";

/* ------------------------------------------------------------
 * String & length
 * ------------------------------------------------------------ */

%typemap(in, pikedesc="tStr") (char *STRING, int LENGTH) {
    if ($input.type != T_STRING)
        Pike_error("Bad argument: Expected a string.\n");
    $1 = ($1_ltype) STR0($input.u.string);
    $2 = ($2_ltype) $input.u.string->length;
}

/* ------------------------------------------------------------
 * ANSI C typemaps
 * ------------------------------------------------------------ */

%typemap(in, pikedesc="tInt") size_t {
    if ($input.type != T_INT)
        Pike_error("Bad argument: Expected an integer.\n");
    $1 = ($1_ltype) $input.u.integer;
}

%typemap(out)      size_t = long;

/* ------------------------------------------------------------
 * Typechecking rules
 * ------------------------------------------------------------ */

%typecheck(SWIG_TYPECHECK_INTEGER)
	 int, short, long,
 	 unsigned int, unsigned short, unsigned long,
	 signed char, unsigned char,
	 long long, unsigned long long,
	 const int &, const short &, const long &,
 	 const unsigned int &, const unsigned short &, const unsigned long &,
	 const long long &, const unsigned long long &,
	 enum SWIGTYPE,
         bool, const bool & 
{
  $1 = ($input.type == T_INT) ? 1 : 0;
}

%typecheck(SWIG_TYPECHECK_DOUBLE)
	float, double,
	const float &, const double &
{
  $1 = (($input.type == T_FLOAT) || ($input.type == T_INT)) ? 1 : 0;
}

%typecheck(SWIG_TYPECHECK_CHAR) char {
  $1 = ($input.type == T_INT) ? 1 : 0;
}

%typecheck(SWIG_TYPECHECK_STRING) char * {
  $1 = ($input.type == T_STRING) ? 1 : 0;
}

%typecheck(SWIG_TYPECHECK_POINTER) SWIGTYPE *, SWIGTYPE &, SWIGTYPE [] {
  void *ptr;
  if (SWIG_ConvertPtr($input.u.object, (void **) &ptr, $1_descriptor, 0) == -1) {
    $1 = 0;
  } else {
    $1 = 1;
  }
}

%typecheck(SWIG_TYPECHECK_POINTER) SWIGTYPE {
  void *ptr;
  if (SWIG_ConvertPtr($input.u.object, (void **) &ptr, $&1_descriptor, 0) == -1) {
    $1 = 0;
  } else {
    $1 = 1;
  }
}

%typecheck(SWIG_TYPECHECK_VOIDPTR) void * {
  void *ptr;
  if (SWIG_ConvertPtr($input.u.object, (void **) &ptr, 0, 0) == -1) {
    $1 = 0;
  } else {
    $1 = 1;
  }
}

/* ------------------------------------------------------------
 * Overloaded operator support
 * ------------------------------------------------------------ */

#ifdef __cplusplus
%rename("`+")      *::operator+;
%rename("`-")      *::operator-;
%rename("`*")      *::operator*;
%rename("`/")      *::operator/;
%rename("`%")      *::operator%;
%rename("`<<")     *::operator<<;
%rename("`>>")     *::operator>>;
%rename("`&")      *::operator&;
%rename("`|")      *::operator|;
%rename("`^")      *::operator^;
%rename("`~")      *::operator~;
%rename("`<")      *::operator<;
%rename("`>")      *::operator>;
%rename("`==")     *::operator==;

/* Special cases */
%rename("`()")     *::operator();

/* Ignored operators */
%ignorewarn("362:operator= ignored") operator=;
%ignorewarn("365:operator+= ignored") operator+=;
%ignorewarn("366:operator-= ignored") operator-=;
%ignorewarn("367:operator*= ignored") operator*=;
%ignorewarn("368:operator/= ignored") operator/=;
%ignorewarn("369:operator%= ignored") operator%=;
%ignorewarn("370:operator^= ignored") operator^=;
%ignorewarn("371:operator&= ignored") operator&=;
%ignorewarn("372:operator|= ignored") operator|=;
%ignorewarn("375:operator<<= ignored") operator<<=;
%ignorewarn("376:operator>>= ignored") operator>>=;
%ignorewarn("378:operator!= ignored") operator!=;
%ignorewarn("379:operator<= ignored") operator<=;
%ignorewarn("380:operator>= ignored") operator>=;
%ignorewarn("381:operator&& ignored") operator&&;
%ignorewarn("382:operator|| ignored") operator||;
%ignorewarn("383:operator++ ignored") operator++;
%ignorewarn("384:operator-- ignored") operator--;
%ignorewarn("386:operator->* ignored") operator->*;
%ignorewarn("389:operator[] ignored (consider using %extend)") operator[];
%ignorewarn("390:operator+() ignored") operator+();
%ignorewarn("390:operator+() const ignored") operator+() const;
%ignorewarn("391:operator-() ignored") operator-();
%ignorewarn("391:operator-() const ignored") operator-() const;

#endif

/* ------------------------------------------------------------
 * The start of the Pike initialization function
 * ------------------------------------------------------------ */

%init %{
#ifdef __cplusplus
extern "C"
#endif
PIKE_MODULE_EXIT {}

#ifdef __cplusplus
extern "C"
#endif
PIKE_MODULE_INIT
{
    struct program *pr;
    int i;
    for (i = 0; swig_types_initial[i]; i++) {
        swig_types[i] = SWIG_TypeRegister(swig_types_initial[i]);
    }
%}

/* pike keywords */
/* please test and activate */
//%include "pikekw.swg"
