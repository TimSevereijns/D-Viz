#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>

template<typename Type>
struct TypeName
{
   static const char* Resolve()
   {
      return typeid(Type).name();
   }
};

template<>
struct TypeName<std::chrono::nanoseconds>
{
   static const char* Resolve()
   {
      return "nanoseconds";
   }
};

template<>
struct TypeName<std::chrono::microseconds>
{
   static const char* Resolve()
   {
      return "microseconds";
   }
};

template<>
struct TypeName<std::chrono::milliseconds>
{
   static const char* Resolve()
   {
      return "milliseconds";
   }
};

template<>
struct TypeName<std::chrono::seconds>
{
   static const char* Resolve()
   {
      return "seconds";
   }
};

template<>
struct TypeName<std::chrono::minutes>
{
   static const char* Resolve()
   {
      return "minutes";
   }
};

template<>
struct TypeName<std::chrono::hours>
{
   static const char* Resolve()
   {
      return "hours";
   }
};

/**
 * @brief The StopWatch class will wrap the function to be timed in a timing block, and then
 * output the elapsed time to the console. Since this is an RAII object, if the function in question
 * throws, timing information will still be provided.
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
class StopWatch
{
   public:
      using LoggingFunction = std::function<void (std::uint64_t, const std::string&)>;

      /**
       * @brief StopWatch constructor that executes the code to be timed after starting the timer.
       *
       * @param[in] functionToTime  A callable object representing the code to be timed.
       * @param[in] logger          Callback to handle the timing result.
       */
      StopWatch(const std::function<void ()>& functionToTime,
                const LoggingFunction& logger)
         : m_logger(logger),
           m_start(std::chrono::high_resolution_clock::now())
      {
         functionToTime();
      }

      ~StopWatch()
      {
         const auto end = std::chrono::high_resolution_clock::now();
         const auto delta = std::chrono::duration_cast<ChronoType>(end - m_start);

         if (m_logger)
         {
            m_logger(delta.count(), TypeName<ChronoType>::Resolve());
         }
      }

   private:
      const LoggingFunction& m_logger;

      std::chrono::high_resolution_clock::time_point m_start;
};

/**
* @example TIME_THIS(Parser::Run(data), "Parsed Data in ");
*/
#define TIME_THIS(code, message)                                  \
   StopWatch<std::chrono::milliseconds>(                          \
      [&] { code; },                                              \
      [&] (std::uint64_t elapsedTime, const std::string& units)   \
      { std::cout << message << elapsedTime << " " << units; });

#endif // STOPWATCH_H
