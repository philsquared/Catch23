//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_INTERNAL_MACRO_UTILS_H
#define CATCHKIT_INTERNAL_MACRO_UTILS_H

// Convert arguments to a string
#define CATCHKIT_INTERNAL_STRINGIFY_ARGS(...) #__VA_ARGS__

// Generate a unique name, given a root
#define CATCHKIT_INTERNAL_UNIQUE_NAME_CAT2( first, second ) first##second
#define CATCHKIT_INTERNAL_UNIQUE_NAME_CAT( first, second ) CATCHKIT_INTERNAL_UNIQUE_NAME_CAT2( first, second )
#define CATCHKIT_INTERNAL_UNIQUE_NAME( name ) CATCHKIT_INTERNAL_UNIQUE_NAME_CAT( name, __LINE__ )

#endif // CATCHKIT_INTERNAL_MACRO_UTILS_H
