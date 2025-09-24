//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_WARNINGS_H
#define CATCHKIT_WARNINGS_H

#include "internal_compilers.h"
#include "internal_macro_utils.h"

// To disable classes of warnings in a portable way

#ifdef CATCHKIT_COMPILER_CLANG
#  define CATCHKIT_CLANG_PRAGMA(w) _Pragma(w)
#  define CATCHKIT_CLANG_PRAGMA_NOTOOLS(w) _Pragma(w) // used for push/ pop - not defined for other compilers
#  define CATCHKIT_GCC_PRAGMA(w)
#  define CATCHKIT_MSVC_PRAGMA(w)

#elifdef CATCHKIT_COMPILER_GCC
#  if CATCHKIT_GCC_VERSION < 150300
#    define CATCHKIT_CLANG_PRAGMA(w)
#  else
#    define CATCHKIT_CLANG_PRAGMA(w) _Pragma(w) // Invoked for clang tools, regardless of compiler
#  endif
#  define CATCHKIT_CLANG_PRAGMA_NOTOOLS(w)
#  define CATCHKIT_GCC_PRAGMA(w) _Pragma(w)
#  define CATCHKIT_MSVC_PRAGMA(w)

#elifdef CATCHKIT_COMPILER_MSVC
#  define CATCHKIT_CLANG_PRAGMA(w)
#  define CATCHKIT_CLANG_PRAGMA_NOTOOLS(w)
#  define CATCHKIT_GCC_PRAGMA(w)
#  define CATCHKIT_MSVC_PRAGMA(w) __pragma(w)

#else
#  define CATCHKIT_CLANG_PRAGMA(w)
#  define CATCHKIT_CLANG_PRAGMA_NOTOOLS(w)
#  define CATCHKIT_GCC_PRAGMA(w)
#  define CATCHKIT_MSVC_PRAGMA(w)
#endif

#define CATCHKIT_CLANG_WARNING_UNSCOPED_DISABLE(w) CATCHKIT_CLANG_PRAGMA_NOTOOLS( CATCHKIT_INTERNAL_STRINGIFY_ARGS(clang diagnostic ignored w) )
#define CATCHKIT_CLANG_WARNING_DISABLE(w) CATCHKIT_CLANG_PRAGMA( CATCHKIT_INTERNAL_STRINGIFY_ARGS(clang diagnostic ignored w) )
#define CATCHKIT_GCC_WARNING_DISABLE(w) CATCHKIT_GCC_PRAGMA( CATCHKIT_INTERNAL_STRINGIFY_ARGS(GCC diagnostic ignored w) )
#define CATCHKIT_MSVC_WARNING_DISABLE(w) CATCHKIT_MSVC_PRAGMA( warning( disable:w ) )

// Start warning suppression block >>>
#define CATCHKIT_WARNINGS_SUPPRESS_START \
    CATCHKIT_GCC_PRAGMA( "GCC diagnostic push" ) \
    CATCHKIT_GCC_PRAGMA( "GCC diagnostic ignored \"-Wunknown-pragmas\"") \
    CATCHKIT_CLANG_PRAGMA_NOTOOLS( "clang diagnostic push" ) \
    CATCHKIT_MSVC_PRAGMA ( warning(push) )

// End warning suppression block <<<
#define CATCHKIT_WARNINGS_SUPPRESS_END \
    CATCHKIT_CLANG_PRAGMA_NOTOOLS( "clang diagnostic pop" ) \
    CATCHKIT_GCC_PRAGMA( "GCC diagnostic pop" ) \
    CATCHKIT_MSVC_PRAGMA( warning(pop) )


#define CATCHKIT_WARNINGS_SUPPRESS_PARENTHESES \
    CATCHKIT_CLANG_WARNING_DISABLE( "-Wparentheses" ) \
    CATCHKIT_GCC_WARNING_DISABLE  ( "-Wparentheses" )

#define CATCHKIT_WARNINGS_SUPPRESS_UNUSED_COMPARISON \
    CATCHKIT_CLANG_WARNING_DISABLE( "-Wunused-comparison" )

#define CATCHKIT_WARNINGS_SUPPRESS_ADDRESS \
    CATCHKIT_CLANG_WARNING_DISABLE( "-Waddress" ) \
    CATCHKIT_GCC_WARNING_DISABLE  ( "-Waddress" )

#define CATCHKIT_WARNINGS_SUPPRESS_SIGN_MISMATCH \
    CATCHKIT_CLANG_WARNING_DISABLE( "-Wsign-compare" ) \
    CATCHKIT_GCC_WARNING_DISABLE  ( "-Wsign-compare" ) \
    CATCHKIT_MSVC_WARNING_DISABLE( 4389 ) /* 'equality-operator' : signed/unsigned mismatch */ \
    CATCHKIT_MSVC_WARNING_DISABLE( 4018 ) /* 'token' : signed/unsigned mismatch */

#define CATCHKIT_WARNINGS_SUPPRESS_UNUSED_PARAMETER \
    CATCHKIT_CLANG_WARNING_DISABLE( "-Wunused-parameter" ) \
    CATCHKIT_GCC_WARNING_DISABLE  ( "-Wunused-parameter" )

#define CATCHKIT_WARNINGS_UNSCOPED_SUPPRESS_UNUSED_PARAMETER \
    CATCHKIT_CLANG_WARNING_UNSCOPED_DISABLE( "-Wunused-parameter" ) \
    CATCHKIT_GCC_WARNING_DISABLE  ( "-Wunused-parameter" )

#define CATCHKIT_WARNINGS_SUPPRESS_FLOAT_RANGE \
    CATCHKIT_CLANG_WARNING_DISABLE( "-Wliteral-range" )

#define CATCHKIT_WARNINGS_SUPPRESS_NULL_CONVERSION \
    CATCHKIT_CLANG_WARNING_DISABLE( "-Wnull-conversion" )

#define CATCHKIT_WARNINGS_SUPPRESS_SHADOW \
    CATCHKIT_CLANG_WARNING_DISABLE( "-Wshadow" ) \
    CATCHKIT_GCC_WARNING_DISABLE  ( "-Wshadow" )

#define CATCHKIT_WARNINGS_UNSCOPED_SUPPRESS_SHADOW \
    CATCHKIT_CLANG_WARNING_UNSCOPED_DISABLE( "-Wshadow" ) \
    CATCHKIT_GCC_WARNING_DISABLE  ( "-Wshadow" )


#endif // CATCHKIT_WARNINGS_H
