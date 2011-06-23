#ifndef KDSOAPGLOBAL_H
#define KDSOAPGLOBAL_H

#include <qglobal.h>

# ifdef KDSOAP_STATICLIB
#  undef KDSOAP_SHAREDLIB
#  define KDSOAPCLIENT_EXPORT
# else
#  ifdef KDSOAP_BUILD_KDSOAP_LIB
#   define KDSOAPCLIENT_EXPORT Q_DECL_EXPORT
#  else
#   define KDSOAPCLIENT_EXPORT Q_DECL_IMPORT
#  endif
# endif

#endif /* KDSOAPGLOBAL_H */

