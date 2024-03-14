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

#ifndef QSIZEPOLICY_H
#define QSIZEPOLICY_H

#include <QtCore/qobject.h>


QT_BEGIN_NAMESPACE


class QVariant;

class Q_GUI_EXPORT QSizePolicy
{
    Q_GADGET
    Q_ENUMS(Policy)

public:
    enum PolicyFlag {
        GrowFlag = 1,
        ExpandFlag = 2,
        ShrinkFlag = 4,
        IgnoreFlag = 8
    };

    enum Policy {
        Fixed = 0,
        Minimum = GrowFlag,
        Maximum = ShrinkFlag,
        Preferred = GrowFlag | ShrinkFlag,
        MinimumExpanding = GrowFlag | ExpandFlag,
        Expanding = GrowFlag | ShrinkFlag | ExpandFlag,
        Ignored = ShrinkFlag | GrowFlag | IgnoreFlag
    };

    enum ControlType {
        DefaultType      = 0x00000001,
        ButtonBox        = 0x00000002,
        CheckBox         = 0x00000004,
        ComboBox         = 0x00000008,
        Frame            = 0x00000010,
        GroupBox         = 0x00000020,
        Label            = 0x00000040,
        Line             = 0x00000080,
        LineEdit         = 0x00000100,
        PushButton       = 0x00000200,
        RadioButton      = 0x00000400,
        Slider           = 0x00000800,
        SpinBox          = 0x00001000,
        TabWidget        = 0x00002000,
        ToolButton       = 0x00004000
    };
    Q_DECLARE_FLAGS(ControlTypes, ControlType)

    QSizePolicy()
        : horzPolicy(QSizePolicy::Fixed),
        vertPolicy(QSizePolicy::Fixed),
        hfw(false),
        wfh(false),
        verStretch(0),
        horStretch(0),
        ctype(QSizePolicy::DefaultType)
    {
    }

    QSizePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical, QSizePolicy::ControlType type = QSizePolicy::DefaultType)
        : horzPolicy(horizontal),
        vertPolicy(vertical),
        hfw(false),
        wfh(false),
        verStretch(0),
        horStretch(0),
        ctype(type)
    {
    }

    inline QSizePolicy::Policy horizontalPolicy() const { return horzPolicy; }
    inline QSizePolicy::Policy verticalPolicy() const { return vertPolicy; }
    inline QSizePolicy::ControlType controlType() const { return ctype; }

    inline void setHorizontalPolicy(QSizePolicy::Policy d) { horzPolicy = d; }
    inline void setVerticalPolicy(QSizePolicy::Policy d) { vertPolicy = d; }
    inline void setControlType(ControlType type) { ctype = type; }

    Qt::Orientations expandingDirections() const {
        Qt::Orientations result = 0;
        if (verticalPolicy() & ExpandFlag)
            result |= Qt::Vertical;
        if (horizontalPolicy() & ExpandFlag)
            result |= Qt::Horizontal;
        return result;
    }

    inline void setHeightForWidth(bool b) { hfw = b; }
    inline bool hasHeightForWidth() const { return hfw; }
    inline void setWidthForHeight(bool b) { wfh = b;  }
    inline bool hasWidthForHeight() const { return wfh; }

    bool operator==(const QSizePolicy &s) const;
    inline bool operator!=(const QSizePolicy &s) const { return !(operator==(s)); }
    operator QVariant() const;

    inline int horizontalStretch() const { return horStretch; }
    inline int verticalStretch() const { return verStretch; }
    inline void setHorizontalStretch(quint8 stretchFactor) { horStretch = stretchFactor; }
    inline void setVerticalStretch(quint8 stretchFactor) { verStretch = stretchFactor; }

    void transpose();

private:
#ifndef QT_NO_DATASTREAM
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QSizePolicy &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QSizePolicy &);
#endif
    QSizePolicy::Policy horzPolicy;
    QSizePolicy::Policy vertPolicy;
    bool hfw;
    bool wfh;
    quint8 verStretch;
    quint8 horStretch;
    QSizePolicy::ControlType ctype;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSizePolicy::ControlTypes)

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QSizePolicy &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QSizePolicy &);
#endif

QT_END_NAMESPACE


#endif // QSIZEPOLICY_H
