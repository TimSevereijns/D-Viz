#ifndef TROMPELOEILADAPTER_H
#define TROMPELOEILADAPTER_H

#ifndef QTTEST_VERSION_STR
#error "<QtTest> must be included before <trompeloeilAdapter.h>"
#endif

#include <trompeloeil.hpp>

namespace trompeloeil
{
    template <>
    inline void reporter<specialized>::send(
        severity severityLevel, const char* file, unsigned long line, const char* message)
    {
        if (severityLevel == severity::fatal) {
            qt_assert(message, file, line);
        } else {
            qt_assert(message, file, line);
        }
    }
} // namespace trompeloeil

#endif // TROMPELOEILADAPTER_H
