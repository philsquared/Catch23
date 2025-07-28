//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_WARNINGS_H
#define CATCHKIT_WARNINGS_H

// To disable classes of warnings in a portable way

#if defined(__clang__)
#define CATCHKIT_WARNINGS_SUPPRESS_START                _Pragma( "clang diagnostic push" )
#define CATCHKIT_WARNINGS_SUPPRESS_END                  _Pragma( "clang diagnostic pop" )
#define CATCHKIT_WARNINGS_SUPPRESS_PARENTHESES          _Pragma( "clang diagnostic ignored \"-Wparentheses\"" )
#define CATCHKIT_WARNINGS_SUPPRESS_UNUSED_COMPARISON    _Pragma( "clang diagnostic ignored \"-Wunused-comparison\"" )
#define CATCHKIT_WARNINGS_SUPPRESS_ADDRESS              _Pragma( "clang diagnostic ignored \"-Waddress\"" )
#define CATCHKIT_WARNINGS_SUPPRESS_SIGN_MISMATCH        _Pragma( "clang diagnostic ignored \"-Wsign-compare\"" )
#define CATCHKIT_WARNINGS_SUPPRESS_UNUSED_PARAMETER     _Pragma( "clang diagnostic ignored \"-Wunused-parameter\"" )
#define CATCHKIT_WARNINGS_SUPPRESS_FLOAT_RANGE          _Pragma( "clang diagnostic ignored \"-Wliteral-range\"" )
#define CATCHKIT_WARNINGS_SUPPRESS_NULL_CONVERSION      _Pragma( "clang diagnostic ignored \"-Wnull-conversion\"" )

#elif defined(__GNUC__) && !defined(__ICC) && !defined(__CUDACC__) // GCC
#define CATCHKIT_WARNINGS_SUPPRESS_START                _Pragma( "GCC diagnostic push" )
#define CATCHKIT_WARNINGS_SUPPRESS_END                  _Pragma( "GCC diagnostic pop" )
#define CATCHKIT_WARNINGS_SUPPRESS_PARENTHESES          _Pragma( "GCC diagnostic ignored \"-Wparentheses\"" )
#define CATCHKIT_WARNINGS_SUPPRESS_UNUSED_COMPARISON
#define CATCHKIT_WARNINGS_SUPPRESS_ADDRESS              _Pragma( "GCC diagnostic ignored \"-Waddress\"" )
#define CATCHKIT_WARNINGS_SUPPRESS_SIGN_MISMATCH        _Pragma( "GCC diagnostic ignored \"-Wsign-compare\"" )
#define CATCHKIT_WARNINGS_SUPPRESS_UNUSED_PARAMETER     _Pragma( "GCC diagnostic ignored \"-Wunused-parameter\"" )
#define CATCHKIT_WARNINGS_SUPPRESS_FLOAT_RANGE
#define CATCHKIT_WARNINGS_SUPPRESS_NULL_CONVERSION

#elif define(_MSC_VER)
#define CATCHKIT_WARNINGS_SUPPRESS_START                __pragma( warning(push) )
#define CATCHKIT_WARNINGS_SUPPRESS_END                  __pragma( warning(pop) )
#define CATCHKIT_WARNINGS_SUPPRESS_PARENTHESES
#define CATCHKIT_WARNINGS_SUPPRESS_UNUSED_COMPARISON
#define CATCHKIT_WARNINGS_SUPPRESS_ADDRESS
#define CATCHKIT_WARNINGS_SUPPRESS_SIGN_MISMATCH        __pragma( warning( disable:4389 ) ) __pragma( warning( disable:4018 ) )
#define CATCHKIT_WARNINGS_SUPPRESS_UNUSED_PARAMETER
#define CATCHKIT_WARNINGS_SUPPRESS_FLOAT_RANGE
#define CATCHKIT_WARNINGS_SUPPRESS_NULL_CONVERSION

#else
// !TBD Other compilers
#define CATCHKIT_WARNINGS_SUPPRESS_START
#define CATCHKIT_WARNINGS_SUPPRESS_END
#define CATCHKIT_WARNINGS_SUPPRESS_PARENTHESES
#define CATCHKIT_WARNINGS_SUPPRESS_UNUSED_COMPARISON
#define CATCHKIT_WARNINGS_SUPPRESS_ADDRESS
#define CATCHKIT_WARNINGS_SUPPRESS_UNUSED_PARAMETER
#define CATCHKIT_WARNINGS_SUPPRESS_FLOAT_RANGE
#define CATCHKIT_WARNINGS_SUPPRESS_NULL_CONVERSION

#endif

#endif // CATCHKIT_WARNINGS_H
