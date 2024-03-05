#ifdef USE_OPENPTY
#include "pseudo_openpty.cpp"
#elif defined(USE_UNIX98)
#include "pseudo_unix98.cpp"
#elif defined(USE_CLASSIC_PTY)
#include "pseudo_classic.cpp"
#else
#error No valid PTY module specified!
#endif
