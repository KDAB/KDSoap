#ifndef KDSOAPGLOBAL_H
#define KDSOAPGLOBAL_H

#include <qglobal.h>

# ifdef KDSOAP_STATICLIB
#  undef KDSOAP_SHAREDLIB
#  define KDSOAP_EXPORT
# else
#  ifdef KDSOAP_BUILD_KDSOAP_LIB
#   define KDSOAP_EXPORT Q_DECL_EXPORT
#  else
#   define KDSOAP_EXPORT Q_DECL_IMPORT
#  endif
# endif

#endif /* KDSOAPGLOBAL_H */

