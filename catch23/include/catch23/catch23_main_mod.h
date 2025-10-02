//
// Created by Phil Nash on 02/10/2025.
//

#ifndef CATCHKIT_CATCH23_MAIN_MOD_H
#define CATCHKIT_CATCH23_MAIN_MOD_H

// Convenience header for using Catch23 with modules in a main.cpp file
// This imports all necessary modules and provides the CATCH23_MIN_MAIN macro
//
// Usage:
//   #include "catch23/catch23_main_mod.h"
//   CATCH23_MIN_MAIN()  // or with ReportOn options

// Import the main module (which re-exports catch23 and catchkit)
import catch23.main;

// Include main.h for the CATCH23_MIN_MAIN macro
#include "catch23/main.h"

#endif // CATCHKIT_CATCH23_MAIN_MOD_H
