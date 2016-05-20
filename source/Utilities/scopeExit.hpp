#ifndef SCOPEEXIT_HPP
#define SCOPEEXIT_HPP

#include <utility>

/**
 * @brief ScopeExit
 */
template<typename LambdaType>
struct ScopeExit
{
   ScopeExit(LambdaType&& lambda) noexcept :
      m_lambda(std::move(lambda))
   {
   }

   ~ScopeExit()
   {
      m_lambda();
   }

   ScopeExit(const ScopeExit&) = delete;
   ScopeExit& operator=(const ScopeExit&) = delete;

   ScopeExit(ScopeExit&&) = default;
   ScopeExit& operator=(ScopeExit&&) = default;

   private:
      LambdaType m_lambda;
};

namespace
{
   struct DummyStruct
   {
      // Left intentionally empty.
   };
}

/**
 * Enables the use of braces to define the contents of the RAII object.
 *
 * @param[in] function              The lambda to be executed upon destruction of the ScopeExit
 *                                  object.
 *
 * @returns An RAII object encapsulating the lambda.
 */
template<typename LambdaType>
inline ScopeExit<LambdaType> operator+(const DummyStruct&, LambdaType&& lambda)
{
   return ScopeExit<LambdaType>{ std::forward<LambdaType>(lambda) };
}

#define JOIN_TWO_STRINGS(str1, str2) str1 ## str2

#define ON_SCOPE_EXIT \
   auto JOIN_TWO_STRINGS(scope_exit_, __LINE__) = DummyStruct{ } + [=] ()

#endif // SCOPEEXIT_HPP
