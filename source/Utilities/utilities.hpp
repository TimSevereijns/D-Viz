#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <sstream>
#include <mutex>

namespace Utilities
{
   namespace
   {
      std::once_flag stringStreamSetupFlag;
   }

   template<typename NumericType>
   static auto StringifyWithDigitSeparators(NumericType number)
   {
      static_assert(std::is_arithmetic_v<NumericType>, "Please pass in a numeric type.");

      static std::wstringstream stream;

      std::call_once(stringStreamSetupFlag,
         [&] () noexcept
      {
         stream.imbue(std::locale{ "" });
      });

      stream.str(std::wstring{ });
      stream.clear();

      stream << number;
      return stream.str();
   }
}

#endif // UTILITIES_HPP
