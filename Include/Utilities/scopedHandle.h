#ifndef SCOPEDHANDLE_H
#define SCOPEDHANDLE_H

#include <QtGlobal>

#ifdef Q_OS_WIN

#include <Windows.h>

using HANDLE = void*;

class ScopedHandle
{
  public:
    explicit ScopedHandle(HANDLE handle) : m_handle{ handle }
    {
    }

    ~ScopedHandle() noexcept
    {
        Close();
    }

    ScopedHandle(const ScopedHandle& other)
    {
        m_handle = ScopedHandle::Duplicate(other.m_handle);
    }

    ScopedHandle& operator=(const ScopedHandle& other)
    {
        if (this != &other) {
            m_handle = ScopedHandle::Duplicate(other.m_handle);
        }

        return *this;
    }

    ScopedHandle(ScopedHandle&& other) noexcept
    {
        m_handle = other.m_handle;
        other.m_handle = nullptr;
    }

    ScopedHandle& operator=(ScopedHandle&& other) noexcept
    {
        if (this != &other) {
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }

        return *this;
    }

    void Close()
    {
        if (IsValid()) {
            CloseHandle(m_handle);
            m_handle = nullptr;
        }
    }

    void Reset(HANDLE handle)
    {
        Close();

        m_handle = handle;
    }

    bool IsValid() const
    {
        return (m_handle != nullptr) && (m_handle != INVALID_HANDLE_VALUE); // NOLINT
    }

    explicit operator HANDLE() const
    {
        return m_handle;
    }

    static HANDLE Duplicate(HANDLE handle)
    {
        if (handle == INVALID_HANDLE_VALUE) // NOLINT
        {
            return nullptr;
        }

        HANDLE duplicate;

        const auto successfullyDuplicated = !DuplicateHandle(
            /* hSourceProcessHandle = */ GetCurrentProcess(),
            /* hSourceHandle = */ handle,
            /* hTargetProcessHandle = */ GetCurrentProcess(),
            /* lpTargetHandle = */ &duplicate,
            /* dwDesiredAccess = */ 0,
            /* bInheritHandle = */ FALSE,
            /* dwOptions = */ DUPLICATE_SAME_ACCESS);

        return successfullyDuplicated ? duplicate : nullptr;
    }

  private:
    HANDLE m_handle;
};

#endif

#endif // SCOPEDHANDLE_H
