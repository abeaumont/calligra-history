/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1997-1998              */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* written for KDE (http://www.kde.org)                           */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Delete Command (header)                                */
/******************************************************************/

#ifndef deletecmd_h
#define deletecmd_h

#include <qrect.h>
#include <qpoint.h>
#include <qsize.h>

#include "command.h"
#include "kpobject.h"

class KPresenterDocument_impl;

/******************************************************************/
/* Class: DeleteCmd                                               */
/******************************************************************/

class DeleteCmd : public Command
{
  Q_OBJECT

public:
  DeleteCmd(QString _name,KPObject *_object,KPresenterDocument_impl *_doc);
  ~DeleteCmd();
  
  virtual void execute();
  virtual void unexecute();

protected:
  DeleteCmd()
    {;}

  KPObject *object;
  KPresenterDocument_impl *doc;

};

#endif
