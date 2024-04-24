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

#ifndef QSHORTCUTMAP_P_H
#define QSHORTCUTMAP_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qlist.h"
#include "qkeysequence.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SHORTCUT

class QKeyEvent;
class QGraphicsWidget;
class QWidget;
class QAction;
class QObject;

/* \internal
    Entry data for QShortcutMap
    Contains:
        Keysequence for entry
        Pointer to parent owning the sequence
*/
struct QShortcutEntry
{
    QShortcutEntry()
        : context(Qt::WindowShortcut), enabled(false), autorepeat(true), id(0), owner(nullptr)
    {
    }

    QShortcutEntry(QObject *o, const QKeySequence &k, Qt::ShortcutContext c, int i, bool a)
        : keyseq(k), context(c), enabled(true), autorepeat(a), id(i), owner(o)
    {
    }

    bool operator<(const QShortcutEntry &f) const
    { return keyseq < f.keyseq; }

    QKeySequence keyseq;
    Qt::ShortcutContext context;
    bool enabled;
    bool autorepeat;
    int id;
    QObject *owner;
};

class QShortcutMap
{
public:
    QShortcutMap();

    int addShortcut(QObject *owner, const QKeySequence &key, Qt::ShortcutContext context);
    int removeShortcut(int id, QObject *owner);
    int setShortcutEnabled(bool enable, int id, QObject *owner);
    int setShortcutAutoRepeat(bool on, int id, QObject *owner);

    bool tryShortcutEvent(QObject *o, QKeyEvent *e) const;
    bool hasShortcutForKeySequence(const QKeySequence &seq) const;

private:
    Q_DISABLE_COPY(QShortcutMap);

    bool correctContext(const QShortcutEntry &item) const;
    bool correctWidgetContext(Qt::ShortcutContext context, QWidget *w, QWidget *active_window) const;
#ifndef QT_NO_GRAPHICSVIEW
    bool correctGraphicsWidgetContext(Qt::ShortcutContext context, QGraphicsWidget *w, QWidget *active_window) const;
#endif
#ifndef QT_NO_ACTION
    bool correctContext(Qt::ShortcutContext context,QAction *a, QWidget *active_window) const;
#endif

    int m_currentId;
    QList<QShortcutEntry> m_shortcuts;
};

#endif // QT_NO_SHORTCUT

QT_END_NAMESPACE

#endif // QSHORTCUTMAP_P_H
