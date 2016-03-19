#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>

template<typename Type>
struct TypeName
{
   // @todo constexpr
   static const char* Resolve()
   {
      return typeid(Type).name();
   }
};

template<>
struct TypeName<std::chrono::nanoseconds>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "nanoseconds";
   }
};

template<>
struct TypeName<std::chrono::microseconds>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "microseconds";
   }
};

template<>
struct TypeName<std::chrono::milliseconds>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "milliseconds";
   }
};

template<>
struct TypeName<std::chrono::seconds>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "seconds";
   }
};

template<>
struct TypeName<std::chrono::minutes>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "minutes";
   }
};

template<>
struct TypeName<std::chrono::hours>
{
   // @todo UPGRADE TO C++11: constexpr
   static const char* Resolve()
   {
      return "hours";
   }
};

/**
 * @brief The Stopwatch class will wrap the function to be timed in a timing block, and then, based
 * on which constructor was called, pass the resulting timing information to either std::cout or a
 * user-defined function upon destruction of the class.
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
      using LoggingFunction = std::function<void (std::uint64_t, const std::string&)>;

      /**
       * @brief Stopwatch constructor that executes the code to be timed after starting the timer.
       * Once the targed code has completed execution, the timing results and corresponding units
       * will be passed to the specified callback function.
       *
       * @param[in] functionToTime  std::function encapsulating the code to be timed.
       * @param[in] logger          Callback to handle the timing result.
       */
      Stopwatch(const std::function<void ()>& functionToTime,
                const LoggingFunction&& logger)
         : m_logger(std::move(logger)),
           m_start(std::chrono::high_resolution_clock::now())
      {
         functionToTime();
      }

      /**
       * @brief Stopwatch constructor that executes the code to be timed after starting the timer.
       * Once the targeted code has completed execution, the timing results will be written out to
       * std::cout, along with the message. Specifically, the message will be written out first,
       * followed by the elapsed time and the corresponding units.
       *
       * @param[in] functionToTime  std::function encapsulating the code to be timed.
       * @param[in] message         Output message.
       */
      Stopwatch(const std::function<void ()>& functionToTime,
         const char* message)
         : m_logger(LoggingFunction()),
           m_message(message),
           m_start(std::chrono::high_resolution_clock::now())
      {
         functionToTime();
      }

      ~Stopwatch()
      {
         const auto end = std::chrono::high_resolution_clock::now();
         const auto delta = std::chrono::duration_cast<ChronoType>(end - m_start);

         if (m_logger)
         {
            m_logger(delta.count(), TypeName<ChronoType>::Resolve());
         }
         else
         {
            std::cout << m_message << delta.count() << " "
               << TypeName<ChronoType>::Resolve() << std::endl;
         }
      }

   private:
      const LoggingFunction m_logger;
      const char* m_message;

      std::chrono::high_resolution_clock::time_point m_start;
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

#endif // STOPWATCH_H
