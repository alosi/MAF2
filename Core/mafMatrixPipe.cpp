/*=========================================================================

 Program: MAF2
 Module: mafMatrixPipe
 Authors: Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------



#include "mafMatrixPipe.h"

#include "mafMatrix.h"
#include "mafVME.h"
#include "mafEventBase.h"
#include "mafDecl.h"

//------------------------------------------------------------------------------
mafCxxTypeMacro(mafMatrixPipe)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
mafMatrixPipe::mafMatrixPipe()
//------------------------------------------------------------------------------
{
  m_UpdateMatrixObserverFlag=1;
  m_Updating=0; // this should be used to avoid updating loops
  m_VME=NULL;
}

//------------------------------------------------------------------------------
mafMatrixPipe::~mafMatrixPipe()
//------------------------------------------------------------------------------
{
  this->SetVME(NULL);
}

//------------------------------------------------------------------------------
void mafMatrixPipe::SetTimeStamp(mafTimeStamp t)
//------------------------------------------------------------------------------
{
  if (t!=m_TimeStamp)
  {
    mafTransformBase::SetTimeStamp(t);Modified(); 
  }
}

//------------------------------------------------------------------------------
mafTimeStamp mafMatrixPipe::GetTimeStamp()
//------------------------------------------------------------------------------
{
  return mafTransformBase::GetTimeStamp();
}

//------------------------------------------------------------------------------
int mafMatrixPipe::SetVME(mafVME *vme)
//------------------------------------------------------------------------------
{
  if (this->Accept(vme) || vme == NULL)
  {
    m_VME=vme;
    Modified();
    return MAF_OK;
  }

  mafErrorMacro("Wrong VME type: "<<vme->GetTypeName());
  return MAF_ERROR;
}

//------------------------------------------------------------------------------
mafMatrixPipe *mafMatrixPipe::MakeACopy()
//------------------------------------------------------------------------------
{
  mafMatrixPipe *newpipe=NewInstance();
  newpipe->DeepCopy(this);
  return newpipe;
}

//------------------------------------------------------------------------------
int mafMatrixPipe::DeepCopy(mafMatrixPipe *pipe)
//------------------------------------------------------------------------------
{
  if (pipe->IsA(GetTypeId()))
  {
    m_VME=pipe->GetVME();
    return MAF_OK;
  }

  return MAF_ERROR;
}

//------------------------------------------------------------------------------
const mafMatrix &mafMatrixPipe::GetMatrix()
//------------------------------------------------------------------------------
{
  if (m_Updating) // !!! to  be checked!!!
    return *m_Matrix;

  return this->Superclass::GetMatrix();
}


//----------------------------------------------------------------------------
// Get the MTime. Take in consideration also modifications to the Input Array
unsigned long mafMatrixPipe::GetMTime()
//------------------------------------------------------------------------------
{
  unsigned long mtime = this->Superclass::GetMTime();

  if (m_VME)
  {
    unsigned long vmeMTime = m_VME->GetMTime();
    if (vmeMTime > mtime)
    {
      return vmeMTime;
    }
  }
  return mtime;
}

//----------------------------------------------------------------------------
void mafMatrixPipe::InternalUpdate()
//----------------------------------------------------------------------------
{
  if (m_VME) m_VME->OnEvent(&mafEventBase(this,VME_MATRIX_UPDATE));
}

//----------------------------------------------------------------------------
void mafMatrixPipe::Update()
//----------------------------------------------------------------------------
{
//  if (m_VME) m_VME->OnEvent(&mafEventBase(this,VME_MATRIX_PREUPDATE));
  
  Superclass::Update();
}
