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

#include "qcolordialog_p.h"

#ifndef QT_NO_COLORDIALOG

#include "qapplication.h"
#include "qdesktopwidget.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qimage.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qmenu.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qpushbutton.h"
#include "qsettings.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qvalidator.h"
#include "qmimedata.h"
#include "qspinbox.h"
#include "qdialogbuttonbox.h"
#include "qguiplatformplugin.h"
#include "qcoreapplication_p.h"
#include "qguicommon_p.h"

QT_BEGIN_NAMESPACE

//////////// QWellArray BEGIN

class QWellArray : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int selectedColumn READ selectedColumn)
    Q_PROPERTY(int selectedRow READ selectedRow)

public:
    QWellArray(int rows, int cols, QWidget* parent = nullptr);
    ~QWellArray() {}

    int selectedColumn() const { return selCol; }
    int selectedRow() const { return selRow; }

    void setCurrent(int row, int col);
    void setSelected(int row, int col);

    QSize sizeHint() const;

    inline int cellWidth() const
        { return cellw; }

    inline int cellHeight() const
        { return cellh; }

    inline int rowAt(int y) const
        { return y / cellh; }

    inline int columnAt(int x) const
        { if (isRightToLeft()) return ncols - (x / cellw) - 1; return x / cellw; }

    inline int rowY(int row) const
        { return cellh * row; }

    inline int columnX(int column) const
        { if (isRightToLeft()) return cellw * (ncols - column - 1); return cellw * column; }

    inline int numRows() const
        { return nrows; }

    inline int numCols() const
        {return ncols; }

    inline QRect cellRect() const
        { return QRect(0, 0, cellw, cellh); }

    inline QSize gridSize() const
        { return QSize(ncols * cellw, nrows * cellh); }

    QRect cellGeometry(int row, int column)
        {
            QRect r;
            if (row >= 0 && row < nrows && column >= 0 && column < ncols)
                r.setRect(columnX(column), rowY(row), cellw, cellh);
            return r;
        }

    inline void updateCell(int row, int column) { update(cellGeometry(row, column)); }

signals:
    void selected(int row, int col);

protected:
    void paintCell(QPainter *, int row, int col, const QRect&);
    virtual void paintCellContents(QPainter *, int row, int col, const QRect&);

    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);
    void paintEvent(QPaintEvent *);

private:
    Q_DISABLE_COPY(QWellArray)

    int nrows;
    int ncols;
    int cellw;
    int cellh;
    int curRow;
    int curCol;
    int selRow;
    int selCol;
};

void QWellArray::paintEvent(QPaintEvent *e)
{
    QRect r = e->rect();
    int cx = r.x();
    int cy = r.y();
    int ch = r.height();
    int cw = r.width();
    int colfirst = columnAt(cx);
    int collast = columnAt(cx + cw);
    int rowfirst = rowAt(cy);
    int rowlast = rowAt(cy + ch);

    if (isRightToLeft()) {
        int t = colfirst;
        colfirst = collast;
        collast = t;
    }

    QPainter painter(this);
    QPainter *p = &painter;
    QRect rect(0, 0, cellWidth(), cellHeight());


    if (collast < 0 || collast >= ncols)
        collast = ncols-1;
    if (rowlast < 0 || rowlast >= nrows)
        rowlast = nrows-1;

    // Go through the rows
    for (int r = rowfirst; r <= rowlast; ++r) {
        // get row position and height
        int rowp = rowY(r);

        // Go through the columns in the row r
        // if we know from where to where, go through [colfirst, collast],
        // else go through all of them
        for (int c = colfirst; c <= collast; ++c) {
            // get position and width of column c
            int colp = columnX(c);
            // Translate painter and draw the cell
            rect.translate(colp, rowp);
            paintCell(p, r, c, rect);
            rect.translate(-colp, -rowp);
        }
    }
}

QWellArray::QWellArray(int rows, int cols, QWidget *parent)
    : QWidget(parent), nrows(rows), ncols(cols)
{
    setFocusPolicy(Qt::StrongFocus);
    cellw = 28;
    cellh = 24;
    curCol = 0;
    curRow = 0;
    selCol = -1;
    selRow = -1;
}

QSize QWellArray::sizeHint() const
{
    ensurePolished();
    return gridSize().boundedTo(QSize(640, 480));
}


void QWellArray::paintCell(QPainter* p, int row, int col, const QRect &rect)
{
    static const int b = 3; //margin

    const QPalette & g = palette();
    QStyleOptionFrame opt;
    int dfw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    opt.lineWidth = dfw;
    opt.midLineWidth = 1;
    opt.rect = rect.adjusted(b, b, -b, -b);
    opt.palette = g;
    opt.state = QStyle::State_Enabled | QStyle::State_Sunken;
    style()->drawPrimitive(QStyle::PE_Frame, &opt, p, this);

    if ((row == curRow) && (col == curCol)) {
        if (hasFocus()) {
            QStyleOptionFocusRect opt;
            opt.palette = g;
            opt.rect = rect;
            opt.state = QStyle::State_None | QStyle::State_KeyboardFocusChange;
            style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, p, this);
        }
    }
    paintCellContents(p, row, col, opt.rect.adjusted(dfw, dfw, -dfw, -dfw));
}

/*!
  Reimplement this function to change the contents of the well array.
 */
void QWellArray::paintCellContents(QPainter *p, int row, int col, const QRect &r)
{
    Q_UNUSED(row);
    Q_UNUSED(col);
    p->fillRect(r, Qt::white);
    p->setPen(Qt::black);
    p->drawLine(r.topLeft(), r.bottomRight());
    p->drawLine(r.topRight(), r.bottomLeft());
}

void QWellArray::mousePressEvent(QMouseEvent *e)
{
    // The current cell marker is set to the cell the mouse is pressed in
    QPoint pos = e->pos();
    setCurrent(rowAt(pos.y()), columnAt(pos.x()));
}

void QWellArray::mouseReleaseEvent(QMouseEvent * /* event */)
{
    // The current cell marker is set to the cell the mouse is clicked in
    setSelected(curRow, curCol);
}


/*
  Sets the cell currently having the focus. This is not necessarily
  the same as the currently selected cell.
*/

void QWellArray::setCurrent(int row, int col)
{
    if ((curRow == row) && (curCol == col))
        return;

    if (row < 0 || col < 0)
        row = col = -1;

    int oldRow = curRow;
    int oldCol = curCol;

    curRow = row;
    curCol = col;

    updateCell(oldRow, oldCol);
    updateCell(curRow, curCol);
}

/*
  Sets the currently selected cell to \a row, \a column. If \a row or
  \a column are less than zero, the current cell is unselected.

  Does not set the position of the focus indicator.
*/
void QWellArray::setSelected(int row, int col)
{
    int oldRow = selRow;
    int oldCol = selCol;

    if (row < 0 || col < 0)
        row = col = -1;

    selCol = col;
    selRow = row;

    updateCell(oldRow, oldCol);
    updateCell(selRow, selCol);
    if (row >= 0)
        emit selected(row, col);

#ifndef QT_NO_MENU
    if (isVisible() && qobject_cast<QMenu*>(parentWidget()))
        parentWidget()->close();
#endif
}

void QWellArray::focusInEvent(QFocusEvent*)
{
    updateCell(curRow, curCol);
}

/*!\reimp
*/

void QWellArray::focusOutEvent(QFocusEvent*)
{
    updateCell(curRow, curCol);
}

/*\reimp
*/
void QWellArray::keyPressEvent(QKeyEvent* e)
{
    switch(e->key()) {                        // Look at the key code
    case Qt::Key_Left:                                // If 'left arrow'-key,
        if(curCol > 0)                        // and cr't not in leftmost col
            setCurrent(curRow, curCol - 1);        // set cr't to next left column
        break;
    case Qt::Key_Right:                                // Correspondingly...
        if(curCol < numCols()-1)
            setCurrent(curRow, curCol + 1);
        break;
    case Qt::Key_Up:
        if(curRow > 0)
            setCurrent(curRow - 1, curCol);
        break;
    case Qt::Key_Down:
        if(curRow < numRows()-1)
            setCurrent(curRow + 1, curCol);
        break;
#if 0
    // bad idea that shouldn't have been implemented; very counterintuitive
    case Qt::Key_Return:
    case Qt::Key_Enter:
        /*
          ignore the key, so that the dialog get it, but still select
          the current row/col
        */
        e->ignore();
        // fallthrough intended
#endif
    case Qt::Key_Space:
        setSelected(curRow, curCol);
        break;
    default:                                // If not an interesting key,
        e->ignore();                        // we don't accept the event
        return;
    }

}

//////////// QWellArray END

static bool initrgb = false;
static QRgb stdrgb[6*8];
static QRgb cusrgb[2*8];


static void initRGB()
{
    if (initrgb)
        return;
    initrgb = true;
    int i = 0;
    for (int g = 0; g < 4; g++)
        for (int r = 0;  r < 4; r++)
            for (int b = 0; b < 3; b++)
                stdrgb[i++] = qRgb(r * 255 / 3, g * 255 / 3, b * 255 / 2);

    for (i = 0; i < 2*8; i++)
        cusrgb[i] = 0xffffffff;
}

/*!
    Returns the number of custom colors supported by QColorDialog. All
    color dialogs share the same custom colors.
*/
int QColorDialog::customCount()
{
    return 2 * 8;
}

/*!
    \since 4.5

    Returns the custom color at the given \a index as a QRgb value.
*/
QRgb QColorDialog::customColor(int index)
{
    if (index >= customCount())
        return qt_whitergb;
    initRGB();
    return cusrgb[index];
}

/*!
    Sets the custom color at \a index to the QRgb \a color value.
*/
void QColorDialog::setCustomColor(int index, QRgb color)
{
    if (index >= customCount())
        return;
    initRGB();
    cusrgb[index] = color;
}

/*!
    Sets the standard color at \a  index to the QRgb \a color value.
*/

void QColorDialog::setStandardColor(int index, QRgb color)
{
    if (index >= int(6 * 8))
        return;
    initRGB();
    stdrgb[index] = color;
}

static inline void rgb2hsv(QRgb rgb, int &h, int &s, int &v)
{
    QColor c;
    c.setRgb(rgb);
    c.getHsv(&h, &s, &v);
}

class QColorWell : public QWellArray
{
public:
    QColorWell(QWidget *parent, int r, int c, QRgb *vals)
        :QWellArray(r, c, parent), values(vals), mousePressed(false), oldCurrent(-1, -1)
    { setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum)); }

protected:
    void paintCellContents(QPainter *, int row, int col, const QRect&);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent(QDragEnterEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);
#endif

private:
    QRgb *values;
    bool mousePressed;
    QPoint pressPos;
    QPoint oldCurrent;

};

void QColorWell::paintCellContents(QPainter *p, int row, int col, const QRect &r)
{
    int i = row + col*numRows();
    p->fillRect(r, QColor(values[i]));
}

void QColorWell::mousePressEvent(QMouseEvent *e)
{
    oldCurrent = QPoint(selectedRow(), selectedColumn());
    QWellArray::mousePressEvent(e);
    mousePressed = true;
    pressPos = e->pos();
}

void QColorWell::mouseMoveEvent(QMouseEvent *e)
{
    QWellArray::mouseMoveEvent(e);
#ifndef QT_NO_DRAGANDDROP
    if (!mousePressed)
        return;
    if ((pressPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
        setCurrent(oldCurrent.x(), oldCurrent.y());
        int i = rowAt(pressPos.y()) + columnAt(pressPos.x()) * numRows();
        QColor col(values[i]);
        QMimeData *mime = new QMimeData;
        mime->setColorData(col);
        QPixmap pix(cellWidth(), cellHeight());
        pix.fill(col);
        QPainter p(&pix);
        p.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
        p.end();
        QDrag *drg = new QDrag(this);
        drg->setMimeData(mime);
        drg->setPixmap(pix);
        mousePressed = false;
        drg->start();
    }
#endif
}

#ifndef QT_NO_DRAGANDDROP
void QColorWell::dragEnterEvent(QDragEnterEvent *e)
{
    if (qvariant_cast<QColor>(e->mimeData()->colorData()).isValid())
        e->accept();
    else
        e->ignore();
}

void QColorWell::dragLeaveEvent(QDragLeaveEvent *)
{
    if (hasFocus())
        parentWidget()->setFocus();
}

void QColorWell::dragMoveEvent(QDragMoveEvent *e)
{
    if (qvariant_cast<QColor>(e->mimeData()->colorData()).isValid()) {
        setCurrent(rowAt(e->pos().y()), columnAt(e->pos().x()));
        e->accept();
    } else {
        e->ignore();
    }
}

void QColorWell::dropEvent(QDropEvent *e)
{
    QColor col = qvariant_cast<QColor>(e->mimeData()->colorData());
    if (col.isValid()) {
        int i = rowAt(e->pos().y()) + columnAt(e->pos().x()) * numRows();
        values[i] = col.rgb();
        update();
        e->accept();
    } else {
        e->ignore();
    }
}

#endif // QT_NO_DRAGANDDROP

void QColorWell::mouseReleaseEvent(QMouseEvent *e)
{
    if (!mousePressed)
        return;
    QWellArray::mouseReleaseEvent(e);
    mousePressed = false;
}

class QColorPicker : public QFrame
{
    Q_OBJECT
public:
    QColorPicker(QWidget* parent);

public slots:
    void setCol(int h, int s);

signals:
    void newCol(int h, int s);

protected:
    QSize sizeHint() const;
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);

private:
    int hue;
    int sat;

    QPoint colPt();
    int huePt(const QPoint &pt);
    int satPt(const QPoint &pt);
    void setCol(const QPoint &pt);

    QPixmap pix;
};

static int pWidth = 220;
static int pHeight = 200;

class QColorLuminancePicker : public QWidget
{
    Q_OBJECT
public:
    QColorLuminancePicker(QWidget* parent = nullptr);
    ~QColorLuminancePicker();

public slots:
    void setCol(int h, int s, int v);
    void setCol(int h, int s);

signals:
    void newHsv(int h, int s, int v);

protected:
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    enum { foff = 3, coff = 4 }; //frame and contents offset
    int val;
    int hue;
    int sat;

    int y2val(int y);
    int val2y(int val);
    void setVal(int v);

    QPixmap *pix;
};


int QColorLuminancePicker::y2val(int y)
{
    int d = height() - 2*coff - 1;
    return 255 - (y - coff)*255/d;
}

int QColorLuminancePicker::val2y(int v)
{
    int d = height() - 2*coff - 1;
    return coff + (255-v)*d/255;
}

QColorLuminancePicker::QColorLuminancePicker(QWidget* parent)
    :QWidget(parent)
{
    hue = 100; val = 100; sat = 100;
    pix = 0;
    //    setAttribute(WA_NoErase, true);
}

QColorLuminancePicker::~QColorLuminancePicker()
{
    delete pix;
}

void QColorLuminancePicker::mouseMoveEvent(QMouseEvent *m)
{
    setVal(y2val(m->y()));
}
void QColorLuminancePicker::mousePressEvent(QMouseEvent *m)
{
    setVal(y2val(m->y()));
}

void QColorLuminancePicker::setVal(int v)
{
    if (val == v)
        return;
    val = qMax(0, qMin(v,255));
    delete pix; pix=0;
    repaint();
    emit newHsv(hue, sat, val);
}

//receives from a hue,sat chooser and relays.
void QColorLuminancePicker::setCol(int h, int s)
{
    setCol(h, s, val);
    emit newHsv(h, s, val);
}

void QColorLuminancePicker::paintEvent(QPaintEvent *)
{
    int w = width() - 5;

    QRect r(0, foff, w, height() - 2*foff);
    int wi = r.width() - 2;
    int hi = r.height() - 2;
    if (!pix || pix->height() != hi || pix->width() != wi) {
        delete pix;
        QImage img(wi, hi, QImage::Format_RGB32);
        int y;
        uint *pixel = (uint *) img.scanLine(0);
        for (y = 0; y < hi; y++) {
            const uint *end = pixel + wi;
            while (pixel < end) {
                QColor c;
                c.setHsv(hue, sat, y2val(y+coff));
                *pixel = c.rgb();
                ++pixel;
            }
        }
        pix = new QPixmap(QPixmap::fromImage(img));
    }
    QPainter p(this);
    p.drawPixmap(1, coff, *pix);
    const QPalette &g = palette();
    qDrawShadePanel(&p, r, g, true);
    p.setPen(g.foreground().color());
    p.setBrush(g.foreground());
    QPolygon a;
    int y = val2y(val);
    a.setPoints(3, w, y, w+5, y+5, w+5, y-5);
    p.eraseRect(w, 0, 5, height());
    p.drawPolygon(a);
}

void QColorLuminancePicker::setCol(int h, int s , int v)
{
    val = v;
    hue = h;
    sat = s;
    delete pix; pix=0;
    repaint();
}

QPoint QColorPicker::colPt()
{
    QRect r = contentsRect();
    return QPoint((360 - hue) * (r.width() - 1) / 360, (255 - sat) * (r.height() - 1) / 255);
}

int QColorPicker::huePt(const QPoint &pt)
{
    QRect r = contentsRect();
    return 360 - pt.x() * 360 / (r.width() - 1);
}

int QColorPicker::satPt(const QPoint &pt)
{
    QRect r = contentsRect();
    return 255 - pt.y() * 255 / (r.height() - 1);
}

void QColorPicker::setCol(const QPoint &pt)
{
    setCol(huePt(pt), satPt(pt));
}

QColorPicker::QColorPicker(QWidget* parent)
    : QFrame(parent)
{
    hue = 0; sat = 0;
    setCol(150, 255);

    setAttribute(Qt::WA_NoSystemBackground);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
}

QSize QColorPicker::sizeHint() const
{
    return QSize(pWidth + 2*frameWidth(), pHeight + 2*frameWidth());
}

void QColorPicker::setCol(int h, int s)
{
    int nhue = qMin(qMax(0,h), 359);
    int nsat = qMin(qMax(0,s), 255);
    if (nhue == hue && nsat == sat)
        return;

    QRect r(colPt(), QSize(20,20));
    hue = nhue; sat = nsat;
    r = r.united(QRect(colPt(), QSize(20,20)));
    r.translate(contentsRect().x()-9, contentsRect().y()-9);
    //    update(r);
    repaint(r);
}

void QColorPicker::mouseMoveEvent(QMouseEvent *m)
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol(p);
    emit newCol(hue, sat);
}

void QColorPicker::mousePressEvent(QMouseEvent *m)
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol(p);
    emit newCol(hue, sat);
}

void QColorPicker::paintEvent(QPaintEvent* )
{
    QPainter p(this);
    drawFrame(&p);
    QRect r = contentsRect();

    p.drawPixmap(r.topLeft(), pix);
    QPoint pt = colPt() + r.topLeft();
    p.setPen(Qt::black);

    p.fillRect(pt.x()-9, pt.y(), 20, 2, Qt::black);
    p.fillRect(pt.x(), pt.y()-9, 2, 20, Qt::black);

}

void QColorPicker::resizeEvent(QResizeEvent *ev)
{
    QFrame::resizeEvent(ev);

    int w = width() - frameWidth() * 2;
    int h = height() - frameWidth() * 2;
    QImage img(w, h, QImage::Format_RGB32);
    int x, y;
    uint *pixel = (uint *) img.scanLine(0);
    for (y = 0; y < h; y++) {
        const uint *end = pixel + w;
        x = 0;
        while (pixel < end) {
            QPoint p(x, y);
            QColor c;
            c.setHsv(huePt(p), satPt(p), 200);
            *pixel = c.rgb();
            ++pixel;
            ++x;
        }
    }
    pix = QPixmap::fromImage(img);
}


class QColSpinBox : public QSpinBox
{
public:
    QColSpinBox(QWidget *parent)
        : QSpinBox(parent) { setRange(0, 255); }
    void setValue(int i) {
        bool block = signalsBlocked();
        blockSignals(true);
        QSpinBox::setValue(i);
        blockSignals(block);
    }
};

class QColorShowLabel;

class QColorShower : public QWidget
{
    Q_OBJECT
public:
    QColorShower(QColorDialog *parent);

    //things that don't emit signals
    void setHsv(int h, int s, int v);

    int currentAlpha() const
    { return (colorDialog->options() & QColorDialog::ShowAlphaChannel) ? alphaEd->value() : 255; }
    void setCurrentAlpha(int a) { alphaEd->setValue(a); rgbEd(); }
    void showAlpha(bool b);
    bool isAlphaVisible() const;

    QRgb currentColor() const { return curCol; }
    QColor currentQColor() const { return curQColor; }
    void retranslateStrings();
    void updateQColor();

public slots:
    void setRgb(QRgb rgb);

signals:
    void newCol(QRgb rgb);
    void currentColorChanged(const QColor &color);

private slots:
    void rgbEd();
    void hsvEd();
private:
    void showCurrentColor();
    int hue, sat, val;
    QRgb curCol;
    QColor curQColor;
    QLabel *lblHue;
    QLabel *lblSat;
    QLabel *lblVal;
    QLabel *lblRed;
    QLabel *lblGreen;
    QLabel *lblBlue;
    QColSpinBox *hEd;
    QColSpinBox *sEd;
    QColSpinBox *vEd;
    QColSpinBox *rEd;
    QColSpinBox *gEd;
    QColSpinBox *bEd;
    QColSpinBox *alphaEd;
    QLabel *alphaLab;
    QColorShowLabel *lab;
    QColorDialog *colorDialog;

    friend class QColorDialog;
    friend class QColorDialogPrivate;
};

class QColorShowLabel : public QFrame
{
    Q_OBJECT

public:
    QColorShowLabel(QWidget *parent) : QFrame(parent) {
        setFrameStyle(QFrame::Panel|QFrame::Sunken);
        setAcceptDrops(true);
        mousePressed = false;
    }
    void setColor(QColor c) { col = c; }

signals:
    void colorDropped(QRgb);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent(QDragEnterEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
#endif

private:
    QColor col;
    bool mousePressed;
    QPoint pressPos;
};

void QColorShowLabel::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    drawFrame(&p);
    p.fillRect(contentsRect()&e->rect(), col);
}

void QColorShower::showAlpha(bool b)
{
    alphaLab->setVisible(b);
    alphaEd->setVisible(b);
}

inline bool QColorShower::isAlphaVisible() const
{
    return alphaLab->isVisible();
}

void QColorShowLabel::mousePressEvent(QMouseEvent *e)
{
    mousePressed = true;
    pressPos = e->pos();
}

void QColorShowLabel::mouseMoveEvent(QMouseEvent *e)
{
#ifdef QT_NO_DRAGANDDROP
    Q_UNUSED(e);
#else
    if (!mousePressed)
        return;
    if ((pressPos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
        QMimeData *mime = new QMimeData;
        mime->setColorData(col);
        QPixmap pix(30, 20);
        pix.fill(col);
        QPainter p(&pix);
        p.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
        p.end();
        QDrag *drg = new QDrag(this);
        drg->setMimeData(mime);
        drg->setPixmap(pix);
        mousePressed = false;
        drg->start();
    }
#endif
}

#ifndef QT_NO_DRAGANDDROP
void QColorShowLabel::dragEnterEvent(QDragEnterEvent *e)
{
    if (qvariant_cast<QColor>(e->mimeData()->colorData()).isValid())
        e->accept();
    else
        e->ignore();
}

void QColorShowLabel::dragLeaveEvent(QDragLeaveEvent *)
{
}

void QColorShowLabel::dropEvent(QDropEvent *e)
{
    QColor color = qvariant_cast<QColor>(e->mimeData()->colorData());
    if (color.isValid()) {
        col = color;
        repaint();
        emit colorDropped(col.rgb());
        e->accept();
    } else {
        e->ignore();
    }
}
#endif // QT_NO_DRAGANDDROP

void QColorShowLabel::mouseReleaseEvent(QMouseEvent *)
{
    if (!mousePressed)
        return;
    mousePressed = false;
}

QColorShower::QColorShower(QColorDialog *parent)
    : QWidget(parent)
{
    colorDialog = parent;

    curCol = qRgb(255, 255, 255);
    curQColor = Qt::white;

    QGridLayout *gl = new QGridLayout(this);
    gl->setMargin(gl->spacing());
    lab = new QColorShowLabel(this);

    lab->setMinimumWidth(60);

    gl->addWidget(lab, 0, 0, -1, 1);
    connect(lab, SIGNAL(colorDropped(QRgb)), this, SIGNAL(newCol(QRgb)));
    connect(lab, SIGNAL(colorDropped(QRgb)), this, SLOT(setRgb(QRgb)));

    hEd = new QColSpinBox(this);
    hEd->setRange(0, 359);
    lblHue = new QLabel(this);
#ifndef QT_NO_SHORTCUT
    lblHue->setBuddy(hEd);
#endif
    lblHue->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(lblHue, 0, 1);
    gl->addWidget(hEd, 0, 2);

    sEd = new QColSpinBox(this);
    lblSat = new QLabel(this);
#ifndef QT_NO_SHORTCUT
    lblSat->setBuddy(sEd);
#endif
    lblSat->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(lblSat, 1, 1);
    gl->addWidget(sEd, 1, 2);

    vEd = new QColSpinBox(this);
    lblVal = new QLabel(this);
#ifndef QT_NO_SHORTCUT
    lblVal->setBuddy(vEd);
#endif
    lblVal->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(lblVal, 2, 1);
    gl->addWidget(vEd, 2, 2);

    rEd = new QColSpinBox(this);
    lblRed = new QLabel(this);
#ifndef QT_NO_SHORTCUT
    lblRed->setBuddy(rEd);
#endif
    lblRed->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(lblRed, 0, 3);
    gl->addWidget(rEd, 0, 4);

    gEd = new QColSpinBox(this);
    lblGreen = new QLabel(this);
#ifndef QT_NO_SHORTCUT
    lblGreen->setBuddy(gEd);
#endif
    lblGreen->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(lblGreen, 1, 3);
    gl->addWidget(gEd, 1, 4);

    bEd = new QColSpinBox(this);
    lblBlue = new QLabel(this);
#ifndef QT_NO_SHORTCUT
    lblBlue->setBuddy(bEd);
#endif
    lblBlue->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(lblBlue, 2, 3);
    gl->addWidget(bEd, 2, 4);

    alphaEd = new QColSpinBox(this);
    alphaLab = new QLabel(this);
#ifndef QT_NO_SHORTCUT
    alphaLab->setBuddy(alphaEd);
#endif
    alphaLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gl->addWidget(alphaLab, 3, 1, 1, 3);
    gl->addWidget(alphaEd, 3, 4);
    alphaEd->hide();
    alphaLab->hide();

    connect(hEd, SIGNAL(valueChanged(int)), this, SLOT(hsvEd()));
    connect(sEd, SIGNAL(valueChanged(int)), this, SLOT(hsvEd()));
    connect(vEd, SIGNAL(valueChanged(int)), this, SLOT(hsvEd()));

    connect(rEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));
    connect(gEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));
    connect(bEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));
    connect(alphaEd, SIGNAL(valueChanged(int)), this, SLOT(rgbEd()));

    retranslateStrings();
}

inline QRgb QColorDialogPrivate::currentColor() const { return cs->currentColor(); }
inline int QColorDialogPrivate::currentAlpha() const { return cs->currentAlpha(); }
inline void QColorDialogPrivate::setCurrentAlpha(int a) { cs->setCurrentAlpha(a); }
inline void QColorDialogPrivate::showAlpha(bool b) { cs->showAlpha(b); }
inline bool QColorDialogPrivate::isAlphaVisible() const { return cs->isAlphaVisible(); }

QColor QColorDialogPrivate::currentQColor() const
{
    return cs->currentQColor();
}

void QColorShower::showCurrentColor()
{
    lab->setColor(currentColor());
    lab->repaint();
}

void QColorShower::rgbEd()
{
    curCol = qRgba(rEd->value(), gEd->value(), bEd->value(), currentAlpha());

    rgb2hsv(currentColor(), hue, sat, val);

    hEd->setValue(hue);
    sEd->setValue(sat);
    vEd->setValue(val);

    showCurrentColor();
    emit newCol(currentColor());
    updateQColor();
}

void QColorShower::hsvEd()
{
    hue = hEd->value();
    sat = sEd->value();
    val = vEd->value();

    QColor c;
    c.setHsv(hue, sat, val);
    curCol = c.rgb();

    rEd->setValue(qRed(currentColor()));
    gEd->setValue(qGreen(currentColor()));
    bEd->setValue(qBlue(currentColor()));

    showCurrentColor();
    emit newCol(currentColor());
    updateQColor();
}

void QColorShower::setRgb(QRgb rgb)
{
    curCol = rgb;

    rgb2hsv(currentColor(), hue, sat, val);

    hEd->setValue(hue);
    sEd->setValue(sat);
    vEd->setValue(val);

    rEd->setValue(qRed(currentColor()));
    gEd->setValue(qGreen(currentColor()));
    bEd->setValue(qBlue(currentColor()));

    showCurrentColor();
    updateQColor();
}

void QColorShower::setHsv(int h, int s, int v)
{
    if (h < -1 || (uint)s > 255 || (uint)v > 255)
        return;

    hue = h; val = v; sat = s;
    QColor c;
    c.setHsv(hue, sat, val);
    curCol = c.rgb();

    hEd->setValue(hue);
    sEd->setValue(sat);
    vEd->setValue(val);

    rEd->setValue(qRed(currentColor()));
    gEd->setValue(qGreen(currentColor()));
    bEd->setValue(qBlue(currentColor()));

    showCurrentColor();
    updateQColor();
}

void QColorShower::retranslateStrings()
{
    lblHue->setText(QColorDialog::tr("Hu&e:"));
    lblSat->setText(QColorDialog::tr("&Sat:"));
    lblVal->setText(QColorDialog::tr("&Val:"));
    lblRed->setText(QColorDialog::tr("&Red:"));
    lblGreen->setText(QColorDialog::tr("&Green:"));
    lblBlue->setText(QColorDialog::tr("Bl&ue:"));
    alphaLab->setText(QColorDialog::tr("A&lpha channel:"));
}

void QColorShower::updateQColor()
{
    QColor oldQColor(curQColor);
    curQColor.setRgba(qRgba(qRed(curCol), qGreen(curCol), qBlue(curCol), currentAlpha()));
    if (curQColor != oldQColor)
        emit currentColorChanged(curQColor);
}

//sets all widgets to display h,s,v
void QColorDialogPrivate::_q_newHsv(int h, int s, int v)
{
    cs->setHsv(h, s, v);
    cp->setCol(h, s);
    lp->setCol(h, s, v);
}

//sets all widgets to display rgb
void QColorDialogPrivate::setCurrentColor(QRgb rgb)
{
    cs->setRgb(rgb);
    _q_newColorTypedIn(rgb);
}

// hack; doesn't keep curCol in sync, so use with care
void QColorDialogPrivate::setCurrentQColor(const QColor &color)
{
    Q_Q(QColorDialog);
    if (cs->curQColor != color) {
        cs->curQColor = color;
        emit q->currentColorChanged(color);
    }
}

bool QColorDialogPrivate::selectColor(const QColor &col)
{
    QRgb color = col.rgb();
    int i = 0, j = 0;
    // Check standard colors
    if (standard) {
        for (i = 0; i < 6; i++) {
            for (j = 0; j < 8; j++) {
                if (color == stdrgb[i + j*6]) {
                    _q_newStandard(i, j);
                    standard->setCurrent(i, j);
                    standard->setSelected(i, j);
                    standard->setFocus();
                    return true;
                }
            }
        }
    }
    // Check custom colors
    if (custom) {
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 8; j++) {
                if (color == cusrgb[i + j*2]) {
                    _q_newCustom(i, j);
                    custom->setCurrent(i, j);
                    custom->setSelected(i, j);
                    custom->setFocus();
                    return true;
                }
            }
        }
    }
    return false;
}

//sets all widgets except cs to display rgb
void QColorDialogPrivate::_q_newColorTypedIn(QRgb rgb)
{
    int h, s, v;
    rgb2hsv(rgb, h, s, v);
    cp->setCol(h, s);
    lp->setCol(h, s, v);
}

void QColorDialogPrivate::_q_newCustom(int r, int c)
{
    int i = r+2*c;
    setCurrentColor(cusrgb[i]);
    nextCust = i;
    if (standard)
        standard->setSelected(-1,-1);
}

void QColorDialogPrivate::_q_newStandard(int r, int c)
{
    setCurrentColor(stdrgb[r+c*6]);
    if (custom)
        custom->setSelected(-1,-1);
}

void QColorDialogPrivate::init(const QColor &initial)
{
    Q_Q(QColorDialog);

    q->setSizeGripEnabled(false);
    q->setWindowTitle(QColorDialog::tr("Select Color"));

    nextCust = 0;
    QVBoxLayout *mainLay = new QVBoxLayout(q);
    // there's nothing in this dialog that benefits from sizing up
    mainLay->setSizeConstraint(QLayout::SetFixedSize);

    QHBoxLayout *topLay = new QHBoxLayout();
    mainLay->addLayout(topLay);

    leftLay = 0;

    // small displays (e.g. PDAs) cannot fit the full color dialog,
    // so just use the color picker.
    smallDisplay = (QApplication::desktop()->width() < 480 || QApplication::desktop()->height() < 350);
    const int lumSpace = topLay->spacing() / 2;

    if (!smallDisplay) {
        leftLay = new QVBoxLayout();
        topLay->addLayout(leftLay);
    }

    initRGB();

    if (!smallDisplay) {
        standard = new QColorWell(q, 6, 8, stdrgb);
        lblBasicColors = new QLabel(q);
#ifndef QT_NO_SHORTCUT
        lblBasicColors->setBuddy(standard);
#endif
        q->connect(standard, SIGNAL(selected(int,int)), SLOT(_q_newStandard(int,int)));
        leftLay->addWidget(lblBasicColors);
        leftLay->addWidget(standard);

        leftLay->addStretch();

        custom = new QColorWell(q, 2, 8, cusrgb);
        custom->setAcceptDrops(true);

        q->connect(custom, SIGNAL(selected(int,int)), SLOT(_q_newCustom(int,int)));
        lblCustomColors = new QLabel(q);
#ifndef QT_NO_SHORTCUT
        lblCustomColors->setBuddy(custom);
#endif
        leftLay->addWidget(lblCustomColors);
        leftLay->addWidget(custom);

        addCusBt = new QPushButton(q);
        QObject::connect(addCusBt, SIGNAL(clicked()), q, SLOT(_q_addCustom()));
        leftLay->addWidget(addCusBt);
    } else {
        // better color picker size for small displays
        pWidth = 150;
        pHeight = 100;
        custom = 0;
        standard = 0;
    }

    QVBoxLayout *rightLay = new QVBoxLayout;
    topLay->addLayout(rightLay);

    QHBoxLayout *pickLay = new QHBoxLayout;
    rightLay->addLayout(pickLay);

    QVBoxLayout *cLay = new QVBoxLayout;
    pickLay->addLayout(cLay);
    cp = new QColorPicker(q);

    cp->setFrameStyle(QFrame::Panel + QFrame::Sunken);

    cLay->addSpacing(lumSpace);
    cLay->addWidget(cp);
    cLay->addSpacing(lumSpace);

    lp = new QColorLuminancePicker(q);
    lp->setFixedWidth(20);
    pickLay->addWidget(lp);

    QObject::connect(cp, SIGNAL(newCol(int,int)), lp, SLOT(setCol(int,int)));
    QObject::connect(lp, SIGNAL(newHsv(int,int,int)), q, SLOT(_q_newHsv(int,int,int)));

    rightLay->addStretch();

    cs = new QColorShower(q);
    QObject::connect(cs, SIGNAL(newCol(QRgb)), q, SLOT(_q_newColorTypedIn(QRgb)));
    QObject::connect(cs, SIGNAL(currentColorChanged(QColor)),
                     q, SIGNAL(currentColorChanged(QColor)));
    rightLay->addWidget(cs);

    buttons = new QDialogButtonBox(q);
    mainLay->addWidget(buttons);

    ok = buttons->addButton(QDialogButtonBox::Ok);
    QObject::connect(ok, SIGNAL(clicked()), q, SLOT(accept()));
    ok->setDefault(true);
    cancel = buttons->addButton(QDialogButtonBox::Cancel);
    QObject::connect(cancel, SIGNAL(clicked()), q, SLOT(reject()));

    retranslateStrings();

    q->setCurrentColor(initial);
}

void QColorDialogPrivate::_q_addCustom()
{
    cusrgb[nextCust] = cs->currentColor();
    if (custom)
        custom->update();
    nextCust = (nextCust+1) % 16;
}

void QColorDialogPrivate::retranslateStrings()
{
    if (!smallDisplay) {
        lblBasicColors->setText(QColorDialog::tr("&Basic colors"));
        lblCustomColors->setText(QColorDialog::tr("&Custom colors"));
        addCusBt->setText(QColorDialog::tr("&Add to Custom Colors"));
    }

    cs->retranslateStrings();
}

static const Qt::WindowFlags DefaultColorWindowFlags =
        Qt::Dialog | Qt::WindowTitleHint
        | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;

/*!
    \class QColorDialog
    \brief The QColorDialog class provides a dialog widget for specifying colors.

    \ingroup standard-dialogs

    The color dialog's function is to allow users to choose colors.
    For example, you might use this in a drawing program to allow the
    user to set the brush color.

    The static functions provide modal color dialogs.
    \omit
    If you require a modeless dialog, use the QColorDialog constructor.
    \endomit

    The static getColor() function shows the dialog, and allows the user to
    specify a color. This function can also be used to let users choose a
    color with a level of transparency: pass the ShowAlphaChannel option as
    an additional argument.

    The user can store customCount() different custom colors. The
    custom colors are shared by all color dialogs, and remembered
    during the execution of the program. Use setCustomColor() to set
    the custom colors, and use customColor() to get them.

    The \l{dialogs/standarddialogs}{Standard Dialogs} example shows
    how to use QColorDialog as well as other built-in Qt dialogs.

    \image plastique-colordialog.png A color dialog in the Plastique widget style.

    \sa QColor, QFileDialog, QPrintDialog, QFontDialog, {Standard Dialogs Example}
*/

/*!
    \since 4.5

    Constructs a color dialog with the given \a parent.
*/
QColorDialog::QColorDialog(QWidget *parent)
    : QDialog(*new QColorDialogPrivate, parent, DefaultColorWindowFlags)
{
    Q_D(QColorDialog);
    d->init(Qt::white);
}

/*!
    \since 4.5

    Constructs a color dialog with the given \a parent and specified
    \a initial color.
*/
QColorDialog::QColorDialog(const QColor &initial, QWidget *parent)
    : QDialog(*new QColorDialogPrivate, parent, DefaultColorWindowFlags)
{
    Q_D(QColorDialog);
    d->init(initial);
}

/*!
    \property QColorDialog::currentColor
    \brief the currently selected color in the dialog
*/

void QColorDialog::setCurrentColor(const QColor &color)
{
    Q_D(QColorDialog);
    d->setCurrentColor(color.rgb());
    d->selectColor(color);
    d->setCurrentAlpha(color.alpha());
}

QColor QColorDialog::currentColor() const
{
    Q_D(const QColorDialog);
    return d->currentQColor();
}


/*!
    Returns the color that the user selected by clicking the \gui{OK}
    or equivalent button.

    \note This color is not always the same as the color held by the
    \l currentColor property since the user can choose different colors
    before finally selecting the one to use.
*/
QColor QColorDialog::selectedColor() const
{
    Q_D(const QColorDialog);
    return d->selectedQColor;
}

/*!
    Sets the given \a option to be enabled if \a on is true;
    otherwise, clears the given \a option.

    \sa options, testOption()
*/
void QColorDialog::setOption(ColorDialogOption option, bool on)
{
    Q_D(QColorDialog);
    if (!(d->opts & option) != !on)
        setOptions(d->opts ^ option);
}

/*!
    \since 4.5

    Returns true if the given \a option is enabled; otherwise, returns
    false.

    \sa options, setOption()
*/
bool QColorDialog::testOption(ColorDialogOption option) const
{
    Q_D(const QColorDialog);
    return (d->opts & option) != 0;
}

/*!
    \property QColorDialog::options
    \brief the various options that affect the look and feel of the dialog

    By default, all options are disabled.

    Options should be set before showing the dialog. Setting them while the
    dialog is visible is not guaranteed to have an immediate effect on the
    dialog (depending on the option and on the platform).

    \sa setOption(), testOption()
*/
void QColorDialog::setOptions(ColorDialogOptions options)
{
    Q_D(QColorDialog);

    ColorDialogOptions changed = (options ^ d->opts);
    if (!changed)
        return;

    d->opts = options;
    d->buttons->setVisible(!(options & NoButtons));
    d->showAlpha(options & ShowAlphaChannel);
}

QColorDialog::ColorDialogOptions QColorDialog::options() const
{
    Q_D(const QColorDialog);
    return d->opts;
}

/*!
    \enum QColorDialog::ColorDialogOption

    \since 4.5

    This enum specifies various options that affect the look and feel
    of a color dialog.

    \value ShowAlphaChannel Allow the user to select the alpha component of a color.
    \value NoButtons Don't display \gui{OK} and \gui{Cancel} buttons. (Useful for "live dialogs".)

    \sa options, setOption(), testOption(), windowModality()
*/

/*!
    \fn void QColorDialog::currentColorChanged(const QColor &color)

    This signal is emitted whenever the current color changes in the dialog.
    The current color is specified by \a color.

    \sa color, colorSelected()
*/

/*!
    \fn void QColorDialog::colorSelected(const QColor &color);

    This signal is emitted just after the user has clicked \gui{OK} to
    select a color to use. The chosen color is specified by \a color.

    \sa color, currentColorChanged()
*/

/*!
    Changes the visibility of the dialog. If \a visible is true, the dialog
    is shown; otherwise, it is hidden.
*/
void QColorDialog::setVisible(bool visible)
{
    Q_D(QColorDialog);

    if (visible){
        if (testAttribute(Qt::WA_WState_ExplicitShowHide) && !testAttribute(Qt::WA_WState_Hidden))
            return;
    } else if (testAttribute(Qt::WA_WState_ExplicitShowHide) && testAttribute(Qt::WA_WState_Hidden))
        return;

    if (visible)
        d->selectedQColor = QColor();

    QDialog::setVisible(visible);
}

/*!
    \overload
    \since 4.5

    Opens the dialog and connects its colorSelected() signal to the slot specified
    by \a receiver and \a member.

    The signal will be disconnected from the slot when the dialog is closed.
*/
void QColorDialog::open(QObject *receiver, const char *member)
{
    Q_D(QColorDialog);
    connect(this, SIGNAL(colorSelected(QColor)), receiver, member);
    d->receiverToDisconnectOnClose = receiver;
    d->memberToDisconnectOnClose = member;
    QDialog::open();
}

/*!
    \fn QColorDialog::open()

    \since 4.5
    Shows the dialog as a \l{QDialog#Modal Dialogs}{window modal dialog},
    returning immediately.

    \sa QDialog::open()
*/

/*
    For Symbian color dialogs
*/
/*!
    \since 4.5

    Pops up a modal color dialog with the given window \a title (or "Select Color" if none is
    specified), lets the user choose a color, and returns that color. The color is initially set
    to \a initial. The dialog is a child of \a parent. It returns an invalid (see
    QColor::isValid()) color if the user cancels the dialog.

    The \a options argument allows you to customize the dialog.

    \a options parameter is only used to define if the native color dialog is
    used or not.
*/
QColor QColorDialog::getColor(const QColor &initial, QWidget *parent, const QString &title,
                              ColorDialogOptions options)
{
    QColorDialog dlg(parent);
    if (!title.isEmpty())
        dlg.setWindowTitle(title);
    dlg.setOptions(options);
    dlg.setCurrentColor(initial);
    dlg.exec();
    return dlg.selectedColor();
}

/*!
    \obsolete

    Pops up a modal color dialog to allow the user to choose a color
    and an alpha channel (transparency) value. The color+alpha is
    initially set to \a initial. The dialog is a child of \a parent.

    If \a ok is non-null, \e *\a ok is set to true if the user clicked
    \gui{OK}, and to false if the user clicked Cancel.

    If the user clicks Cancel, the \a initial value is returned.

    Use QColorDialog::getColor() instead, passing the
    QColorDialog::ShowAlphaChannel option.
*/

QRgb QColorDialog::getRgba(QRgb initial, bool *ok, QWidget *parent)
{
    QColor color(getColor(QColor(initial), parent, QString(), ShowAlphaChannel));
    QRgb result = color.isValid() ? color.rgba() : initial;
    if (ok)
        *ok = color.isValid();
    return result;
}

/*!
    Destroys the color dialog.
*/

QColorDialog::~QColorDialog()
{
}


/*!
    \reimp
*/
void QColorDialog::changeEvent(QEvent *e)
{
    Q_D(QColorDialog);
    if (e->type() == QEvent::LanguageChange)
        d->retranslateStrings();
    QDialog::changeEvent(e);
}

/*!
  Closes the dialog and sets its result code to \a result. If this dialog
  is shown with exec(), done() causes the local event loop to finish,
  and exec() to return \a result.

  \sa QDialog::done()
*/
void QColorDialog::done(int result)
{
    Q_D(QColorDialog);
    QDialog::done(result);
    if (result == Accepted) {
        d->selectedQColor = d->currentQColor();
        emit colorSelected(d->selectedQColor);
    } else {
        d->selectedQColor = QColor();
    }
    if (d->receiverToDisconnectOnClose) {
        disconnect(this, SIGNAL(colorSelected(QColor)),
                   d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
        d->receiverToDisconnectOnClose = 0;
    }
    d->memberToDisconnectOnClose.clear();
}

QT_END_NAMESPACE

#include "moc_qcolordialog.h"
#include "moc_qcolordialog.cpp"

#endif // QT_NO_COLORDIALOG

/*!
    \fn QColor QColorDialog::getColor(const QColor &init, QWidget *parent, const char *name)
    \compat
*/

/*!
    \fn QRgb QColorDialog::getRgba(QRgb rgba, bool *ok, QWidget *parent, const char *name)
    \compat
*/


