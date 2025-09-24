//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCHKIT_INTERNAL_PLATFORM_H
#define CATCHKIT_INTERNAL_PLATFORM_H

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
#  define CATCHKIT_PLATFORM_WINDOWS
#else
#  define CATCHKIT_PLATFORM_POSIX
#  if defined(linux) || defined(__linux) || defined(__linux__)
#    define CATCHKIT_PLATFORM_LINUX
#  elifdef __APPLE__
#    define CATCHKIT_PLATFORM_APPLE
#    include <TargetConditionals.h>
#    if TARGET_OS_OSX == 1
#      define CATCHKIT_PLATFORM_MAC
#    elif TARGET_OS_IPHONE == 1
#      define CATCHKIT_PLATFORM_IPHONE
#    endif
#  elif defined(linux) || defined(__linux) || defined(__linux__)
#    define CATCHKIT_PLATFORM_LINUX
#  endif
#endif

#endif // CATCHKIT_INTERNAL_PLATFORM_H
