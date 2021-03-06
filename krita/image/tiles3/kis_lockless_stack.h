/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  References:
 *    * Maged M. Michael, Safe memory reclamation for dynamic
 *      lock-free objects using atomic reads and writes,
 *      Proceedings of the twenty-first annual symposium on
 *      Principles of distributed computing, July 21-24, 2002,
 *      Monterey, California
 *
 *    * Idea of m_deleteBlockers is taken from Andrey Gulin's blog
 *      http://users.livejournal.com/_foreseer/34284.html
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

#ifndef __KIS_LOCKLESS_STACK_H
#define __KIS_LOCKLESS_STACK_H

#include <QAtomicPointer>

template<class T>
class KisLocklessStack
{
private:
    struct Node {
        Node *next;
        T data;
    };

public:
    KisLocklessStack() { }
    ~KisLocklessStack() {
        T temp;

        while(pop(temp)) {
        }

        cleanUpNodes();
    }

    void push(T data) {
        Node *newNode = new Node();
        newNode->data = data;

        Node *top;

        do {
            top = m_top;
            newNode->next = top;
        } while (!m_top.testAndSetOrdered(top, newNode));

        m_numNodes.ref();
    }

    bool pop(T &value) {
        bool result = false;

        m_deleteBlockers.ref();

        while(1) {
            Node *top = (Node*) m_top;
            if(!top) break;

            // This is safe as we ref'ed m_deleteBlockers
            Node *next = top->next;

            if(m_top.testAndSetOrdered(top, next)) {
                m_numNodes.deref();
                result = true;

                value = top->data;

                /**
                 * Test if we are the only delete blocker left
                 * (it means that we are the only owner of 'top')
                 * If there is someone else in "delete-blocked section",
                 * then just add the struct to the list of free nodes.
                 */
                if (m_deleteBlockers == 1) {
                    cleanUpNodes();
                    delete top;
                }
                else {
                    releaseNode(top);
                }

                break;
            }
        }

        m_deleteBlockers.deref();

        return result;
    }

    /**
     * This is impossible to measure the size of the stack
     * in highly concurrent environment. So we return approximate
     * value! Do not rely on this value much!
     */
    qint32 size() {
        return m_numNodes;
    }

    bool isEmpty() {
        return !m_numNodes;
    }

private:

    inline void releaseNode(Node *node) {
        Node *top;
        do {
            top = m_freeNodes;
            node->next = top;
        } while (!m_freeNodes.testAndSetOrdered(top, node));
    }

    inline void cleanUpNodes() {
        Node *top = m_freeNodes;
        if(top) {
            if(m_freeNodes.testAndSetOrdered(top, 0))
                freeList(top);
        }
    }

    inline void freeList(Node *first) {
        Node *next;
        while (first) {
            next = first->next;
            delete first;
            first = next;
        }
    }


private:
    Q_DISABLE_COPY(KisLocklessStack);

    QAtomicPointer<Node> m_top;
    QAtomicPointer<Node> m_freeNodes;

    QAtomicInt m_deleteBlockers;
    QAtomicInt m_numNodes;
};

#endif /* __KIS_LOCKLESS_STACK_H */

