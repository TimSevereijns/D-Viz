#ifndef NOTANOTHERWORDCOMPILER_HPP
#define NOTANOTHERWORDCOMPILER_HPP

/**
* @brief Silences warnings about one or more unused variables.
*
* @note This should work on all compilers (according to Herb Sutter), and the const is there
* to ensure that even rvalues will work.
*
* @see https://herbsutter.com/2009/10/18/mailbag-shutting-up-compiler-warnings/
*/
template<typename... Types>
void IgnoreUnused(const Types&...)
{
}

#endif // NOTANOTHERWORDCOMPILER_HPP
