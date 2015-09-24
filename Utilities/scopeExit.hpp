#ifndef SCOPEEXIT_HPP
#define SCOPEEXIT_HPP

#include <utility>

/**
 * This struct is templated because the type of a C++11 lambda is only fully defined at compile
 * time, and as such cannot be referred to prior to that. By using a lambda we can get around this
 * without having to wrap the lambda inside of a std::function. While std::functions can wrap any
 * callable type, it can incur quite a bit of overhead to achieve this, including heap allocation.
 *
 * Since operations performed on scope exit are likely to be small and trivial in nature, limiting
 * overhead is highly desirable.
 */
template <typename FnType>
struct ScopeExit
{
  ScopeExit(FnType&& lambda)
     : m_lambda(std::forward<FnType>(lambda))
  {
  }

  ~ScopeExit()
  {
     m_lambda();
  }

  FnType m_lambda;
};

template <typename FnType>
ScopeExit<FnType> MakeScopeExit(FnType&& lambda)
{
  return ScopeExit<FnType>(std::forward<FnType>(lambda));
};

#define JOIN_TWO_STRINGS(str1, str2) str1 ## str2

#define ON_SCOPE_EXIT(code) \
   auto JOIN_TWO_STRINGS(scope_exit_, __LINE__) = MakeScopeExit([=](){ code; })

#endif // SCOPEEXIT_HPP

