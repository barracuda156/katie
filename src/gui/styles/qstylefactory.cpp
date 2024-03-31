/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtGui module of the Katie Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qstylefactory.h"
#include "qstyleplugin.h"
#include "qfactoryloader_p.h"
#include "qapplication.h"
#include "qwindowsstyle.h"
#ifndef QT_NO_STYLE_CLEANLOOKS
#include "qcleanlooksstyle.h"
#endif

QT_BEGIN_NAMESPACE

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, stylesloader, (QString::fromLatin1("/styles")))
#endif

/*!
    \class QStyleFactory
    \brief The QStyleFactory class creates QStyle objects.

    \ingroup appearance

    The QStyle class is an abstract base class that encapsulates the
    look and feel of a GUI. QStyleFactory creates a QStyle object
    using the create() function and a key identifying the style. The
    styles are either built-in or dynamically loaded from a style
    plugin (see QStylePlugin).

    The valid keys can be retrieved using the keys() function.
    Typically they include "windows" and "cleanlooks". Note that keys
    are case-insensitive.

    \sa QStyle
*/

/*!
    Creates and returns a QStyle object that matches the given \a key, or
    returns 0 if no matching style is found.

    Both built-in styles and styles from style plugins are queried for a
    matching style.

    \note The keys used are case insensitive.

    \sa keys()
*/
QStyle *QStyleFactory::create(const QString& key)
{
#ifndef QT_NO_STYLE_CLEANLOOKS
    if (key.compare(QLatin1String("Cleanlooks"), Qt::CaseInsensitive) == 0) {
        return new QCleanlooksStyle();
    }
#endif
#ifndef QT_NO_STYLE_WINDOWS
    if (key.compare(QLatin1String("Windows"), Qt::CaseInsensitive) == 0) {
        return new QWindowsStyle();
    }
#endif
#if !defined(QT_NO_LIBRARY)
    if (QStylePlugin *plugin = qobject_cast<QStylePlugin*>(stylesloader()->instance(key))) {
        QStyle *ret = plugin->create(key);
        if (ret) {
            // QApplicationPrivate::x11_apply_settings() relies on object name
            if (ret->objectName().isEmpty()) {
                ret->setObjectName(key);
            }
            return ret;
        }
    }
#endif
    return nullptr;
}

/*!
    Returns the list of valid keys, i.e. the keys this factory can
    create styles for.

    \sa create()
*/
QStringList QStyleFactory::keys()
{
#if !defined(QT_NO_LIBRARY)
    QStringList list = stylesloader()->keys();
#else
    QStringList list;
#endif
#ifndef QT_NO_STYLE_CLEANLOOKS
    list << QLatin1String("Cleanlooks");
#endif
#ifndef QT_NO_STYLE_WINDOWS
    list << QLatin1String("Windows");
#endif
    return list;
}

QT_END_NAMESPACE
