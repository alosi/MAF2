/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: exScalarApp.cpp,v $
  Language:  C++
  Date:      $Date: 2006-06-14 13:34:46 $
  Version:   $Revision: 1.1 $
  Authors:   Paolo Quadrani
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "exScalarApp.h"
#include "mafDecl.h"

#include "mafVMEFactory.h"
#include "mafPics.h"
#include "mmgMDIFrame.h"

#include "mafNodeFactory.h"
#include "mafNodeGeneric.h"
#include "mafNodeRoot.h"
#include "mafVMERoot.h"
#include "mafVMESurface.h"
#include "mafPipeFactoryVME.h"

#include "mmoVTKExporter.h"

#ifdef MAF_USE_ITK
  #include "mmoASCIIImporter.h"
  #include "mafViewScalar.h"
#endif

#include "mafViewVTK.h"

//--------------------------------------------------------------------------------
// Create the Application
//--------------------------------------------------------------------------------
IMPLEMENT_APP(exScalarApp)

//--------------------------------------------------------------------------------
bool exScalarApp::OnInit()
//--------------------------------------------------------------------------------
{
  mafPics.Initialize();	

  int result = mafVMEFactory::Initialize();
  assert(result==MAF_OK);
  
  // Initialize and Fill of PipeFactory -- could be a SideEffect of the node plug
  result = mafPipeFactoryVME::Initialize();
  assert(result==MAF_OK);

  m_Logic = new mafLogicWithManagers();
  m_Logic->Configure();

  m_Logic->GetTopWin()->SetTitle("ScalarsApp");
  SetTopWindow(mafGetFrame());  

  //------------------------------------------------------------
  // Importer Menu':
  //------------------------------------------------------------
#ifdef MAF_USE_ITK
  m_Logic->Plug(new mmoASCIIImporter("ASCII"));
#endif
  //------------------------------------------------------------

  //------------------------------------------------------------
  // Exporter Menu':
  //------------------------------------------------------------
  m_Logic->Plug(new mmoVTKExporter("VTK"));

  //------------------------------------------------------------

  //------------------------------------------------------------
  // Operation Menu':
  //------------------------------------------------------------

  //------------------------------------------------------------
  
  //------------------------------------------------------------
  // View Menu':
  //------------------------------------------------------------
#ifdef MAF_USE_ITK
  m_Logic->Plug(new mafViewScalar());
#endif
  //------------------------------------------------------------

  //wxHandleFatalExceptions();

  //m_Logic->ShowSplashScreen();
  m_Logic->Show();
  mafString app_stamp;
  app_stamp = "OPEN_ALL_DATA";
  m_Logic->SetApplicationStamp(app_stamp);
  m_Logic->Init(0,NULL); // calls FileNew - which create the root
  return TRUE;
}
//--------------------------------------------------------------------------------
int exScalarApp::OnExit()
//--------------------------------------------------------------------------------
{
  cppDEL(m_Logic);
  return 0;
}
//--------------------------------------------------------------------------------
void exScalarApp::OnFatalException()
//--------------------------------------------------------------------------------
{
  m_Logic->HandleException();
}