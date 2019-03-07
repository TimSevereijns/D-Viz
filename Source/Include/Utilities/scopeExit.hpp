#ifndef SCOPEEXIT_HPP
#define SCOPEEXIT_HPP

#include <utility>

/**
 * @brief ScopeExit
 */
template <typename LambdaType> class ScopeExit
{
  public:
    ScopeExit(LambdaType&& lambda) noexcept : m_lambda{ std::move(lambda) }
    {
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

namespace Detail
{
    struct DummyStruct
    {
        // Left intentionally empty.
    };
} // namespace Detail

/**
 * Enables the use of braces to define the contents of the RAII object.
 *
 * @param[in] function              The lambda to be executed upon destruction of the ScopeExit
 *                                  object.
 *
 * @returns An RAII object encapsulating the lambda.
 */
template <typename LambdaType>
inline ScopeExit<LambdaType> operator+(const Detail::DummyStruct&, LambdaType&& lambda)
{
    return ScopeExit<LambdaType>{ std::forward<LambdaType>(lambda) };
}

#define NONEXPANDING_CONCAT(A, B) A##B

#define CONCAT(A, B) NONEXPANDING_CONCAT(A, B)

#define ON_SCOPE_EXIT auto CONCAT(scope_exit_, __LINE__) = Detail::DummyStruct{} + [=]()

#endif // SCOPEEXIT_HPP
