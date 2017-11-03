#ifndef GLOBALS_H
#define GLOBALS_H

#include "constants.h"

/**
 * @note This namespace is intended for globally visible objects that are supposed to be mutable.
 * Simply using the `static` keyword isn't enough in these cases, since the `static` keyword gives
 * a variable internal linkage, meaning that each translation copy will end up with its own copy of
 * the relevant object. Declaring such a variable as an `extern` variable isn't super clean either
 * since you can't declare and define all at once, and it doesn't necessarily solve all linkage
 * issues that might arise---these issues, however, appear to have been addressed in C++17. The
 * `Globals::Detail` namespace works around this issue by defining functions that return references
 * to "magic statics."
 */
namespace Globals
{
   /**
    * @note Defining globals through a static global reference to a value returned by a function
    * has several advantages. First, it eliminates lazy evaluation of the global; second, there's no
    * atomic boolean check anymore to see if the global static has been initialized, and instead the
    * value is access directly through what is essentially a pointer.
    *
    * @note In C++17, this whole mess can be replaced with the `inline` keyword; that is, just
    * `extern` the variable definition in a header somewhere, and then `inline` the declaration in
    * a CPP file elsewhere.
    *
    * @note All this was inspired by Nir Friedman's 2017 presentation at CppCon: "What C++
    * Developers Should Know About Globals (and the Linker)."
    */
   namespace Detail
   {
      inline auto& GetActivePrefix()
      {
         static Constants::FileSize::Prefix instance{ Constants::FileSize::Prefix::BINARY };
         return instance;
      }
   }

   static auto& ActivePrefix = Detail::GetActivePrefix();
}

#endif // GLOBALS_H
