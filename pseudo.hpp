#ifdef USE_OPENPTY
#include "pseudo_openpty.hpp"
#elif defined(USE_UNIX98)
#include "pseudo_unix98.hpp"
#elif defined(USE_CLASSIC_PTY)
#include "pseudo_classic.hpp"
#else
#error No valid PTY module specified!
#endif
