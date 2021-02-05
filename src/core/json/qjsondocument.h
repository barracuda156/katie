/****************************************************************************
**
** Copyright (c) 2012-2015 Barbara Geller
** Copyright (c) 2012-2015 Ansel Sermersheim
** Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
** Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtCore module of the Katie Toolkit.
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
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QJSONDOCUMENT_H
#define QJSONDOCUMENT_H

#include <QtCore/qjsonvalue.h>

QT_BEGIN_NAMESPACE


class QDebug;

namespace QJsonPrivate {
class Parser;
}

struct Q_CORE_EXPORT QJsonParseError {
   enum ParseError {
      NoError = 0,
      UnterminatedObject,
      MissingNameSeparator,
      UnterminatedArray,
      MissingValueSeparator,
      IllegalValue,
      TerminationByNumber,
      IllegalNumber,
      IllegalEscapeSequence,
      IllegalUTF8String,
      UnterminatedString,
      MissingObject,
      DeepNesting,
      DocumentTooLarge
   };

   QString    errorString() const;

   int        offset;
   ParseError error;
};

class Q_CORE_EXPORT QJsonDocument
{
 public:

#ifdef Q_LITTLE_ENDIAN
   static const uint BinaryFormatTag = ('q') | ('b' << 8) | ('j' << 16) | ('s' << 24);
#else
   static const uint BinaryFormatTag = ('q' << 24) | ('b' << 16) | ('j' << 8) | ('s');
#endif

   QJsonDocument();
   explicit QJsonDocument(const QJsonObject &object);
   explicit QJsonDocument(const QJsonArray &array);
   ~QJsonDocument();

   QJsonDocument(const QJsonDocument &other);
   QJsonDocument &operator =(const QJsonDocument &other);

   enum DataValidation {
      Validate,
      BypassValidation
   };

   static QJsonDocument fromRawData(const char *data, int size, DataValidation validation = Validate);
   const char *rawData(int *size) const;

   static QJsonDocument fromBinaryData(const QByteArray &data, DataValidation validation  = Validate);
   QByteArray toBinaryData() const;

   static QJsonDocument fromVariant(const QVariant &variant);
   QVariant toVariant() const;

   enum JsonFormat {
      Indented,
      Compact
   };

   static QJsonDocument fromJson(const QByteArray &json, QJsonParseError *error = 0);

   QByteArray toJson(JsonFormat format = Indented) const;

   bool isEmpty() const;
   bool isArray() const;
   bool isObject() const;

   QJsonObject object() const;
   QJsonArray array() const;

   void setObject(const QJsonObject &object);
   void setArray(const QJsonArray &array);

   bool operator==(const QJsonDocument &other) const;
   bool operator!=(const QJsonDocument &other) const {
      return !(*this == other);
   }

   bool isNull() const;

 private:
   friend class QJsonValue;
   friend class QJsonPrivate::Data;
   friend class QJsonPrivate::Parser;
   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QJsonDocument &);

   QJsonDocument(QJsonPrivate::Data *data);

   QJsonPrivate::Data *d;
};

Q_CORE_EXPORT QDebug operator<<(QDebug, const QJsonDocument &);

QT_END_NAMESPACE

#endif // QJSONDOCUMENT_H
