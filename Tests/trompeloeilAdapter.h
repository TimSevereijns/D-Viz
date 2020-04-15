#ifndef TROMPELOEILADAPTER_H
#define TROMPELOEILADAPTER_H

#ifndef QTTEST_VERSION_STR //** 1 **//
#error "<QtTest> must be included before <trompeloeilAdapter.h>"
#endif

#include <trompeloeil.hpp>

namespace trompeloeil
{
    template <>
    inline void
    reporter<specialized>::send(severity s, const char* file, unsigned long line, const char* msg)
    {
        if (s == severity::fatal) {
            qt_assert(msg, file, line);
        } else {
            qt_assert(msg, file, line);
        }
    }
} // namespace trompeloeil

#endif // TROMPELOEILADAPTER_H
