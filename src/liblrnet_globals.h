#include <QtCore/QtGlobal>

#if defined(LIBLRNET_LIBRARY)
#  define LIBLRNET_EXPORT Q_DECL_EXPORT
#else
#  define LIBLRNET_EXPORT Q_DECL_IMPORT
#endif
