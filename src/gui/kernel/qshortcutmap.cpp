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

#include "qshortcutmap_p.h"
#include "qkeysequence.h"
#include "qgraphicsscene.h"
#include "qgraphicsview.h"
#include "qevent.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qmenu.h"
#include "qshortcut.h"
#include "qapplication_p.h"
#include "qaction_p.h"
#include "qwidget_p.h"
#include "qdebug.h"

#ifndef QT_NO_SHORTCUT

QT_BEGIN_NAMESPACE

// To enable verbose output uncomment below
// #define DEBUG_QSHORTCUTMAP

/* \internal
    Entry data for QShortcutMap
    Contains:
        Keysequence for entry
        Pointer to parent owning the sequence
*/
struct QShortcutEntry
{
    QShortcutEntry()
        : keyseq(0), context(Qt::WindowShortcut), enabled(false), autorepeat(true), id(0), owner(nullptr)
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

/* \internal
    Private data for QShortcutMap
*/
class QShortcutMapPrivate
{
public:
    QShortcutMapPrivate()
        : currentId(0)
    {
    }

    int currentId;
    QList<QShortcutEntry> sequences;
};


/*! \internal
    QShortcutMap constructor.
*/
QShortcutMap::QShortcutMap()
    : d_ptr(new QShortcutMapPrivate())
{
}

/*! \internal
    QShortcutMap destructor.
*/
QShortcutMap::~QShortcutMap()
{
    delete d_ptr;
}

/*! \internal
    Adds a shortcut to the global map.
    Returns the id of the newly added shortcut.
*/
int QShortcutMap::addShortcut(QObject *owner, const QKeySequence &key, Qt::ShortcutContext context)
{
    Q_ASSERT_X(owner, "QShortcutMap::addShortcut", "All shortcuts need an owner");
    Q_ASSERT_X(!key.isEmpty(), "QShortcutMap::addShortcut", "Cannot add keyless shortcuts to map");
    Q_D(QShortcutMap);

    QShortcutEntry newEntry(owner, key, context, --(d->currentId), true);
    QList<QShortcutEntry>::iterator it = qUpperBound(d->sequences.begin(), d->sequences.end(), newEntry);
    d->sequences.insert(it, newEntry); // Insert sorted
#if defined(DEBUG_QSHORTCUTMAP)
    qDebug().nospace()
        << "QShortcutMap::addShortcut(" << owner << ", "
        << key << ", " << context << ") = " << d->currentId;
#endif
    return d->currentId;
}

/*! \internal
    Removes a shortcut from the global map.
    If \a owner is 0, all entries in the map with the key sequence specified
    is removed. If \a key is null, all sequences for \a owner is removed from
    the map. If \a id is 0, all sequences owned by \a owner are removed.
    Returns the number of sequences removed from the map.
*/

int QShortcutMap::removeShortcut(int id, QObject *owner)
{
    Q_D(QShortcutMap);
    int itemsRemoved = 0;
    bool allOwners = (owner == 0);
    bool allIds = (id == 0);

    // Special case, remove everything
    if (allOwners && allIds) {
        itemsRemoved = d->sequences.size();
        d->sequences.clear();
        return itemsRemoved;
    }

    int i = d->sequences.size()-1;
    while (i>=0)
    {
        const QShortcutEntry &entry = d->sequences.at(i);
        int entryId = entry.id;
        if ((allOwners || entry.owner == owner)
            && (allIds || entry.id == id)) {
            d->sequences.removeAt(i);
            ++itemsRemoved;
        }
        if (id == entryId)
            return itemsRemoved;
        --i;
    }
#if defined(DEBUG_QSHORTCUTMAP)
    qDebug().nospace()
        << "QShortcutMap::removeShortcut(" << id << ", " << owner << ") = " << itemsRemoved;
#endif
    return itemsRemoved;
}

/*! \internal
    Changes the enable state of a shortcut to \a enable.
    If \a owner is 0, all entries in the map with the key sequence specified
    is removed. If \a key is null, all sequences for \a owner is removed from
    the map. If \a id is 0, all key sequences owned by \a owner are changed.
    Returns the number of sequences which are matched in the map.
*/
int QShortcutMap::setShortcutEnabled(bool enable, int id, QObject *owner)
{
    Q_D(QShortcutMap);
    int itemsChanged = 0;
    bool allOwners = (owner == 0);
    bool allIds = id == 0;

    int i = d->sequences.size()-1;
    while (i>=0)
    {
        QShortcutEntry entry = d->sequences.at(i);
        if ((allOwners || entry.owner == owner)
            && (allIds || entry.id == id)) {
            d->sequences[i].enabled = enable;
            ++itemsChanged;
        }
        if (id == entry.id)
            return itemsChanged;
        --i;
    }
#if defined(DEBUG_QSHORTCUTMAP)
    qDebug().nospace()
        << "QShortcutMap::setShortcutEnabled(" << enable << ", " << id << ", "
        << owner << ") = " << itemsChanged;
#endif
    return itemsChanged;
}

/*! \internal
    Changes the auto repeat state of a shortcut to \a enable.
    If \a owner is 0, all entries in the map with the key sequence specified
    is removed. If \a key is null, all sequences for \a owner is removed from
    the map. If \a id is 0, all key sequences owned by \a owner are changed.
    Returns the number of sequences which are matched in the map.
*/
int QShortcutMap::setShortcutAutoRepeat(bool on, int id, QObject *owner)
{
    Q_D(QShortcutMap);
    int itemsChanged = 0;
    bool allOwners = (owner == 0);
    bool allIds = (id == 0);

    int i = d->sequences.size()-1;
    while (i>=0)
    {
        QShortcutEntry entry = d->sequences.at(i);
        if ((allOwners || entry.owner == owner)
            && (allIds || entry.id == id)) {
                d->sequences[i].autorepeat = on;
                ++itemsChanged;
        }
        if (id == entry.id)
            return itemsChanged;
        --i;
    }
#if defined(DEBUG_QSHORTCUTMAP)
    qDebug().nospace()
        << "QShortcutMap::setShortcutAutoRepeat(" << on << ", " << id << ", "
        << owner << ") = " << itemsChanged;
#endif
    return itemsChanged;
}

/*! \internal
    Uses ShortcutOverride event to see if any widgets want to override
    the event. If not, sends a QShortcutEvent event.
*/
bool QShortcutMap::tryShortcutEvent(QObject *o, QKeyEvent *e) const
{
    Q_D(const QShortcutMap);

    if (e->key() == Qt::Key_unknown) {
        return false;
    }

    bool wasAccepted = e->isAccepted();
    bool wasSpontaneous = e->spont;
    QEvent::Type orgType = e->t;
    e->t = QEvent::ShortcutOverride;
    e->ignore();
    QApplication::sendEvent(o, e);
    e->t = orgType;
    e->spont = wasSpontaneous;
    if (e->isAccepted()) {
#if defined(DEBUG_QSHORTCUTMAP)
        qDebug() << "override event accepted by" << o;
#endif
        if (!wasAccepted)
            e->ignore();
        return false;
    }

    QList<QShortcutEntry> matches;
    const QKeySequence eventks = QKeySequence(e->key() | e->modifiers());
#if defined(DEBUG_QSHORTCUTMAP)
    qDebug() << "looking for match for" << eventks;
#endif
    foreach (const QShortcutEntry &entry, d->sequences) {
        if (!entry.enabled) {
            continue;
        }
        if (e->isAutoRepeat() && !entry.autorepeat) {
            continue;
        }
        const QKeySequence::SequenceMatch match = entry.keyseq.matches(eventks);
        if (match != QKeySequence::NoMatch && correctContext(entry)) {
#if defined(DEBUG_QSHORTCUTMAP)
            qDebug() << "match for" << eventks << entry.keyseq << match << entry.owner;
#endif
            if (match == QKeySequence::ExactMatch) {
                // exact matches first
                matches.prepend(entry);
            } else {
                matches.append(entry);
            }
        }
    }

    bool stateWasAccepted = e->isAccepted();
    if (wasAccepted) {
        e->accept();
    } else {
        e->ignore();
    }

    const bool matched = (matches.size() > 0);
#if defined(DEBUG_QSHORTCUTMAP)
    if (!matched) {
        qDebug() << "no matches for" << eventks;
    }
#endif
    const bool ambiguous = (matches.size() > 1);
    foreach (const QShortcutEntry &entry, matches) {
#if defined(DEBUG_QSHORTCUTMAP)
        qDebug() << "sending event to" << entry.owner;
#endif
        QShortcutEvent se(entry.keyseq, entry.id, ambiguous);
        se.ignore();
        QApplication::sendEvent(entry.owner, &se);
        if (se.isAccepted()) {
#if defined(DEBUG_QSHORTCUTMAP)
            qDebug() << "event accepted by" << entry.owner;
#endif
            break;
        }
    }
    return matched;
}

/*! \internal
    Determines if an enabled shortcut has a matcing key sequence.
*/
bool QShortcutMap::hasShortcutForKeySequence(const QKeySequence &seq) const
{
    Q_D(const QShortcutMap);
    foreach (const QShortcutEntry &entry, d->sequences) {
        if (!entry.enabled) {
            continue;
        }
        if (entry.keyseq.matches(seq) != QKeySequence::NoMatch && correctContext(entry)) {
            return true;
        }
    }
    return false;
}

/*! \internal
    Returns true if the widget \a w is a logical sub window of the current
    top-level widget.
*/
bool QShortcutMap::correctContext(const QShortcutEntry &item) const
{
    Q_ASSERT_X(item.owner, "QShortcutMap", "Shortcut has no owner. Illegal map state!");

    QWidget *active_window = QApplication::activeWindow();

    // popups do not become the active window,
    // so we fake it here to get the correct context
    // for the shortcut system.
    if (QApplication::activePopupWidget())
        active_window = QApplication::activePopupWidget();

    if (!active_window)
        return false;
#ifndef QT_NO_ACTION
    if (QAction *a = qobject_cast<QAction *>(item.owner))
        return correctContext(item.context, a, active_window);
#endif
#ifndef QT_NO_GRAPHICSVIEW
    if (QGraphicsWidget *gw = qobject_cast<QGraphicsWidget *>(item.owner))
        return correctGraphicsWidgetContext(item.context, gw, active_window);
#endif
    QWidget *w = qobject_cast<QWidget *>(item.owner);
    if (!w) {
        QShortcut *s = qobject_cast<QShortcut *>(item.owner);
        w = s->parentWidget();
    }
    return correctWidgetContext(item.context, w, active_window);
}

bool QShortcutMap::correctWidgetContext(Qt::ShortcutContext context, QWidget *w, QWidget *active_window) const
{
    if (!w->isVisible() || !w->isEnabled())
        return false;

    if (context == Qt::ApplicationShortcut)
        return QApplicationPrivate::tryModalHelper(w); // true, unless w is shadowed by a modal dialog

    if (context == Qt::WidgetShortcut)
        return w == QApplication::focusWidget();

    if (context == Qt::WidgetWithChildrenShortcut) {
        const QWidget *tw = QApplication::focusWidget();
        while (tw && tw != w && (tw->windowType() == Qt::Widget || tw->windowType() == Qt::Popup))
            tw = tw->parentWidget();
        return tw == w;
    }

    // Below is Qt::WindowShortcut context
    QWidget *tlw = w->window();
#ifndef QT_NO_GRAPHICSVIEW
    if (QWExtra *topData = tlw->d_func()->extra) {
        if (topData->proxyWidget) {
            bool res = correctGraphicsWidgetContext(context, (QGraphicsWidget *)topData->proxyWidget, active_window);
            return res;
        }
    }
#endif

    /* if a floating tool window is active, keep shortcuts on the
     * parent working */
    if (active_window != tlw && active_window && active_window->windowType() == Qt::Tool && active_window->parentWidget()) {
        active_window = active_window->parentWidget()->window();
    }

    if (active_window  != tlw)
        return false;

    /* if we live in a MDI subwindow, ignore the event if we are
       not the active document window */
    const QWidget* sw = w;
    while (sw && !(sw->windowType() == Qt::SubWindow) && !sw->isWindow())
        sw = sw->parentWidget();
    if (sw && (sw->windowType() == Qt::SubWindow)) {
        QWidget *focus_widget = QApplication::focusWidget();
        while (focus_widget && focus_widget != sw)
            focus_widget = focus_widget->parentWidget();
        return sw == focus_widget;
    }
    return true;
}

#ifndef QT_NO_GRAPHICSVIEW
bool QShortcutMap::correctGraphicsWidgetContext(Qt::ShortcutContext context, QGraphicsWidget *w, QWidget *active_window) const
{
    if (!w->isVisible() || !w->isEnabled() || !w->scene())
        return false;

    if (context == Qt::ApplicationShortcut) {
        // Applicationwide shortcuts are always reachable unless their owner
        // is shadowed by modality. In QGV there's no modality concept, but we
        // must still check if all views are shadowed.
        QList<QGraphicsView *> views = w->scene()->views();
        for (int i = 0; i < views.size(); ++i) {
            if (QApplicationPrivate::tryModalHelper(views.at(i)))
                return true;
        }
        return false;
    }

    if (context == Qt::WidgetShortcut)
        return static_cast<QGraphicsItem *>(w) == w->scene()->focusItem();

    if (context == Qt::WidgetWithChildrenShortcut) {
        const QGraphicsItem *ti = w->scene()->focusItem();
        if (ti && ti->isWidget()) {
            const QGraphicsWidget *tw = static_cast<const QGraphicsWidget *>(ti);
            while (tw && tw != w && (tw->windowType() == Qt::Widget || tw->windowType() == Qt::Popup))
                tw = tw->parentWidget();
            return tw == w;
        }
        return false;
    }

    // Below is Qt::WindowShortcut context

    // Find the active view (if any).
    QList<QGraphicsView *> views = w->scene()->views();
    QGraphicsView *activeView = 0;
    for (int i = 0; i < views.size(); ++i) {
        QGraphicsView *view = views.at(i);
        if (view->window() == active_window) {
            activeView = view;
            break;
        }
    }
    if (!activeView)
        return false;

    // The shortcut is reachable if owned by a windowless widget, or if the
    // widget's window is the same as the focus item's window.
    QGraphicsWidget *a = w->scene()->activeWindow();
    return !w->window() || a == w->window();
}
#endif

#ifndef QT_NO_ACTION
bool QShortcutMap::correctContext(Qt::ShortcutContext context, QAction *a, QWidget *active_window) const
{
    const QList<QWidget *> &widgets = a->d_func()->widgets;
#if defined(DEBUG_QSHORTCUTMAP)
    if (widgets.isEmpty())
        qDebug() << a << "not connected to any widgets; won't trigger";
#endif
    for (int i = 0; i < widgets.size(); ++i) {
        QWidget *w = widgets.at(i);
#ifndef QT_NO_MENU
        if (QMenu *menu = qobject_cast<QMenu *>(w)) {
            QAction *a = menu->menuAction();
            if (correctContext(context, a, active_window))
                return true;
        } else
#endif
            if (correctWidgetContext(context, w, active_window))
                return true;
    }

#ifndef QT_NO_GRAPHICSVIEW
    const QList<QGraphicsWidget *> &graphicsWidgets = a->d_func()->graphicsWidgets;
#if defined(DEBUG_QSHORTCUTMAP)
    if (graphicsWidgets.isEmpty())
        qDebug() << a << "not connected to any widgets; won't trigger";
#endif
    for (int i = 0; i < graphicsWidgets.size(); ++i) {
        QGraphicsWidget *w = graphicsWidgets.at(i);
        if (correctGraphicsWidgetContext(context, w, active_window))
            return true;
    }
#endif
    return false;
}
#endif // QT_NO_ACTION

QT_END_NAMESPACE

#endif // QT_NO_SHORTCUT
