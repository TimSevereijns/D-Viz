#include "ScopedHandle.h"

#include <Windows.h>

namespace
{
   HANDLE Duplicate(HANDLE handle)
   {
      if (handle == INVALID_HANDLE_VALUE)
      {
         return nullptr;
      }

      HANDLE duplicate;

      const auto successfullyDuplicated = !DuplicateHandle(
         /* sourceProcessHandle = */ GetCurrentProcess(),
         /* sourceHandle = */ handle,
         /* targetProcessHandle = */ GetCurrentProcess(),
         /* targetHandle = */ &duplicate,
         /* desiredAccess = */ 0,
         /* inheritHandle = */ FALSE,
         /* options = */ DUPLICATE_SAME_ACCESS);

      return successfullyDuplicated ? duplicate : nullptr;
   }
}

ScopedHandle::ScopedHandle(HANDLE handle) :
   m_handle{ handle }
{
}

ScopedHandle::~ScopedHandle()
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

ScopedHandle::ScopedHandle(ScopedHandle&& other)
{
   m_handle = other.m_handle;
   other.m_handle = nullptr;
}

ScopedHandle& ScopedHandle::operator=(ScopedHandle&& other)
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
   return (m_handle != nullptr) && (m_handle != INVALID_HANDLE_VALUE);
}

ScopedHandle::operator HANDLE() const
{
   return m_handle;
}
