/**
* The MIT License (MIT)
*
* Copyright (c) 2016 Tim Severeijns
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <typeinfo>

namespace StopwatchInternals
{
   template<typename Type>
   struct TypeName
   {
      // Evaluated at runtime:
      static decltype(typeid(Type).name()) value;
   };

   template<typename Type>
   decltype(typeid(Type).name()) TypeName<Type>::value = typeid(Type).name();

   template<>
   struct TypeName<std::chrono::nanoseconds>
   {
      static constexpr auto value = "nanoseconds";
   };

   template<>
   struct TypeName<std::chrono::microseconds>
   {
      static constexpr auto value = "microseconds";
   };

   template<>
   struct TypeName<std::chrono::milliseconds>
   {
      static constexpr auto value = "milliseconds";
   };

   template<>
   struct TypeName<std::chrono::seconds>
   {
      static constexpr auto value = "seconds";
   };

   template<>
   struct TypeName<std::chrono::minutes>
   {
      static constexpr auto value = "minutes";
   };

   template<>
   struct TypeName<std::chrono::hours>
   {
      static constexpr auto value = "hours";
   };
}

/**
* @brief The Stopwatch class will wrap the callable object to be timed in a timing block, and then,
* based on which constructor was called, pass the resulting timing information to either std::cout or a
* user-defined output stream or function upon completion of timing.
*
* @tparam ChronoType               One of the following std::chrono time representations:
*                                     @li std::chrono::nanoseconds
*                                     @li std::chrono::microseconds
*                                     @li std::chrono::milliseconds
*                                     @li std::chrono::seconds
*                                     @li std::chrono::minutes
*                                     @li std::chrono::hours
*/
template<typename ChronoType>
class Stopwatch
{
public:
   using CallbackType = std::function<void(ChronoType, std::string)>;

   /**
   * @brief This Stopwatch constructor executes and times the code encapsulated within the
   * callable object.
   *
   * Once the callable object has completed execution, the timing results and corresponding units
   * will be passed to the specified callback function.
   *
   * @param[in] callable            A callable object encapsulating the code to be timed.
   * @param[in] callback            Callback to handle the timing result.
   */
   template<typename CallableType>
   Stopwatch(
      CallableType&& callable,
      const CallbackType& callback)
      noexcept(noexcept(ExecuteAndTime(std::forward<CallableType>(callable))))
   {
      ExecuteAndTime(std::forward<CallableType>(callable));

      if (callback)
      {
         callback(m_elapsedTime, std::move(StopwatchInternals::TypeName<ChronoType>::value));
      }
   }

   /**
   * @brief This Stopwatch constructor will execute and time the code encapsulated within the
   * std::function object.
   *
   * Once the targeted code has completed execution, the timing results will be written out to
   * output stream, along with the specified message. Specifically, the message will be written out
   * first, followed immediately by the elapsed time and the corresponding units.
   *
   * @param[in] callable            A callable object encapsulating the code to be timed.
   * @param[in] message             Output message.
   * @param[in] outputStream        The std::ostream object to pipe the message and time to.
   */
   template<typename CallableType>
   Stopwatch(
      CallableType&& callable,
      const char* const message,
      std::ostream& outputStream = std::cout)
      noexcept(noexcept(ExecuteAndTime(std::forward<CallableType>(callable))))
   {
      ExecuteAndTime(std::forward<CallableType>(callable));

      outputStream
         << message
         << m_elapsedTime.count()
         << " "
         << StopwatchInternals::TypeName<ChronoType>::value
         << "."
         << std::endl;
   }

   /**
   * @brief This Stopwatch constructor will time the code encapsulated in the std::function object
   * and then save the result to a member variable.
   *
   * In order to retrieve the elapsed time, call GetElapsedTime(). @See GetElapsedTime().
   */
   template<typename CallableType>
   Stopwatch(CallableType&& callable)
      noexcept(noexcept(ExecuteAndTime(std::forward<CallableType>(callable))))
   {
      ExecuteAndTime(std::forward<CallableType>(callable));
   }

   /**
   * @returns The elapsed time in ChronoType units.
   */
   ChronoType GetElapsedTime() const
   {
      return m_elapsedTime;
   }

   /**
   * @returns A character array containing the chrono resolution name.
   */
   constexpr auto GetUnitsAsCharacterArray() const
   {
      return StopwatchInternals::TypeName<ChronoType>::value;
   }

private:

   /**
   * @todo Once C++17 arrives: `noexcept(std::is_nothrow_callable_v<CallableType>)`
   */
   template<typename CallableType>
   void ExecuteAndTime(CallableType&& callable) noexcept(noexcept(callable))
   {
      const auto start = std::chrono::high_resolution_clock::now();

      callable();

      const auto end = std::chrono::high_resolution_clock::now();
      m_elapsedTime = std::chrono::duration_cast<ChronoType>(end - start);
   }

   ChronoType m_elapsedTime;
};

#define TIME_IN_NANOSECONDS(code, message)   \
   Stopwatch<std::chrono::nanoseconds>(      \
      [&] { code; },                         \
      message);

#define TIME_IN_MICROSECONDS(code, message)  \
   Stopwatch<std::chrono::microseconds>(     \
      [&] { code; },                         \
      message);

#define TIME_IN_MILLISECONDS(code, message)  \
   Stopwatch<std::chrono::milliseconds>(     \
      [&] { code; },                         \
      message);

#define TIME_IN_SECONDS(code, message)       \
   Stopwatch<std::chrono::seconds>(          \
      [&] { code; },                         \
      message);

#define TIME_IN_MINUTES(code, message)       \
   Stopwatch<std::chrono::minutes>(          \
      [&] { code; },                         \
      message);

#define TIME_IN_HOURS(code, message)         \
   Stopwatch<std::chrono::hours>(            \
      [&] { code; },                         \
      message);
