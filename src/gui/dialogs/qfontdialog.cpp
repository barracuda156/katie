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

#include "qwindowdefs.h"

#ifndef QT_NO_FONTDIALOG

#include "qfontdialog.h"
#include "qfontdialog_p.h"

#include "qapplication.h"
#include "qcheckbox.h"
#include "qcombobox.h"
#include "qevent.h"
#include "qfontdatabase.h"
#include "qgroupbox.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qstyle.h"
#include "qdialogbuttonbox.h"
#include "qheaderview.h"
#include "qlistview.h"
#include "qstringlistmodel.h"
#include "qvalidator.h"
#include "qdialog_p.h"
#include "qfont_p.h"
#include "qfontdatabase_p.h"

QT_BEGIN_NAMESPACE

class QFontListView : public QListView
{
    Q_OBJECT
public:
    QFontListView(QWidget *parent);
    inline QStringListModel *model() const {
        return static_cast<QStringListModel *>(QListView::model());
    }
    inline void setCurrentItem(int item) {
        QListView::setCurrentIndex(static_cast<QAbstractListModel*>(model())->index(item));
    }
    inline int currentItem() const {
        return QListView::currentIndex().row();
    }
    inline int count() const {
        return model()->rowCount();
    }
    inline QString currentText() const {
        int row = QListView::currentIndex().row();
        return row < 0 ? QString() : model()->stringList().at(row);
    }
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) {
        QListView::currentChanged(current, previous);
        if (current.isValid())
            emit highlighted(current.row());
    }
    QString text(int i) const {
        return model()->stringList().at(i);
    }
signals:
    void highlighted(int);
};

QFontListView::QFontListView(QWidget *parent)
    : QListView(parent)
{
    setModel(new QStringListModel(parent));
    setEditTriggers(NoEditTriggers);
}

static const Qt::WindowFlags DefaultFontWindowFlags =
        Qt::Dialog | Qt::WindowSystemMenuHint;

/*!
  \class QFontDialog
  \ingroup standard-dialogs

  \brief The QFontDialog class provides a dialog widget for selecting a font.

    A font dialog is created through one of the static getFont()
    functions.

  Examples:

  \snippet doc/src/snippets/code/src_gui_dialogs_qfontdialog.cpp 0

    The dialog can also be used to set a widget's font directly:
  \snippet doc/src/snippets/code/src_gui_dialogs_qfontdialog.cpp 1
  If the user clicks OK the font they chose will be used for myWidget,
  and if they click Cancel the original font is used.

  \image plastique-fontdialog.png A font dialog in the Plastique widget style.

  \sa QFont, QFontMetrics, QColorDialog, QFileDialog, QPrintDialog,
      {Standard Dialogs Example}
*/

/*!
    \since 4.5

    Constructs a standard font dialog.

    Use setCurrentFont() to set the initial font attributes.

    The \a parent parameter is passed to the QDialog constructor.

    \sa getFont()
*/
QFontDialog::QFontDialog(QWidget *parent)
    : QDialog(*new QFontDialogPrivate, parent, DefaultFontWindowFlags)
{
    Q_D(QFontDialog);
    d->init();
}

/*!
    \since 4.5

    Constructs a standard font dialog with the given \a parent and specified
    \a initial font.
*/
QFontDialog::QFontDialog(const QFont &initial, QWidget *parent)
    : QDialog(*new QFontDialogPrivate, parent, DefaultFontWindowFlags)
{
    Q_D(QFontDialog);
    d->init();
    setCurrentFont(initial);
}

void QFontDialogPrivate::init()
{
    Q_Q(QFontDialog);

    q->setSizeGripEnabled(true);
    q->setWindowTitle(QFontDialog::tr("Select Font"));

    // grid
    familyEdit = new QLineEdit(q);
    familyEdit->setReadOnly(true);
    familyList = new QFontListView(q);
    familyEdit->setFocusProxy(familyList);

    familyAccel = new QLabel(q);
#ifndef QT_NO_SHORTCUT
    familyAccel->setBuddy(familyList);
#endif
    familyAccel->setIndent(2);

    styleEdit = new QLineEdit(q);
    styleEdit->setReadOnly(true);
    styleList = new QFontListView(q);
    styleEdit->setFocusProxy(styleList);

    styleAccel = new QLabel(q);
#ifndef QT_NO_SHORTCUT
    styleAccel->setBuddy(styleList);
#endif
    styleAccel->setIndent(2);

    sizeEdit = new QLineEdit(q);
    sizeEdit->setFocusPolicy(Qt::ClickFocus);
    QIntValidator *validator = new QIntValidator(1, 512, q);
    sizeEdit->setValidator(validator);
    sizeList = new QFontListView(q);

    sizeAccel = new QLabel(q);
#ifndef QT_NO_SHORTCUT
    sizeAccel->setBuddy(sizeEdit);
#endif
    sizeAccel->setIndent(2);

    // effects box
    effects = new QGroupBox(q);
    QVBoxLayout *vbox = new QVBoxLayout(effects);
    strikeout = new QCheckBox(effects);
    vbox->addWidget(strikeout);
    underline = new QCheckBox(effects);
    vbox->addWidget(underline);

    sample = new QGroupBox(q);
    QHBoxLayout *hbox = new QHBoxLayout(sample);
    sampleEdit = new QLineEdit(sample);
    sampleEdit->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    sampleEdit->setAlignment(Qt::AlignCenter);
    // Note that the sample text is *not* translated with tr(), as the
    // characters used depend on the charset encoding.
    sampleEdit->setText(QLatin1String("AaBbYyZz"));
    hbox->addWidget(sampleEdit);

    QObject::connect(familyList, SIGNAL(highlighted(int)), q, SLOT(_q_familyHighlighted(int)));
    QObject::connect(styleList, SIGNAL(highlighted(int)), q, SLOT(_q_styleHighlighted(int)));
    QObject::connect(sizeList, SIGNAL(highlighted(int)), q, SLOT(_q_sizeHighlighted(int)));
    QObject::connect(sizeEdit, SIGNAL(textChanged(QString)), q, SLOT(_q_sizeChanged(QString)));

    QObject::connect(strikeout, SIGNAL(clicked()), q, SLOT(_q_updateSample()));
    QObject::connect(underline, SIGNAL(clicked()), q, SLOT(_q_updateSample()));

    updateFamilies();
    if (familyList->count() != 0)
        familyList->setCurrentItem(0);

    // grid layout
    QGridLayout *mainGrid = new QGridLayout(q);

    int spacing = mainGrid->spacing();
    if (spacing >= 0) {     // uniform spacing
       mainGrid->setSpacing(0);

       mainGrid->setColumnMinimumWidth(1, spacing);
       mainGrid->setColumnMinimumWidth(3, spacing);

       int margin = 0;
       mainGrid->getContentsMargins(0, 0, 0, &margin);

       mainGrid->setRowMinimumHeight(3, margin);
       mainGrid->setRowMinimumHeight(6, 2);
       mainGrid->setRowMinimumHeight(8, margin);
    }

    mainGrid->addWidget(familyAccel, 0, 0);
    mainGrid->addWidget(familyEdit, 1, 0);
    mainGrid->addWidget(familyList, 2, 0);

    mainGrid->addWidget(styleAccel, 0, 2);
    mainGrid->addWidget(styleEdit, 1, 2);
    mainGrid->addWidget(styleList, 2, 2);

    mainGrid->addWidget(sizeAccel, 0, 4);
    mainGrid->addWidget(sizeEdit, 1, 4);
    mainGrid->addWidget(sizeList, 2, 4);

    mainGrid->setColumnStretch(0, 38);
    mainGrid->setColumnStretch(2, 24);
    mainGrid->setColumnStretch(4, 10);

    mainGrid->addWidget(effects, 4, 0);

    mainGrid->addWidget(sample, 4, 2, 4, 3);

    buttonBox = new QDialogButtonBox(q);
    mainGrid->addWidget(buttonBox, 9, 0, 1, 5);

    QPushButton *button
            = static_cast<QPushButton *>(buttonBox->addButton(QDialogButtonBox::Ok));
    QObject::connect(buttonBox, SIGNAL(accepted()), q, SLOT(accept()));
    button->setDefault(true);

    buttonBox->addButton(QDialogButtonBox::Cancel);
    QObject::connect(buttonBox, SIGNAL(rejected()), q, SLOT(reject()));

    q->resize(500, 360);

    sizeEdit->installEventFilter(q);
    familyList->installEventFilter(q);
    styleList->installEventFilter(q);
    sizeList->installEventFilter(q);

    familyList->setFocus();
    retranslateStrings();
}

/*!
  \internal
 Destroys the font dialog and frees up its storage.
*/

QFontDialog::~QFontDialog()
{
}

/*!
  Executes a modal font dialog and returns a font.

  If the user clicks \gui OK, the selected font is returned. If the user
  clicks \gui Cancel, the \a initial font is returned.

  The dialog is constructed with the given \a parent and the options specified
  in \a options. \a title is shown as the window title of the dialog and  \a
  initial is the initially selected font. If the \a ok parameter is not-null,
  the value it refers to is set to true if the user clicks \gui OK, and set to
  false if the user clicks \gui Cancel.

  Examples:
  \snippet doc/src/snippets/code/src_gui_dialogs_qfontdialog.cpp 2

    The dialog can also be used to set a widget's font directly:
  \snippet doc/src/snippets/code/src_gui_dialogs_qfontdialog.cpp 3
  In this example, if the user clicks OK the font they chose will be
  used, and if they click Cancel the original font is used.

  \warning Do not delete \a parent during the execution of the dialog.
           If you want to do this, you should create the dialog
           yourself using one of the QFontDialog constructors.
*/
QFont QFontDialog::getFont(bool *ok, const QFont &initial, QWidget *parent, const QString &title,
                           FontDialogOptions options)
{
    QFontDialog dlg(parent);
    dlg.setOptions(options);
    dlg.setCurrentFont(initial);
    if (!title.isEmpty()) {
        dlg.setWindowTitle(title);
    }

    int ret = (dlg.exec() || (options & QFontDialog::NoButtons));
    if (ok) {
        *ok = !!ret;
    }
    if (ret) {
        return dlg.selectedFont();
    }
    return initial;
}

/*!
    \overload

  Executes a modal font dialog and returns a font.

  If the user clicks \gui OK, the selected font is returned. If the user
  clicks \gui Cancel, the Qt default font is returned.

  The dialog is constructed with the given \a parent.
  If the \a ok parameter is not-null, the value it refers to is set
  to true if the user clicks \gui OK, and false if the user clicks
  \gui Cancel.

  Example:
  \snippet doc/src/snippets/code/src_gui_dialogs_qfontdialog.cpp 4

  \warning Do not delete \a parent during the execution of the dialog.
           If you want to do this, you should create the dialog
           yourself using one of the QFontDialog constructors.
*/
QFont QFontDialog::getFont(bool *ok, QWidget *parent)
{
    QFont initial;
    return QFontDialog::getFont(ok, initial, parent, QString(), 0);
}

/*!
    \internal
    An event filter to make the Up, Down, PageUp and PageDown keys work
    correctly in the line edits. The source of the event is the object
    \a o and the event is \a e.
*/

bool QFontDialog::eventFilter(QObject *o , QEvent *e)
{
    Q_D(QFontDialog);
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *k = (QKeyEvent *)e;
        if (o == d->sizeEdit &&
        (k->key() == Qt::Key_Up ||
             k->key() == Qt::Key_Down ||
         k->key() == Qt::Key_PageUp ||
         k->key() == Qt::Key_PageDown)) {

            int ci = d->sizeList->currentItem();
            (void)QApplication::sendEvent(d->sizeList, k);

            if (ci != d->sizeList->currentItem()
                    && style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, 0, this))
                d->sizeEdit->selectAll();
            return true;
        } else if ((o == d->familyList || o == d->styleList) &&
                    (k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter)) {
            k->accept();
        accept();
            return true;
        }
    } else if (e->type() == QEvent::FocusIn
               && style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, 0, this)) {
        if (o == d->familyList)
            d->familyEdit->selectAll();
        else if (o == d->styleList)
            d->styleEdit->selectAll();
        else if (o == d->sizeList)
            d->sizeEdit->selectAll();
    } else if (e->type() == QEvent::MouseButtonPress && o == d->sizeList) {
            d->sizeEdit->setFocus();
    }
    return QDialog::eventFilter(o, e);
}

/*
    Updates the contents of the "font family" list box. This
    function can be reimplemented if you have special requirements.
*/

void QFontDialogPrivate::updateFamilies()
{
    Q_Q(QFontDialog);

    enum match_t { MATCH_NONE = 0, MATCH_LAST_RESORT = 1, MATCH_APP = 2, MATCH_FAMILY = 3 };

    QStringList familyNames = fdb.families();

    const bool scalable = (opts & QFontDialog::ScalableFonts);
    const bool mono = (opts & QFontDialog::MonospacedFonts);
    if (!scalable || !mono) {
        QMutableListIterator<QString> familyIter(familyNames);
        while (familyIter.hasNext()) {
            const QString family = familyIter.next();
            if (scalable && !fdb.isScalable(family)) {
                familyIter.remove();
                continue;
            }
            if (mono && !fdb.isFixedPitch(family)) {
                familyIter.remove();
            }
        }
    }

    familyList->model()->setStringList(familyNames);

    QString foundryName1, familyName1, foundryName2, familyName2;
    int bestFamilyMatch = -1;
    match_t bestFamilyType = MATCH_NONE;

    QFont f;

    // ##### do the right thing for a list of family names in the font.
    QFontDatabasePrivate::parseFontName(family, foundryName1, familyName1);

    QStringList::const_iterator it = familyNames.constBegin();
    int i = 0;
    for(; it != familyNames.constEnd(); ++it, ++i) {
        QFontDatabasePrivate::parseFontName(*it, foundryName2, familyName2);

        //try to match...
        if (familyName1 == familyName2) {
            bestFamilyType = MATCH_FAMILY;
            if (foundryName1 == foundryName2) {
                bestFamilyMatch = i;
                break;
            }
            if (bestFamilyMatch < MATCH_FAMILY)
                bestFamilyMatch = i;
        }

        //and try some fall backs
        match_t type = MATCH_NONE;
        if (bestFamilyType <= MATCH_NONE && familyName2 == f.lastResortFamily())
            type = MATCH_LAST_RESORT;
        if (bestFamilyType <= MATCH_LAST_RESORT && familyName2 == f.family())
            type = MATCH_APP;
        if (type != MATCH_NONE) {
            bestFamilyType = type;
            bestFamilyMatch = i;
        }
    }

    if (i != -1 && bestFamilyType != MATCH_NONE)
        familyList->setCurrentItem(bestFamilyMatch);
    else
        familyList->setCurrentItem(0);
    familyEdit->setText(familyList->currentText());
    if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, 0, q)
        && familyList->hasFocus()) {
        familyEdit->selectAll();
    }

    updateStyles();
}

/*
    Updates the contents of the "font style" list box. This
    function can be reimplemented if you have special requirements.
*/
void QFontDialogPrivate::updateStyles()
{
    Q_Q(QFontDialog);
    QStringList styles = fdb.styles(familyList->currentText());
    styleList->model()->setStringList(styles);

    if (styles.isEmpty()) {
        styleEdit->clear();
        smoothScalable = false;
    } else {
        if (!style.isEmpty()) {
            bool found = false;
            bool first = true;
            QString cstyle = style;

        redo:
            for (int i = 0; i < (int)styleList->count(); i++) {
                if (cstyle == styleList->text(i)) {
                     styleList->setCurrentItem(i);
                     found = true;
                     break;
                 }
            }
            if (!found && first) {
                if (cstyle.contains(QLatin1String("Italic"))) {
                    cstyle.replace(QLatin1String("Italic"), QLatin1String("Oblique"));
                    first = false;
                    goto redo;
                } else if (cstyle.contains(QLatin1String("Oblique"))) {
                    cstyle.replace(QLatin1String("Oblique"), QLatin1String("Italic"));
                    first = false;
                    goto redo;
                }
            }
            if (!found)
                styleList->setCurrentItem(0);
        } else {
            styleList->setCurrentItem(0);
        }

        styleEdit->setText(styleList->currentText());
        if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, 0, q)
                && styleList->hasFocus())
            styleEdit->selectAll();

        smoothScalable = fdb.isScalable(familyList->currentText(), styleList->currentText());
    }

    updateSizes();
}

/*!
    \internal
    Updates the contents of the "font size" list box. This
  function can be reimplemented if you have special requirements.
*/

void QFontDialogPrivate::updateSizes()
{
    Q_Q(QFontDialog);

    if (!familyList->currentText().isEmpty()) {
        QList<int> sizes = fdb.pointSizes(familyList->currentText(), styleList->currentText());

        int i = 0;
        int current = -1;
        QStringList str_sizes;
        for(QList<int>::const_iterator it = sizes.constBegin(); it != sizes.constEnd(); ++it) {
            str_sizes.append(QString::number(*it));
            if (current == -1 && *it >= size)
                current = i;
            ++i;
        }
        sizeList->model()->setStringList(str_sizes);
        if (current == -1) {
            // we request a size bigger than the ones in the list, select the biggest one
            current = sizeList->count() - 1;
        }
        sizeList->setCurrentItem(current);

        sizeEdit->blockSignals(true);
        sizeEdit->setText((smoothScalable ? QString::number(size) : sizeList->currentText()));
        if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, 0, q)
                && sizeList->hasFocus())
            sizeEdit->selectAll();
        sizeEdit->blockSignals(false);
    } else {
        sizeEdit->clear();
    }

    _q_updateSample();
}

void QFontDialogPrivate::_q_updateSample()
{
    // compute new font
    int pSize = sizeEdit->text().toInt();
    QFont newFont(fdb.font(familyList->currentText(), style, pSize));
    newFont.setStrikeOut(strikeout->isChecked());
    newFont.setUnderline(underline->isChecked());

    if (familyList->currentText().isEmpty())
        sampleEdit->clear();

    updateSampleFont(newFont);
}

void QFontDialogPrivate::updateSampleFont(const QFont &newFont)
{
    Q_Q(QFontDialog);
    if (newFont != sampleEdit->font()) {
        sampleEdit->setFont(newFont);
        emit q->currentFontChanged(newFont);
    }
}

/*!
    \internal
*/
void QFontDialogPrivate::_q_familyHighlighted(int i)
{
    Q_Q(QFontDialog);
    family = familyList->text(i);
    familyEdit->setText(family);
    if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, 0, q)
            && familyList->hasFocus())
        familyEdit->selectAll();

    updateStyles();
}


/*!
    \internal
*/

void QFontDialogPrivate::_q_styleHighlighted(int index)
{
    Q_Q(QFontDialog);
    QString s = styleList->text(index);
    styleEdit->setText(s);
    if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, 0, q)
            && styleList->hasFocus())
        styleEdit->selectAll();

    style = s;

    updateSizes();
}


/*!
    \internal
*/

void QFontDialogPrivate::_q_sizeHighlighted(int index)
{
    Q_Q(QFontDialog);
    QString s = sizeList->text(index);
    sizeEdit->setText(s);
    if (q->style()->styleHint(QStyle::SH_FontDialog_SelectAssociatedText, 0, q)
            && sizeEdit->hasFocus())
        sizeEdit->selectAll();

    size = s.toInt();
    _q_updateSample();
}

/*!
    \internal
    This slot is called if the user changes the font size.
    The size is passed in the \a s argument as a \e string.
*/

void QFontDialogPrivate::_q_sizeChanged(const QString &s)
{
    // no need to check if the conversion is valid, since we have an QIntValidator in the size edit
    int size = s.toInt();
    if (this->size == size)
        return;

    this->size = size;
    if (sizeList->count() != 0) {
        int i;
        for (i = 0; i < sizeList->count() - 1; i++) {
            if (sizeList->text(i).toInt() >= this->size)
                break;
        }
        sizeList->blockSignals(true);
        sizeList->setCurrentItem(i);
        sizeList->blockSignals(false);
    }
    _q_updateSample();
}

void QFontDialogPrivate::retranslateStrings()
{
    familyAccel->setText(QFontDialog::tr("&Font"));
    styleAccel->setText(QFontDialog::tr("Font st&yle"));
    sizeAccel->setText(QFontDialog::tr("&Size"));
    // Removed the title due to lack of screen estate in small S60 screen.
    // The effects are descriptive without a title (strikeout, underline).
    effects->setTitle(QFontDialog::tr("Effects"));
    strikeout->setText(QFontDialog::tr("Stri&keout"));
    underline->setText(QFontDialog::tr("&Underline"));
    sample->setTitle(QFontDialog::tr("Sample"));
}

/*!
    \reimp
*/
void QFontDialog::changeEvent(QEvent *e)
{
    Q_D(QFontDialog);
    if (e->type() == QEvent::LanguageChange) {
        d->retranslateStrings();
    }
    QDialog::changeEvent(e);
}

/*!
    \since 4.5

    \property QFontDialog::currentFont
    \brief the current font of the dialog.
*/

/*!
    \since 4.5

    Sets the font highlighted in the QFontDialog to the given \a font.

    \sa selectedFont()
*/
void QFontDialog::setCurrentFont(const QFont &font)
{
    Q_D(QFontDialog);
    d->family = font.family();
    d->style = d->fdb.styleString(font);
    d->size = font.pointSize();
    if (d->size == -1) {
        QFont sf = d->fdb.font(font.family(), font.styleName(), font.pointSize());
        d->size = sf.pointSize();
    }
    d->strikeout->setChecked(font.strikeOut());
    d->underline->setChecked(font.underline());
    d->updateFamilies();
}

/*!
    \since 4.5

    Returns the current font.

    \sa selectedFont()
*/
QFont QFontDialog::currentFont() const
{
    Q_D(const QFontDialog);
    return d->sampleEdit->font();
}

/*!
    Returns the font that the user selected by clicking the \gui{OK}
    or equivalent button.

    \note This font is not always the same as the font held by the
    \l currentFont property since the user can choose different fonts
    before finally selecting the one to use.
*/
QFont QFontDialog::selectedFont() const
{
    Q_D(const QFontDialog);
    return d->selectedFont;
}

/*!
    \enum QFontDialog::FontDialogOption
    \since 4.5

    This enum specifies various options that affect the look and feel
    of a font dialog.

    \value NoButtons Don't display \gui{OK} and \gui{Cancel} buttons. (Useful for "live dialogs".)
    \value ScalableFonts Show scalable fonts
    \value MonospacedFonts Show monospaced fonts
    \value AllFonts Show scalable and monospaced fonts

    \sa options, setOption(), testOption()
*/

/*!
    Sets the given \a option to be enabled if \a on is true;
    otherwise, clears the given \a option.

    \sa options, testOption()
*/
void QFontDialog::setOption(FontDialogOption option, bool on)
{
    Q_D(QFontDialog);
    if (!(d->opts & option) != !on) {
        setOptions(d->opts ^ option);
    }
}

/*!
    Returns true if the given \a option is enabled; otherwise, returns
    false.

    \sa options, setOption()
*/
bool QFontDialog::testOption(FontDialogOption option) const
{
    Q_D(const QFontDialog);
    return (d->opts & option);
}

/*!
    \property QFontDialog::options
    \brief the various options that affect the look and feel of the dialog
    \since 4.5

    By default, QFontDialog::AllFonts is set.

    Options should be set before showing the dialog. Setting them while the
    dialog is visible is not guaranteed to have an immediate effect on the
    dialog (depending on the option and on the platform).

    \sa setOption(), testOption()
*/
void QFontDialog::setOptions(FontDialogOptions options)
{
    Q_D(QFontDialog);
    if (d->opts == options) {
        return;
    }
    d->opts = options;
    d->buttonBox->setVisible(!(options & NoButtons));
    d->updateFamilies();
}

QFontDialog::FontDialogOptions QFontDialog::options() const
{
    Q_D(const QFontDialog);
    return d->opts;
}

/*!
    \since 4.5
    \overload

    Opens the dialog and connects its fontSelected() signal to the slot specified
    by \a receiver and \a member.

    The signal will be disconnected from the slot when the dialog is closed.
*/
void QFontDialog::open(QObject *receiver, const char *member)
{
    Q_D(QFontDialog);
    connect(this, SIGNAL(fontSelected(QFont)), receiver, member);
    d->receiverToDisconnectOnClose = receiver;
    d->memberToDisconnectOnClose = member;
    QDialog::open();
}

/*!
    \since 4.5

    \fn void QFontDialog::currentFontChanged(const QFont &font)

    This signal is emitted when the current font is changed. The new font is
    specified in \a font.

    The signal is emitted while a user is selecting a font. Ultimately, the
    chosen font may differ from the font currently selected.

    \sa currentFont, fontSelected(), selectedFont()
*/

/*!
    \since 4.5

    \fn void QFontDialog::fontSelected(const QFont &font)

    This signal is emitted when a font has been selected. The selected font is
    specified in \a font.

    The signal is only emitted when a user has chosen the final font to be
    used. It is not emitted while the user is changing the current font in the
    font dialog.

    \sa selectedFont(), currentFontChanged(), currentFont
*/

/*!
  Closes the dialog and sets its result code to \a result. If this dialog
  is shown with exec(), done() causes the local event loop to finish,
  and exec() to return \a result.

  \sa QDialog::done()
*/
void QFontDialog::done(int result)
{
    Q_D(QFontDialog);
    QDialog::done(result);
    if (result == Accepted) {
        // We check if this is the same font we had before, if so we emit currentFontChanged
        QFont selectedFont = currentFont();
        if (selectedFont != d->selectedFont) {
            emit(currentFontChanged(selectedFont));
        }
        d->selectedFont = selectedFont;
        emit fontSelected(d->selectedFont);
    } else
        d->selectedFont = QFont();
    if (d->receiverToDisconnectOnClose) {
        disconnect(this, SIGNAL(fontSelected(QFont)),
                   d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
        d->receiverToDisconnectOnClose = 0;
    }
    d->memberToDisconnectOnClose.clear();
}

/*!
    \fn QFont QFontDialog::getFont(bool *ok, const QFont &initial, QWidget* parent, const char* name)
    \since 4.5

    Call getFont(\a ok, \a initial, \a parent) instead.

    \warning Do not delete \a parent during the execution of the dialog.
             If you want to do this, you should create the dialog
             yourself using one of the QFontDialog constructors.

    The \a name parameter is ignored.
*/

/*!
    \fn QFont QFontDialog::getFont(bool *ok, QWidget* parent, const char* name)

    Call getFont(\a ok, \a parent) instead.

  \warning Do not delete \a parent during the execution of the dialog.
           If you want to do this, you should create the dialog
           yourself using one of the QFontDialog constructors.

    The \a name parameter is ignored.
*/

QT_END_NAMESPACE

#include "moc_qfontdialog.cpp"
#include "moc_qfontdialog.h"


#endif // QT_NO_FONTDIALOG
