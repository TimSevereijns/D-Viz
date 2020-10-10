#ifndef SCOPEDCURSOR_H
#define SCOPEDCURSOR_H

#include <QApplication>

/**
 * @brief Provides an easy wait to set a specific cursor for the duration of the resulting variable.
 */
class ScopedCursor
{
  public:
    explicit ScopedCursor(Qt::CursorShape desiredCursor)
    {
        QApplication::setOverrideCursor(desiredCursor);
    }

    ~ScopedCursor() noexcept
    {
        QApplication::restoreOverrideCursor();
    }

    ScopedCursor(const ScopedCursor& other) = default;
    ScopedCursor(ScopedCursor&& other) = default;

    ScopedCursor& operator=(const ScopedCursor&) = default;
    ScopedCursor& operator=(ScopedCursor&&) = default;
};

#endif // SCOPEDCURSOR_H
