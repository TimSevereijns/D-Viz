#ifndef NULLTASKBARBUTTON_H
#define NULLTASKBARBUTTON_H

#if defined(Q_OS_LINUX)
#include <memory>

class QObject;
class QWindow;

struct NullProgress
{
    void reset() const
    {
    }
};

struct NullTaskbarButton
{
    NullTaskbarButton(QObject*)
    {
    }

    const NullProgress* progress() const
    {
        return s_progress.get();
    }

    void setWindow(QWindow*) const
    {
    }
    
  private:
    static std::shared_ptr<NullProgress> s_progress;
};
#endif // Q_OS_LINUX

#endif // NULLTASKBARBUTTON_H
