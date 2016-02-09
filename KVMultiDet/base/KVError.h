//$Id: KVError.h,v 1.3 2007/04/19 12:11:28 franklan Exp $

/*******************************************************
KVError.h

Defines macros for standard error messages.

In classes which inherit from TObject, we use the
methods TObject::Error, TObject::Warning etc.

In classes which do not inherit from TObject, we
can still use the same standard messages, by adding */
#include "TError.h"
#include "TClass.h"
/* and by defining the following macro */
#define KV__ERROR(method) Form("%s::"#method,IsA()->GetName())
/*
we can then use the following to give e.g. a Warning in
a method whose name is SomeMethod:

   Warning( KVERROR(SomeMethod), "Something is wrong, x=%d", x);

the Warning will appear with "ClassName::SomeMethod",
ClassName being the class of the actual object (i.e. name of derived
class if object inherits from class in which SomeMethod is defined).
*/
