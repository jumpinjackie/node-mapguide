/* -----------------------------------------------------------------------------
 * csharp.swg
 *
 * CSharp typemaps
 * ----------------------------------------------------------------------------- */

%include "csharphead.swg"

/* The ctype, imtype and cstype typemaps work together and so there should be one of each. 
 * The ctype typemap contains the PInvoke type used in the PInvoke (C/C++) code. 
 * The imtype typemap contains the C# type used in the intermediary class. 
 * The cstype typemap contains the C# type used in the C# proxy classes, type wrapper classes and module class. */

/* Primitive types */
%typemap(ctype) bool,               const bool &               "unsigned int"
%typemap(ctype) char,               const char &               "char"
%typemap(ctype) signed char,        const signed char &        "signed char"
%typemap(ctype) unsigned char,      const unsigned char &      "unsigned short"
%typemap(ctype) short,              const short &              "short"
%typemap(ctype) unsigned short,     const unsigned short &     "unsigned short"
%typemap(ctype) int,                const int &                "int"
%typemap(ctype) unsigned int,       const unsigned int &       "unsigned int"
%typemap(ctype) long,               const long &               "long"
%typemap(ctype) unsigned long,      const unsigned long &      "unsigned long"
%typemap(ctype) long long,          const long long &          "long long"
%typemap(ctype) unsigned long long, const unsigned long long & "unsigned long long"
%typemap(ctype) float,              const float &              "float"
%typemap(ctype) double,             const double &             "double"
%typemap(ctype) char *                                         "char *"
%typemap(ctype) wchar_t *                                      "wchar_t *"
%typemap(ctype) STRINGPARAM                                    "wchar_t *"
%typemap(ctype) STRING                                         "void *"
%typemap(ctype) void                                           "void"

%typemap(imtype) bool,               const bool &               "bool"
%typemap(imtype) char,               const char &               "char"
%typemap(imtype) signed char,        const signed char &        "sbyte"
%typemap(imtype) unsigned char,      const unsigned char &      "byte"
%typemap(imtype) short,              const short &              "short"
%typemap(imtype) unsigned short,     const unsigned short &     "ushort"
%typemap(imtype) int,                const int &                "int"
%typemap(imtype) unsigned int,       const unsigned int &       "uint"
%typemap(imtype) long,               const long &               "int"
%typemap(imtype) unsigned long,      const unsigned long &      "uint"
%typemap(imtype) long long,          const long long &          "long"
%typemap(imtype) unsigned long long, const unsigned long long & "ulong"
%typemap(imtype) float,              const float &              "float"
%typemap(imtype) double,             const double &             "double"
%typemap(imtype) char *                                         "string"
%typemap(imtype) wchar_t *                                      "string"
%typemap(imtype) STRINGPARAM                                    "[MarshalAs(UnmanagedType.LPWStr)] String"
%typemap(imtype) STRING                                         "IntPtr"
%typemap(imtype) void                                           "void"

%typemap(cstype) bool,               const bool &               "bool"
%typemap(cstype) char,               const char &               "char"
%typemap(cstype) signed char,        const signed char &        "sbyte"
%typemap(cstype) unsigned char,      const unsigned char &      "byte"
%typemap(cstype) short,              const short &              "short"
%typemap(cstype) unsigned short,     const unsigned short &     "ushort"
%typemap(cstype) int,                const int &                "int"
%typemap(cstype) unsigned int,       const unsigned int &       "uint"
%typemap(cstype) long,               const long &               "int"
%typemap(cstype) unsigned long,      const unsigned long &      "uint"
%typemap(cstype) long long,          const long long &          "long"
%typemap(cstype) unsigned long long, const unsigned long long & "ulong"
%typemap(cstype) float,              const float &              "float"
%typemap(cstype) double,             const double &             "double"
%typemap(cstype) char *                                         "string"
%typemap(cstype) wchar_t *                                      "string"
%typemap(cstype) STRINGPARAM                                    "string"
%typemap(cstype) STRING                                         "string"
%typemap(cstype) void                                           "void"

%typemap(ctype) char[ANY]                "char *"
%typemap(imtype) char[ANY]               "char *" 
%typemap(cstype) char[ANY]               "string"

/* Non primitive types */
%typemap(ctype) SWIGTYPE "void *"
%typemap(imtype) SWIGTYPE "IntPtr"
%typemap(cstype) SWIGTYPE "$&csclassname"

%typemap(ctype) SWIGTYPE [] "void *"
%typemap(imtype) SWIGTYPE [] "IntPtr"
%typemap(cstype) SWIGTYPE [] "$csclassname"

%typemap(ctype) SWIGTYPE * "void *"
%typemap(imtype) SWIGTYPE * "IntPtr"
%typemap(cstype) SWIGTYPE * "$csclassname"

%typemap(ctype) SWIGTYPE & "void *"
%typemap(imtype) SWIGTYPE & "IntPtr"
%typemap(cstype) SWIGTYPE & "$csclassname"

%typemap(ctype) enum SWIGTYPE "int"
%typemap(imtype) enum SWIGTYPE "int"
%typemap(cstype) enum SWIGTYPE "int"

/* pointer to a class member */
%typemap(ctype) SWIGTYPE (CLASS::*) "int"
%typemap(imtype) SWIGTYPE (CLASS::*) "IntPtr"
%typemap(cstype) SWIGTYPE (CLASS::*) "$csclassname"

/* The following are the in and out typemaps. These are the PInvoke code generating typemaps for converting from C# to C and visa versa. */

/* primitive types */
%typemap(in) bool
%{ $1 = $input ? true : false; %}

%typemap(in) char, 
             signed char, 
             unsigned char, 
             short, 
             unsigned short, 
             int, 
             unsigned int, 
             long, 
             unsigned long, 
             long long, 
             unsigned long long, 
             float, 
             double, 
             enum SWIGTYPE
%{ $1 = ($1_ltype)$input; %}

%typemap(out) bool               %{ $result = $1; %}
%typemap(out) char               %{ $result = $1; %}
%typemap(out) signed char        %{ $result = $1; %}
%typemap(out) unsigned char      %{ $result = $1; %}
%typemap(out) short              %{ $result = $1; %}
%typemap(out) unsigned short     %{ $result = $1; %}
%typemap(out) int                %{ $result = $1; %}
%typemap(out) unsigned int       %{ $result = $1; %}
%typemap(out) long               %{ $result = $1; %}
%typemap(out) unsigned long      %{ $result = $1; %}
%typemap(out) long long          %{ $result = $1; %}
%typemap(out) unsigned long long %{ $result = $1; %}
%typemap(out) float              %{ $result = $1; %}
%typemap(out) double             %{ $result = $1; %}
%typemap(out) enum SWIGTYPE      %{ $result = $1; %}

/* char * - treat as String */
%typemap(in) char * %{ $1 = $input; %}
%typemap(out) char * %{ $result = $1; %}

/* wchar_t * - treat as String */
%typemap(in)  wchar_t * %{ $1 = $input; %}
%typemap(out) wchar_t * %{ $result = $1; %}
%typemap(in)  STRINGPARAM %{ $1 = $input; %}
%typemap(out) STRING %{ $result = $1; %}

%typemap(out) void ""

/* primitive types by const reference */
%typemap(in) const bool & (bool temp)
%{ temp = $input ? true : false; 
   $1 = &temp; %}

%typemap(in) const char & (char temp), 
             const signed char & (signed char temp), 
             const unsigned char & (unsigned char temp), 
             const short & (short temp), 
             const unsigned short & (unsigned short temp), 
             const int & (int temp), 
             const unsigned int & (unsigned int temp), 
             const long & (long temp), 
             const unsigned long & (unsigned long temp), 
             const long long & ($*1_ltype temp), 
             const unsigned long long & ($*1_ltype temp), 
             const float & (float temp), 
             const double & (double temp)
%{ temp = ($*1_ltype)$input; 
$1 = &temp; %}

%typemap(out) const bool &               %{ $result = *$1; %}
%typemap(out) const char &               %{ $result = *$1; %}
%typemap(out) const signed char &        %{ $result = *$1; %}
%typemap(out) const unsigned char &      %{ $result = *$1; %}
%typemap(out) const short &              %{ $result = *$1; %}
%typemap(out) const unsigned short &     %{ $result = *$1; %}
%typemap(out) const int &                %{ $result = *$1; %}
%typemap(out) const unsigned int &       %{ $result = *$1; %}
%typemap(out) const long &               %{ $result = *$1; %}
%typemap(out) const unsigned long &      %{ $result = *$1; %}
%typemap(out) const long long &          %{ $result = *$1; %}
%typemap(out) const unsigned long long & %{ $result = *$1; %}
%typemap(out) const float &              %{ $result = *$1; %}
%typemap(out) const double &             %{ $result = *$1; %}

/* Default handling. Object passed by value. Convert to a pointer */
%typemap(in) SWIGTYPE ($&1_type argp)
%{ argp = ($&1_ltype)$input; 
   if (!argp) {
     SWIG_CSharpThrowException(SWIG_CSharpNullReferenceException, "Attempt to dereference null $1_type");
   }
   $1 = *argp; %}
%typemap(out) SWIGTYPE 
#ifdef __cplusplus
%{$result = new $1_ltype(($1_ltype &)$1); %}
#else
{
  $&1_ltype $1ptr = ($&1_ltype) malloc(sizeof($1_ltype));
  memmove($1ptr, &$1, sizeof($1_type));
  $result = $1ptr;
}
#endif

/* Generic pointers and references */
%typemap(in) SWIGTYPE * %{ $1 = ($1_ltype)$input; %}
%typemap(in) SWIGTYPE (CLASS::*) %{ $1 = *($&1_ltype)&$input; %}
%typemap(in) SWIGTYPE & %{ $1 = ($1_ltype)$input;
  if(!$1) {
    SWIG_CSharpThrowException(SWIG_CSharpNullReferenceException, "$1_type reference is null");
  } %}
%typemap(out) SWIGTYPE *, SWIGTYPE & %{ $result = (void *)$1; %} 
%typemap(out) SWIGTYPE (CLASS::*) %{ *($&1_ltype)&$result = $1; %} 


/* Default array handling */
%typemap(in) SWIGTYPE [] %{ $1 = ($1_ltype)$input; %}
%typemap(out) SWIGTYPE [] %{ $result = $1; %} 

/* char[ANY] - treat as String */
%typemap(in) char[ANY] %{ $1 = $input; %}
%typemap(out) char[ANY] %{ $result = $1; %}


/* Typecheck typemaps - The purpose of these is merely to issue a warning for overloaded C++ functions 
 * that cannot be overloaded in C# as more than one C++ type maps to a single C# type */

%typecheck(SWIG_TYPECHECK_BOOL) /* Java boolean */
    bool,
    const bool &
    ""

%typecheck(SWIG_TYPECHECK_CHAR) /* Java char */
    char, 
    const char &
    ""

%typecheck(SWIG_TYPECHECK_INT8) /* Java byte */
    signed char,
    const signed char &
    ""

%typecheck(SWIG_TYPECHECK_INT16) /* Java short */
    unsigned char, 
    short, 
    const unsigned char &, 
    const short &
    ""

%typecheck(SWIG_TYPECHECK_INT32) /* Java int */
    unsigned short, 
    int, 
    long, 
    const unsigned short &, 
    const int &, 
    const long &,
    enum SWIGTYPE
    ""

%typecheck(SWIG_TYPECHECK_INT64) /* Java long */
    unsigned int, 
    unsigned long, 
    long long, 
    const unsigned int &, 
    const unsigned long &, 
    const long long &
    ""

%typecheck(SWIG_TYPECHECK_INT128) /* Java BigInteger */
    unsigned long long
    ""

%typecheck(SWIG_TYPECHECK_FLOAT) /* Java float */
    float,
    const float &
    ""

%typecheck(SWIG_TYPECHECK_DOUBLE) /* Java double */
    double,
    const double &
    ""

%typecheck(SWIG_TYPECHECK_STRING) /* Java String */
    char *,
    wchar_t *,
    STRINGPARAM,
    STRING,
    char[ANY]
    ""

%typecheck(SWIG_TYPECHECK_POINTER) /* Default */
    SWIGTYPE, 
    SWIGTYPE *, 
    SWIGTYPE &, 
    SWIGTYPE [],
    SWIGTYPE (CLASS::*)
    ""

/* Exception handling */

%typemap(throws) int, 
                 long, 
                 short, 
                 unsigned int, 
                 unsigned long, 
                 unsigned short {
  char error_msg[256];
  sprintf(error_msg, "C++ $1_type exception thrown, value: %d", $1);
  SWIG_CSharpThrowException(SWIG_CSharpException, error_msg);
}

%typemap(throws) SWIGTYPE {
  SWIG_CSharpThrowException(SWIG_CSharpException, "C++ $1_type exception thrown");
}

%typemap(throws) char * {
  SWIG_CSharpThrowException(SWIG_CSharpException, $1);
}


/* Typemaps for code generation in proxy classes and C# type wrapper classes */

/* The csin typemap is used for converting function parameter types from the type 
 * used in the proxy, module or type wrapper class to the type used in the PInvoke class. */
%typemap(csin)   bool,               const bool &,
                 char,               const char &,
                 signed char,        const signed char &,
                 unsigned char,      const unsigned char &,
                 short,              const short &,
                 unsigned short,     const unsigned short &,
                 int,                const int &,
                 unsigned int,       const unsigned int &,
                 long,               const long &,
                 unsigned long,      const unsigned long &,
                 long long,          const long long &,
                 unsigned long long, const unsigned long long &,
                 float,              const float &,
                 double,             const double &,
                 char *,
                 wchar_t *,
                 STRINGPARAM,
                 char[ANY],
                 enum SWIGTYPE 
    "$csinput"
%typemap(csin) SWIGTYPE "$&csclassname.getCPtr($csinput)"
%typemap(csin) SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) "$csclassname.getCPtr($csinput)"

/* The csout typemap is used for converting function return types from the return type
 * used in the PInvoke class to the type returned by the proxy, module or type wrapper class. */
%typemap(csout)   bool,               const bool &,
                  char,               const char &,
                  signed char,        const signed char &,
                  unsigned char,      const unsigned char &,
                  short,              const short &,
                  unsigned short,     const unsigned short &,
                  int,                const int &,
                  unsigned int,       const unsigned int &,
                  long,               const long &,
                  unsigned long,      const unsigned long &,
                  long long,          const long long &,
                  unsigned long long, const unsigned long long &,
                  float,              const float &,
                  double,             const double &,
                  char *,
                  wchar_t *,
                  STRING,
                  char[ANY],
                  enum SWIGTYPE {
    return $imcall;
  }
%typemap(csout) void {
    $imcall;
  }
%typemap(csout) STRING {
    IntPtr cPtr = $imcall;
    String str = System.Runtime.InteropServices.Marshal.PtrToStringUni(cPtr);
    System.Runtime.InteropServices.Marshal.FreeCoTaskMem(cPtr);
    return str;
  }
%typemap(csout) SWIGTYPE {
    return new $&csclassname($imcall, true);
  }
%typemap(csout) SWIGTYPE & {
    return new $csclassname($imcall, $owner);
  }
%typemap(csout) SWIGTYPE *, SWIGTYPE [], SWIGTYPE (CLASS::*) {
    IntPtr cPtr = $imcall;
    return (cPtr == IntPtr.Zero) ? null : new $csclassname(cPtr, $owner);
  }

/* Properties */
%typemap(csvarin) SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) %{
    set {
      $imcall;
    } %}

%typemap(csvarout) bool,               const bool &,
                   char,               const char &,
                   signed char,        const signed char &,
                   unsigned char,      const unsigned char &,
                   short,              const short &,
                   unsigned short,     const unsigned short &,
                   int,                const int &,
                   unsigned int,       const unsigned int &,
                   long,               const long &,
                   unsigned long,      const unsigned long &,
                   long long,          const long long &,
                   unsigned long long, const unsigned long long &,
                   float,              const float &,
                   double,             const double &,
                   char *,
                   wchar_t *,
                   STRINGPARAM,
                   STRING,
                   char[ANY],
                   enum SWIGTYPE %{
    get {
      return $imcall;
    } %}
%typemap(csvarout) void %{
    get {
      $imcall;
    } %}
%typemap(csvarout) SWIGTYPE %{
    get {
      return new $&csclassname($imcall, true);
    } %}
%typemap(csvarout) SWIGTYPE & %{
    get {
      return new $csclassname($imcall, $owner);
    } %}
%typemap(csvarout) SWIGTYPE *, SWIGTYPE [], SWIGTYPE (CLASS::*) %{
    get {
      IntPtr cPtr = $imcall;
      return (cPtr == IntPtr.Zero) ? null : new $csclassname(cPtr, $owner);
    } %}

/* Typemaps used for the generation of proxy and type wrapper class code */
%typemap(csbase)                      SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(csclassmodifiers)            SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) "public"
%typemap(cscode)                      SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(csimports)                   SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) "using System;"
%typemap(csinterfaces)                SWIGTYPE "IDisposable"
%typemap(csinterfaces_derived)                  SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) ""
%typemap(csptrconstructormodifiers) SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) "public"

%typemap(csfinalize) SWIGTYPE %{
  ~$csclassname() {
    Dispose();
  }
%}

%typemap(csdestruct, methodname="Dispose") SWIGTYPE {
    if(swigCPtr != IntPtr.Zero && swigCMemOwn) {
      swigCMemOwn = false;
      $imcall;
    }
    swigCPtr = IntPtr.Zero;
    GC.SuppressFinalize(this);
  }

%typemap(csdestruct_derived, methodname="Dispose") SWIGTYPE {
    if(swigCPtr != IntPtr.Zero && swigCMemOwn) {
      swigCMemOwn = false;
      $imcall;
    }
    swigCPtr = IntPtr.Zero;
    GC.SuppressFinalize(this);
    base.Dispose();
  }

%typemap(csgetcptr) SWIGTYPE, SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) %{
  public static IntPtr getCPtr($csclassname obj) {
    return (obj == null) ? IntPtr.Zero : obj.swigCPtr;
  }
%}

/* C# specific directives */
#define %csconst(flag)     %feature("cs:const","flag")
#define %csmethodmodifiers %feature("cs:methodmodifiers")

%pragma(csharp) moduleimports=%{
using System;
%}

%pragma(csharp) imclassimports=%{
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Collections;
%}

/* Some ANSI C typemaps */

%apply long { size_t };

/* csharp keywords */
/* please test and activate */
//%include "csharpkw.swg"
