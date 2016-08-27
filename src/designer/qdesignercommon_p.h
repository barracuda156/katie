#ifndef QDESIGNERCOMMON_P_H
#define QDESIGNERCOMMON_P_H

#include "abstractformeditor.h"
#include "qwidget.h"

enum { FormLayoutColumns = 2 };

static const char *Geometry = "Geometry";
static const char *SplitterPosition = "SplitterPosition";

// Find the form editor in the hierarchy.
// We know that the parent of the sheet is the extension manager
// whose parent is the core.

static QDesignerFormEditorInterface *formEditorForObject(QObject *o) {
    do {
        if (QDesignerFormEditorInterface* core = qobject_cast<QDesignerFormEditorInterface*>(o))
            return core;
        o = o->parent();
    } while(o);
    Q_ASSERT(o);
    return 0;
}

static void recursiveUpdate(QWidget *w)
{
    w->update();

    const QObjectList &l = w->children();
    const QObjectList::const_iterator cend = l.constEnd();
    for (QObjectList::const_iterator it = l.constBegin(); it != cend; ++it) {
        if (QWidget *w = qobject_cast<QWidget*>(*it))
            recursiveUpdate(w);
    }
}

Q_DECLARE_METATYPE(QWidgetList)
Q_DECLARE_METATYPE(QAction*)

#endif // QDESIGNERCOMMON_P_H
