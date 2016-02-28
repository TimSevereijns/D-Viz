#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <iostream>
#include <functional>
#include <string>

template<typename Type>
class TypeName
{
   public:
      static const char* Get()
      {
         return typeid(Type).name();
      }
};

template<>
class TypeName<std::chrono::nanoseconds>
{
   public:
      static const char* Get()
      {
         return "nanoseconds";
      }
};

template<>
class TypeName<std::chrono::microseconds>
{
   public:
      static const char* Get()
      {
         return "microseconds";
      }
};

template<>
class TypeName<std::chrono::milliseconds>
{
   public:
      static const char* Get()
      {
         return "milliseconds";
      }
};

template<>
class TypeName<std::chrono::seconds>
{
   public:
      static const char* Get()
      {
         return "seconds";
      }
};

template<>
class TypeName<std::chrono::minutes>
{
   public:
      static const char* Get()
      {
         return "minutes";
      }
};

template<>
class TypeName<std::chrono::hours>
{
   public:
      static const char* Get()
      {
         return "hours";
      }
};

/**
 * @brief The StopWatch class will wrap the function to be timed in a timing block, and then
 * output the elapsed time to the console. Since this is an RAII object, if the function in question
 * throws, timing information will still be provided.
 *
 * @tparam TimeResolutionType       One of the following std::chrono time representations:
 *                                     @li std::chrono::nanoseconds
 *                                     @li std::chrono::microseconds
 *                                     @li std::chrono::milliseconds
 *                                     @li std::chrono::seconds
 *                                     @li std::chrono::minutes
 *                                     @li std::chrono::hours
 */
template<typename TimeResolutionType>
class StopWatch
{
   public:
      /**
       * @brief StopWatch constructor that executes the code to be timed after starting the timer.
       *
       * @param[in] functionToTime  A callable object representing the code to be timed.
       * @param[in] message         A brief description of the task at hand, which will be written
       *                            out to the console. @example "Task Execution Time: " @note The
       *                            timing units will be appended after the message with the correct
       *                            units (which will be derived from the template type).
       */
      StopWatch(std::function<void ()> functionToTime, const std::wstring& message)
         : m_message(message),
           m_start(std::chrono::high_resolution_clock::now())
      {
         functionToTime();
      }

      ~StopWatch()
      {
         const auto end = std::chrono::high_resolution_clock::now();
         const auto delta = std::chrono::duration_cast<TimeResolutionType>(end - m_start);

         if (!m_message.empty())
         {
            std::wcout << m_message << delta.count()
               << L" " << TypeName<TimeResolutionType>::Get() << std::endl;
         }
      }

   private:
      const std::wstring& m_message;

      std::chrono::high_resolution_clock::time_point m_start;
};

#endif // STOPWATCH_H
