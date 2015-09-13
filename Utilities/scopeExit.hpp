#ifndef SCOPEEXIT_HPP
#define SCOPEEXIT_HPP

/**
 * This struct is templated because the type of a C++11 lambda is only fully defined at compile
 * time, and as such cannot be referred to prior to that. By using a lambda we can get around this
 * without having to wrap the lambda inside of a std::function. While std::functions can wrap any
 * callable type, it can incur quite a bit of overhead to achieve this, including heap allocation.
 *
 * Since operations performed on scope exit are likely to be small and trivial in nature, limiting
 * overhead is strongly desirable.
 */
template <typename LAMBDA>
struct ScopeExit
{
  ScopeExit(LAMBDA lambda)
     : m_lambda(lambda)
  {
  }

  ~ScopeExit()
  {
     m_lambda();
  }

  LAMBDA m_lambda;
};

template <typename LAMBDA>
ScopeExit<LAMBDA> MakeScopeExit(LAMBDA lambda)
{
  return ScopeExit<LAMBDA>(lambda);
};

#define JOIN_TWO_STRINGS(str1, str2) str1 ## str2

#define ON_SCOPE_EXIT(code) \
   auto JOIN_TWO_STRINGS(scope_exit_, __LINE__) = MakeScopeExit([=](){ code; })

#endif // SCOPEEXIT_HPP

