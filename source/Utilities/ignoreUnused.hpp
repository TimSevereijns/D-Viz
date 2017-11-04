#pragma once

/**
* @brief Silences warnings about one or more unused variables.
*
* @see https://herbsutter.com/2009/10/18/mailbag-shutting-up-compiler-warnings/
*/
template<typename... Types>
void IgnoreUnused(Types&&...)
{
}
