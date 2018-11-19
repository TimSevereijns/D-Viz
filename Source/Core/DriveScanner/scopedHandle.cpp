#include "scopedHandle.h"

#ifdef Q_OS_WIN

#include <Windows.h>

namespace
{
   HANDLE Duplicate(HANDLE handle)
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
}

ScopedHandle::ScopedHandle(HANDLE handle) :
   m_handle{ handle }
{
}

ScopedHandle::~ScopedHandle() noexcept
{
   Close();
}

ScopedHandle::ScopedHandle(const ScopedHandle& other)
{
   m_handle = Duplicate(other.m_handle);
}

ScopedHandle& ScopedHandle::operator=(const ScopedHandle& other)
{
   if (this != &other)
   {
      m_handle = Duplicate(other.m_handle);
   }

   return *this;
}

ScopedHandle::ScopedHandle(ScopedHandle&& other) noexcept
{
   m_handle = other.m_handle;
   other.m_handle = nullptr;
}

ScopedHandle& ScopedHandle::operator=(ScopedHandle&& other) noexcept
{
   if (this != &other)
   {
      m_handle = other.m_handle;
      other.m_handle = nullptr;
   }

   return *this;
}

void ScopedHandle::Close()
{
   if (IsValid())
   {
      CloseHandle(m_handle);
      m_handle = nullptr;
   }
}

void ScopedHandle::Reset(HANDLE handle)
{
   Close();

   m_handle = handle;
}

bool ScopedHandle::IsValid() const
{
   return (m_handle != nullptr) && (m_handle != INVALID_HANDLE_VALUE); // NOLINT
}

ScopedHandle::operator HANDLE() const
{
   return m_handle;
}

#endif
