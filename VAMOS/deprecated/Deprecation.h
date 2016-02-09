// These macros are used to deprecate a class or a specific function. The use of
// a deprecated class or function results in a compiler warning.
//
// Author: Peter Wigg (peter.wigg.314159@gmail.com)

#ifndef __DEPRECATION_H__
#define __DEPRECATION_H__

// Used to deprecate specific functions at their declaration. For example:
// DEPRECATED void Print(); in the appropriate header file will deprecate the
// Print() function.

#ifdef __GNUC__
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif

// Add a "DEPRECATED_CLASS(classname);" to the end of a class header file to
// declare the class deprecated (after the closing brace of the class
// definition).

#ifdef __GNUC__
#define DEPRECATED_CLASS(classname) \
   typedef classname __attribute__((deprecated)) classname
#elif defined(_MSC_VER)
#define DEPRECATED_CLASS(classname) \
   typedef classname __declspec(deprecated) classname
#else
#pragma message("WARNING: You need to implement DEPRECATED_CLASS for this compiler")
#define DEPRECATED_CLASS(classname) classname
#endif

#endif

