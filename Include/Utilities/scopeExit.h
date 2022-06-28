#ifndef SCOPEEXIT_HPP
#define SCOPEEXIT_HPP

#include <utility>

/**
 * @brief An RAII wrapper that will execute an action when the wrapper falls out of scope, or is
 * otherwise destroyed.
 */
template <typename LambdaType> class ScopeExit
{
  public:
    ScopeExit(LambdaType&& lambda) noexcept : m_lambda{ std::move<LambdaType>(lambda) }
    {
        static_assert(
            std::is_nothrow_invocable<LambdaType>::value,
            "Since the callable type is invoked from the destructor, exceptions are not allowed.");
    }

    ~ScopeExit() noexcept
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

#endif // SCOPEEXIT_HPP
