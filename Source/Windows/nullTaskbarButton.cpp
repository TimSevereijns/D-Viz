#include "Windows/nullTaskbarButton.h"

#if defined(Q_OS_LINUX)
std::shared_ptr<NullProgress> NullTaskbarButton::s_progress = std::make_shared<NullProgress>();
#endif // Q_OS_LINUX
