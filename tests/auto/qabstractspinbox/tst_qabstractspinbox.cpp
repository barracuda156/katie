/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the test suite of the Katie Toolkit.
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


#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qabstractspinbox.h>
#include <qlineedit.h>
#include <qspinbox.h>


//TESTED_CLASS=
//TESTED_FILES=

class tst_QAbstractSpinBox : public QObject
{
Q_OBJECT

public:
    tst_QAbstractSpinBox();
    virtual ~tst_QAbstractSpinBox();

private slots:
    void getSetCheck();

    // task-specific tests below me:
    void task183108_clear();
};

tst_QAbstractSpinBox::tst_QAbstractSpinBox()
{
}

tst_QAbstractSpinBox::~tst_QAbstractSpinBox()
{
}

class MyAbstractSpinBox : public QAbstractSpinBox
{
public:
    MyAbstractSpinBox() : QAbstractSpinBox() {}
    QLineEdit *lineEdit() { return QAbstractSpinBox::lineEdit(); }
    void setLineEdit(QLineEdit *le) { QAbstractSpinBox::setLineEdit(le); }
};

// Testing get/set functions
void tst_QAbstractSpinBox::getSetCheck()
{
    MyAbstractSpinBox obj1;
    // ButtonSymbols QAbstractSpinBox::buttonSymbols()
    // void QAbstractSpinBox::setButtonSymbols(ButtonSymbols)
    obj1.setButtonSymbols(QAbstractSpinBox::ButtonSymbols(QAbstractSpinBox::UpDownArrows));
    QCOMPARE(QAbstractSpinBox::ButtonSymbols(QAbstractSpinBox::UpDownArrows), obj1.buttonSymbols());
    obj1.setButtonSymbols(QAbstractSpinBox::ButtonSymbols(QAbstractSpinBox::PlusMinus));
    QCOMPARE(QAbstractSpinBox::ButtonSymbols(QAbstractSpinBox::PlusMinus), obj1.buttonSymbols());

    // bool QAbstractSpinBox::wrapping()
    // void QAbstractSpinBox::setWrapping(bool)
    obj1.setWrapping(false);
    QCOMPARE(false, obj1.wrapping());
    obj1.setWrapping(true);
    QCOMPARE(true, obj1.wrapping());

    // QLineEdit * QAbstractSpinBox::lineEdit()
    // void QAbstractSpinBox::setLineEdit(QLineEdit *)
    QLineEdit *var3 = new QLineEdit(0);
    obj1.setLineEdit(var3);
    QCOMPARE(var3, obj1.lineEdit());
#ifdef QT_NO_DEBUG
    obj1.setLineEdit((QLineEdit *)0); // Will assert in debug, so only test in release
    QCOMPARE(var3, obj1.lineEdit()); // Setting 0 should keep the current editor
#endif
    // delete var3; // No delete, since QAbstractSpinBox takes ownership
}

void tst_QAbstractSpinBox::task183108_clear()
{
    QAbstractSpinBox *sbox;

    sbox = new QSpinBox;
    sbox->clear();
    sbox->show();
    qApp->processEvents();
    QVERIFY(sbox->text().isEmpty());

    delete sbox;
    sbox = new QSpinBox;
    sbox->clear();
    sbox->show();
    sbox->hide();
    qApp->processEvents();
    QCOMPARE(sbox->text(), QString());

    delete sbox;
    sbox = new QSpinBox;
    sbox->show();
    sbox->clear();
    qApp->processEvents();
    QCOMPARE(sbox->text(), QString());

    delete sbox;
    sbox = new QSpinBox;
    sbox->show();
    sbox->clear();
    sbox->hide();
    qApp->processEvents();
    QCOMPARE(sbox->text(), QString());

    delete sbox;
}

QTEST_MAIN(tst_QAbstractSpinBox)

#include "moc_tst_qabstractspinbox.cpp"
