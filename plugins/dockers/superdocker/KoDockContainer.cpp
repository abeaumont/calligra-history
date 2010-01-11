/*
 *
 */

#include "KoDockContainer.h"
#include <QVBoxLayout>
#include <kdebug.h>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDockWidget>
#include <QVBoxLayout>
#include <KoDockRegistry.h>

#include <QEvent>
#include <kgenericfactory.h>

KoDockContainer::KoDockContainer()
{
    //setWindowFlags(windowFlags() | Qt::Widget);
    show();
    setLayout(new QVBoxLayout());
    setAcceptDrops(true);
//    installEventFilter(, new QMouseEvent);
    kDebug() << "nb docker" << KoDockRegistry::instance()->count();
    QList<KoDockFactory*> list = KoDockRegistry::instance()->values();
    list.at(0)->createDockWidget();
    list.at(0)->createDockWidget();
    for(int i = 0; i < list.size(); i++){
//        installEventFilter(list.at(i)->createDockWidget());
    }
}

void KoDockContainer::addDockWidget(QDockWidget *widget)
{
    installEventFilter(widget);
}

void KoDockContainer::catchDockWidget(QDockWidget *widget)
{
    if(m_widgets.contains(widget)) return;
    m_widgets.push_back(widget);
    m_widgetsFlags.push_back(widget->windowFlags());
    widget->setVisible(false);
    widget->setWindowFlags(Qt::Popup);
    layout()->addWidget(widget);
}

bool KoDockContainer::event(QEvent *event)
{
    qDebug() << event->type();
    return QWidget::event(event);
}

void KoDockContainer::releaseDockWidget(QDockWidget* widget)
{
    int idx = m_widgets.indexOf(widget);
    if(idx != -1 )
    {
        QDockWidget* widget = m_widgets[idx];
        widget->setWindowFlags(m_widgetsFlags[idx]);
        m_widgets.removeAt(idx);
        m_widgetsFlags.removeAt(idx);
    }
}

bool KoDockContainer::eventFilter(QObject *object, QEvent *event)
{
    qDebug() << "eventFilter" << event->type();
    QDockWidget* dockWidget = dynamic_cast<QDockWidget*>(object);
    if(dockWidget && event->type() == QEvent::MouseButtonRelease){
        kDebug() << "Test 1 Ok!";
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if(mouseEvent){
            kDebug() << "catch";
            if(isOver(mouseEvent->pos())){
                catchDockWidget(dockWidget);
            }
            return false;
        }
    }
    return true;
}

bool KoDockContainer::isOver(const QPoint& globalPos )
{
    QPoint localPos = mapFromGlobal(globalPos);
    return (localPos.x() < geometry().width() && localPos.y() < geometry().height());
}

void KoDockContainer::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "drag enter event";
    event->acceptProposedAction();
}

void KoDockContainer::dropEvent(QDropEvent *event)
{
    qDebug() << "DROPPED !!";
    event->acceptProposedAction();
}

void KoDockContainer::dragMoveEvent(QDragMoveEvent *event)
{
    qDebug() << "drag move event";
    event->acceptProposedAction();
}

void KoDockContainer::dragLeaveEvent(QDragLeaveEvent *event)
{
    qDebug() << "drag move event";
//    event->acceptProposedAction();
}

