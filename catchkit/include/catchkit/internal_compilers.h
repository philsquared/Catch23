//
// Created by Phil Nash on 24/09/2025.
//

#ifndef CATCHKIT_INTERNAL_COMPILERS_H
#define CATCHKIT_INTERNAL_COMPILERS_H

#if defined(__clang__)
#  define CATCHKIT_COMPILER_CLANG
#elif defined(__GNUC__) && !defined(__ICC) && !defined(__CUDACC__) // GCC
#  define CATCHKIT_COMPILER_GCC
#  define CATCHKIT_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#elif define(_MSC_VER)
#  define CATCHKIT_COMPILER_MSVC
#else
#  define CATCHKIT_COMPILER_OTHER
#endif

#endif // CATCHKIT_INTERNAL_COMPILERS_H