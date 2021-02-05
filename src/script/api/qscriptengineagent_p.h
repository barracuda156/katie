/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtScript module of the Katie Toolkit.
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

#ifndef QSCRIPTENGINEAGENT_P_H
#define QSCRIPTENGINEAGENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobjectdefs.h>
#include "qscriptengineagent.h"

#include "CallFrame.h"
#include "SourceCode.h"
#include "UString.h"
#include "Debugger.h"
#include "DebuggerCallFrame.h"

QT_BEGIN_NAMESPACE

class QScriptEnginePrivate;

class QScriptEngineAgent;
class Q_SCRIPT_EXPORT QScriptEngineAgentPrivate : public JSC::Debugger
{
    Q_DECLARE_PUBLIC(QScriptEngineAgent)
public:
    static QScriptEngineAgent* get(QScriptEngineAgentPrivate* p) {return p->q_func();}
    static QScriptEngineAgentPrivate* get(QScriptEngineAgent* p) {return p->d_func();}

    QScriptEngineAgentPrivate(){}
    virtual ~QScriptEngineAgentPrivate(){}

    void attachAgent();
    void detachAgent();

    //scripts
    virtual void sourceParsed(JSC::ExecState*, const JSC::SourceCode&, int /*errorLine*/, const JSC::UString& /*errorMsg*/) {}
    virtual void scriptUnload(qint64 id)
    {
        q_ptr->scriptUnload(id);
    }
    virtual void scriptLoad(qint64 id, const JSC::UString &program,
                         const JSC::UString &fileName, int baseLineNumber)
    {
        q_ptr->scriptLoad(id,program, fileName, baseLineNumber);
    }

    //exceptions
    virtual void exception(const JSC::DebuggerCallFrame& frame, intptr_t sourceID, int lineno, bool hasHandler)
    {
        Q_UNUSED(frame);
        Q_UNUSED(sourceID);
        Q_UNUSED(lineno);
        Q_UNUSED(hasHandler);
    }
    virtual void exceptionThrow(const JSC::DebuggerCallFrame& frame, intptr_t sourceID, bool hasHandler);
    virtual void exceptionCatch(const JSC::DebuggerCallFrame& frame, intptr_t sourceID);

    //statements
    virtual void atStatement(const JSC::DebuggerCallFrame&, intptr_t sourceID, int lineno/*, int column*/);
    virtual void callEvent(const JSC::DebuggerCallFrame&, intptr_t sourceID, int lineno)
    {
        Q_UNUSED(lineno);
        q_ptr->contextPush();
        q_ptr->functionEntry(sourceID);
    }
    virtual void returnEvent(const JSC::DebuggerCallFrame& frame, intptr_t sourceID, int lineno);
    virtual void willExecuteProgram(const JSC::DebuggerCallFrame& frame, intptr_t sourceID, int lineno)
    {
        Q_UNUSED(frame);
        Q_UNUSED(sourceID);
        Q_UNUSED(lineno);
    }
    virtual void didExecuteProgram(const JSC::DebuggerCallFrame& frame, intptr_t sourceID, int lineno)
    {
        Q_UNUSED(frame);
        Q_UNUSED(sourceID);
        Q_UNUSED(lineno);
    }
    virtual void functionExit(const JSC::JSValue& returnValue, intptr_t sourceID);
    //others
    virtual void didReachBreakpoint(const JSC::DebuggerCallFrame& frame, intptr_t sourceID, int lineno/*, int column*/);

    virtual void evaluateStart(intptr_t sourceID)
    {
        q_ptr->functionEntry(sourceID);
    }
    virtual void evaluateStop(const JSC::JSValue& returnValue, intptr_t sourceID);

    QScriptEnginePrivate *engine;
    QScriptEngineAgent *q_ptr;
};

QT_END_NAMESPACE

#endif
