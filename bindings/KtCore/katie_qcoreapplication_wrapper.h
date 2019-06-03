/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the FOO module of the Katie Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SBK_QCOREAPPLICATIONWRAPPER_H
#define SBK_QCOREAPPLICATIONWRAPPER_H

#include <qcoreapplication.h>

class QCoreApplicationWrapper : public Katie::QCoreApplication
{
public:
    QCoreApplicationWrapper(int & argc, char ** argv);
    inline void aboutToQuit_protected() { Katie::QCoreApplication::aboutToQuit(); }
    inline void connectNotify_protected(const char * signal) { Katie::QCoreApplication::connectNotify(signal); }
    void connectNotify(const char * signal) override;
    inline void destroyed_protected(Katie::QObject * arg__1 = Q_NULLPTR) { Katie::QCoreApplication::destroyed(arg__1); }
    inline void disconnectNotify_protected(const char * signal) { Katie::QCoreApplication::disconnectNotify(signal); }
    void disconnectNotify(const char * signal) override;
    inline int receivers_protected(const char * signal) const { return Katie::QCoreApplication::receivers(signal); }
    inline Katie::QObject * sender_protected() const { return Katie::QCoreApplication::sender(); }
    inline int senderSignalIndex_protected() const { return Katie::QCoreApplication::senderSignalIndex(); }
    inline void unixSignal_protected(int arg__1) { Katie::QCoreApplication::unixSignal(arg__1); }
    ~QCoreApplicationWrapper();
};

#  ifndef SBK_QOBJECTWRAPPER_H
#  define SBK_QOBJECTWRAPPER_H

// Inherited base class:
class QObjectWrapper : public Katie::QObject
{
public:
    QObjectWrapper(Katie::QObject * parent = Q_NULLPTR);
    inline void connectNotify_protected(const char * signal) { Katie::QObject::connectNotify(signal); }
    void connectNotify(const char * signal) override;
    inline void destroyed_protected(Katie::QObject * arg__1 = Q_NULLPTR) { Katie::QObject::destroyed(arg__1); }
    inline void disconnectNotify_protected(const char * signal) { Katie::QObject::disconnectNotify(signal); }
    void disconnectNotify(const char * signal) override;
    inline int receivers_protected(const char * signal) const { return Katie::QObject::receivers(signal); }
    inline Katie::QObject * sender_protected() const { return Katie::QObject::sender(); }
    inline int senderSignalIndex_protected() const { return Katie::QObject::senderSignalIndex(); }
    ~QObjectWrapper();
};

#  endif // SBK_QOBJECTWRAPPER_H

#endif // SBK_QCOREAPPLICATIONWRAPPER_H

