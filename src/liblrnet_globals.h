#include <QtCore/QtGlobal>
#ifndef STATIC_LRNET
#if defined(LIBLRNET_LIBRARY)
#  define LIBLRNET_EXPORT Q_DECL_EXPORT
#else
#  define LIBLRNET_EXPORT Q_DECL_IMPORT
#endif
#else
#define LIBLRNET_EXPORT
#endif
