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

#ifndef QSYNTAXHIGHLIGHTER_H
#define QSYNTAXHIGHLIGHTER_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_SYNTAXHIGHLIGHTER

#include <QtCore/qobject.h>
#include <QtGui/qtextobject.h>


QT_BEGIN_NAMESPACE

class QTextDocument;
class QSyntaxHighlighterPrivate;
class QTextCharFormat;
class QFont;
class QColor;
class QTextBlockUserData;
class QTextEdit;

class Q_GUI_EXPORT QSyntaxHighlighter : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSyntaxHighlighter)
public:
    QSyntaxHighlighter(QObject *parent);
    QSyntaxHighlighter(QTextDocument *parent);
    QSyntaxHighlighter(QTextEdit *parent);
    virtual ~QSyntaxHighlighter();

    void setDocument(QTextDocument *doc);
    QTextDocument *document() const;

public Q_SLOTS:
    void rehighlight();
    void rehighlightBlock(const QTextBlock &block);

protected:
    virtual void highlightBlock(const QString &text) = 0;

    void setFormat(int start, int count, const QTextCharFormat &format);
    void setFormat(int start, int count, const QColor &color);
    void setFormat(int start, int count, const QFont &font);
    QTextCharFormat format(int pos) const;

    int previousBlockState() const;
    int currentBlockState() const;
    void setCurrentBlockState(int newState);

    void setCurrentBlockUserData(QTextBlockUserData *data);
    QTextBlockUserData *currentBlockUserData() const;

    QTextBlock currentBlock() const;

private:
    Q_DISABLE_COPY(QSyntaxHighlighter)
};

QT_END_NAMESPACE


#endif // QT_NO_SYNTAXHIGHLIGHTER

#endif // QSYNTAXHIGHLIGHTER_H
