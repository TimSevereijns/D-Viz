#ifndef SCOPEEXIT_HPP
#define SCOPEEXIT_HPP

#include <utility>

/**
 * This struct is templated because the type of a C++11 lambda is only fully defined at compile
 * time, and as such cannot be referred to prior to that. By using a lambda we can get around this
 * without having to wrap the lambda inside of a std::function. While std::functions can wrap any
 * callable type, it can incur additional overhead, including heap allocation, in certain cases.
 */
template <typename FunctionType>
struct ScopeExit
{
  ScopeExit(FunctionType&& lambda)
     : m_lambda(std::forward<FunctionType>(lambda))
  {
  }

  ~ScopeExit()
  {
     m_lambda();
  }

private:
  FunctionType m_lambda;
};

template <typename FunctionType>
ScopeExit<FunctionType> MakeScopeExit(FunctionType&& lambda)
{
  return ScopeExit<FunctionType>(std::forward<FunctionType>(lambda));
}

#define JOIN_TWO_STRINGS(str1, str2) str1 ## str2

#define ON_SCOPE_EXIT(code) \
   auto JOIN_TWO_STRINGS(scope_exit_, __LINE__) = MakeScopeExit([=](){ code; })

#endif // SCOPEEXIT_HPP

