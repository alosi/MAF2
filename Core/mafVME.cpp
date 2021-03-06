/*=========================================================================

 Program: MAF2
 Module: mafVME
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

#include "mafVME.h"
#include "mafDecl.h"
#include "mafGUI.h"

#include "mafVMEItem.h"
#include "mafVMEOutput.h"
#include "mafAbsMatrixPipe.h"
#include "mafMatrixPipe.h"
#include "mafDataPipe.h"
#include "mafEventIO.h"
#include "mafEvent.h"
#include "mafEventSource.h"
#include "mafTagArray.h"
#include "mafOBB.h"
#include "mafTransform.h"
#include "mmuTimeSet.h"
#include "mafIndent.h"
#include "mafStorageElement.h"

#include <assert.h>

//-------------------------------------------------------------------------
mafCxxAbstractTypeMacro(mafVME)
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
mafVME::mafVME()
//-------------------------------------------------------------------------
{
  m_TestMode = false;

  m_Output        = NULL;
  m_Behavior      = NULL;

  m_AbsMatrixPipe = mafAbsMatrixPipe::New();

  m_CurrentTime   = 0.0;
  m_Crypting      = 0;

  m_VisualMode = DEFAULT_VISUAL_MODE;
}

//-------------------------------------------------------------------------
mafVME::~mafVME()
//-------------------------------------------------------------------------
{
  // Pipes must be destroyed in the right order
  // to take into consideration dependencies
  cppDEL(m_Output);

  m_DataPipe=NULL; // smart pointer
  
  m_AbsMatrixPipe->SetVME(NULL);
  m_AbsMatrixPipe=NULL; // smart pointer
    
  m_MatrixPipe=NULL; // smart pointer
}

//-------------------------------------------------------------------------
int mafVME::InternalInitialize()
//-------------------------------------------------------------------------
{

  if (Superclass::InternalInitialize()==MAF_OK)
  {
    assert(m_AbsMatrixPipe);
    // force the abs matrix pipe to update its inputs
    m_AbsMatrixPipe->SetVME(this);

    return MAF_OK;
  }

  return MAF_ERROR;
}

//-------------------------------------------------------------------------
mafVME *mafVME::GetParent() const
//-------------------------------------------------------------------------
{
  assert(m_Parent==NULL||m_Parent->IsA(typeid(mafVME)));
  return (mafVME *)Superclass::GetParent();
}

//-------------------------------------------------------------------------
int mafVME::DeepCopy(mafNode *a)
//-------------------------------------------------------------------------
{ 
  if (Superclass::DeepCopy(a)==MAF_OK)
  {
    mafVME *vme=mafVME::SafeDownCast(a);
    
    SetMatrixPipe(vme->GetMatrixPipe()?vme->GetMatrixPipe()->MakeACopy():NULL);
    SetDataPipe(vme->GetDataPipe()?vme->GetDataPipe()->MakeACopy():NULL);

    // Runtime properties
    //AutoUpdateAbsMatrix=vme->GetAutoUpdateAbsMatrix();
    SetTimeStamp(vme->GetTimeStamp());

    SetMatrix(*vme->GetOutput()->GetMatrix());
    //SetAbsMatrix(*vme->GetOutput()->GetAbsMatrix());

    return MAF_OK;
  }
  else
  {
    mafErrorMacro("Cannot copy VME of type "<<a->GetTypeName()<<" into a VME \
    VME of type "<<GetTypeName());

    return MAF_ERROR;
  }
}

//-------------------------------------------------------------------------
int mafVME::ShallowCopy(mafVME *a)
//-------------------------------------------------------------------------
{  
  // for basic VME ShallowCopy is the same of DeepCopy (no data stored inside)
  return mafVME::DeepCopy(a);
}

//-------------------------------------------------------------------------
bool mafVME::Equals(mafVME *vme)
//-------------------------------------------------------------------------
{
  if (Superclass::Equals(vme))
  {
    if (GetTimeStamp() == vme->GetTimeStamp())
    {
      if (GetParent())
      {
        if (GetOutput()->GetAbsMatrix()->Equals(vme->GetOutput()->GetAbsMatrix()))
        {
          return true;
        }
      }
      else
      {
        if (GetOutput()->GetMatrix()->Equals(vme->GetOutput()->GetMatrix()))
        {
          return true;
        }
      }
    }
  }
  return false;
}

//-------------------------------------------------------------------------
bool mafVME::Equals(mafNode *node)
//-------------------------------------------------------------------------
{
  if (mafVME *vme=mafVME::SafeDownCast(node))
  {
    return Equals(vme);
  }
  return false;
}

//-------------------------------------------------------------------------
int mafVME::SetParent(mafNode *parent)
//-------------------------------------------------------------------------
{
  if (Superclass::SetParent(parent)==MAF_OK)
  {
    // this forces the pipe to Update its input and input frame
    m_AbsMatrixPipe->SetVME(this);
    return MAF_OK;
  }
  return MAF_ERROR;
}

//-------------------------------------------------------------------------
void mafVME::SetTimeStamp(mafTimeStamp t)
//-------------------------------------------------------------------------
{
  if (t<0)
    t=0;

  m_CurrentTime = t;

  // Must keep a time variable also on the
  // pipes to allow multiple pipes contemporary 
  // working at different times
  // 
  if (m_DataPipe)
    m_DataPipe->SetTimeStamp(t);

  if (m_MatrixPipe)
    m_MatrixPipe->SetTimeStamp(t);
  
  if (m_AbsMatrixPipe)
    m_AbsMatrixPipe->SetTimeStamp(t);

  Modified();

  // TODO: consider if to add a flag to disable event issuing
  GetEventSource()->InvokeEvent(this,VME_TIME_SET);
}

//-------------------------------------------------------------------------
mafTimeStamp mafVME::GetTimeStamp() 
//-------------------------------------------------------------------------
{
  return m_CurrentTime;
}

//-------------------------------------------------------------------------
void mafVME::SetTreeTime(mafTimeStamp t)
//-------------------------------------------------------------------------
{
  SetTimeStamp(t);
  ForwardDownEvent(&mafEventBase(this,VME_TIME_SET,&t));
}

//-------------------------------------------------------------------------
bool mafVME::IsAnimated()
//-------------------------------------------------------------------------
{
  return false;
}

//-------------------------------------------------------------------------
bool mafVME::IsDataAvailable()
//-------------------------------------------------------------------------
{
  return true;
}

//-------------------------------------------------------------------------
int mafVME::GetNumberOfLocalTimeStamps()
//-------------------------------------------------------------------------
{
  std::vector<mafTimeStamp> timestamps;
  GetLocalTimeStamps(timestamps);
  return timestamps.size();
}

//-------------------------------------------------------------------------
int mafVME::GetNumberOfTimeStamps()
//-------------------------------------------------------------------------
{
  std::vector<mafTimeStamp> timestamps;
  GetTimeStamps(timestamps);
  return timestamps.size();
}

//-------------------------------------------------------------------------
void mafVME::GetTimeStamps(std::vector<mafTimeStamp> &kframes)
//-------------------------------------------------------------------------
{
  GetLocalTimeStamps(kframes);
  
  std::vector<mafTimeStamp> subKFrames;

  for (int i=0;i<GetNumberOfChildren();i++)
  {
    if (mafVME *vme=mafVME::SafeDownCast(GetChild(i)))
    {
      vme->GetTimeStamps(subKFrames);
    }

    mmuTimeSet::Merge(kframes,subKFrames,kframes);
  }
}

//-------------------------------------------------------------------------
void mafVME::GetAbsTimeStamps(std::vector<mafTimeStamp> &kframes)
//-------------------------------------------------------------------------
{
  GetLocalTimeStamps(kframes);
  
  std::vector<mafTimeStamp> parentKFrames;

  for (mafVME *parent=mafVME::SafeDownCast(GetParent());parent;parent=mafVME::SafeDownCast(parent->GetParent()))
  {
    parent->GetLocalTimeStamps(parentKFrames);

    mmuTimeSet::Merge(kframes,parentKFrames,kframes);
  }
}

//-------------------------------------------------------------------------
bool mafVME::CanReparentTo(mafNode *parent)
//-------------------------------------------------------------------------
{
  return (parent == NULL)|| (parent->IsA(typeid(mafVME)) && !IsInTree(parent));
}

//-------------------------------------------------------------------------
void mafVME::SetPose(const mafMatrix &mat,mafTimeStamp t)
//-------------------------------------------------------------------------
{
  mafMatrix new_mat(mat);
  new_mat.SetTimeStamp(t);
  SetMatrix(new_mat);
}

//-------------------------------------------------------------------------
void mafVME::SetPose(double x,double y,double z,double rx,double ry,double rz, mafTimeStamp t)
//-------------------------------------------------------------------------
{
  double txyz[3],trxyz[3];
  txyz[0]=x; txyz[1]=y; txyz[2]=z;
  trxyz[0]=rx; trxyz[1]=ry; trxyz[2]=rz;
  SetPose(txyz,trxyz,t);
}

//-------------------------------------------------------------------------
void mafVME::SetPose(double xyz[3],double rxyz[3], mafTimeStamp t)
//-------------------------------------------------------------------------
{
  t=(t<0)?t=m_CurrentTime:t;

  mafMatrix matrix;

  mafTransform::SetOrientation(matrix,rxyz);
  mafTransform::SetPosition(matrix,xyz);
  matrix.SetTimeStamp(t);
 
  SetMatrix(matrix);
}

//----------------------------------------------------------------------------
void mafVME::ApplyMatrix(const mafMatrix &matrix,int premultiply,mafTimeStamp t)
//----------------------------------------------------------------------------
{
  t=(t<0)?m_CurrentTime:t;

  mafTransform new_pose;
  mafMatrix pose;
  GetOutput()->GetMatrix(pose,t);
  new_pose.SetMatrix(pose);
  new_pose.Concatenate(matrix,premultiply);
  new_pose.SetTimeStamp(t);
  SetMatrix(new_pose.GetMatrix());
}

//-------------------------------------------------------------------------
void mafVME::SetAbsPose(double x,double y,double z,double rx,double ry,double rz, mafTimeStamp t)
//-------------------------------------------------------------------------
{
  double txyz[3],trxyz[3];
  txyz[0]=x; txyz[1]=y; txyz[2]=z;
  trxyz[0]=rx; trxyz[1]=ry; trxyz[2]=rz;
  SetAbsPose(txyz,trxyz,t);
}

//-------------------------------------------------------------------------
void mafVME::SetAbsPose(double xyz[3],double rxyz[3], mafTimeStamp t)
//-------------------------------------------------------------------------
{
  t=(t<0)?m_CurrentTime:t;
  
  mafMatrix matrix;

  mafTransform::SetOrientation(matrix,rxyz);
  mafTransform::SetPosition(matrix,xyz);
  matrix.SetTimeStamp(t);

  SetAbsMatrix(matrix);
}

//-------------------------------------------------------------------------
void mafVME::SetAbsMatrix(const mafMatrix &matrix,mafTimeStamp t)
//-------------------------------------------------------------------------
{
  t=(t<0)?m_CurrentTime:t;

  mafMatrix mat=matrix;
  mat.SetTimeStamp(t);
  SetAbsMatrix(mat);
}
//-------------------------------------------------------------------------
void mafVME::SetAbsMatrix(const mafMatrix &matrix)
//-------------------------------------------------------------------------
{
  if (GetParent())
  {
    mafMatrix pmat;
    GetParent()->GetOutput()->GetAbsMatrix(pmat,matrix.GetTimeStamp());

    pmat.Invert();

    mafMatrix::Multiply4x4(pmat,matrix,pmat);

    //inherit timestamp from user provided!
    pmat.SetTimeStamp(matrix.GetTimeStamp()); //modified by Vladik Aranov 25-03-2005
    
    SetMatrix(pmat);
    return;
  }
  
  SetMatrix(matrix);
}

//----------------------------------------------------------------------------
void mafVME::ApplyAbsMatrix(const mafMatrix &matrix,int premultiply,mafTimeStamp t)
//----------------------------------------------------------------------------
{
  t=(t<0)?m_CurrentTime:t;
  mafTransform new_pose;
  mafMatrix pose;
  GetOutput()->GetAbsMatrix(pose,t);
  new_pose.SetMatrix(pose);
  new_pose.Concatenate(matrix,premultiply);
  new_pose.SetTimeStamp(t);
  SetAbsMatrix(new_pose.GetMatrix());
}

/*
//-------------------------------------------------------------------------
void mafVME::OutputDataUpdateCallback(void *arg)
//-------------------------------------------------------------------------
{
  // route events toward VME observers
  mafVME *self=(mafVME *)arg;

  if (self->GetDataPipe()) // allocate data pipe if not done yet
  {
    if (self->m_DataPipe->GetOutput()!=self->CurrentData.GetPointer())
    {
      self->SetCurrentData(self->m_DataPipe->GetOutput());
    
      // advise observers the output data has changed
      self->InvokeEvent(mafVME::OutputDataChangedEvent);
    }

    self->InvokeEvent(mafVME::OutputDataUpdateEvent);
  }
  else
  {
    vtkErrorWithObjectMacro(self,"Received Update Output data event from NULL data pipe!");
  }
}

//-------------------------------------------------------------------------
void mafVME::MatrixUpdateCallback(void *arg)
//-------------------------------------------------------------------------
{
  // route events toward VME observers
  mafVME *self=(mafVME *)arg;
  self->InvokeEvent(mafVME::MatrixUpdateEvent,self);
}

//-------------------------------------------------------------------------
void mafVME::AbsMatrixUpdateCallback(void *arg)
//-------------------------------------------------------------------------
{
  // route events toward VME observers
  mafVME *self=(mafVME *)arg;
  self->InvokeEvent(mafVME::AbsMatrixUpdateEvent,self);
}
*/
//-------------------------------------------------------------------------
void mafVME::Print(std::ostream& os, const int tabs)// const
//-------------------------------------------------------------------------
{
  Superclass::Print(os,tabs);
  
  mafIndent indent(tabs);

  os << indent << "Current Time: "<<m_CurrentTime<<"\n";

  os << indent << "Output:\n";
  GetOutput()->Print(os,indent.GetNextIndent());

  os << indent << "Matrix Pipe: ";
  if (m_MatrixPipe)
  {
    os << "\n";
    m_MatrixPipe->Print(os,indent.GetNextIndent());
  }
  else
    os << std::endl;
  
  os << indent << "DataPipe: ";
  if (m_DataPipe) // allocate data pipe if not done yet
  {
    os << "\n";
    m_DataPipe->Print(os,indent.GetNextIndent());
  }
  else
    os << std::endl;
}

/*
//-------------------------------------------------------------------------
int mafVME::SetAuxiliaryRefSys(mafTransform *AuxRefSys, const char *RefSysName, int type)
//-------------------------------------------------------------------------
{
  if (AuxRefSys)
  {
    return SetAuxiliaryRefSys(AuxRefSys->GetMatrix(),RefSysName,type);
  }

  return MAF_ERROR;
}

//-------------------------------------------------------------------------
int mafVME::SetAuxiliaryRefSys(mafMatrix *AuxRefSys, const char *RefSysName, int type)
//-------------------------------------------------------------------------
{
  if (AuxRefSys)
  {
	  //check for type existence
	  if (type == MFL_LOCAL_FRAME_TAG || type ==MFL_GLOBAL_FRAME_TAG)
	  {
      vtkTagItem item;
	    item.SetName(RefSysName);

	    for ( int i = 0; i < 4; i++)
	    {
		    for ( int j = 0; j < 4; j++)
		    {
		    item.AddComponent(AuxRefSys->GetElement(i,j));
		    }
	    }

	    item.SetType(type);
  
      GetTagArray()->AddTag(item);
	  
	    return MAF_OK;
	  }
  }

	return MAF_ERROR;
}


//-------------------------------------------------------------------------
int mafVME::GetAuxiliaryRefSys(mafTransform *AuxRefSys, const char *RefSysName, int type)
//-------------------------------------------------------------------------
{
  if (AuxRefSys)
  {
    mafMatrix matrix;
    if (GetAuxiliaryRefSys(matrix,RefSysName,type)==0)
    {
      AuxRefSys->SetMatrix(matrix);

      return MAF_OK;
    }
  }

  return MAF_ERROR;
}

//-------------------------------------------------------------------------
int mafVME::GetAuxiliaryRefSys(mafMatrix *AuxRefSys, const char *RefSysName, int type)
//-------------------------------------------------------------------------
{
	//check for type existence
  if (type == MFL_LOCAL_FRAME_TAG || type ==MFL_GLOBAL_FRAME_TAG)
  {
    if (AuxRefSys)
    {
      if (vtkString::Compare(RefSysName,"Global") == 0)
		  {
        if (GetParent())
        {
          AuxRefSys->DeepCopy(GetParent()->GetAbsMatrix());
        }
        else
        {
          AuxRefSys->Identity();
        }

        return MAF_OK;
      }

		  
      vtkTagItem *item=GetTagArray()->GetTag(RefSysName);
	    
      if (item)
      {
			  if (item->GetStaticType() == type)
			  {			
			      //copy from tag item in frame
					  int item_component = 1;

					  for ( int i = 0; i < 4; i++)
					  {
						  for ( int j = 0; j < 4; j++)
						  {
              
						    AuxRefSys->SetElement(i, j, item->GetComponentAsDouble(item_component));
						    item_component++;
						  }
					  }
          
					  return MAF_OK;
			  }
		  }
      else if (RefSysName == "Default")
      {
        // if not Default reference system was specified return the Local reference system
        // i.e. the identity!
        AuxRefSys->Identity();
        return MAF_OK;
      }
	  }
  }
  return MAF_ERROR;
}
*/

//-------------------------------------------------------------------------
void mafVME::SetOutput(mafVMEOutput *output)
//-------------------------------------------------------------------------
{
  cppDEL(m_Output);

  m_Output=output;
  
  if (m_Output)
  {
    m_Output->SetVME(this);
  }
  
  // force the update of the abs matrix pipe
  if (m_AbsMatrixPipe.GetPointer())
    m_AbsMatrixPipe->SetVME(this);
}

//-------------------------------------------------------------------------
int mafVME::SetMatrixPipe(mafMatrixPipe *mpipe)
//-------------------------------------------------------------------------
{
  if (mpipe!=m_MatrixPipe)
  {
    if (mpipe==NULL||mpipe->SetVME(this)==MAF_OK)
    { 
      if (m_MatrixPipe)
      {
        // detach the old pipe
        m_MatrixPipe->SetVME(NULL);
        m_MatrixPipe->SetTimeStamp(m_CurrentTime);
      }
      
      m_MatrixPipe = mpipe;

      if (mpipe)
      {
        mpipe->SetVME(this);
        mpipe->SetTimeStamp(GetTimeStamp());
      }
      
      // this forces the the pipe to Update its input and input frame
      if (m_AbsMatrixPipe)
        m_AbsMatrixPipe->SetVME(this);

      GetEventSource()->InvokeEvent(this,VME_MATRIX_CHANGED);

      return MAF_OK;
    }
    else
    {
      return MAF_ERROR;
    }
  }

  return MAF_OK;
}

//-------------------------------------------------------------------------
void mafVME::Update()
//-------------------------------------------------------------------------
{
  //InternalPreUpdate();
  //InternalUpdate();

  if (GetMatrixPipe())
    GetMatrixPipe()->Update();

  if (GetDataPipe())
    GetDataPipe()->Update();

  m_VisualMode = IsDataAvailable() ? DEFAULT_VISUAL_MODE : NO_DATA_VISUAL_MODE;
}

//-------------------------------------------------------------------------
void mafVME::SetCrypting(int crypting)
//-------------------------------------------------------------------------
{
  if(crypting > 0)
    m_Crypting = 1;
  else
    m_Crypting = 0;

  if (m_Gui != NULL)
  {
    m_Gui->Update();
  }

  Modified();
  mafEvent ev(this,VME_MODIFIED,this);
  ForwardUpEvent(ev);
}

//-------------------------------------------------------------------------
int mafVME::GetCrypting()
//-------------------------------------------------------------------------
{
  return m_Crypting;
}

//-------------------------------------------------------------------------
int mafVME::SetDataPipe(mafDataPipe *dpipe)
//-------------------------------------------------------------------------
{
  if (dpipe==m_DataPipe.GetPointer())
    return MAF_OK;

  if (dpipe==NULL||dpipe->SetVME(this)==MAF_OK)
  { 
    // if we had an observer...
    if (m_DataPipe)
    {
      // detach the old pipe
      m_DataPipe->SetVME(NULL);
    }

    m_DataPipe = dpipe;
    
    if (m_DataPipe)
    {
      m_DataPipe->SetVME(this);
      m_DataPipe->SetTimeStamp(m_CurrentTime);
    }

    // advise listeners the data pipe has changed
    GetEventSource()->InvokeEvent(this,VME_OUTPUT_DATA_CHANGED);

    return MAF_OK;
  }
  else
  {
    return MAF_ERROR;
  }
}

//-------------------------------------------------------------------------
void mafVME::OnEvent(mafEventBase *maf_event)
//-------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    Superclass::OnEvent(maf_event);
  }
  else if (maf_event->GetId() == mafVMEItem::VME_ITEM_DATA_MODIFIED)
  {
    // Paolo 25-05-2007: Intercept the item data modified to update the output
    this->GetOutput()->Update();
  }
  else if (maf_event->GetChannel()==MCH_DOWN)
  {
    switch (maf_event->GetId())
    {
      case VME_TIME_SET:
        SetTimeStamp(*((mafTimeStamp *)maf_event->GetData()));
        Superclass::OnEvent(maf_event);
      break;
			default:
				Superclass::OnEvent(maf_event);
    }
  }
  else if (maf_event->GetChannel()==MCH_UP)
  {
    switch (maf_event->GetId())
    {
      case VME_OUTPUT_DATA_PREUPDATE:      
        InternalPreUpdate();  // self process the event
        GetEventSource()->InvokeEvent(maf_event); // forward event to observers
      break;
      case VME_OUTPUT_DATA_UPDATE:
        InternalUpdate();   // self process the event
        GetEventSource()->InvokeEvent(maf_event); // forward event to observers
      break;
      case VME_MATRIX_UPDATE:
			{
				mafEventBase absEvent(this, VME_ABSMATRIX_UPDATE);
				if (maf_event->GetSender() == m_AbsMatrixPipe)
				{
					GetEventSource()->InvokeEvent(&absEvent);
				}
				else
        {
					GetEventSource()->InvokeEvent(maf_event); // forward event to observers
        }

				for (int i = 0; i < this->GetNumberOfChildren(); i++)
				{
					((mafVME*)GetChild(i))->GetEventSource()->InvokeEvent(&absEvent);
				}
			}
      break;
      default:
        Superclass::OnEvent(maf_event);
    }
  }
  else if (maf_event->GetChannel() == MCH_NODE)
  {
    Superclass::OnEvent(maf_event);
  }
}

//-------------------------------------------------------------------------
int mafVME::InternalStore(mafStorageElement *parent)
//-------------------------------------------------------------------------
{
  if (Superclass::InternalStore(parent)==MAF_OK)
  {
    parent->SetAttribute("Crypting",mafString(m_Crypting));
    return MAF_OK;
  }
  return MAF_ERROR;
}

//-------------------------------------------------------------------------
int mafVME::InternalRestore(mafStorageElement *node)
//-------------------------------------------------------------------------
{
  if (Superclass::InternalRestore(node)==MAF_OK)
  {
    mafID crypt;
    node->GetAttributeAsInteger("Crypting",crypt);
    SetCrypting(crypt);

    return MAF_OK;
  }
  return MAF_ERROR;
}

//-------------------------------------------------------------------------
mafGUI *mafVME::CreateGui()
//-------------------------------------------------------------------------
{
  m_Gui = mafNode::CreateGui(); // Called to show info about vmes' type and name
  m_Gui->SetListener(this);

  mafString anim_text;
  anim_text = _("not animated");
  if (IsAnimated())
  {
    anim_text = _("animated");
  }
  m_Gui->Label(anim_text);
	m_Gui->Divider();

  return m_Gui;
}
//-------------------------------------------------------------------------
void mafVME::SetVisualMode(int mode)
//-------------------------------------------------------------------------
{
  if (m_VisualMode != mode)
  {
    m_VisualMode = mode;
    mafEvent updateModalityEvent(this, VME_VISUAL_MODE_CHANGED, this);
    Superclass::OnEvent(&updateModalityEvent);
  }
}
