/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_doc2_test.h"

#include <KUndoStack>
#include <qtest_kde.h>

#include "kis_doc2.h"
#include "kis_undo_adapter.h"

class KisCommandHistoryListenerFake : public KisCommandHistoryListener
{
public:
    KisCommandHistoryListenerFake() {
        reset();
    }

    void reset() {
        m_command = 0;
        m_executed = false;
        m_added = false;
    }

    void notifyCommandAdded(const QUndoCommand * cmd) {
        if(!m_command) {
            m_command = cmd;
        }
        Q_ASSERT(cmd == m_command);

        m_added = true;
    }

    void notifyCommandExecuted(const QUndoCommand * cmd) {
        if(!m_command) {
            m_command = cmd;
        }
        Q_ASSERT(cmd == m_command);

        m_executed = true;
    }

    bool wasAdded() {
        return m_added;
    }

    bool wasExecuted() {
        return m_executed;
    }

    const QUndoCommand* command() {
        return m_command;
    }

private:
    const QUndoCommand* m_command;
    bool m_executed;
    bool m_added;
};


class TestCommand : public QUndoCommand
{
public:
    void undo() {}
    void redo() {}
};

void KisDoc2Test::testUndoRedoNotify()
{
    KisDoc2 doc;

    QUndoCommand *testCommand1 = new TestCommand();
    QUndoCommand *testCommand2 = new TestCommand();

    KisCommandHistoryListenerFake listener;

    doc.undoAdapter()->setCommandHistoryListener(&listener);

    qDebug() << "Undo index:" << doc.undoStack()->index();

    qDebug() << "Adding one command";
    listener.reset();
    doc.undoAdapter()->addCommand(testCommand1);
    QVERIFY(listener.wasAdded());
    QVERIFY(!listener.wasExecuted());
    QCOMPARE(listener.command(), testCommand1);
    qDebug() << "Undo index:" << doc.undoStack()->index();

    qDebug() << "Adding one more command";
    listener.reset();
    doc.undoAdapter()->addCommand(testCommand2);
    QVERIFY(listener.wasAdded());
    QVERIFY(!listener.wasExecuted());
    QCOMPARE(listener.command(), testCommand2);
    qDebug() << "Undo index:" << doc.undoStack()->index();

    qDebug() << "Undo";
    listener.reset();
    doc.undoStack()->undo();
    QVERIFY(!listener.wasAdded());
    QVERIFY(listener.wasExecuted());
    QCOMPARE(listener.command(), testCommand2);
    qDebug() << "Undo index:" << doc.undoStack()->index();

    qDebug() << "Undo";
    listener.reset();
    doc.undoStack()->undo();
    QVERIFY(!listener.wasAdded());
    QVERIFY(listener.wasExecuted());
    QCOMPARE(listener.command(), testCommand1);
    qDebug() << "Undo index:" << doc.undoStack()->index();

    qDebug() << "Redo";
    listener.reset();
    doc.undoStack()->redo();
    QVERIFY(!listener.wasAdded());
    QVERIFY(listener.wasExecuted());
    QCOMPARE(listener.command(), testCommand2);
    qDebug() << "Undo index:" << doc.undoStack()->index();

}


QTEST_KDEMAIN(KisDoc2Test, GUI)
#include "kis_doc2_test.moc"

