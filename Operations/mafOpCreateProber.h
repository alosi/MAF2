/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafOpCreateProber.h,v $
  Language:  C++
  Date:      $Date: 2008-03-06 11:55:06 $
  Version:   $Revision: 1.1 $
  Authors:   Paolo Quadrani
==========================================================================
Copyright (c) 2002/2004
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __mafOpCreateProber_H__
#define __mafOpCreateProber_H__

#include "mafOp.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVMEProber;
class mafGui;
class mafEvent;
//----------------------------------------------------------------------------
// mafOpCreateProber :
//----------------------------------------------------------------------------
/** */
class mafOpCreateProber: public mafOp
{
public:
  mafOpCreateProber(const wxString &label = "CreateProber");
 ~mafOpCreateProber(); 

  mafTypeMacro(mafOpCreateProber, mafOp);

  mafOp* Copy();

  bool Accept(mafNode *node);
  void OpRun();
  void OpDo();

protected: 
  mafVMEProber *m_Prober;
};
#endif