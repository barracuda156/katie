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

#include <limits.h>

#include <qcolor.h>
#include <qdebug.h>

//TESTED_CLASS=
//TESTED_FILES=

class tst_QColor : public QObject
{
    Q_OBJECT

public:
    tst_QColor();

private slots:
    void getSetCheck();
    void isValid_data();
    void isValid();

    void name_data();
    void name();
    void setNamedColor();

    void constructNamedColorWithSpace();

    void colorNames();

    void spec();

    void globalColors_data();
    void globalColors();

    void alpha();
    void setAlpha();

    void red();
    void green();
    void blue();

    void setRed();
    void setGreen();
    void setBlue();

    void getRgb();
    void setRgb();

    void rgba();
    void setRgba();

    void rgb();

    void hue();
    void saturation();
    void value();

    void getHsv();
    void setHsv();

    void cyan();
    void magenta();
    void yellow();
    void black();

    void getCmyk();
    void setCmyk();

    void hueHsl();
    void saturationHsl();
    void lightness();

    void getHsl();
    void setHsl();

    void toRgb_data();
    void toRgb();
    void toRgbNonDestructive();

    void toHsv_data();
    void toHsv();
    void toHsvNonDestructive();

    void toCmyk_data();
    void toCmyk();
    void toCmykNonDestructive();

    void toHsl_data();
    void toHsl();;
    void toHslNonDestructive();

    void convertTo();

    void fromRgb();
    void fromHsv();
    void fromCmyk();
    void fromHsl();

    void light();
    void dark();

    void assignmentOoperator();
    void equalityOperator();

    void specConstructor_data();
    void specConstructor();

    void achromaticHslHue();

#ifdef Q_WS_X11
    void allowX11ColorNames();
    void setallowX11ColorNames();
#endif
};

// Testing get/set functions
void tst_QColor::getSetCheck()
{
    QColor obj1;
    // int QColor::alpha()
    // void QColor::setAlpha(int)
    obj1.setAlpha(0);
    QCOMPARE(obj1.alpha(), 0);
    obj1.setAlpha(-1);
    QCOMPARE(obj1.alpha(), 0); // range<0, 255>
    obj1.setAlpha(INT_MIN);
    QCOMPARE(obj1.alpha(), 0); // range<0, 255>
    obj1.setAlpha(255);
    QCOMPARE(obj1.alpha(), 255); // range<0, 255>
    obj1.setAlpha(INT_MAX);
    QCOMPARE(obj1.alpha(), 255); // range<0, 255>

    // qreal QColor::alphaF()
    // void QColor::setAlphaF(qreal)
    obj1.setAlphaF(0.0);
    QCOMPARE(obj1.alphaF(), qreal(0.0)); // range<0.0, 1.0>
    obj1.setAlphaF(-0.2);
    QCOMPARE(obj1.alphaF(), qreal(0.0)); // range<0.0, 1.0>
    obj1.setAlphaF(1.0);
    QCOMPARE(obj1.alphaF(), qreal(1.0)); // range<0.0, 1.0>
    obj1.setAlphaF(1.1);
    QCOMPARE(obj1.alphaF(), qreal(1.0)); // range<0.0, 1.0>

    // int QColor::red()
    // void QColor::setRed(int)
    obj1.setRed(0);
    QCOMPARE(obj1.red(), 0);
    obj1.setRed(-1);
    QCOMPARE(obj1.red(), 0); // range<0, 255>
    obj1.setRed(INT_MIN);
    QCOMPARE(obj1.red(), 0); // range<0, 255>
    obj1.setRed(255);
    QCOMPARE(obj1.red(), 255); // range<0, 255>
    obj1.setRed(INT_MAX);
    QCOMPARE(obj1.red(), 255); // range<0, 255>

    // int QColor::green()
    // void QColor::setGreen(int)
    obj1.setGreen(0);
    QCOMPARE(obj1.green(), 0);
    obj1.setGreen(-1);
    QCOMPARE(obj1.green(), 0); // range<0, 255>
    obj1.setGreen(INT_MIN);
    QCOMPARE(obj1.green(), 0); // range<0, 255>
    obj1.setGreen(255);
    QCOMPARE(obj1.green(), 255); // range<0, 255>
    obj1.setGreen(INT_MAX);
    QCOMPARE(obj1.green(), 255); // range<0, 255>

    // int QColor::blue()
    // void QColor::setBlue(int)
    obj1.setBlue(0);
    QCOMPARE(obj1.blue(), 0);
    obj1.setBlue(-1);
    QCOMPARE(obj1.blue(), 0); // range<0, 255>
    obj1.setBlue(INT_MIN);
    QCOMPARE(obj1.blue(), 0); // range<0, 255>
    obj1.setBlue(255);
    QCOMPARE(obj1.blue(), 255); // range<0, 255>
    obj1.setBlue(INT_MAX);
    QCOMPARE(obj1.blue(), 255); // range<0, 255>

    // qreal QColor::redF()
    // void QColor::setRedF(qreal)
    obj1.setRedF(0.0);
    QCOMPARE(obj1.redF(), qreal(0.0));
    obj1.setRedF(-0.2);
    QCOMPARE(obj1.redF(), qreal(0.0)); // range<0.0, 1.0
    obj1.setRedF(1.1);
    QCOMPARE(obj1.redF(), qreal(1.0)); // range<0.0, 1.0

    // qreal QColor::greenF()
    // void QColor::setGreenF(qreal)
    obj1.setGreenF(0.0);
    QCOMPARE(obj1.greenF(), qreal(0.0));
    obj1.setGreenF(-0.2);
    QCOMPARE(obj1.greenF(), qreal(0.0)); // range<0.0, 1.0
    obj1.setGreenF(1.1);
    QCOMPARE(obj1.greenF(), qreal(1.0)); // range<0.0, 1.0

    // qreal QColor::blueF()
    // void QColor::setBlueF(qreal)
    obj1.setBlueF(0.0);
    QCOMPARE(obj1.blueF(), qreal(0.0));
    obj1.setBlueF(-0.2);
    QCOMPARE(obj1.blueF(), qreal(0.0)); // range<0.0, 1.0
    obj1.setBlueF(1.1);
    QCOMPARE(obj1.blueF(), qreal(1.0)); // range<0.0, 1.0

    // QRgb QColor::rgba()
    // void QColor::setRgba(QRgb)
    QRgb var9(qRgba(10, 20, 30, 40));
    obj1.setRgba(var9);
    QCOMPARE(obj1.rgba(), var9);
    obj1.setRgba(QRgb(0));
    QCOMPARE(obj1.rgba(), QRgb(0));

    // QRgb QColor::rgb()
    // void QColor::setRgb(QRgb)
    QRgb var10(qRgb(10, 20, 30));
    obj1.setRgb(var10);
    QCOMPARE(obj1.rgb(), var10);
    obj1.setRgb(QRgb(0));
    QCOMPARE(obj1.rgb(), qRgb(0, 0, 0));
}

Q_DECLARE_METATYPE(QColor)


tst_QColor::tst_QColor()
{
}

void tst_QColor::isValid_data()
{
    QTest::addColumn<QColor>("color");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("defaultConstructor") << QColor() << false;
    QTest::newRow("rgbConstructor-valid") << QColor(2,5,7) << true;
    QTest::newRow("rgbConstructor-invalid") << QColor(2,5,999) << false;
    QTest::newRow("nameQStringConstructor-valid") << QColor(QString("#ffffff")) << true;
    QTest::newRow("nameQStringConstructor-invalid") << QColor(QString("#ffffgg")) << false;
    QTest::newRow("nameQStringConstructor-empty") << QColor(QString("")) << false;
#ifndef QT_NO_COLORNAMES
    QTest::newRow("nameQStringConstructor-named") << QColor(QString("red")) << true;
#else
    QTest::newRow("nameQStringConstructor-named") << QColor(QString("red")) << false;
#endif
    QTest::newRow("nameCharConstructor-valid") << QColor("#ffffff") << true;
    QTest::newRow("nameCharConstructor-invalid") << QColor("#ffffgg") << false;
    QTest::newRow("nameCharConstructor-invalid-2") << QColor("#fffffg") << false;
}

void tst_QColor::isValid()
{
    QFETCH(QColor, color);
    QFETCH(bool, isValid);
    QVERIFY(color.isValid() == isValid);
}

void tst_QColor::name_data()
{
    QTest::addColumn<QColor>("color");
    QTest::addColumn<QString>("name");

    QTest::newRow("invalid") << QColor() << "#000000";
    QTest::newRow("global color black") << QColor(Qt::black) << "#000000";
    QTest::newRow("global color white") << QColor(Qt::white) << "#ffffff";
    QTest::newRow("global color darkGray") << QColor(Qt::darkGray) << "#808080";
    QTest::newRow("global color gray") << QColor(Qt::gray) << "#a0a0a4";
    QTest::newRow("global color lightGray") << QColor(Qt::lightGray) << "#c0c0c0";
    QTest::newRow("global color red") << QColor(Qt::red) << "#ff0000";
    QTest::newRow("global color green") << QColor(Qt::green) << "#00ff00";
    QTest::newRow("global color blue") << QColor(Qt::blue) << "#0000ff";
    QTest::newRow("global color cyan") << QColor(Qt::cyan) << "#00ffff";
    QTest::newRow("global color magenta") << QColor(Qt::magenta) << "#ff00ff";
    QTest::newRow("global color yellow") << QColor(Qt::yellow) << "#ffff00";
    QTest::newRow("global color darkRed") << QColor(Qt::darkRed) << "#800000";
    QTest::newRow("global color darkGreen") << QColor(Qt::darkGreen) << "#008000";
    QTest::newRow("global color darkBlue") << QColor(Qt::darkBlue) << "#000080";
    QTest::newRow("global color darkCyan") << QColor(Qt::darkCyan) << "#008080";
    QTest::newRow("global color darkMagenta") << QColor(Qt::darkMagenta) << "#800080";
    QTest::newRow("global color darkYellow") << QColor(Qt::darkYellow) << "#808000";
}

void tst_QColor::name()
{
    QFETCH(QColor, color);
    QFETCH(QString, name);
    QCOMPARE(color.name(), name);
}

void tst_QColor::globalColors_data()
{
    QTest::addColumn<QColor>("color");
    QTest::addColumn<uint>("argb");

    QTest::newRow("invalid") << QColor() << 0xff000000;
    QTest::newRow("global color black") << QColor(Qt::black) << 0xff000000;
    QTest::newRow("global color white") << QColor(Qt::white) << 0xffffffff;
    QTest::newRow("global color darkGray") << QColor(Qt::darkGray) << 0xff808080;
    QTest::newRow("global color gray") << QColor(Qt::gray) << 0xffa0a0a4;
    QTest::newRow("global color lightGray") << QColor(Qt::lightGray) << 0xffc0c0c0;
    QTest::newRow("global color red") << QColor(Qt::red) << 0xffff0000;
    QTest::newRow("global color green") << QColor(Qt::green) << 0xff00ff00;
    QTest::newRow("global color blue") << QColor(Qt::blue) << 0xff0000ff;
    QTest::newRow("global color cyan") << QColor(Qt::cyan) << 0xff00ffff;
    QTest::newRow("global color magenta") << QColor(Qt::magenta) << 0xffff00ff;
    QTest::newRow("global color yellow") << QColor(Qt::yellow) << 0xffffff00;
    QTest::newRow("global color darkRed") << QColor(Qt::darkRed) << 0xff800000;
    QTest::newRow("global color darkGreen") << QColor(Qt::darkGreen) << 0xff008000;
    QTest::newRow("global color darkBlue") << QColor(Qt::darkBlue) << 0xff000080;
    QTest::newRow("global color darkCyan") << QColor(Qt::darkCyan) << 0xff008080;
    QTest::newRow("global color darkMagenta") << QColor(Qt::darkMagenta) << 0xff800080;
    QTest::newRow("global color darkYellow") << QColor(Qt::darkYellow) << 0xff808000;
    QTest::newRow("global color transparent") << QColor(Qt::transparent) << 0x00000000u;
}


void tst_QColor::globalColors()
{
    QFETCH(QColor, color);
    QFETCH(uint, argb);
    QCOMPARE(color.rgba(), argb);
}

/*
  CSS color names = SVG 1.0 color names + transparent (rgba(0,0,0,0))
*/
#define RGBCOLOR(r,g,b) (0xff000000 | (r << 16) |  (g << 8) | b)
static const struct RGBData {
    const char *name;
    const uint value;
} rgbTbl[] = {
    { "aliceblue", RGBCOLOR(240, 248, 255) },
    { "antiquewhite", RGBCOLOR(250, 235, 215) },
    { "aqua", RGBCOLOR( 0, 255, 255) },
    { "aquamarine", RGBCOLOR(127, 255, 212) },
    { "azure", RGBCOLOR(240, 255, 255) },
    { "beige", RGBCOLOR(245, 245, 220) },
    { "bisque", RGBCOLOR(255, 228, 196) },
    { "black", RGBCOLOR(0, 0, 0) },
    { "blanchedalmond", RGBCOLOR(255, 235, 205) },
    { "blue", RGBCOLOR(0, 0, 255) },
    { "blueviolet", RGBCOLOR(138, 43, 226) },
    { "brown", RGBCOLOR(165, 42, 42) },
    { "burlywood", RGBCOLOR(222, 184, 135) },
    { "cadetblue", RGBCOLOR( 95, 158, 160) },
    { "chartreuse", RGBCOLOR(127, 255, 0) },
    { "chocolate", RGBCOLOR(210, 105, 30) },
    { "coral", RGBCOLOR(255, 127, 80) },
    { "cornflowerblue", RGBCOLOR(100, 149, 237) },
    { "cornsilk", RGBCOLOR(255, 248, 220) },
    { "crimson", RGBCOLOR(220, 20, 60) },
    { "cyan", RGBCOLOR(0, 255, 255) },
    { "darkblue", RGBCOLOR(0, 0, 139) },
    { "darkcyan", RGBCOLOR(0, 139, 139) },
    { "darkgoldenrod", RGBCOLOR(184, 134, 11) },
    { "darkgray", RGBCOLOR(169, 169, 169) },
    { "darkgreen", RGBCOLOR(0, 100, 0) },
    { "darkgrey", RGBCOLOR(169, 169, 169) },
    { "darkkhaki", RGBCOLOR(189, 183, 107) },
    { "darkmagenta", RGBCOLOR(139, 0, 139) },
    { "darkolivegreen", RGBCOLOR( 85, 107, 47) },
    { "darkorange", RGBCOLOR(255, 140, 0) },
    { "darkorchid", RGBCOLOR(153, 50, 204) },
    { "darkred", RGBCOLOR(139, 0, 0) },
    { "darksalmon", RGBCOLOR(233, 150, 122) },
    { "darkseagreen", RGBCOLOR(143, 188, 143) },
    { "darkslateblue", RGBCOLOR(72, 61, 139) },
    { "darkslategray", RGBCOLOR(47, 79, 79) },
    { "darkslategrey", RGBCOLOR(47, 79, 79) },
    { "darkturquoise", RGBCOLOR(0, 206, 209) },
    { "darkviolet", RGBCOLOR(148, 0, 211) },
    { "deeppink", RGBCOLOR(255, 20, 147) },
    { "deepskyblue", RGBCOLOR( 0, 191, 255) },
    { "dimgray", RGBCOLOR(105, 105, 105) },
    { "dimgrey", RGBCOLOR(105, 105, 105) },
    { "dodgerblue", RGBCOLOR( 30, 144, 255) },
    { "firebrick", RGBCOLOR(178, 34, 34) },
    { "floralwhite", RGBCOLOR(255, 250, 240) },
    { "forestgreen", RGBCOLOR(34, 139, 34) },
    { "fuchsia", RGBCOLOR(255, 0, 255) },
    { "gainsboro", RGBCOLOR(220, 220, 220) },
    { "ghostwhite", RGBCOLOR(248, 248, 255) },
    { "gold", RGBCOLOR(255, 215, 0) },
    { "goldenrod", RGBCOLOR(218, 165, 32) },
    { "gray", RGBCOLOR(128, 128, 128) },
    { "green", RGBCOLOR(0, 128, 0) },
    { "greenyellow", RGBCOLOR(173, 255, 47) },
    { "grey", RGBCOLOR(128, 128, 128) },
    { "honeydew", RGBCOLOR(240, 255, 240) },
    { "hotpink", RGBCOLOR(255, 105, 180) },
    { "indianred", RGBCOLOR(205, 92, 92) },
    { "indigo", RGBCOLOR(75, 0, 130) },
    { "ivory", RGBCOLOR(255, 255, 240) },
    { "khaki", RGBCOLOR(240, 230, 140) },
    { "lavender", RGBCOLOR(230, 230, 250) },
    { "lavenderblush", RGBCOLOR(255, 240, 245) },
    { "lawngreen", RGBCOLOR(124, 252, 0) },
    { "lemonchiffon", RGBCOLOR(255, 250, 205) },
    { "lightblue", RGBCOLOR(173, 216, 230) },
    { "lightcoral", RGBCOLOR(240, 128, 128) },
    { "lightcyan", RGBCOLOR(224, 255, 255) },
    { "lightgoldenrodyellow", RGBCOLOR(250, 250, 210) },
    { "lightgray", RGBCOLOR(211, 211, 211) },
    { "lightgreen", RGBCOLOR(144, 238, 144) },
    { "lightgrey", RGBCOLOR(211, 211, 211) },
    { "lightpink", RGBCOLOR(255, 182, 193) },
    { "lightsalmon", RGBCOLOR(255, 160, 122) },
    { "lightseagreen", RGBCOLOR( 32, 178, 170) },
    { "lightskyblue", RGBCOLOR(135, 206, 250) },
    { "lightslategray", RGBCOLOR(119, 136, 153) },
    { "lightslategrey", RGBCOLOR(119, 136, 153) },
    { "lightsteelblue", RGBCOLOR(176, 196, 222) },
    { "lightyellow", RGBCOLOR(255, 255, 224) },
    { "lime", RGBCOLOR(0, 255, 0) },
    { "limegreen", RGBCOLOR(50, 205, 50) },
    { "linen", RGBCOLOR(250, 240, 230) },
    { "magenta", RGBCOLOR(255, 0, 255) },
    { "maroon", RGBCOLOR(128, 0, 0) },
    { "mediumaquamarine", RGBCOLOR(102, 205, 170) },
    { "mediumblue", RGBCOLOR(0, 0, 205) },
    { "mediumorchid", RGBCOLOR(186, 85, 211) },
    { "mediumpurple", RGBCOLOR(147, 112, 219) },
    { "mediumseagreen", RGBCOLOR( 60, 179, 113) },
    { "mediumslateblue", RGBCOLOR(123, 104, 238) },
    { "mediumspringgreen", RGBCOLOR(0, 250, 154) },
    { "mediumturquoise", RGBCOLOR(72, 209, 204) },
    { "mediumvioletred", RGBCOLOR(199, 21, 133) },
    { "midnightblue", RGBCOLOR(25, 25, 112) },
    { "mintcream", RGBCOLOR(245, 255, 250) },
    { "mistyrose", RGBCOLOR(255, 228, 225) },
    { "moccasin", RGBCOLOR(255, 228, 181) },
    { "navajowhite", RGBCOLOR(255, 222, 173) },
    { "navy", RGBCOLOR(0, 0, 128) },
    { "oldlace", RGBCOLOR(253, 245, 230) },
    { "olive", RGBCOLOR(128, 128, 0) },
    { "olivedrab", RGBCOLOR(107, 142, 35) },
    { "orange", RGBCOLOR(255, 165, 0) },
    { "orangered", RGBCOLOR(255, 69, 0) },
    { "orchid", RGBCOLOR(218, 112, 214) },
    { "palegoldenrod", RGBCOLOR(238, 232, 170) },
    { "palegreen", RGBCOLOR(152, 251, 152) },
    { "paleturquoise", RGBCOLOR(175, 238, 238) },
    { "palevioletred", RGBCOLOR(219, 112, 147) },
    { "papayawhip", RGBCOLOR(255, 239, 213) },
    { "peachpuff", RGBCOLOR(255, 218, 185) },
    { "peru", RGBCOLOR(205, 133, 63) },
    { "pink", RGBCOLOR(255, 192, 203) },
    { "plum", RGBCOLOR(221, 160, 221) },
    { "powderblue", RGBCOLOR(176, 224, 230) },
    { "purple", RGBCOLOR(128, 0, 128) },
    { "red", RGBCOLOR(255, 0, 0) },
    { "rosybrown", RGBCOLOR(188, 143, 143) },
    { "royalblue", RGBCOLOR(65, 105, 225) },
    { "saddlebrown", RGBCOLOR(139, 69, 19) },
    { "salmon", RGBCOLOR(250, 128, 114) },
    { "sandybrown", RGBCOLOR(244, 164, 96) },
    { "seagreen", RGBCOLOR(46, 139, 87) },
    { "seashell", RGBCOLOR(255, 245, 238) },
    { "sienna", RGBCOLOR(160, 82, 45) },
    { "silver", RGBCOLOR(192, 192, 192) },
    { "skyblue", RGBCOLOR(135, 206, 235) },
    { "slateblue", RGBCOLOR(106, 90, 205) },
    { "slategray", RGBCOLOR(112, 128, 144) },
    { "slategrey", RGBCOLOR(112, 128, 144) },
    { "snow", RGBCOLOR(255, 250, 250) },
    { "springgreen", RGBCOLOR( 0, 255, 127) },
    { "steelblue", RGBCOLOR(70, 130, 180) },
    { "tan", RGBCOLOR(210, 180, 140) },
    { "teal", RGBCOLOR(0, 128, 128) },
    { "thistle", RGBCOLOR(216, 191, 216) },
    { "tomato", RGBCOLOR(255, 99, 71) },
    { "transparent", 0 },
    { "turquoise", RGBCOLOR(64, 224, 208) },
    { "violet", RGBCOLOR(238, 130, 238) },
    { "wheat", RGBCOLOR(245, 222, 179) },
    { "white", RGBCOLOR(255, 255, 255) },
    { "whitesmoke", RGBCOLOR(245, 245, 245) },
    { "yellow", RGBCOLOR(255, 255, 0) },
    { "yellowgreen", RGBCOLOR(154, 205, 50) }
};
#undef RGBCOLOR

static const qint16 rgbTblSize = sizeof(rgbTbl) / sizeof(RGBData);

void tst_QColor::setNamedColor()
{
    for (int i = 0; i < rgbTblSize; ++i) {
        QColor color;
        color.setNamedColor(QLatin1String(rgbTbl[i].name));
        QColor expected;
        expected.setRgba(rgbTbl[i].value);
        QCOMPARE(color, expected);
    }
}

void tst_QColor::constructNamedColorWithSpace()
{
    QColor whiteSmoke("white smoke");
    QCOMPARE(whiteSmoke, QColor(245, 245, 245));
}

void tst_QColor::colorNames()
{
    DEPENDS_ON("setNamedColor()");

    QStringList all = QColor::colorNames();
    QCOMPARE(all.size(), (int)rgbTblSize);
    for (int i = 0; i < all.size(); ++i)
        QCOMPARE(all.at(i), QString::fromLatin1(rgbTbl[i].name));
}

void tst_QColor::spec()
{
    QColor invalid;
    QCOMPARE(invalid.spec(), QColor::Invalid);

    QColor rgb = QColor::fromRgb(0, 0, 0);
    QCOMPARE(rgb.spec(), QColor::Rgb);

    QColor hsv = QColor::fromHsv(0, 0, 0);
    QCOMPARE(hsv.spec(), QColor::Hsv);

    QColor cmyk = QColor::fromCmyk(0, 0, 0, 0);
    QCOMPARE(cmyk.spec(), QColor::Cmyk);

    QColor hsl = QColor::fromHsl(0, 0, 0, 0);
    QCOMPARE(hsl.spec(), QColor::Hsl);

}

void tst_QColor::alpha()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::red()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::green()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::blue()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::getRgb()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::setAlpha()
{ DEPENDS_ON(setRgb()); }

bool veryFuzzyCompare(double a, double b)
{
    return qAbs(a - b) < 0.01;
}

void tst_QColor::setRed()
{
    DEPENDS_ON(setRgb());

    QColor c = QColor(Qt::blue).toHsv();
    c.setRed(127);
    QCOMPARE(c.red(), 127);
    QCOMPARE(c.green(), 0);
    QCOMPARE(c.blue(), 255);

    c = QColor(Qt::blue).toHsv();
    c.setRedF(0.5);
    QVERIFY(veryFuzzyCompare(c.redF(), 0.5));
    QCOMPARE(c.greenF(), qreal(0.0));
    QCOMPARE(c.blueF(), qreal(1.0));
}

void tst_QColor::setGreen()
{
    DEPENDS_ON(setRgb());

    QColor c = QColor(Qt::blue).toHsv();
    c.setGreen(127);
    QCOMPARE(c.red(), 0);
    QCOMPARE(c.green(), 127);
    QCOMPARE(c.blue(), 255);

    c = QColor(Qt::blue).toHsv();
    c.setGreenF(0.5);
    QCOMPARE(c.redF(), qreal(0.0));
    QVERIFY(veryFuzzyCompare(c.greenF(), 0.5));
    QCOMPARE(c.blueF(), qreal(1.0));
}

void tst_QColor::setBlue()
{
    DEPENDS_ON(setRgb());

    QColor c = QColor(Qt::red).toHsv();
    c.setBlue(127);
    QCOMPARE(c.red(), 255);
    QCOMPARE(c.green(), 0);
    QCOMPARE(c.blue(), 127);

    c = QColor(Qt::red).toHsv();
    c.setBlueF(0.5);
    QCOMPARE(c.redF(), qreal(1.0));
    QCOMPARE(c.greenF(), qreal(0.0));
    QVERIFY(veryFuzzyCompare(c.blueF(), 0.5));
}


void tst_QColor::setRgb()
{
    QColor color;

    for (int A = 0; A <= USHRT_MAX; ++A) {
        {
            // 0-255
            int a = A >> 8;
            QRgb rgb = qRgba(0, 0, 0, a);

            color.setRgb(0, 0, 0, a);
            QCOMPARE(color.alpha(), a);
            QCOMPARE(color.rgb(),  qRgb(0, 0, 0));

            color.setRgb(rgb);
            QCOMPARE(color.alpha(), 255);
            QCOMPARE(color.rgb(),   qRgb(0, 0, 0));

            int r, g, b, a2;
            color.setRgb(0, 0, 0, a);
            color.getRgb(&r, &g, &b, &a2);
            QCOMPARE(a2, a);

            QColor c(0, 0, 0);
            c.setAlpha(a);
            QCOMPARE(c.alpha(), a);
        }

        {
            // 0.0-1.0
            qreal a = A / qreal(USHRT_MAX);
            color.setRgbF(0.0, 0.0, 0.0, a);
            QCOMPARE(color.alphaF(), a);

            qreal r, g, b, a2;
            color.getRgbF(&r, &g, &b, &a2);
            QCOMPARE(a2, a);

            QColor c(0, 0, 0);
            c.setAlphaF(a);

            QCOMPARE(c.alphaF(), a);
        }
    }

    for (int R = 0; R <= USHRT_MAX; ++R) {
        {
            // 0-255
            int r = R >> 8;
            QRgb rgb = qRgb(r, 0, 0);

            color.setRgb(r, 0, 0);
            QCOMPARE(color.red(), r);
            QCOMPARE(color.rgb(), rgb);

            color.setRgb(rgb);
            QCOMPARE(color.red(), r);
            QCOMPARE(color.rgb(), rgb);

            int r2, g, b, a;
            color.getRgb(&r2, &g, &b, &a);
            QCOMPARE(r2, r);
        }

        {
            // 0.0-1.0
            qreal r = R / qreal(USHRT_MAX);
            color.setRgbF(r, 0.0, 0.0);
            QCOMPARE(color.redF(), r);

            qreal r2, g, b, a;
            color.getRgbF(&r2, &g, &b, &a);
            QCOMPARE(r2, r);
        }
    }

    for (int G = 0; G <= USHRT_MAX; ++G) {
        {
            // 0-255
            int g = G >> 8;
            QRgb rgb = qRgb(0, g, 0);

            color.setRgb(0, g, 0);
            QCOMPARE(color.green(), g);
            QCOMPARE(color.rgb(),   rgb);

            color.setRgb(rgb);
            QCOMPARE(color.green(), g);
            QCOMPARE(color.rgb(),   rgb);

            int r, g2, b, a;
            color.getRgb(&r, &g2, &b, &a);
            QCOMPARE(g2, g);
        }

        {
            // 0.0-1.0
            qreal g = G / qreal(USHRT_MAX);
            color.setRgbF(0.0, g, 0.0);
            QCOMPARE(color.greenF(), g);

            qreal r, g2, b, a;
            color.getRgbF(&r, &g2, &b, &a);
            QCOMPARE(g2, g);
        }
    }

    for (int B = 0; B <= USHRT_MAX; ++B) {
        {
            // 0-255
            int b = B >> 8;
            QRgb rgb = qRgb(0, 0, b);

            color.setRgb(0, 0, b);
            QCOMPARE(color.blue(),  b);
            QCOMPARE(color.rgb(),   rgb);

            color.setRgb(rgb);
            QCOMPARE(color.blue(),  b);
            QCOMPARE(color.rgb(),   rgb);

            int r, g, b2, a;
            color.getRgb(&r, &g, &b2, &a);
            QCOMPARE(b2, b);
        }

        {
            // 0.0-1.0
            qreal b = B / qreal(USHRT_MAX);
            color.setRgbF(0.0, 0.0, b);
            QCOMPARE(color.blueF(), b);

            qreal r, g, b2, a;
            color.getRgbF(&r, &g, &b2, &a);
            QCOMPARE(b2, b);
        }
    }
}

void tst_QColor::rgba()
{ DEPENDS_ON("setRgba()"); }

void tst_QColor::setRgba()
{
    for (int a = 0; a < 255; ++a) {
        const QRgb rgba1 = qRgba(a, a, a, a);
        QColor color;
        color.setRgba(rgba1);
        QCOMPARE(color.alpha(), a);
        const QRgb rgba2 = color.rgba();
        QCOMPARE(rgba2, rgba1);
        QCOMPARE(qAlpha(rgba2), a);
    }
}

void tst_QColor::rgb()
{ DEPENDS_ON(setRgb()); }

void tst_QColor::hue()
{ DEPENDS_ON(setHsv()); }

void tst_QColor::saturation()
{ DEPENDS_ON(setHsv()); }

void tst_QColor::value()
{ DEPENDS_ON(setHsv()); }

void tst_QColor::getHsv()
{ DEPENDS_ON(setHsv()); }

void tst_QColor::setHsv()
{
    QColor color;

    for (int A = 0; A <= USHRT_MAX; ++A) {
        {
            // 0-255
            int a = A >> 8;
            color.setHsv(0, 0, 0, a);
            QCOMPARE(color.alpha(), a);

            int h, s, v, a2;
            color.getHsv(&h, &s, &v, &a2);
            QCOMPARE(a2, a);
        }

        {
            // 0.0-1.0
            qreal a = A / qreal(USHRT_MAX);
            color.setHsvF(0.0, 0.0, 0.0, a); QCOMPARE(color.alphaF(), a);

            qreal h, s, v, a2;
            color.getHsvF(&h, &s, &v, &a2);
            QCOMPARE(a2, a);
        }
    }

    for (int H = 0; H < 36000; ++H) {
        {
            // 0-255
            int h = H / 100;

            color.setHsv(h, 0, 0, 0);
            QCOMPARE(color.hue(), h);

            int h2, s, v, a;
            color.getHsv(&h2, &s, &v, &a);
            QCOMPARE(h2, h);
        }

        {
            // 0.0-1.0
            qreal h = H / 36000.0;
            color.setHsvF(h, 0.0, 0.0, 0.0);
            QCOMPARE(color.hueF(), h);

            qreal h2, s, v, a;
            color.getHsvF(&h2, &s, &v, &a);
            QCOMPARE(h2, h);
        }
    }

    for (int S = 0; S <= USHRT_MAX; ++S) {
        {
            // 0-255
            int s = S >> 8;
            color.setHsv(0, s, 0, 0);
            QCOMPARE(color.saturation(), s);

            int h, s2, v, a;
            color.getHsv(&h, &s2, &v, &a);
            QCOMPARE(s2, s);
        }

        {
            // 0.0-1.0
            qreal s = S / qreal(USHRT_MAX);
            color.setHsvF(0.0, s, 0.0, 0.0);
            QCOMPARE(color.saturationF(), s);

            qreal h, s2, v, a;
            color.getHsvF(&h, &s2, &v, &a);
            QCOMPARE(s2, s);
        }
    }

    for (int V = 0; V <= USHRT_MAX; ++V) {
        {
            // 0-255
            int v = V >> 8;
            color.setHsv(0, 0, v, 0);
            QCOMPARE(color.value(),  v);

            int h, s, v2, a;
            color.getHsv(&h, &s, &v2, &a);
            QCOMPARE(v2, v);
        }

        {
            // 0.0-1.0
            qreal v = V / qreal(USHRT_MAX);
            color.setHsvF(0.0, 0.0, v, 0.0);
            QCOMPARE(color.valueF(), v);

            qreal h, s, v2, a;
            color.getHsvF(&h, &s, &v2, &a);
            QCOMPARE(v2, v);
        }
    }
}

void tst_QColor::cyan()
{ DEPENDS_ON(setCmyk()); }

void tst_QColor::magenta()
{ DEPENDS_ON(setCmyk()); }

void tst_QColor::yellow()
{ DEPENDS_ON(setCmyk()); }

void tst_QColor::black()
{ DEPENDS_ON(setCmyk()); }

void tst_QColor::getCmyk()
{ DEPENDS_ON(setCmyk()); }

void tst_QColor::setCmyk()
{
    QColor color;

    for (int A = 0; A <= USHRT_MAX; ++A) {
        {
            // 0-255
            int a = A >> 8;
            color.setCmyk(0, 0, 0, 0, a);
            QCOMPARE(color.alpha(), a);

            int c, m, y, k, a2;
            color.getCmyk(&c, &m, &y, &k, &a2);
            QCOMPARE(a2, a);
        }

        {
            // 0.0-1.0
            qreal a = A / qreal(USHRT_MAX);
            color.setCmykF(0.0, 0.0, 0.0, 0.0, a);
            QCOMPARE(color.alphaF(), a);

            qreal c, m, y, k, a2;
            color.getCmykF(&c, &m, &y, &k, &a2);
            QCOMPARE(a2, a);
        }
    }

    for (int C = 0; C <= USHRT_MAX; ++C) {
        {
            // 0-255
            int c = C >> 8;
            color.setCmyk(c, 0, 0, 0, 0);
            QCOMPARE(color.cyan(), c);

            int c2, m, y, k, a;
            color.getCmyk(&c2, &m, &y, &k, &a);
            QCOMPARE(c2, c);
        }

        {
            // 0.0-1.0
            qreal c = C / qreal(USHRT_MAX);
            color.setCmykF(c, 0.0, 0.0, 0.0, 0.0);
            QCOMPARE(color.cyanF(), c);

            qreal c2, m, y, k, a;
            color.getCmykF(&c2, &m, &y, &k, &a);
            QCOMPARE(c2, c);
        }
    }

    for (int M = 0; M <= USHRT_MAX; ++M) {
        {
            // 0-255
            int m = M >> 8;
            color.setCmyk(0, m, 0, 0, 0);
            QCOMPARE(color.magenta(), m);

            int c, m2, y, k, a;
            color.getCmyk(&c, &m2, &y, &k, &a);
            QCOMPARE(m2, m);
        }

        {
            // 0.0-1.0
            qreal m = M / qreal(USHRT_MAX);
            color.setCmykF(0.0, m, 0.0, 0.0, 0.0);
            QCOMPARE(color.magentaF(), m);

            qreal c, m2, y, k, a;
            color.getCmykF(&c, &m2, &y, &k, &a);
            QCOMPARE(m2, m);
        }
    }

    for (int Y = 0; Y <= USHRT_MAX; ++Y) {
        {
            // 0-255
            int y = Y >> 8;
            color.setCmyk(0, 0, y, 0, 0);
            QCOMPARE(color.yellow(), y);

            int c, m, y2, k, a;
            color.getCmyk(&c, &m, &y2, &k, &a);
            QCOMPARE(y2, y);
        }

        {
            // 0.0-1.0
            qreal y = Y / qreal(USHRT_MAX);
            color.setCmykF(0.0, 0.0, y, 0.0, 0.0);
            QCOMPARE(color.yellowF(), y);

            qreal c, m, y2, k, a;
            color.getCmykF(&c, &m, &y2, &k, &a);
            QCOMPARE(y2, y);
        }
    }

    for (int K = 0; K <= USHRT_MAX; ++K) {
        {
            // 0-255
            int k = K >> 8;
            color.setCmyk(0, 0, 0, k, 0);
            QCOMPARE(color.black(), k);

            int c, m, y, k2, a;
            color.getCmyk(&c, &m, &y, &k2, &a);
            QCOMPARE(k2, k);
        }

        {
            // 0.0-1.0
            qreal k = K / qreal(USHRT_MAX);
            color.setCmykF(0.0, 0.0, 0.0, k, 0.0);
            QCOMPARE(color.blackF(), k);

            qreal c, m, y, k2, a;
            color.getCmykF(&c, &m, &y, &k2, &a);
            QCOMPARE(k2, k);
        }
    }
}

void tst_QColor::hueHsl()
{ DEPENDS_ON(setHsl()); }

void tst_QColor::saturationHsl()
{ DEPENDS_ON(setHsl()); }

void tst_QColor::lightness()
{ DEPENDS_ON(setHsl()); }

void tst_QColor::getHsl()
{ DEPENDS_ON(setHsl()); }

void tst_QColor::setHsl()
{
    QColor color;

    for (int A = 0; A <= USHRT_MAX; ++A) {
        {
            // 0-255
            int a = A >> 8;
            color.setHsl(0, 0, 0, a);
            QCOMPARE(color.alpha(), a);

            int h, s, l, a2;
            color.getHsv(&h, &s, &l, &a2);
            QCOMPARE(a2, a);
        }

        {
            // 0.0-1.0
            qreal a = A / qreal(USHRT_MAX);
            color.setHslF(0.0, 0.0, 0.0, a); QCOMPARE(color.alphaF(), a);

            qreal h, s, l, a2;
            color.getHslF(&h, &s, &l, &a2);
            QCOMPARE(a2, a);
        }
    }

    for (int H = 0; H < 36000; ++H) {
        {
            // 0-255
            int h = H / 100;

            color.setHsl(h, 0, 0, 0);
            QCOMPARE(color.hslHue(), h);

            int h2, s, l, a;
            color.getHsl(&h2, &s, &l, &a);
            QCOMPARE(h2, h);
        }

        {
            // 0.0-1.0
            qreal h = H / 36000.0;
            color.setHslF(h, 0.0, 0.0, 0.0);
            QCOMPARE(color.hslHueF(), h);

            qreal h2, s, l, a;
            color.getHslF(&h2, &s, &l, &a);
            QCOMPARE(h2, h);
        }
    }

    for (int S = 0; S <= USHRT_MAX; ++S) {
        {
            // 0-255
            int s = S >> 8;
            color.setHsl(0, s, 0, 0);
            QCOMPARE(color.hslSaturation(), s);

            int h, s2, l, a;
            color.getHsl(&h, &s2, &l, &a);
            QCOMPARE(s2, s);
        }

        {
            // 0.0-1.0
            qreal s = S / qreal(USHRT_MAX);
            color.setHslF(0.0, s, 0.0, 0.0);
            QCOMPARE(color.hslSaturationF(), s);

            qreal h, s2, l, a;
            color.getHslF(&h, &s2, &l, &a);
            QCOMPARE(s2, s);
        }
    }

    for (int L = 0; L <= USHRT_MAX; ++L) {
        {
            // 0-255
            int l = L >> 8;
            color.setHsl(0, 0, l, 0);
            QCOMPARE(color.lightness(),  l);

            int h, s, l2, a;
            color.getHsl(&h, &s, &l2, &a);
            QCOMPARE(l2, l);
        }

        {
            // 0.0-1.0
            qreal l = L / qreal(USHRT_MAX);
            color.setHslF(0.0, 0.0, l, 0.0);
            QCOMPARE(color.lightnessF(), l);

            qreal h, s, l2, a;
            color.getHslF(&h, &s, &l2, &a);
            QCOMPARE(l2, l);
        }
    }
}

void tst_QColor::toRgb_data()
{
    QTest::addColumn<QColor>("expectedColor");
    QTest::addColumn<QColor>("hsvColor");
    QTest::addColumn<QColor>("cmykColor");
    QTest::addColumn<QColor>("hslColor");

    QTest::newRow("black")
        << QColor::fromRgbF(0.0, 0.0, 0.0)
        << QColor::fromHsvF(-1.0, 0.0, 0.0)
        << QColor::fromCmykF(0.0, 0.0, 0.0, 1.0)
        << QColor::fromHslF(-1.0, 0.0, 0.0);

    QTest::newRow("white")
        << QColor::fromRgbF(1.0, 1.0, 1.0)
        << QColor::fromHsvF(-1.0, 0.0, 1.0)
        << QColor::fromCmykF(0.0, 0.0, 0.0, 0.0)
        << QColor::fromHslF(-1.0, 0.0, 1.0);

    QTest::newRow("red")
        << QColor::fromRgbF(1.0, 0.0, 0.0)
        << QColor::fromHsvF(0.0, 1.0, 1.0)
        << QColor::fromCmykF(0.0, 1.0, 1.0, 0.0)
        << QColor::fromHslF(0.0, 1.0, 0.5, 1.0);

    QTest::newRow("green")
        << QColor::fromRgbF(0.0, 1.0, 0.0)
        << QColor::fromHsvF(0.33333, 1.0, 1.0)
        << QColor::fromCmykF(1.0, 0.0, 1.0, 0.0)
        << QColor::fromHslF(0.33333, 1.0, 0.5);

    QTest::newRow("blue")
        << QColor::fromRgbF(0.0, 0.0, 1.0)
        << QColor::fromHsvF(0.66667, 1.0, 1.0)
        << QColor::fromCmykF(1.0, 1.0, 0.0, 0.0)
        << QColor::fromHslF(0.66667, 1.0, 0.5);

    QTest::newRow("cyan")
        << QColor::fromRgbF(0.0, 1.0, 1.0)
        << QColor::fromHsvF(0.5, 1.0, 1.0)
        << QColor::fromCmykF(1.0, 0.0, 0.0, 0.0)
        << QColor::fromHslF(0.5, 1.0, 0.5);

    QTest::newRow("magenta")
        << QColor::fromRgbF(1.0, 0.0, 1.0)
        << QColor::fromHsvF(0.83333, 1.0, 1.0)
        << QColor::fromCmykF(0.0, 1.0, 0.0, 0.0)
        << QColor::fromHslF(0.83333, 1.0, 0.5);

    QTest::newRow("yellow")
        << QColor::fromRgbF(1.0, 1.0, 0.0)
        << QColor::fromHsvF(0.16667, 1.0, 1.0)
        << QColor::fromCmykF(0.0, 0.0, 1.0, 0.0)
        << QColor::fromHslF(0.16667, 1.0, 0.5);

    QTest::newRow("gray")
        << QColor::fromRgbF(0.6431375, 0.6431375, 0.6431375)
        << QColor::fromHsvF(-1.0, 0.0, 0.6431375)
        << QColor::fromCmykF(0.0, 0.0, 0.0, 0.356863)
        << QColor::fromHslF(-1.0, 0.0, 0.6431375);

    // ### add colors using the 0-255 functions
}

void tst_QColor::toRgb()
{
    // invalid should remain invalid
    QVERIFY(!QColor().toRgb().isValid());

    QFETCH(QColor, expectedColor);
    QFETCH(QColor, hsvColor);
    QFETCH(QColor, cmykColor);
    QFETCH(QColor, hslColor);
    QCOMPARE(hsvColor.toRgb(), expectedColor);
    QCOMPARE(cmykColor.toRgb(), expectedColor);
    QCOMPARE(hslColor.toRgb(), expectedColor);

}

void tst_QColor::toHsv_data()
{
    QTest::addColumn<QColor>("expectedColor");
    QTest::addColumn<QColor>("rgbColor");
    QTest::addColumn<QColor>("cmykColor");
    QTest::addColumn<QColor>("hslColor");

    QTest::newRow("data0")
        << QColor::fromHsv(300, 255, 255)
        << QColor(255, 0, 255)
        << QColor::fromCmyk(0, 255, 0, 0)
        << QColor::fromHslF(300./360., 1., 0.5, 1.0);

    QTest::newRow("data1")
        << QColor::fromHsvF(1., 1., 1., 1.)
        << QColor(255, 0, 0, 255)
        << QColor::fromCmykF(0., 1., 1., 0.)
        << QColor::fromHsvF(1., 1., 1., 1.);
}

void tst_QColor::toRgbNonDestructive()
{
    QColor aColor = QColor::fromRgbF(0.11, 0.22, 0.33, 0.44);
    QCOMPARE(aColor, aColor.toRgb());
}

void tst_QColor::toHsv()
{
    // invalid should remain invalid
    QVERIFY(!QColor().toHsv().isValid());

    QFETCH(QColor, expectedColor);
    QFETCH(QColor, rgbColor);
    QFETCH(QColor, cmykColor);
    QFETCH(QColor, hslColor);
    QCOMPARE(rgbColor.toHsv(), expectedColor);
    QCOMPARE(cmykColor.toHsv(), expectedColor);
    QCOMPARE(hslColor.toHsv(), expectedColor);
}

void tst_QColor::toHsvNonDestructive()
{
    QColor aColor = QColor::fromHsvF(0.11, 0.22, 0.33, 0.44);
    QCOMPARE(aColor, aColor.toHsv());
}

void tst_QColor::toCmyk_data()
{
    QTest::addColumn<QColor>("expectedColor");
    QTest::addColumn<QColor>("rgbColor");
    QTest::addColumn<QColor>("hsvColor");
    QTest::addColumn<QColor>("hslColor");

    QTest::newRow("data0")
        << QColor::fromCmykF(1.0, 0.0, 0.0, 0.0)
        << QColor(0, 255, 255)
        << QColor::fromHsv(180, 255, 255)
        << QColor::fromHslF(180./360., 1., 0.5, 1.0);

    QTest::newRow("data1")
        << QColor::fromCmyk(255, 255, 255, 255)
        << QColor::fromRgb(0, 0, 0)
        << QColor::fromRgb(0, 0, 0).toHsv()
        << QColor::fromRgb(0, 0, 0).toHsl();
}

void tst_QColor::toCmyk()
{
    // invalid should remain invalid
    QVERIFY(!QColor().toCmyk().isValid());

    QFETCH(QColor, expectedColor);
    QFETCH(QColor, rgbColor);
    QFETCH(QColor, hsvColor);
    QFETCH(QColor, hslColor);
    QCOMPARE(rgbColor.toHsv().toCmyk(), expectedColor);
    QCOMPARE(hsvColor.toCmyk(), expectedColor);
    QCOMPARE(hslColor.toCmyk(), expectedColor);
}

void tst_QColor::toCmykNonDestructive()
{
    QColor aColor = QColor::fromCmykF(0.11, 0.22, 0.33, 0.44);
    QCOMPARE(aColor, aColor.toCmyk());
}

void tst_QColor::toHsl_data()
{
    QTest::addColumn<QColor>("expectedColor");
    QTest::addColumn<QColor>("hsvColor");
    QTest::addColumn<QColor>("rgbColor");
    QTest::addColumn<QColor>("cmykColor");


    QTest::newRow("data0")
        << QColor::fromHslF(300./360., 1., 0.5, 1.0)
        << QColor::fromHsv(300, 255, 255)
        << QColor(255, 0, 255)
        << QColor::fromCmyk(0, 255, 0, 0);

    QTest::newRow("data1")
        << QColor::fromHslF(1., 1., 0.5, 1.0)
        << QColor::fromHsvF(1., 1., 1., 1.)
        << QColor(255, 0, 0, 255)
        << QColor::fromCmykF(0., 1., 1., 0.);
}

void tst_QColor::toHsl()
{
    // invalid should remain invalid
    QVERIFY(!QColor().toHsl().isValid());

    QFETCH(QColor, expectedColor);
    QFETCH(QColor, rgbColor);
    QFETCH(QColor, cmykColor);
    QFETCH(QColor, hsvColor);

    QCOMPARE(rgbColor.toHsl(), expectedColor);
    QCOMPARE(cmykColor.toHsl(), expectedColor);
    QCOMPARE(hsvColor.toHsl(), expectedColor);

}


void tst_QColor::toHslNonDestructive()
{
    QColor aColor = QColor::fromHslF(0.11, 0.22, 0.33, 0.44);
    QCOMPARE(aColor, aColor.toHsl());
}


void tst_QColor::convertTo()
{
    QColor color(Qt::black);

    QColor rgb = color.convertTo(QColor::Rgb);
    QVERIFY(rgb.spec() == QColor::Rgb);

    QColor hsv = color.convertTo(QColor::Hsv);
    QVERIFY(hsv.spec() == QColor::Hsv);

    QColor cmyk = color.convertTo(QColor::Cmyk);
    QVERIFY(cmyk.spec() == QColor::Cmyk);

    QColor hsl = color.convertTo(QColor::Hsl);
    QVERIFY(hsl.spec() == QColor::Hsl);

    QColor invalid = color.convertTo(QColor::Invalid);
    QVERIFY(invalid.spec() == QColor::Invalid);

    DEPENDS_ON(toRgb());
    DEPENDS_ON(toHsv());
    DEPENDS_ON(toCmyk());
    DEPENDS_ON(toHsl());
}

void tst_QColor::fromRgb()
{ DEPENDS_ON(convertTo()); }

void tst_QColor::fromHsv()
{ DEPENDS_ON(convertTo()); }

void tst_QColor::fromCmyk()
{ DEPENDS_ON(convertTo()); }

void tst_QColor::fromHsl()
{ DEPENDS_ON(convertTo()); }

void tst_QColor::light()
{
    QColor gray(Qt::gray);
    QColor lighter = gray.light();
    QVERIFY(lighter.value() > gray.value());
}

void tst_QColor::dark()
{
    QColor gray(Qt::gray);
    QColor darker = gray.dark();
    QVERIFY(darker.value() < gray.value());
}

void tst_QColor::assignmentOoperator()
{ DEPENDS_ON(convertTo()); }

void tst_QColor::equalityOperator()
{ DEPENDS_ON(convertTo()); }

Q_DECLARE_METATYPE(QColor::Spec);

void tst_QColor::specConstructor_data()
{
    QTest::addColumn<QColor::Spec>("spec");

    QTest::newRow("Invalid") << QColor::Invalid;
    QTest::newRow("Rgb") << QColor::Rgb;
    QTest::newRow("Hsv") << QColor::Hsv;
    QTest::newRow("Cmyk") << QColor::Cmyk;
}

void tst_QColor::specConstructor()
{
    QFETCH(QColor::Spec, spec);
    QColor color = spec;
    QCOMPARE(color.spec(), spec);
}

void tst_QColor::achromaticHslHue()
{
    QColor color = Qt::black;

    QColor hsl = color.toHsl();
    QCOMPARE(hsl.hslHue(), -1);
}

#ifdef Q_WS_X11
void tst_QColor::allowX11ColorNames()
{
    DEPENDS_ON(setallowX11ColorNames());
}

void tst_QColor::setallowX11ColorNames()
{
    RGBData x11RgbTbl[] = {
        // a few standard X11 color names
        { "DodgerBlue1", qRgb(30, 144, 255) },
        { "DodgerBlue2", qRgb(28, 134, 238) },
        { "DodgerBlue3", qRgb(24, 116, 205) },
        { "DodgerBlue4", qRgb(16, 78, 139) },
        { "SteelBlue1", qRgb(99, 184, 255) },
        { "SteelBlue2", qRgb(92, 172, 238) },
        { "SteelBlue3", qRgb(79, 148, 205) },
        { "SteelBlue4", qRgb(54, 100, 139) },
        { "DeepSkyBlue1", qRgb(0, 191, 255) },
        { "DeepSkyBlue2", qRgb(0, 178, 238) },
        { "DeepSkyBlue3", qRgb(0, 154, 205) },
        { "DeepSkyBlue4", qRgb(0, 104, 139) },
        { "SkyBlue1", qRgb(135, 206, 255) },
        { "SkyBlue2", qRgb(126, 192, 238) },
        { "SkyBlue3", qRgb(108, 166, 205) },
        { "SkyBlue4", qRgb(74, 112, 139) }
    };
    static const int x11RgbTblSize = sizeof(x11RgbTbl) / sizeof(RGBData);

    // X11 color names should not work by default
    QVERIFY(!QColor::allowX11ColorNames());
    for (int i = 0; i < x11RgbTblSize; ++i) {
        QString colorName = QLatin1String(x11RgbTbl[i].name);
        QColor color;
        color.setNamedColor(colorName);
        QVERIFY(!color.isValid());
    }

    // enable X11 color names
    QColor::setAllowX11ColorNames(true);
    QVERIFY(QColor::allowX11ColorNames());
    for (int i = 0; i < x11RgbTblSize; ++i) {
        QString colorName = QLatin1String(x11RgbTbl[i].name);
        QColor color;
        color.setNamedColor(colorName);
        QColor expected(x11RgbTbl[i].value);
        QCOMPARE(color, expected);
    }

    // should be able to turn off X11 color names
    QColor::setAllowX11ColorNames(false);
    QVERIFY(!QColor::allowX11ColorNames());
    for (int i = 0; i < x11RgbTblSize; ++i) {
        QString colorName = QLatin1String(x11RgbTbl[i].name);
        QColor color;
        color.setNamedColor(colorName);
        QVERIFY(!color.isValid());
    }
}
#endif // Q_WS_X11

QTEST_MAIN(tst_QColor)

#include "moc_tst_qcolor.cpp"
