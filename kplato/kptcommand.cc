/* This file is part of the KDE project
  Copyright (C) 2004 - 2007 Dag Andersen <danders@get2net.dk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "kptcommand.h"
#include "kptaccount.h"
#include "kptappointment.h"
#include "kptpart.h"
#include "kptproject.h"
#include "kpttask.h"
#include "kptcalendar.h"
#include "kptrelation.h"
#include "kptresource.h"
#include "kptview.h"
#include "viewlist/kptviewlist.h"

#include <kdebug.h>
#include <klocale.h>

#include <QHash>
#include <QLineEdit>
#include <QMap>

namespace KPlato
{

void NamedCommand::setCommandType( int type )
{
    if ( m_part )
        m_part->setCommandType( type );
}

void NamedCommand::setSchDeleted()
{
    QMap<Schedule*, bool>::Iterator it;
    for ( it = m_schedules.begin(); it != m_schedules.end(); ++it ) {
        //kDebug() << it.key() ->id() <<":" << it.value();
        it.key() ->setDeleted( it.value() );
    }
}
void NamedCommand::setSchDeleted( bool state )
{
    QMap<Schedule*, bool>::Iterator it;
    for ( it = m_schedules.begin(); it != m_schedules.end(); ++it ) {
        //kDebug() << it.key() ->id() <<":" << state;
        it.key() ->setDeleted( state );
    }
}
void NamedCommand::setSchScheduled()
{
    QMap<Schedule*, bool>::Iterator it;
    for ( it = m_schedules.begin(); it != m_schedules.end(); ++it ) {
        //kDebug() << it.key() ->name() <<":" << it.value();
        it.key() ->setScheduled( it.value() );
    }
}
void NamedCommand::setSchScheduled( bool state )
{
    QMap<Schedule*, bool>::Iterator it;
    for ( it = m_schedules.begin(); it != m_schedules.end(); ++it ) {
        //kDebug() << it.key() ->name() <<":" << state;
        it.key() ->setScheduled( state );
    }
}
void NamedCommand::addSchScheduled( Schedule *sch )
{
    //kDebug() << sch->id() <<":" << sch->isScheduled();
    m_schedules.insert( sch, sch->isScheduled() );
    foreach ( Appointment * a, sch->appointments() ) {
        if ( a->node() == sch ) {
            m_schedules.insert( a->resource(), a->resource() ->isScheduled() );
        } else if ( a->resource() == sch ) {
            m_schedules.insert( a->node(), a->node() ->isScheduled() );
        }
    }
}
void NamedCommand::addSchDeleted( Schedule *sch )
{
    //kDebug() << sch->id() <<":" << sch->isDeleted();
    m_schedules.insert( sch, sch->isDeleted() );
    foreach ( Appointment * a, sch->appointments() ) {
        if ( a->node() == sch ) {
            m_schedules.insert( a->resource(), a->resource() ->isDeleted() );
        } else if ( a->resource() == sch ) {
            m_schedules.insert( a->node(), a->node() ->isDeleted() );
        }
    }
}

//-------------------------------------------------
CalendarAddCmd::CalendarAddCmd( Part *part, Project *project, Calendar *cal, Calendar *parent, const QString& name )
        : NamedCommand( part, name ),
        m_project( project ),
        m_cal( cal ),
        m_parent( parent ),
        m_mine( true )
{
    //kDebug()<<cal->name();
    Q_ASSERT( project != 0 );
}
CalendarAddCmd::~CalendarAddCmd()
{
    if ( m_mine )
        delete m_cal;
}
void CalendarAddCmd::execute()
{
    if ( m_project ) {
        m_project->addCalendar( m_cal, m_parent );
        m_mine = false;
    }
    setCommandType( 0 );
    //kDebug()<<m_cal->name()<<" added to:"<<m_project->name();
}

void CalendarAddCmd::unexecute()
{
    if ( m_project ) {
        m_project->takeCalendar( m_cal );
        m_mine = true;
    }
    setCommandType( 0 );
    //kDebug()<<m_cal->name();
}

CalendarRemoveCmd::CalendarRemoveCmd( Part *part, Project *project, Calendar *cal, const QString& name )
        : NamedCommand( part, name ),
        m_project( project ),
        m_parent( cal->parentCal() ),
        m_cal( cal ),
        m_mine( false ),
        m_cmd( new K3MacroCommand("") )
{
    Q_ASSERT( project != 0 );

/*    foreach ( Schedule * s, project->schedules() ) {
        addSchScheduled( s );
    }*/
    foreach ( Resource *r, project->resourceList() ) {
        if ( r->calendar( true ) == cal ) {
            m_cmd->addCommand( new ModifyResourceCalendarCmd( part, r, 0 ) );
            break;
        }
    }
    if ( project->defaultCalendar() == cal ) {
        m_cmd->addCommand( new ProjectModifyDefaultCalendarCmd( part, project, 0 ) );
    }
    foreach ( Calendar *c, cal->calendars() ) {
        m_cmd->addCommand( new CalendarRemoveCmd( part, project, c ) );
    }
}
CalendarRemoveCmd::~CalendarRemoveCmd()
{
    delete m_cmd;
    if ( m_mine )
        delete m_cal;
}
void CalendarRemoveCmd::execute()
{
//    setSchScheduled( false );
    m_cmd->execute();
    m_project->takeCalendar( m_cal );
    m_mine = true;
    setCommandType( 1 );
}
void CalendarRemoveCmd::unexecute()
{
    m_project->addCalendar( m_cal, m_parent );
    m_cmd->unexecute();
//    setSchScheduled();
    m_mine = false;
    setCommandType( 0 );
}

CalendarModifyNameCmd::CalendarModifyNameCmd( Part *part, Calendar *cal, const QString& newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_cal( cal )
{

    m_oldvalue = cal->name();
    m_newvalue = newvalue;
    //kDebug()<<cal->name();
}
void CalendarModifyNameCmd::execute()
{
    m_cal->setName( m_newvalue );
    setCommandType( 0 );
    //kDebug()<<m_cal->name();
}
void CalendarModifyNameCmd::unexecute()
{
    m_cal->setName( m_oldvalue );
    setCommandType( 0 );
    //kDebug()<<m_cal->name();
}

CalendarModifyParentCmd::CalendarModifyParentCmd( Part *part, Project *project, Calendar *cal, Calendar *newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_project( project ),
        m_cal( cal ),
        m_cmd( new K3MacroCommand( "" ) )
{
    m_oldvalue = cal->parentCal();
    m_newvalue = newvalue;
    if ( newvalue ) {
        m_cmd->addCommand( new CalendarModifyTimeZoneCmd( part, cal, newvalue->timeZone() ) );
    }
    //kDebug()<<cal->name();
/*    if ( part ) {
        foreach ( Schedule * s, part->getProject().schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
CalendarModifyParentCmd::~CalendarModifyParentCmd()
{
    delete m_cmd;
}
void CalendarModifyParentCmd::execute()
{
    m_project->takeCalendar( m_cal );
    m_project->addCalendar( m_cal, m_newvalue );
    m_cmd->execute();
//    setSchScheduled( false );
    setCommandType( 1 );
}
void CalendarModifyParentCmd::unexecute()
{
    m_cmd->unexecute();
    m_project->takeCalendar( m_cal );
    m_project->addCalendar( m_cal, m_oldvalue );
//    setSchScheduled();
    setCommandType( 1 );
}

CalendarModifyTimeZoneCmd::CalendarModifyTimeZoneCmd( Part *part, Calendar *cal, const KTimeZone &value, const QString& name )
        : NamedCommand( part, name ),
        m_cal( cal ),
        m_newvalue( value ),
        m_cmd( new K3MacroCommand( "" ) )
{
    m_oldvalue = cal->timeZone();
    foreach ( Calendar *c, cal->calendars() ) {
        m_cmd->addCommand( new CalendarModifyTimeZoneCmd( part, c, value ) );
    }
    //kDebug()<<cal->name();
/*    if ( part ) {
        foreach ( Schedule * s, part->getProject().schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
CalendarModifyTimeZoneCmd::~CalendarModifyTimeZoneCmd()
{
    delete m_cmd;
}
void CalendarModifyTimeZoneCmd::execute()
{
    m_cmd->execute();
    m_cal->setTimeZone( m_newvalue );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void CalendarModifyTimeZoneCmd::unexecute()
{
    m_cal->setTimeZone( m_oldvalue );
    m_cmd->unexecute();
//    setSchScheduled();
    setCommandType( 1 );
}

CalendarAddDayCmd::CalendarAddDayCmd( Part *part, Calendar *cal, CalendarDay *newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_cal( cal ),
        m_mine( true )
{

    m_newvalue = newvalue;
    //kDebug()<<cal->name();
/*    if ( part ) {
        foreach ( Schedule * s, part->getProject().schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
CalendarAddDayCmd::~CalendarAddDayCmd()
{
    //kDebug();
    if ( m_mine )
        delete m_newvalue;
}
void CalendarAddDayCmd::execute()
{
    //kDebug()<<m_cal->name();
    m_cal->addDay( m_newvalue );
    m_mine = false;
//    setSchScheduled( false );
    setCommandType( 1 );
}
void CalendarAddDayCmd::unexecute()
{
    //kDebug()<<m_cal->name();
    m_cal->takeDay( m_newvalue );
    m_mine = true;
//    setSchScheduled();
    setCommandType( 1 );
}

CalendarRemoveDayCmd::CalendarRemoveDayCmd( Part *part, Calendar *cal,CalendarDay *day, const QString& name )
        : NamedCommand( part, name ),
        m_cal( cal ),
        m_value( day ),
        m_mine( false )
{
    //kDebug()<<cal->name();
    // TODO check if any resources uses this calendar
    init();
}
CalendarRemoveDayCmd::CalendarRemoveDayCmd( Part *part, Calendar *cal, const QDate &day, const QString& name )
        : NamedCommand( part, name ),
        m_cal( cal ),
        m_mine( false )
{

    m_value = cal->findDay( day );
    //kDebug()<<cal->name();
    // TODO check if any resources uses this calendar
    init();
}
void CalendarRemoveDayCmd::init()
{
/*    if ( m_part ) {
        foreach ( Schedule * s, m_part->getProject().schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
void CalendarRemoveDayCmd::execute()
{
    //kDebug()<<m_cal->name();
    m_cal->takeDay( m_value );
    m_mine = true;
//    setSchScheduled( false );
    setCommandType( 1 );
}
void CalendarRemoveDayCmd::unexecute()
{
    //kDebug()<<m_cal->name();
    m_cal->addDay( m_value );
    m_mine = false;
//    setSchScheduled();
    setCommandType( 1 );
}

CalendarModifyDayCmd::CalendarModifyDayCmd( Part *part, Calendar *cal, CalendarDay *value, const QString& name )
        : NamedCommand( part, name ),
        m_cal( cal ),
        m_mine( true )
{

    m_newvalue = value;
    m_oldvalue = cal->findDay( value->date() );
    //kDebug()<<cal->name()<<" old:("<<m_oldvalue<<") new:("<<m_newvalue<<")";
/*    if ( part ) {
        foreach ( Schedule * s, part->getProject().schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
CalendarModifyDayCmd::~CalendarModifyDayCmd()
{
    //kDebug();
    if ( m_mine ) {
        delete m_newvalue;
    } else {
        delete m_oldvalue;
    }
}
void CalendarModifyDayCmd::execute()
{
    //kDebug();
    m_cal->takeDay( m_oldvalue );
    m_cal->addDay( m_newvalue );
    m_mine = false;
//    setSchScheduled( false );
    setCommandType( 1 );
}
void CalendarModifyDayCmd::unexecute()
{
    //kDebug();
    m_cal->takeDay( m_newvalue );
    m_cal->addDay( m_oldvalue );
    m_mine = true;
//    setSchScheduled();
    setCommandType( 1 );
}

CalendarModifyStateCmd::CalendarModifyStateCmd( Part *part, Calendar *calendar, CalendarDay *day, CalendarDay::State value, const QString& name )
        : NamedCommand( part, name ),
        m_calendar( calendar ),
        m_day( day ),
        m_cmd( new K3MacroCommand( "" ) )
{

    m_newvalue = value;
    m_oldvalue = (CalendarDay::State)day->state();
    if ( value != CalendarDay::Working ) {
        foreach ( TimeInterval *ti, day->workingIntervals() ) {
            m_cmd->addCommand( new CalendarRemoveTimeIntervalCmd( part, calendar, day, ti ) );
        }
    }
}
CalendarModifyStateCmd::~CalendarModifyStateCmd()
{
    delete m_cmd;
}
void CalendarModifyStateCmd::execute()
{
    //kDebug();
    m_cmd->execute();
    m_calendar->setState( m_day, m_newvalue );
    setCommandType( 1 );
}
void CalendarModifyStateCmd::unexecute()
{
    //kDebug();
    m_calendar->setState( m_day, m_oldvalue );
    m_cmd->unexecute();
    setCommandType( 0 );
}

CalendarModifyTimeIntervalCmd::CalendarModifyTimeIntervalCmd( Part *part, Calendar *calendar, TimeInterval &newvalue, TimeInterval *value, const QString& name )
        : NamedCommand( part, name ),
        m_calendar( calendar )
{

    m_value = value; // keep pointer
    m_oldvalue = *value; // save value
    m_newvalue = newvalue;
}
void CalendarModifyTimeIntervalCmd::execute()
{
    //kDebug();
    m_calendar->setWorkInterval( m_value, m_newvalue );
    setCommandType( 1 );
}
void CalendarModifyTimeIntervalCmd::unexecute()
{
    //kDebug();
    m_calendar->setWorkInterval( m_value, m_oldvalue );
    setCommandType( 0 );
}

CalendarAddTimeIntervalCmd::CalendarAddTimeIntervalCmd( Part *part, Calendar *calendar, CalendarDay *day, TimeInterval *value, const QString& name )
    : NamedCommand( part, name ),
    m_calendar( calendar ),
    m_day( day ),
    m_value( value ),
    m_mine( true )
{
}
CalendarAddTimeIntervalCmd::~CalendarAddTimeIntervalCmd()
{
    if ( m_mine )
        delete m_value;
}
void CalendarAddTimeIntervalCmd::execute()
{
    //kDebug();
    m_calendar->addWorkInterval( m_day, m_value );
    m_mine = false;
    setCommandType( 1 );
}
void CalendarAddTimeIntervalCmd::unexecute()
{
    //kDebug();
    m_calendar->takeWorkInterval( m_day, m_value );
    m_mine = true;
    setCommandType( 0 );
}

CalendarRemoveTimeIntervalCmd::CalendarRemoveTimeIntervalCmd( Part *part, Calendar *calendar, CalendarDay *day, TimeInterval *value, const QString& name )
    : CalendarAddTimeIntervalCmd( part, calendar, day, value, name )
{
    m_mine = false ;
}
void CalendarRemoveTimeIntervalCmd::execute()
{
    CalendarAddTimeIntervalCmd::unexecute();
}
void CalendarRemoveTimeIntervalCmd::unexecute()
{
    CalendarAddTimeIntervalCmd::execute();
}

CalendarModifyWeekdayCmd::CalendarModifyWeekdayCmd( Part *part, Calendar *cal, int weekday, CalendarDay *value, const QString& name )
        : NamedCommand( part, name ),
        m_weekday( weekday ),
        m_cal( cal ),
        m_value( value ),
        m_orig( *( cal->weekday( weekday ) ) )
{

    //kDebug() << cal->name() <<" (" << value <<")";
/*    if ( part ) {
        foreach ( Schedule * s, part->getProject().schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
CalendarModifyWeekdayCmd::~CalendarModifyWeekdayCmd()
{
    //kDebug() << m_weekday <<":" << m_value;
    delete m_value;

}
void CalendarModifyWeekdayCmd::execute()
{
    m_cal->setWeekday( m_weekday, *m_value );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void CalendarModifyWeekdayCmd::unexecute()
{
    m_cal->setWeekday( m_weekday, m_orig );
//    setSchScheduled();
    setCommandType( 1 );
}

CalendarModifyDateCmd::CalendarModifyDateCmd( Part *part, Calendar *cal, CalendarDay *day, QDate &value, const QString& name )
    : NamedCommand( part, name ),
    m_cal( cal ),
    m_day( day ),
    m_newvalue( value ),
    m_oldvalue( day->date() )
{
    //kDebug() << cal->name() <<" (" << value <<")";
/*    if ( part ) {
        foreach ( Schedule * s, part->getProject().schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
void CalendarModifyDateCmd::execute()
{
    m_cal->setDate( m_day, m_newvalue );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void CalendarModifyDateCmd::unexecute()
{
    m_cal->setDate( m_day, m_oldvalue );
//    setSchScheduled();
    setCommandType( 0 );
}

ProjectModifyDefaultCalendarCmd::ProjectModifyDefaultCalendarCmd( Part *part, Project *project, Calendar *cal, const QString& name )
    : NamedCommand( part, name ),
    m_project( project ),
    m_newvalue( cal ),
    m_oldvalue( project->defaultCalendar() )
{
    //kDebug() << cal->name() <<" (" << value <<")";
}
void ProjectModifyDefaultCalendarCmd::execute()
{
    m_project->setDefaultCalendar( m_newvalue );
    setCommandType( 1 );
}
void ProjectModifyDefaultCalendarCmd::unexecute()
{
    m_project->setDefaultCalendar( m_oldvalue );
    setCommandType( 0 );
}

NodeDeleteCmd::NodeDeleteCmd( Part *part, Node *node, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        m_index( -1 )
{

    m_parent = node->parentNode();
    m_mine = false;
    
    m_project = static_cast<Project*>( node->projectNode() );
    if ( m_project ) {
        foreach ( Schedule * s, m_project->schedules() ) {
            if ( s && s->isScheduled() ) {
                // Only invalidate schedules this node is part of
                Schedule *ns = node->findSchedule( s->id() );
                if ( ns && ! ns->isDeleted() ) {
                    addSchScheduled( s );
                }
            }
        }
    }
    m_cmd = new K3MacroCommand( "" );
    foreach ( Relation * r, node->dependChildNodes() ) {
        m_cmd->addCommand( new DeleteRelationCmd( part, r ) );
    }
    foreach ( Relation * r, node->dependParentNodes() ) {
        m_cmd->addCommand( new DeleteRelationCmd( part, r ) );
    }
    QList<Node*> lst = node->childNodeIterator();
    for ( int i = lst.count(); i > 0; --i ) {
        m_cmd->addCommand( new NodeDeleteCmd( part, lst[ i - 1 ] ) );
    }

}
NodeDeleteCmd::~NodeDeleteCmd()
{
    if ( m_mine )
        delete m_node;
    delete m_cmd;
    while ( !m_appointments.isEmpty() )
        delete m_appointments.takeFirst();
}
void NodeDeleteCmd::execute()
{
    if ( m_parent && m_project ) {
        m_index = m_parent->findChildNode( m_node );
        //kDebug()<<m_node->name()<<""<<m_index;
/*        foreach ( Appointment * a, m_node->appointments() ) {
            a->detach();
            m_appointments.append( a );
        }*/
        if ( m_cmd ) {
            m_cmd->execute();
        }
        m_project->takeTask( m_node );
        m_mine = true;
        setSchScheduled( false );
        setCommandType( 1 );
    }
}
void NodeDeleteCmd::unexecute()
{
    if ( m_parent && m_project ) {
        //kDebug()<<m_node->name()<<""<<m_index;
        m_project->addSubTask( m_node, m_index, m_parent );
        if ( m_cmd ) {
            m_cmd->unexecute();
        }
/*        while ( !m_appointments.isEmpty() ) {
            m_appointments.takeFirst() ->attach();
        }*/
        m_mine = false;
        setSchScheduled();
        setCommandType( 1 );
    }
}

TaskAddCmd::TaskAddCmd( Part *part, Project *project, Node *node, Node *after, const QString& name )
        : NamedCommand( part, name ),
        m_project( project ),
        m_node( node ),
        m_after( after ),
        m_added( false )
{

    // set some reasonable defaults for normally calculated values
    if ( after && after->parentNode() && after->parentNode() != project ) {
        node->setStartTime( after->parentNode() ->startTime() );
        node->setEndTime( node->startTime() + node->duration() );
    } else {
        if ( project->constraint() == Node::MustFinishOn ) {
            node->setEndTime( project->endTime() );
            node->setStartTime( node->endTime() - node->duration() );
        } else {
            node->setStartTime( project->startTime() );
            node->setEndTime( node->startTime() + node->duration() );
        }
    }
    node->setEarlyStart( node->startTime() );
    node->setLateFinish( node->endTime() );
    node->setWorkStartTime( node->startTime() );
    node->setWorkEndTime( node->endTime() );
}
TaskAddCmd::~TaskAddCmd()
{
    if ( !m_added )
        delete m_node;
}
void TaskAddCmd::execute()
{
    //kDebug()<<m_node->name();
    m_project->addTask( m_node, m_after );
    m_added = true;

    setCommandType( 1 );
}
void TaskAddCmd::unexecute()
{
    m_project->takeTask( m_node );
    m_added = false;

    setCommandType( 1 );
}

SubtaskAddCmd::SubtaskAddCmd( Part *part, Project *project, Node *node, Node *parent, const QString& name )
        : NamedCommand( part, name ),
        m_project( project ),
        m_node( node ),
        m_parent( parent ),
        m_added( false ),
        m_cmd( 0 )
{

    // set some reasonable defaults for normally calculated values
    node->setStartTime( parent->startTime() );
    node->setEndTime( node->startTime() + node->duration() );
    node->setEarlyStart( node->startTime() );
    node->setLateFinish( node->endTime() );
    node->setWorkStartTime( node->startTime() );
    node->setWorkEndTime( node->endTime() );
    
    // Summarytasks can't have resources, so remove resource requests from the new parent
    ResourceRequestCollection *rc = parent->requests();
    if ( rc ) {
        foreach ( ResourceGroupRequest *r, rc->requests() ) {
            if ( m_cmd == 0 ) m_cmd = new K3MacroCommand( "" );
            m_cmd->addCommand( new RemoveResourceGroupRequestCmd( m_part, r ) );
        }
    }
}
SubtaskAddCmd::~SubtaskAddCmd()
{
    delete m_cmd;
    if ( !m_added )
        delete m_node;
}
void SubtaskAddCmd::execute()
{
    m_project->addSubTask( m_node, m_parent );
    if ( m_cmd ) {
        m_cmd->execute();
    }
    m_added = true;

    setCommandType( 1 );
}
void SubtaskAddCmd::unexecute()
{
    m_project->takeTask( m_node );
    if ( m_cmd ) {
        m_cmd->unexecute();
    }
    m_added = false;

    setCommandType( 1 );
}

NodeModifyNameCmd::NodeModifyNameCmd( Part *part, Node &node, const QString& nodename, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newName( nodename ),
        oldName( node.name() )
{
}
void NodeModifyNameCmd::execute()
{
    m_node.setName( newName );

    setCommandType( 0 );
}
void NodeModifyNameCmd::unexecute()
{
    m_node.setName( oldName );

    setCommandType( 0 );
}

NodeModifyLeaderCmd::NodeModifyLeaderCmd( Part *part, Node &node, const QString& leader, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newLeader( leader ),
        oldLeader( node.leader() )
{
}
void NodeModifyLeaderCmd::execute()
{
    m_node.setLeader( newLeader );

    setCommandType( 0 );
}
void NodeModifyLeaderCmd::unexecute()
{
    m_node.setLeader( oldLeader );

    setCommandType( 0 );
}

NodeModifyDescriptionCmd::NodeModifyDescriptionCmd( Part *part, Node &node, const QString& description, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newDescription( description ),
        oldDescription( node.description() )
{
}
void NodeModifyDescriptionCmd::execute()
{
    m_node.setDescription( newDescription );

    setCommandType( 0 );
}
void NodeModifyDescriptionCmd::unexecute()
{
    m_node.setDescription( oldDescription );

    setCommandType( 0 );
}

NodeModifyConstraintCmd::NodeModifyConstraintCmd( Part *part, Node &node, Node::ConstraintType c, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newConstraint( c ),
        oldConstraint( static_cast<Node::ConstraintType>( node.constraint() ) )
{

/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
}
void NodeModifyConstraintCmd::execute()
{
    m_node.setConstraint( newConstraint );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void NodeModifyConstraintCmd::unexecute()
{
    m_node.setConstraint( oldConstraint );
//    setSchScheduled();
    setCommandType( 1 );
}

NodeModifyConstraintStartTimeCmd::NodeModifyConstraintStartTimeCmd( Part *part, Node &node, const QDateTime& dt, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newTime( dt ),
        oldTime( node.constraintStartTime() )
{
    m_spec = part->getProject().timeSpec();
/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
}
void NodeModifyConstraintStartTimeCmd::execute()
{
    m_node.setConstraintStartTime( DateTime( newTime, m_spec ) );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void NodeModifyConstraintStartTimeCmd::unexecute()
{
    m_node.setConstraintStartTime( oldTime );
//    setSchScheduled();
    setCommandType( 1 );
}

NodeModifyConstraintEndTimeCmd::NodeModifyConstraintEndTimeCmd( Part *part, Node &node, const QDateTime& dt, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newTime( dt ),
        oldTime( node.constraintEndTime() )
{
    m_spec = part->getProject().timeSpec();
/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
}
void NodeModifyConstraintEndTimeCmd::execute()
{
    m_node.setConstraintEndTime( DateTime( newTime, m_spec ) );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void NodeModifyConstraintEndTimeCmd::unexecute()
{
    m_node.setConstraintEndTime( oldTime );
//    setSchScheduled();
    setCommandType( 1 );
}

NodeModifyStartTimeCmd::NodeModifyStartTimeCmd( Part *part, Node &node, const QDateTime& dt, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newTime( dt ),
        oldTime( node.startTime() )
{
    m_spec = part->getProject().timeSpec();
}
void NodeModifyStartTimeCmd::execute()
{
    m_node.setStartTime( DateTime( newTime, m_spec ) );

    setCommandType( 1 );
}
void NodeModifyStartTimeCmd::unexecute()
{
    m_node.setStartTime( oldTime );

    setCommandType( 1 );
}

NodeModifyEndTimeCmd::NodeModifyEndTimeCmd( Part *part, Node &node, const QDateTime& dt, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newTime( dt ),
        oldTime( node.endTime() )
{
    m_spec = part->getProject().timeSpec();
}
void NodeModifyEndTimeCmd::execute()
{
    m_node.setEndTime( DateTime( newTime, m_spec ) );

    setCommandType( 1 );
}
void NodeModifyEndTimeCmd::unexecute()
{
    m_node.setEndTime( oldTime );

    setCommandType( 1 );
}

NodeModifyIdCmd::NodeModifyIdCmd( Part *part, Node &node, const QString& id, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newId( id ),
        oldId( node.id() )
{
}
void NodeModifyIdCmd::execute()
{
    m_node.setId( newId );

    setCommandType( 0 );
}
void NodeModifyIdCmd::unexecute()
{
    m_node.setId( oldId );

    setCommandType( 0 );
}

NodeIndentCmd::NodeIndentCmd( Part *part, Node &node, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        m_newparent( 0 ),
        m_newindex( -1 ),
        m_cmd( 0 )
{
}
NodeIndentCmd::~NodeIndentCmd()
{
    delete m_cmd;
}
void NodeIndentCmd::execute()
{
    m_oldparent = m_node.parentNode();
    m_oldindex = m_oldparent->findChildNode( &m_node );
    Project *p = dynamic_cast<Project *>( m_node.projectNode() );
    if ( p && p->indentTask( &m_node, m_newindex ) ) {
        m_newparent = m_node.parentNode();
        m_newindex = m_newparent->findChildNode( &m_node );
        // Summarytasks can't have resources, so remove resource requests from the new parent
        if ( m_cmd == 0 ) {
            ResourceRequestCollection *rc = m_newparent->requests();
            if ( rc ) {
                foreach ( ResourceGroupRequest *r, rc->requests() ) {
                    if ( m_cmd == 0 ) m_cmd = new K3MacroCommand( "" );
                    m_cmd->addCommand( new RemoveResourceGroupRequestCmd( m_part, r ) );
                }
            }
        }
        if ( m_cmd ) {
            m_cmd->execute();
        }
    }

    setCommandType( 1 );
}
void NodeIndentCmd::unexecute()
{
    Project * p = dynamic_cast<Project *>( m_node.projectNode() );
    if ( m_newindex != -1 && p && p->unindentTask( &m_node ) ) {
        m_newindex = -1;
        if ( m_cmd ) {
            m_cmd->unexecute();
        }
    }

    setCommandType( 1 );
}

NodeUnindentCmd::NodeUnindentCmd( Part *part, Node &node, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        m_newparent( 0 ),
        m_newindex( -1 )
{}
void NodeUnindentCmd::execute()
{
    m_oldparent = m_node.parentNode();
    m_oldindex = m_oldparent->findChildNode( &m_node );
    Project *p = dynamic_cast<Project *>( m_node.projectNode() );
    if ( p && p->unindentTask( &m_node ) ) {
        m_newparent = m_node.parentNode();
        m_newindex = m_newparent->findChildNode( &m_node );
    }

    setCommandType( 1 );
}
void NodeUnindentCmd::unexecute()
{
    Project * p = dynamic_cast<Project *>( m_node.projectNode() );
    if ( m_newindex != -1 && p && p->indentTask( &m_node, m_oldindex ) ) {
        m_newindex = -1;
    }

    setCommandType( 1 );
}

NodeMoveUpCmd::NodeMoveUpCmd( Part *part, Node &node, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        m_moved( false )
{

    m_project = static_cast<Project *>( m_node.projectNode() );
}
void NodeMoveUpCmd::execute()
{
    if ( m_project ) {
        m_moved = m_project->moveTaskUp( &m_node );
    }

    setCommandType( 0 );
}
void NodeMoveUpCmd::unexecute()
{
    if ( m_project && m_moved ) {
        m_project->moveTaskDown( &m_node );
    }
    m_moved = false;
    setCommandType( 0 );
}

NodeMoveDownCmd::NodeMoveDownCmd( Part *part, Node &node, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        m_moved( false )
{

    m_project = static_cast<Project *>( m_node.projectNode() );
}
void NodeMoveDownCmd::execute()
{
    if ( m_project ) {
        m_moved = m_project->moveTaskDown( &m_node );
    }
    setCommandType( 0 );
}
void NodeMoveDownCmd::unexecute()
{
    if ( m_project && m_moved ) {
        m_project->moveTaskUp( &m_node );
    }
    m_moved = false;
    setCommandType( 0 );
}

NodeMoveCmd::NodeMoveCmd( Part *part, Project *project, Node *node, Node *newParent, int newPos, const QString& name )
    : NamedCommand( part, name ),
    m_project( project ),
    m_node( node ),
    m_newparent( newParent ),
    m_newpos( newPos ),
    m_moved( false ),
    m_cmd( 0 )
{
    m_oldparent = node->parentNode();
    Q_ASSERT( m_oldparent );
    m_oldpos = m_oldparent->indexOf( node );
    
    // Summarytasks can't have resources, so remove resource requests from the new parent
    ResourceRequestCollection *rc = newParent->requests();
    if ( rc ) {
        foreach ( ResourceGroupRequest *r, rc->requests() ) {
            if ( m_cmd == 0 ) m_cmd = new K3MacroCommand( "" );
            m_cmd->addCommand( new RemoveResourceGroupRequestCmd( m_part, r ) );
        }
    }
    // TODO appointments ??
}
NodeMoveCmd::~NodeMoveCmd()
{
    delete m_cmd;
}
void NodeMoveCmd::execute()
{
    if ( m_project ) {
        m_moved = m_project->moveTask( m_node, m_newparent, m_newpos );
        if ( m_moved && m_cmd ) {
            m_cmd->execute();
        }
    }
    setCommandType( 0 );
}
void NodeMoveCmd::unexecute()
{
    if ( m_project && m_moved ) {
        m_moved = m_project->moveTask( m_node, m_oldparent, m_oldpos );
        if ( m_moved && m_cmd ) {
            m_cmd->unexecute();
        }
    }
    m_moved = false;
    setCommandType( 0 );
}

AddRelationCmd::AddRelationCmd( Part *part, Relation *rel, const QString& name )
        : NamedCommand( part, name ),
        m_rel( rel ),
        m_project( part->getProject() )
{
    m_taken = true;
/*    Node *p = rel->parent() ->projectNode();
    if ( p ) {
        foreach ( Schedule * s, p->schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
AddRelationCmd::~AddRelationCmd()
{
    if ( m_taken )
        delete m_rel;
}
void AddRelationCmd::execute()
{
    //kDebug()<<m_rel->parent()<<" to"<<m_rel->child();
    m_taken = false;
    m_project.addRelation( m_rel, false );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void AddRelationCmd::unexecute()
{
    m_taken = true;
    m_project.takeRelation( m_rel );
//    setSchScheduled();
    setCommandType( 1 );
}

DeleteRelationCmd::DeleteRelationCmd( Part *part, Relation *rel, const QString& name )
        : NamedCommand( part, name ),
        m_rel( rel ),
        m_project( part->getProject() )
{

    m_taken = false;
/*    Node *p = rel->parent() ->projectNode();
    if ( p ) {
        foreach ( Schedule * s, p->schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
DeleteRelationCmd::~DeleteRelationCmd()
{
    if ( m_taken )
        delete m_rel;
}
void DeleteRelationCmd::execute()
{
    //kDebug()<<m_rel->parent()<<" to"<<m_rel->child();
    m_taken = true;
    m_project.takeRelation( m_rel );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void DeleteRelationCmd::unexecute()
{
    m_taken = false;
    m_project.addRelation( m_rel, false );
//    setSchScheduled();
    setCommandType( 1 );
}

ModifyRelationTypeCmd::ModifyRelationTypeCmd( Part *part, Relation *rel, Relation::Type type, const QString& name )
        : NamedCommand( part, name ),
        m_rel( rel ),
        m_newtype( type )
{

    m_oldtype = rel->type();
    m_project = dynamic_cast<Project*>( rel->parent() ->projectNode() );
/*    if ( m_project ) {
        foreach ( Schedule * s, m_project->schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
void ModifyRelationTypeCmd::execute()
{
    if ( m_project ) {
        m_project->setRelationType( m_rel, m_newtype );
//        setSchScheduled( false );
        setCommandType( 1 );
    }
}
void ModifyRelationTypeCmd::unexecute()
{
    if ( m_project ) {
        m_project->setRelationType( m_rel, m_oldtype );
//        setSchScheduled();
        setCommandType( 1 );
    }
}

ModifyRelationLagCmd::ModifyRelationLagCmd( Part *part, Relation *rel, Duration lag, const QString& name )
        : NamedCommand( part, name ),
        m_rel( rel ),
        m_newlag( lag )
{

    m_oldlag = rel->lag();
    m_project = dynamic_cast<Project*>( rel->parent() ->projectNode() );
/*    if ( m_project ) {
        foreach ( Schedule * s, m_project->schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
void ModifyRelationLagCmd::execute()
{
    if ( m_project ) {
        m_project->setRelationLag( m_rel, m_newlag );
//        setSchScheduled( false );
        setCommandType( 1 );
    }
}
void ModifyRelationLagCmd::unexecute()
{
    if ( m_project ) {
        m_project->setRelationLag( m_rel, m_oldlag );
//        setSchScheduled();
        setCommandType( 1 );
    }
}

AddResourceRequestCmd::AddResourceRequestCmd( Part *part, ResourceGroupRequest *group, ResourceRequest *request, const QString& name )
        : NamedCommand( part, name ),
        m_group( group ),
        m_request( request )
{

    m_mine = true;
}
AddResourceRequestCmd::~AddResourceRequestCmd()
{
    if ( m_mine )
        delete m_request;
}
void AddResourceRequestCmd::execute()
{
    //kDebug()<<"group="<<m_group<<" req="<<m_request;
    m_group->addResourceRequest( m_request );
    m_mine = false;
//    setSchScheduled( false );
    setCommandType( 1 );
}
void AddResourceRequestCmd::unexecute()
{
    //kDebug()<<"group="<<m_group<<" req="<<m_request;
    m_group->takeResourceRequest( m_request );
    m_mine = true;
//    setSchScheduled();
    setCommandType( 1 );
}

RemoveResourceRequestCmd::RemoveResourceRequestCmd( Part *part, ResourceGroupRequest *group, ResourceRequest *request, const QString& name )
        : NamedCommand( part, name ),
        m_group( group ),
        m_request( request )
{

    m_mine = false;
    //kDebug()<<"group req="<<group<<" req="<<request<<" to gr="<<m_group->group();
/*    Task *t = request->task();
    if ( t ) { // safety, something is seriously wrong!
        foreach ( Schedule * s, t->schedules() ) {
            addSchScheduled( s );
        }
    }*/
}
RemoveResourceRequestCmd::~RemoveResourceRequestCmd()
{
    if ( m_mine )
        delete m_request;
}
void RemoveResourceRequestCmd::execute()
{
    m_group->takeResourceRequest( m_request );
    m_mine = true;
//    setSchScheduled( false );
    setCommandType( 1 );
}
void RemoveResourceRequestCmd::unexecute()
{
    m_group->addResourceRequest( m_request );
    m_mine = false;
//    setSchScheduled();
    setCommandType( 1 );
}

ModifyEstimateCmd::ModifyEstimateCmd( Part *part, Node &node, Duration oldvalue, Duration newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_estimate( node.estimate() ),
        m_oldvalue( oldvalue ),
        m_newvalue( newvalue ),
        m_cmd( 0 )
{
/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
    if ( newvalue == Duration::zeroDuration ) {
        // Milestones can't have resources, so remove resource requests
        ResourceRequestCollection *rc = node.requests();
        if ( rc ) {
            foreach ( ResourceGroupRequest *r, rc->requests() ) {
                if ( m_cmd == 0 ) m_cmd = new K3MacroCommand( "" );
                m_cmd->addCommand( new RemoveResourceGroupRequestCmd( m_part, r ) );
            }
        }
    }
}
ModifyEstimateCmd::~ModifyEstimateCmd()
{
    delete m_cmd;
}
void ModifyEstimateCmd::execute()
{
    m_estimate->set( m_newvalue );
//    setSchScheduled( false );
    if ( m_cmd ) {
        m_cmd->execute();
    }
    setCommandType( 1 );
}
void ModifyEstimateCmd::unexecute()
{
    m_estimate->set( m_oldvalue );
    if ( m_cmd ) {
        m_cmd->unexecute();
    }
//    setSchScheduled();
    setCommandType( 1 );
}

EstimateModifyOptimisticRatioCmd::EstimateModifyOptimisticRatioCmd( Part *part, Node &node, int oldvalue, int newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_estimate( node.estimate() ),
        m_oldvalue( oldvalue ),
        m_newvalue( newvalue )
{

/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
}
void EstimateModifyOptimisticRatioCmd::execute()
{
    m_estimate->setOptimisticRatio( m_newvalue );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void EstimateModifyOptimisticRatioCmd::unexecute()
{
    m_estimate->setOptimisticRatio( m_oldvalue );
//    setSchScheduled();
    setCommandType( 1 );
}

EstimateModifyPessimisticRatioCmd::EstimateModifyPessimisticRatioCmd( Part *part, Node &node, int oldvalue, int newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_estimate( node.estimate() ),
        m_oldvalue( oldvalue ),
        m_newvalue( newvalue )
{

/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
}
void EstimateModifyPessimisticRatioCmd::execute()
{
    m_estimate->setPessimisticRatio( m_newvalue );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void EstimateModifyPessimisticRatioCmd::unexecute()
{
    m_estimate->setPessimisticRatio( m_oldvalue );
//    setSchScheduled();
    setCommandType( 1 );
}

ModifyEstimateTypeCmd::ModifyEstimateTypeCmd( Part *part, Node &node, int oldvalue, int newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_estimate( node.estimate() ),
        m_oldvalue( oldvalue ),
        m_newvalue( newvalue )
{

/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
}
void ModifyEstimateTypeCmd::execute()
{
    m_estimate->setType( static_cast<Estimate::Type>( m_newvalue ) );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void ModifyEstimateTypeCmd::unexecute()
{
    m_estimate->setType( static_cast<Estimate::Type>( m_oldvalue ) );
//    setSchScheduled();
    setCommandType( 1 );
}

ModifyEstimateUnitCmd::ModifyEstimateUnitCmd( Part *part, Node &node, Duration::Unit oldvalue, Duration::Unit newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_estimate( node.estimate() ),
        m_oldvalue( oldvalue ),
        m_newvalue( newvalue )
{
}
void ModifyEstimateUnitCmd::execute()
{
    m_estimate->setDisplayUnit( m_newvalue );
    setCommandType( 0 );
}
void ModifyEstimateUnitCmd::unexecute()
{
    m_estimate->setDisplayUnit( m_oldvalue );
}

EstimateModifyRiskCmd::EstimateModifyRiskCmd( Part *part, Node &node, int oldvalue, int newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_estimate( node.estimate() ),
        m_oldvalue( oldvalue ),
        m_newvalue( newvalue )
{

/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
}
void EstimateModifyRiskCmd::execute()
{
    m_estimate->setRisktype( static_cast<Estimate::Risktype>( m_newvalue ) );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void EstimateModifyRiskCmd::unexecute()
{
    m_estimate->setRisktype( static_cast<Estimate::Risktype>( m_oldvalue ) );
//    setSchScheduled();
    setCommandType( 1 );
}

AddResourceGroupRequestCmd::AddResourceGroupRequestCmd( Part *part, Task &task, ResourceGroupRequest *request, const QString& name )
        : NamedCommand( part, name ),
        m_task( task ),
        m_request( request )
{

    m_mine = true;
}
void AddResourceGroupRequestCmd::execute()
{
    //kDebug()<<"group="<<m_request;
    m_task.addRequest( m_request );
    m_mine = false;

    setCommandType( 1 );
}
void AddResourceGroupRequestCmd::unexecute()
{
    //kDebug()<<"group="<<m_request;
    m_task.takeRequest( m_request ); // group should now be empty of resourceRequests
    m_mine = true;

    setCommandType( 1 );
}

RemoveResourceGroupRequestCmd::RemoveResourceGroupRequestCmd( Part *part, ResourceGroupRequest *request, const QString& name )
        : NamedCommand( part, name ),
        m_task( request->parent() ->task() ),
        m_request( request )
{

    m_mine = false;
}

RemoveResourceGroupRequestCmd::RemoveResourceGroupRequestCmd( Part *part, Task &task, ResourceGroupRequest *request, const QString& name )
        : NamedCommand( part, name ),
        m_task( task ),
        m_request( request )
{

    m_mine = false;
}
void RemoveResourceGroupRequestCmd::execute()
{
    //kDebug()<<"group="<<m_request;
    m_task.takeRequest( m_request ); // group should now be empty of resourceRequests
    m_mine = true;

    setCommandType( 1 );
}
void RemoveResourceGroupRequestCmd::unexecute()
{
    //kDebug()<<"group="<<m_request;
    m_task.addRequest( m_request );
    m_mine = false;

    setCommandType( 1 );
}

AddResourceCmd::AddResourceCmd( Part *part, ResourceGroup *group, Resource *resource, const QString& name )
        : NamedCommand( part, name ),
        m_group( group ),
        m_resource( resource )
{
    m_index = group->indexOf( resource );
    m_mine = true;
}
AddResourceCmd::~AddResourceCmd()
{
    if ( m_mine ) {
        //kDebug()<<"delete:"<<m_resource;
        delete m_resource;
    }
}
void AddResourceCmd::execute()
{
    Q_ASSERT( m_group->project() );
    if ( m_group->project() ) {
        m_group->project()->addResource( m_group, m_resource, m_index );
        m_mine = false;
        //kDebug()<<"added:"<<m_resource;
    }
    setCommandType( 0 );
}
void AddResourceCmd::unexecute()
{
    Q_ASSERT( m_group->project() );
    if ( m_group->project() ) {
        m_group->project()->takeResource( m_group, m_resource );
        //kDebug()<<"removed:"<<m_resource;
        m_mine = true;
    }
    setCommandType( 0 );
    Q_ASSERT( m_group->project() );
}

RemoveResourceCmd::RemoveResourceCmd( Part *part, ResourceGroup *group, Resource *resource, const QString& name )
        : AddResourceCmd( part, group, resource, name )
{
    //kDebug()<<resource;
    m_mine = false;
    m_requests = m_resource->requests();

    if ( group->project() ) {
        foreach ( Schedule * s, group->project()->schedules() ) {
            Schedule *rs = resource->findSchedule( s->id() );
            if ( rs && ! rs->isDeleted() ) {
                kDebug()<<s->name();
                addSchScheduled( s );
            }
        }
    }
}
RemoveResourceCmd::~RemoveResourceCmd()
{
    while ( !m_appointments.isEmpty() )
        delete m_appointments.takeFirst();
}
void RemoveResourceCmd::execute()
{
    foreach ( ResourceRequest * r, m_requests ) {
        r->parent() ->takeResourceRequest( r );
        //kDebug()<<"Remove request for"<<r->resource()->name();
    }
/*    foreach ( Appointment * a, m_resource->appointments() ) {
        m_appointments.append( a );
    }
    foreach ( Appointment * a, m_appointments ) {
        a->detach(); //NOTE: removes from m_resource->appointments()
        //kDebug()<<"detached:"<<a;
    }*/
    AddResourceCmd::unexecute();
    setSchScheduled( false );
}
void RemoveResourceCmd::unexecute()
{
/*    while ( !m_appointments.isEmpty() ) {
        //kDebug()<<"attach:"<<m_appointments.first();
        m_appointments.takeFirst() ->attach();
    }*/
    foreach ( ResourceRequest * r, m_requests ) {
        r->parent() ->addResourceRequest( r );
        //kDebug()<<"Add request for"<<r->resource()->name();
    }
    AddResourceCmd::execute();
    setSchScheduled();
}

ModifyResourceNameCmd::ModifyResourceNameCmd( Part *part, Resource *resource, const QString& value, const QString& name )
        : NamedCommand( part, name ),
        m_resource( resource ),
        m_newvalue( value )
{
    m_oldvalue = resource->name();
}
void ModifyResourceNameCmd::execute()
{
    m_resource->setName( m_newvalue );

    setCommandType( 0 );
}
void ModifyResourceNameCmd::unexecute()
{
    m_resource->setName( m_oldvalue );

    setCommandType( 0 );
}
ModifyResourceInitialsCmd::ModifyResourceInitialsCmd( Part *part, Resource *resource, const QString& value, const QString& name )
        : NamedCommand( part, name ),
        m_resource( resource ),
        m_newvalue( value )
{
    m_oldvalue = resource->initials();
}
void ModifyResourceInitialsCmd::execute()
{
    m_resource->setInitials( m_newvalue );

    setCommandType( 0 );
}
void ModifyResourceInitialsCmd::unexecute()
{
    m_resource->setInitials( m_oldvalue );

    setCommandType( 0 );
}
ModifyResourceEmailCmd::ModifyResourceEmailCmd( Part *part, Resource *resource, const QString& value, const QString& name )
        : NamedCommand( part, name ),
        m_resource( resource ),
        m_newvalue( value )
{
    m_oldvalue = resource->email();
}
void ModifyResourceEmailCmd::execute()
{
    m_resource->setEmail( m_newvalue );

    setCommandType( 0 );
}
void ModifyResourceEmailCmd::unexecute()
{
    m_resource->setEmail( m_oldvalue );

    setCommandType( 0 );
}
ModifyResourceTypeCmd::ModifyResourceTypeCmd( Part *part, Resource *resource, int value, const QString& name )
        : NamedCommand( part, name ),
        m_resource( resource ),
        m_newvalue( value )
{
    m_oldvalue = resource->type();

/*    foreach ( Schedule * s, resource->schedules() ) {
        addSchScheduled( s );
    }*/
}
void ModifyResourceTypeCmd::execute()
{
    m_resource->setType( ( Resource::Type ) m_newvalue );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void ModifyResourceTypeCmd::unexecute()
{
    m_resource->setType( ( Resource::Type ) m_oldvalue );
//    setSchScheduled();
    setCommandType( 1 );
}
ModifyResourceUnitsCmd::ModifyResourceUnitsCmd( Part *part, Resource *resource, int value, const QString& name )
        : NamedCommand( part, name ),
        m_resource( resource ),
        m_newvalue( value )
{
    m_oldvalue = resource->units();

/*    foreach ( Schedule * s, resource->schedules() ) {
        addSchScheduled( s );
    }*/
}
void ModifyResourceUnitsCmd::execute()
{
    m_resource->setUnits( m_newvalue );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void ModifyResourceUnitsCmd::unexecute()
{
    m_resource->setUnits( m_oldvalue );
//    setSchScheduled();
    setCommandType( 1 );
}

ModifyResourceAvailableFromCmd::ModifyResourceAvailableFromCmd( Part *part, Resource *resource, const QDateTime& value, const QString& name )
        : NamedCommand( part, name ),
        m_resource( resource ),
        m_newvalue( value )
{
    m_oldvalue = resource->availableFrom();
    m_spec = resource->timeSpec();
/*    DateTime v = DateTime( value, m_spec );
    if ( resource->project() ) {
        DateTime s;
        DateTime e;
        foreach ( Schedule * rs, resource->schedules() ) {
            Schedule * sch = resource->project() ->findSchedule( rs->id() );
            if ( sch ) {
                s = sch->start();
                e = sch->end();
                //kDebug() <<"old=" << m_oldvalue <<" new=" << value <<" s=" << s <<" e=" << e;
            }
            if ( !s.isValid() || !e.isValid() || ( ( m_oldvalue > s || v > s ) && ( m_oldvalue < e || v < e ) ) ) {
                addSchScheduled( rs );
            }
        }
    }*/
}
void ModifyResourceAvailableFromCmd::execute()
{
    m_resource->setAvailableFrom( DateTime( m_newvalue, m_spec ) );
//    setSchScheduled( false );
    setCommandType( 1 ); //FIXME
}
void ModifyResourceAvailableFromCmd::unexecute()
{
    m_resource->setAvailableFrom( m_oldvalue );
//    setSchScheduled();
    setCommandType( 1 ); //FIXME
}

ModifyResourceAvailableUntilCmd::ModifyResourceAvailableUntilCmd( Part *part, Resource *resource, const QDateTime& value, const QString& name )
        : NamedCommand( part, name ),
        m_resource( resource ),
        m_newvalue( value )
{
    m_oldvalue = resource->availableUntil();
    m_spec = resource->timeSpec();
/*    DateTime v = DateTime( value, m_spec );
    if ( resource->project() ) {
        DateTime s;
        DateTime e;
        foreach ( Schedule * rs, resource->schedules() ) {
            Schedule * sch = resource->project() ->findSchedule( rs->id() );
            if ( sch ) {
                s = sch->start();
                e = sch->end();
                //kDebug() <<"old=" << m_oldvalue <<" new=" << value <<" s=" << s <<" e=" << e;
            }
            if ( !s.isValid() || !e.isValid() || ( ( m_oldvalue > s || v > s ) && ( m_oldvalue < e || v < e ) ) ) {
                addSchScheduled( rs );
            }
        }
    }*/
}
void ModifyResourceAvailableUntilCmd::execute()
{
    m_resource->setAvailableUntil( DateTime( m_newvalue, m_spec ) );
//    setSchScheduled( false );
    setCommandType( 1 ); //FIXME
}
void ModifyResourceAvailableUntilCmd::unexecute()
{
    m_resource->setAvailableUntil( m_oldvalue );
//    setSchScheduled();
    setCommandType( 1 ); //FIXME
}

ModifyResourceNormalRateCmd::ModifyResourceNormalRateCmd( Part *part, Resource *resource, double value, const QString& name )
        : NamedCommand( part, name ),
        m_resource( resource ),
        m_newvalue( value )
{
    m_oldvalue = resource->normalRate();
}
void ModifyResourceNormalRateCmd::execute()
{
    m_resource->setNormalRate( m_newvalue );

    setCommandType( 0 );
}
void ModifyResourceNormalRateCmd::unexecute()
{
    m_resource->setNormalRate( m_oldvalue );

    setCommandType( 0 );
}
ModifyResourceOvertimeRateCmd::ModifyResourceOvertimeRateCmd( Part *part, Resource *resource, double value, const QString& name )
        : NamedCommand( part, name ),
        m_resource( resource ),
        m_newvalue( value )
{
    m_oldvalue = resource->overtimeRate();
}
void ModifyResourceOvertimeRateCmd::execute()
{
    m_resource->setOvertimeRate( m_newvalue );

    setCommandType( 0 );
}
void ModifyResourceOvertimeRateCmd::unexecute()
{
    m_resource->setOvertimeRate( m_oldvalue );

    setCommandType( 0 );
}

ModifyResourceCalendarCmd::ModifyResourceCalendarCmd( Part *part, Resource *resource, Calendar *value, const QString& name )
        : NamedCommand( part, name ),
        m_resource( resource ),
        m_newvalue( value )
{
    m_oldvalue = resource->calendar( true );

/*    foreach ( Schedule * s, resource->schedules() ) {
        addSchScheduled( s );
    }*/
}
void ModifyResourceCalendarCmd::execute()
{
    m_resource->setCalendar( m_newvalue );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void ModifyResourceCalendarCmd::unexecute()
{
    m_resource->setCalendar( m_oldvalue );
//    setSchScheduled();
    setCommandType( 1 );
}

RemoveResourceGroupCmd::RemoveResourceGroupCmd( Part *part, Project *project, ResourceGroup *group, const QString& name )
        : NamedCommand( part, name ),
        m_group( group ),
        m_project( project ),
        m_cmd( 0 )
{
    m_index = project->indexOf( group );
    m_mine = false;
    if ( !m_group->requests().isEmpty() ) {
        m_cmd = new K3MacroCommand("");
        foreach( ResourceGroupRequest * r, m_group->requests() ) {
            m_cmd->addCommand( new RemoveResourceGroupRequestCmd( part, r ) );
        }
    }
}
RemoveResourceGroupCmd::~RemoveResourceGroupCmd()
{
    delete m_cmd;
    if ( m_mine )
        delete m_group;
}
void RemoveResourceGroupCmd::execute()
{
    // remove all requests to this group
    int c = 0;
    if ( m_cmd ) {
        m_cmd->execute();
        c = 1;
    }
    if ( m_project )
        m_project->takeResourceGroup( m_group );
    m_mine = true;

    setCommandType( c );
}
void RemoveResourceGroupCmd::unexecute()
{
    int c = 0;
    if ( m_project )
        m_project->addResourceGroup( m_group, m_index );

    m_mine = false;
    // add all requests
    if ( m_cmd ) {
        m_cmd->unexecute();
        c = 1;
    }
    setCommandType( c );
}

AddResourceGroupCmd::AddResourceGroupCmd( Part *part, Project *project, ResourceGroup *group, const QString& name )
        : RemoveResourceGroupCmd( part, project, group, name )
{
    m_mine = true;
}
void AddResourceGroupCmd::execute()
{
    RemoveResourceGroupCmd::unexecute();
}
void AddResourceGroupCmd::unexecute()
{
    RemoveResourceGroupCmd::execute();
}

ModifyResourceGroupNameCmd::ModifyResourceGroupNameCmd( Part *part, ResourceGroup *group, const QString& value, const QString& name )
        : NamedCommand( part, name ),
        m_group( group ),
        m_newvalue( value )
{
    m_oldvalue = group->name();
}
void ModifyResourceGroupNameCmd::execute()
{
    m_group->setName( m_newvalue );

    setCommandType( 0 );
}
void ModifyResourceGroupNameCmd::unexecute()
{
    m_group->setName( m_oldvalue );

    setCommandType( 0 );
}

ModifyResourceGroupTypeCmd::ModifyResourceGroupTypeCmd( Part *part, ResourceGroup *group, int value, const QString& name )
    : NamedCommand( part, name ),
        m_group( group ),
        m_newvalue( value )
{
    m_oldvalue = group->type();
}
void ModifyResourceGroupTypeCmd::execute()
{
    m_group->setType( static_cast<ResourceGroup::Type>( m_newvalue) );

    setCommandType( 0 );
}
void ModifyResourceGroupTypeCmd::unexecute()
{
    m_group->setType( static_cast<ResourceGroup::Type>( m_oldvalue ) );

    setCommandType( 0 );
}

ModifyCompletionEntrymodeCmd::ModifyCompletionEntrymodeCmd( Part *part, Completion &completion, Completion::Entrymode value, const QString& name )
        : NamedCommand( part, name ),
        m_completion( completion ),
        oldvalue( m_completion.entrymode() ),
        newvalue( value )
{
}
void ModifyCompletionEntrymodeCmd::execute()
{
    m_completion.setEntrymode( newvalue );

    setCommandType( 0 );
}
void ModifyCompletionEntrymodeCmd::unexecute()
{
    m_completion.setEntrymode( oldvalue );

    setCommandType( 0 );
}

ModifyCompletionStartedCmd::ModifyCompletionStartedCmd( Part *part, Completion &completion, bool value, const QString& name )
        : NamedCommand( part, name ),
        m_completion( completion ),
        oldvalue( m_completion.isStarted() ),
        newvalue( value )
{
}
void ModifyCompletionStartedCmd::execute()
{
    m_completion.setStarted( newvalue );

    setCommandType( 0 );
}
void ModifyCompletionStartedCmd::unexecute()
{
    m_completion.setStarted( oldvalue );

    setCommandType( 0 );
}

ModifyCompletionFinishedCmd::ModifyCompletionFinishedCmd( Part *part, Completion &completion, bool value, const QString& name )
        : NamedCommand( part, name ),
        m_completion( completion ),
        oldvalue( m_completion.isFinished() ),
        newvalue( value )
{
}
void ModifyCompletionFinishedCmd::execute()
{
    m_completion.setFinished( newvalue );

    setCommandType( 0 );
}
void ModifyCompletionFinishedCmd::unexecute()
{
    m_completion.setFinished( oldvalue );

    setCommandType( 0 );
}

ModifyCompletionStartTimeCmd::ModifyCompletionStartTimeCmd( Part *part, Completion &completion, const QDateTime &value, const QString& name )
        : NamedCommand( part, name ),
        m_completion( completion ),
        oldvalue( m_completion.startTime() ),
        newvalue( value )
{
    m_spec = part->getProject().timeSpec();
}
void ModifyCompletionStartTimeCmd::execute()
{
    m_completion.setStartTime( DateTime( newvalue, m_spec ) );

    setCommandType( 0 );
}
void ModifyCompletionStartTimeCmd::unexecute()
{
    m_completion.setStartTime( oldvalue );

    setCommandType( 0 );
}

ModifyCompletionFinishTimeCmd::ModifyCompletionFinishTimeCmd( Part *part, Completion &completion, const QDateTime &value, const QString& name )
        : NamedCommand( part, name ),
        m_completion( completion ),
        oldvalue( m_completion.finishTime() ),
        newvalue( value )
{
    m_spec = part->getProject().timeSpec();
}
void ModifyCompletionFinishTimeCmd::execute()
{
    m_completion.setFinishTime( DateTime( newvalue, m_spec ) );

    setCommandType( 0 );
}
void ModifyCompletionFinishTimeCmd::unexecute()
{
    m_completion.setFinishTime( oldvalue );

    setCommandType( 0 );
}

AddCompletionEntryCmd::AddCompletionEntryCmd( Part *part, Completion &completion, const QDate &date, Completion::Entry *value, const QString& name )
        : NamedCommand( part, name ),
        m_completion( completion ),
        m_date( date ),
        newvalue( value ),
        m_newmine( true )
{
}
AddCompletionEntryCmd::~AddCompletionEntryCmd()
{
    if ( m_newmine )
        delete newvalue;
}
void AddCompletionEntryCmd::execute()
{
    Q_ASSERT( ! m_completion.entries().contains( m_date ) );
    m_completion.addEntry( m_date, newvalue );
    m_newmine = false;
    setCommandType( 0 );
}
void AddCompletionEntryCmd::unexecute()
{
    m_completion.takeEntry( m_date );
    m_newmine = true;
    setCommandType( 0 );
}

RemoveCompletionEntryCmd::RemoveCompletionEntryCmd( Part *part, Completion &completion, const QDate &date, const QString& name )
        : NamedCommand( part, name ),
        m_completion( completion ),
        m_date( date ),
        m_mine( false )
{
    value = m_completion.entry( date );
}
RemoveCompletionEntryCmd::~RemoveCompletionEntryCmd()
{
    if ( m_mine )
        delete value;
}
void RemoveCompletionEntryCmd::execute()
{
    Q_ASSERT( m_completion.entries().contains( m_date ) );
    if ( value ) {
        m_completion.takeEntry( m_date );
        m_mine = true;
    }
    setCommandType( 0 );
}
void RemoveCompletionEntryCmd::unexecute()
{
    if ( value ) {
        m_completion.addEntry( m_date, value );
    }
    m_mine = false;
    setCommandType( 0 );
}


ModifyCompletionEntryCmd::ModifyCompletionEntryCmd( Part *part, Completion &completion, const QDate &date, Completion::Entry *value, const QString& name )
        : NamedCommand( part, name )
{
    cmd = new K3MacroCommand("");
    cmd->addCommand( new RemoveCompletionEntryCmd( part, completion, date ) );
    cmd->addCommand( new AddCompletionEntryCmd( part, completion, date, value ) );
}
ModifyCompletionEntryCmd::~ModifyCompletionEntryCmd()
{
    delete cmd;
}
void ModifyCompletionEntryCmd::execute()
{
    cmd->execute();
}
void ModifyCompletionEntryCmd::unexecute()
{
    cmd->unexecute();
}

AddCompletionUsedEffortCmd::AddCompletionUsedEffortCmd( Part *part, Completion &completion, const Resource *resource, Completion::UsedEffort *value, const QString& name )
        : NamedCommand( part, name ),
        m_completion( completion ),
        m_resource( resource ),
        newvalue( value ),
        m_newmine( true ),
        m_oldmine( false)
{
    oldvalue = m_completion.usedEffort( resource );
}
AddCompletionUsedEffortCmd::~AddCompletionUsedEffortCmd()
{
    if ( m_oldmine )
        delete oldvalue;
    if ( m_newmine )
        delete newvalue;
}
void AddCompletionUsedEffortCmd::execute()
{
    if ( oldvalue ) {
        m_completion.takeUsedEffort( m_resource );
        m_oldmine = true;
    }
    m_completion.addUsedEffort( m_resource, newvalue );
    m_newmine = false;
    setCommandType( 0 );
}
void AddCompletionUsedEffortCmd::unexecute()
{
    m_completion.takeUsedEffort( m_resource );
    if ( oldvalue ) {
        m_completion.addUsedEffort( m_resource, oldvalue );
    }
    m_newmine = true;
    m_oldmine = false;
    setCommandType( 0 );
}

AddCompletionActualEffortCmd::AddCompletionActualEffortCmd( Part *part, Completion::UsedEffort &ue, const QDate &date, Completion::UsedEffort::ActualEffort *value, const QString& name )
        : NamedCommand( part, name ),
        m_usedEffort( ue ),
        m_date( date ),
        newvalue( value ),
        m_newmine( true ),
        m_oldmine( false)
{
    oldvalue = ue.effort( date );
}
AddCompletionActualEffortCmd::~AddCompletionActualEffortCmd()
{
    if ( m_oldmine )
        delete oldvalue;
    if ( m_newmine )
        delete newvalue;
}
void AddCompletionActualEffortCmd::execute()
{
    if ( oldvalue ) {
        m_usedEffort.takeEffort( m_date );
        m_oldmine = true;
    }
    m_usedEffort.setEffort( m_date, newvalue );
    m_newmine = false;
    setCommandType( 0 );
}
void AddCompletionActualEffortCmd::unexecute()
{
    m_usedEffort.takeEffort( m_date );
    if ( oldvalue ) {
        m_usedEffort.setEffort( m_date, oldvalue );
    }
    m_newmine = true;
    m_oldmine = false;
    setCommandType( 0 );
}

AddAccountCmd::AddAccountCmd( Part *part, Project &project, Account *account, const QString& parent, const QString& name )
        : NamedCommand( part, name ),
        m_project( project ),
        m_account( account ),
        m_parent( 0 ),
        m_parentName( parent )
{
    m_mine = true;
}

AddAccountCmd::AddAccountCmd( Part *part, Project &project, Account *account, Account *parent, const QString& name )
        : NamedCommand( part, name ),
        m_project( project ),
        m_account( account ),
        m_parent( parent )
{
    m_mine = true;
}

AddAccountCmd::~AddAccountCmd()
{
    if ( m_mine )
        delete m_account;
}

void AddAccountCmd::execute()
{
    if ( m_parent == 0 && !m_parentName.isEmpty() ) {
        m_parent = m_project.accounts().findAccount( m_parentName );
    }
    m_project.accounts().insert( m_account, m_parent );

    setCommandType( 0 );
    m_mine = false;
}
void AddAccountCmd::unexecute()
{
    m_project.accounts().take( m_account );

    setCommandType( 0 );
    m_mine = true;
}

RemoveAccountCmd::RemoveAccountCmd( Part *part, Project &project, Account *account, const QString& name )
        : NamedCommand( part, name ),
        m_project( project ),
        m_account( account ),
        m_parent( account->parent() )
{
    if ( m_parent ) {
        m_index = m_parent->accountList().indexOf( account );
    } else {
        m_index = project.accounts().accountList().indexOf( account );
    }
    m_mine = false;
    m_isDefault = account == project.accounts().defaultAccount();
}

RemoveAccountCmd::~RemoveAccountCmd()
{
    if ( m_mine )
        delete m_account;
}

void RemoveAccountCmd::execute()
{
    if ( m_isDefault ) {
        m_project.accounts().setDefaultAccount( 0 );
    }
    m_project.accounts().take( m_account );

    setCommandType( 0 );
    m_mine = true;
}
void RemoveAccountCmd::unexecute()
{
    m_project.accounts().insert( m_account, m_parent, m_index );
    if ( m_isDefault ) {
        m_project.accounts().setDefaultAccount( m_account );
    }
    setCommandType( 0 );
    m_mine = false;
}

RenameAccountCmd::RenameAccountCmd( Part *part, Account *account, const QString& value, const QString& name )
        : NamedCommand( part, name ),
        m_account( account )
{
    m_oldvalue = account->name();
    m_newvalue = value;
}

void RenameAccountCmd::execute()
{
    m_account->setName( m_newvalue );
    setCommandType( 0 );
}
void RenameAccountCmd::unexecute()
{
    m_account->setName( m_oldvalue );
    setCommandType( 0 );
}

ModifyAccountDescriptionCmd::ModifyAccountDescriptionCmd( Part *part, Account *account, const QString& value, const QString& name )
        : NamedCommand( part, name ),
        m_account( account )
{
    m_oldvalue = account->description();
    m_newvalue = value;
}

void ModifyAccountDescriptionCmd::execute()
{
    m_account->setDescription( m_newvalue );
    setCommandType( 0 );
}
void ModifyAccountDescriptionCmd::unexecute()
{
    m_account->setDescription( m_oldvalue );
    setCommandType( 0 );
}


NodeModifyStartupCostCmd::NodeModifyStartupCostCmd( Part *part, Node &node, double value, const QString& name )
        : NamedCommand( part, name ),
        m_node( node )
{
    m_oldvalue = node.startupCost();
    m_newvalue = value;
}

void NodeModifyStartupCostCmd::execute()
{
    m_node.setStartupCost( m_newvalue );
    setCommandType( 0 );
}
void NodeModifyStartupCostCmd::unexecute()
{
    m_node.setStartupCost( m_oldvalue );
    setCommandType( 0 );
}

NodeModifyShutdownCostCmd::NodeModifyShutdownCostCmd( Part *part, Node &node, double value, const QString& name )
        : NamedCommand( part, name ),
        m_node( node )
{
    m_oldvalue = node.startupCost();
    m_newvalue = value;
}

void NodeModifyShutdownCostCmd::execute()
{
    m_node.setShutdownCost( m_newvalue );
    setCommandType( 0 );
}
void NodeModifyShutdownCostCmd::unexecute()
{
    m_node.setShutdownCost( m_oldvalue );
    setCommandType( 0 );
}

NodeModifyRunningAccountCmd::NodeModifyRunningAccountCmd( Part *part, Node &node, Account *oldvalue, Account *newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_node( node )
{
    m_oldvalue = oldvalue;
    m_newvalue = newvalue;
    //kDebug();
}
void NodeModifyRunningAccountCmd::execute()
{
    //kDebug();
    if ( m_oldvalue ) {
        m_oldvalue->removeRunning( m_node );
    }
    if ( m_newvalue ) {
        m_newvalue->addRunning( m_node );
    }
    setCommandType( 0 );
}
void NodeModifyRunningAccountCmd::unexecute()
{
    //kDebug();
    if ( m_newvalue ) {
        m_newvalue->removeRunning( m_node );
    }
    if ( m_oldvalue ) {
        m_oldvalue->addRunning( m_node );
    }
    setCommandType( 0 );
}

NodeModifyStartupAccountCmd::NodeModifyStartupAccountCmd( Part *part, Node &node, Account *oldvalue, Account *newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_node( node )
{
    m_oldvalue = oldvalue;
    m_newvalue = newvalue;
    //kDebug();
}

void NodeModifyStartupAccountCmd::execute()
{
    //kDebug();
    if ( m_oldvalue ) {
        m_oldvalue->removeStartup( m_node );
    }
    if ( m_newvalue ) {
        m_newvalue->addStartup( m_node );
    }
    setCommandType( 0 );
}
void NodeModifyStartupAccountCmd::unexecute()
{
    //kDebug();
    if ( m_newvalue ) {
        m_newvalue->removeStartup( m_node );
    }
    if ( m_oldvalue ) {
        m_oldvalue->addStartup( m_node );
    }
    setCommandType( 0 );
}

NodeModifyShutdownAccountCmd::NodeModifyShutdownAccountCmd( Part *part, Node &node, Account *oldvalue, Account *newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_node( node )
{
    m_oldvalue = oldvalue;
    m_newvalue = newvalue;
    //kDebug();
}

void NodeModifyShutdownAccountCmd::execute()
{
    //kDebug();
    if ( m_oldvalue ) {
        m_oldvalue->removeShutdown( m_node );
    }
    if ( m_newvalue ) {
        m_newvalue->addShutdown( m_node );
    }
    setCommandType( 0 );
}
void NodeModifyShutdownAccountCmd::unexecute()
{
    //kDebug();
    if ( m_newvalue ) {
        m_newvalue->removeShutdown( m_node );
    }
    if ( m_oldvalue ) {
        m_oldvalue->addShutdown( m_node );
    }
    setCommandType( 0 );
}

ModifyDefaultAccountCmd::ModifyDefaultAccountCmd( Part *part, Accounts &acc, Account *oldvalue, Account *newvalue, const QString& name )
        : NamedCommand( part, name ),
        m_accounts( acc )
{
    m_oldvalue = oldvalue;
    m_newvalue = newvalue;
    //kDebug();
}

void ModifyDefaultAccountCmd::execute()
{
    //kDebug();
    m_accounts.setDefaultAccount( m_newvalue );
    setCommandType( 0 );
}
void ModifyDefaultAccountCmd::unexecute()
{
    //kDebug();
    m_accounts.setDefaultAccount( m_oldvalue );
    setCommandType( 0 );
}

ProjectModifyConstraintCmd::ProjectModifyConstraintCmd( Part *part, Project &node, Node::ConstraintType c, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newConstraint( c ),
        oldConstraint( static_cast<Node::ConstraintType>( node.constraint() ) )
{

/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
}
void ProjectModifyConstraintCmd::execute()
{
    m_node.setConstraint( newConstraint );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void ProjectModifyConstraintCmd::unexecute()
{
    m_node.setConstraint( oldConstraint );
//    setSchScheduled();
    setCommandType( 1 );
}

ProjectModifyStartTimeCmd::ProjectModifyStartTimeCmd( Part *part, Project &node, const QDateTime& dt, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newTime( dt ),
        oldTime( node.startTime() )
{
    m_spec = node.timeSpec();
/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
}

void ProjectModifyStartTimeCmd::execute()
{
    m_node.setConstraintStartTime( DateTime( newTime, m_spec ) );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void ProjectModifyStartTimeCmd::unexecute()
{
    m_node.setConstraintStartTime( oldTime );
//    setSchScheduled();
    setCommandType( 1 );
}

ProjectModifyEndTimeCmd::ProjectModifyEndTimeCmd( Part *part, Project &node, const QDateTime& dt, const QString& name )
        : NamedCommand( part, name ),
        m_node( node ),
        newTime( dt ),
        oldTime( node.endTime() )
{
    m_spec = node.timeSpec();
/*    foreach ( Schedule * s, node.schedules() ) {
        addSchScheduled( s );
    }*/
}
void ProjectModifyEndTimeCmd::execute()
{
    m_node.setEndTime( DateTime( newTime, m_spec ) );
    m_node.setConstraintEndTime( DateTime( newTime, m_spec ) );
//    setSchScheduled( false );
    setCommandType( 1 );
}
void ProjectModifyEndTimeCmd::unexecute()
{
    m_node.setConstraintEndTime( oldTime );
//    setSchScheduled();
    setCommandType( 1 );
}

//----------------------------
AddScheduleManagerCmd::AddScheduleManagerCmd( Part *part, Project &node, ScheduleManager *sm, const QString& name )
    : NamedCommand( part, name ),
    m_node( node ),
    m_parent( sm->parentManager() ),
    m_sm( sm ),
    m_index( -1 ),
    m_exp( sm->expected() ),
    m_opt( sm->optimistic() ),
    m_pess( sm->pessimistic() ),
    m_mine( true)
{
}

AddScheduleManagerCmd::AddScheduleManagerCmd( Part *part, ScheduleManager *parent, ScheduleManager *sm, const QString& name )
    : NamedCommand( part, name ),
    m_node( parent->project() ),
    m_parent( parent ),
    m_sm( sm ),
    m_index( -1 ),
    m_exp( sm->expected() ),
    m_opt( sm->optimistic() ),
    m_pess( sm->pessimistic() ),
    m_mine( true)
{
}

AddScheduleManagerCmd::~AddScheduleManagerCmd()
{
    if ( m_mine ) {
        m_sm->setParentManager( 0 );
        delete m_sm;
    }
}

void AddScheduleManagerCmd::execute()
{
    m_node.addScheduleManager( m_sm, m_parent );
    m_sm->setExpected( m_exp );
    m_sm->setOptimistic( m_opt );
    m_sm->setPessimistic( m_pess );
    m_mine = false;
}

void AddScheduleManagerCmd::unexecute()
{
    m_index = m_node.takeScheduleManager( m_sm );
    m_sm->setExpected( 0 );
    m_sm->setOptimistic( 0 );
    m_sm->setPessimistic( 0 );
    m_mine = true;
}

DeleteScheduleManagerCmd::DeleteScheduleManagerCmd( Part *part, Project &node, ScheduleManager *sm, const QString& name )
    : AddScheduleManagerCmd( part, node, sm, name )
{
    m_mine = false;
}

void DeleteScheduleManagerCmd::execute()
{
    AddScheduleManagerCmd::unexecute();
}

void DeleteScheduleManagerCmd::unexecute()
{
    AddScheduleManagerCmd::execute();
}

ModifyScheduleManagerNameCmd::ModifyScheduleManagerNameCmd( Part *part, ScheduleManager &sm, const QString& value, const QString& name )
    : NamedCommand( part, name ),
    m_sm( sm ),
    oldvalue( sm.name() ),
    newvalue( value )
{
}

void ModifyScheduleManagerNameCmd::execute()
{
    m_sm.setName( newvalue );
}

void ModifyScheduleManagerNameCmd::unexecute()
{
    m_sm.setName( oldvalue );
}

ModifyScheduleManagerAllowOverbookingCmd::ModifyScheduleManagerAllowOverbookingCmd( Part *part, ScheduleManager &sm, bool value, const QString& name )
    : NamedCommand( part, name ),
    m_sm( sm ),
    oldvalue( sm.allowOverbooking() ),
    newvalue( value )
{
}

void ModifyScheduleManagerAllowOverbookingCmd::execute()
{
    m_sm.setAllowOverbooking( newvalue );
}

void ModifyScheduleManagerAllowOverbookingCmd::unexecute()
{
    m_sm.setAllowOverbooking( oldvalue );
}

ModifyScheduleManagerDistributionCmd::ModifyScheduleManagerDistributionCmd( Part *part, ScheduleManager &sm, bool value, const QString& name )
    : NamedCommand( part, name ),
    m_sm( sm ),
    oldvalue( sm.usePert() ),
    newvalue( value )
{
}

void ModifyScheduleManagerDistributionCmd::execute()
{
    m_sm.setUsePert( newvalue );
}

void ModifyScheduleManagerDistributionCmd::unexecute()
{
    m_sm.setUsePert( oldvalue );
}

ModifyScheduleManagerCalculateAllCmd::ModifyScheduleManagerCalculateAllCmd( Part *part, ScheduleManager &sm, bool value, const QString& name )
    : NamedCommand( part, name ),
    m_sm( sm ),
    oldvalue( sm.calculateAll() ),
    newvalue( value )
{
}

void ModifyScheduleManagerCalculateAllCmd::execute()
{
    m_sm.setCalculateAll( newvalue );
}

void ModifyScheduleManagerCalculateAllCmd::unexecute()
{
    m_sm.setCalculateAll( oldvalue );
}

CalculateScheduleCmd::CalculateScheduleCmd( Part *part, Project &node, ScheduleManager &sm, const QString& name )
    : NamedCommand( part, name ),
    m_node( node ),
    m_sm( sm ),
    m_first( true ),
    m_oldexpected( m_sm.expected() ),
    m_oldoptimistic( m_sm.optimistic() ),
    m_oldpessimistic( m_sm.pessimistic() ),
    m_newexpected( 0 ),
    m_newoptimistic( 0 ),
    m_newpessimistic( 0 )
{
}

void CalculateScheduleCmd::execute()
{
    if ( m_first ) {
        m_first = false;
        m_node.calculate( m_sm );
        m_newexpected = m_sm.expected();
        m_newoptimistic = m_sm.optimistic();
        m_newpessimistic = m_sm.pessimistic();
        return;
    }
    m_sm.setExpected( m_newexpected );
    m_sm.setOptimistic( m_newoptimistic );
    m_sm.setPessimistic( m_newpessimistic );
}

void CalculateScheduleCmd::unexecute()
{
    m_sm.setExpected( m_oldexpected );
    m_sm.setOptimistic( m_oldoptimistic );
    m_sm.setPessimistic( m_oldpessimistic );
}

//------------------------
ModifyStandardWorktimeYearCmd::ModifyStandardWorktimeYearCmd( Part *part, StandardWorktime *wt, double oldvalue, double newvalue, const QString& name )
        : NamedCommand( part, name ),
        swt( wt ),
        m_oldvalue( oldvalue ),
        m_newvalue( newvalue )
{
}
void ModifyStandardWorktimeYearCmd::execute()
{
    swt->setYear( m_newvalue );
    setCommandType( 0 );
}
void ModifyStandardWorktimeYearCmd::unexecute()
{
    swt->setYear( m_oldvalue );
    setCommandType( 0 );
}

ModifyStandardWorktimeMonthCmd::ModifyStandardWorktimeMonthCmd( Part *part, StandardWorktime *wt, double oldvalue, double newvalue, const QString& name )
        : NamedCommand( part, name ),
        swt( wt ),
        m_oldvalue( oldvalue ),
        m_newvalue( newvalue )
{
}
void ModifyStandardWorktimeMonthCmd::execute()
{
    swt->setMonth( m_newvalue );
    setCommandType( 0 );
}
void ModifyStandardWorktimeMonthCmd::unexecute()
{
    swt->setMonth( m_oldvalue );
    setCommandType( 0 );
}

ModifyStandardWorktimeWeekCmd::ModifyStandardWorktimeWeekCmd( Part *part, StandardWorktime *wt, double oldvalue, double newvalue, const QString& name )
        : NamedCommand( part, name ),
        swt( wt ),
        m_oldvalue( oldvalue ),
        m_newvalue( newvalue )
{
}
void ModifyStandardWorktimeWeekCmd::execute()
{
    swt->setWeek( m_newvalue );
    setCommandType( 0 );
}
void ModifyStandardWorktimeWeekCmd::unexecute()
{
    swt->setWeek( m_oldvalue );
    setCommandType( 0 );
}

ModifyStandardWorktimeDayCmd::ModifyStandardWorktimeDayCmd( Part *part, StandardWorktime *wt, double oldvalue, double newvalue, const QString& name )
        : NamedCommand( part, name ),
        swt( wt ),
        m_oldvalue( oldvalue ),
        m_newvalue( newvalue )
{
}

void ModifyStandardWorktimeDayCmd::execute()
{
    swt->setDay( m_newvalue );
    setCommandType( 0 );
}
void ModifyStandardWorktimeDayCmd::unexecute()
{
    swt->setDay( m_oldvalue );
    setCommandType( 0 );
}

InsertEmbeddedDocumentCmd::InsertEmbeddedDocumentCmd( Part *part, ViewListWidget *list, ViewListItem *item, QTreeWidgetItem *parent, const QString& name )
        : NamedCommand( part, name ),
        m_list( list ),
        m_parent( parent ),
        m_item( item ),
        m_index( -1 ),
        m_mine( false )
{
}
InsertEmbeddedDocumentCmd::~InsertEmbeddedDocumentCmd()
{
    if ( m_mine ) {
        delete m_item;
    }
}
void InsertEmbeddedDocumentCmd::execute()
{
    m_list->insertViewListItem( m_item, m_parent, m_index );
    m_item->documentChild()->setDeleted( false );
    setCommandType( 0 );
}
void InsertEmbeddedDocumentCmd::unexecute()
{
    m_item->documentChild()->setDeleted( true );
    m_index = m_list->takeViewListItem( m_item );
    setCommandType( 0 );
}

DeleteEmbeddedDocumentCmd::DeleteEmbeddedDocumentCmd( Part *part, ViewListWidget *list, ViewListItem *item, const QString& name )
        : NamedCommand( part, name ),
        m_list( list ),
        m_parent( item->parent() ),
        m_item( item ),
        m_index( -1 ),
        m_mine( false )
{
}
DeleteEmbeddedDocumentCmd::~DeleteEmbeddedDocumentCmd()
{
    if ( m_mine ) {
        delete m_item->view();
        delete m_item;
    }
}
void DeleteEmbeddedDocumentCmd::execute()
{
    m_index = m_list->takeViewListItem( m_item );
    m_item->documentChild()->setDeleted( true );
    setCommandType( 0 );
}
void DeleteEmbeddedDocumentCmd::unexecute()
{
    m_item->documentChild()->setDeleted( true );
    m_list->insertViewListItem( m_item, m_parent, m_index );
    setCommandType( 0 );
}


}  //KPlato namespace
