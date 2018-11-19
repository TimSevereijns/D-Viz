#ifndef SCOPEDHANDLE_H
#define SCOPEDHANDLE_H

#include <QtGlobal>

#ifdef Q_OS_WIN

using HANDLE = void*;

class ScopedHandle
{
public:

   ScopedHandle(HANDLE handle);
   ~ScopedHandle() noexcept;

   ScopedHandle(const ScopedHandle& other);
   ScopedHandle& operator=(const ScopedHandle& other);

   ScopedHandle(ScopedHandle&& other) noexcept;
   ScopedHandle& operator=(ScopedHandle&& other) noexcept;

   void Close();

   void Reset(HANDLE handle);

   bool IsValid() const;

   explicit operator HANDLE() const;

private:

   HANDLE m_handle;
};

#endif

#endif // SCOPEDHANDLE_H
