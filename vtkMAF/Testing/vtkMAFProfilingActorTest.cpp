/*==============================================================================

  Program:   Multimod Application Framework
  Module:    $RCSfile: vtkMAFProfilingActorTest.cpp,v $
  Language:  C++
  Date:      $Date: 2009-09-09 08:16:31 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Alberto Losi

================================================================================
  Copyright (c) 2007 Cineca, UK (www.cineca.it)
  All rights reserved.
===============================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "vtkMAFGridActor.h"
#include "vtkMAFProfilingActor.h"
#include "vtkMAFProfilingActorTest.h"

#include <cppunit/config/SourcePrefix.h>

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkMAFSmartPointer.h"
#include "vtkActor.h"
#include "vtkCamera.h"

#include "vtkWindowToImageFilter.h"
#include "vtkImageMathematics.h"
#include "vtkImageData.h"
#include "vtkJPEGWriter.h"
#include "vtkJPEGReader.h"
#include "vtkPointData.h"

#include "mafString.h"

//------------------------------------------------------------
void vtkMAFProfilingActorTest::setUp()
//------------------------------------------------------------
{
}
//------------------------------------------------------------
void vtkMAFProfilingActorTest::tearDown()
//------------------------------------------------------------
{
}
//------------------------------------------------------------
void vtkMAFProfilingActorTest::PrepareToRender(vtkRenderer *renderer, vtkRenderWindow *renderWindow)
//------------------------------------------------------------
{
  renderer->SetBackground(0.0, 0.0, 0.0);

  vtkCamera *camera = renderer->GetActiveCamera();
  camera->ParallelProjectionOn();
  camera->Modified();

  //vtkMAFSmartPointer<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(640, 480);
  renderWindow->SetPosition(100,0);

  vtkMAFSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);
}
//------------------------------------------------------------
void vtkMAFProfilingActorTest::TestDynamicAllocation()
//------------------------------------------------------------
{
  vtkMAFProfilingActor *profActor = vtkMAFProfilingActor::New();
  profActor->Delete();
}
//--------------------------------------------
void vtkMAFProfilingActorTest::TestRenderOverlay()
//--------------------------------------------
{
  vtkMAFSmartPointer<vtkRenderer> renderer;
  vtkMAFSmartPointer<vtkRenderWindow> renderWindow;
  PrepareToRender(renderer,renderWindow);
  
  vtkMAFProfilingActor *profActor = vtkMAFProfilingActor::New();
  
  renderer->AddActor(profActor);
  renderWindow->Render();

  CPPUNIT_ASSERT(profActor->RenderOverlay((vtkViewport*)renderer) == 1);

  renderWindow->Render();

  CompareImages(renderWindow);
  mafSleep(2000);
  mafDEL(profActor);
}
//--------------------------------------------
void vtkMAFProfilingActorTest::TestRenderOpaqueGeometry()
//--------------------------------------------
{
  vtkMAFSmartPointer<vtkRenderer> renderer;
  vtkMAFSmartPointer<vtkRenderWindow> renderWindow;
  PrepareToRender(renderer,renderWindow);

  vtkMAFProfilingActor *profActor = vtkMAFProfilingActor::New();

  renderer->AddActor(profActor);
  renderWindow->Render();

  CPPUNIT_ASSERT(profActor->RenderOpaqueGeometry((vtkViewport*)renderer) == 0);

  renderWindow->Render();

  CompareImages(renderWindow);
  mafSleep(2000);
  mafDEL(profActor);
}
//--------------------------------------------
void vtkMAFProfilingActorTest::TestRenderTranslucentGeometry()
//--------------------------------------------
{
  vtkMAFSmartPointer<vtkRenderer> renderer;
  vtkMAFSmartPointer<vtkRenderWindow> renderWindow;
  PrepareToRender(renderer,renderWindow);

  vtkMAFProfilingActor *profActor = vtkMAFProfilingActor::New();

  renderer->AddActor(profActor);

  CPPUNIT_ASSERT(profActor->RenderTranslucentGeometry((vtkViewport*)renderer) == 0); //This method only returns 0

  mafDEL(profActor);
  //renderWindow->Render();

  //CompareImages(renderWindow);
  //mafSleep(2000);
}
//----------------------------------------------------------------------------
void vtkMAFProfilingActorTest::CompareImages(vtkRenderWindow * renwin)
//----------------------------------------------------------------------------
{
  char *file = __FILE__;
  std::string name(file);
  std::string path(file);
  int slashIndex =  name.find_last_of('\\');

  
  name = name.substr(slashIndex+1);
  path = path.substr(0,slashIndex);

  int pointIndex =  name.find_last_of('.');

  name = name.substr(0, pointIndex);


  mafString controlOriginFile;
  controlOriginFile<<path.c_str();
  controlOriginFile<<"\\";
  controlOriginFile<<name.c_str();
  controlOriginFile<<"_";
  controlOriginFile<<"image";
  controlOriginFile<<m_TestNumber;
  controlOriginFile<<".jpg";

  fstream controlStream;
  controlStream.open(controlOriginFile.GetCStr()); 

  // visualization control
  renwin->OffScreenRenderingOn();
  vtkWindowToImageFilter *w2i;
  vtkNEW(w2i);
  w2i->SetInput(renwin);
  //w2i->SetMagnification(magnification);
  w2i->Update();
  renwin->OffScreenRenderingOff();

  //write comparing image
  vtkJPEGWriter *w;
  vtkNEW(w);
  w->SetInput(w2i->GetOutput());
  mafString imageFile="";

  if(!controlStream)
  {
    imageFile<<path.c_str();
    imageFile<<"\\";
    imageFile<<name.c_str();
    imageFile<<"_";
    imageFile<<"image";
  }
  else
  {
    imageFile<<path.c_str();
    imageFile<<"\\";
    imageFile<<name.c_str();
    imageFile<<"_";
    imageFile<<"comp";
  }

  imageFile<<m_TestNumber;
  imageFile<<".jpg";
  w->SetFileName(imageFile.GetCStr());
  w->Write();

  if(!controlStream)
  {
    controlStream.close();
    vtkDEL(w);
    vtkDEL(w2i);
    return;
  }
  controlStream.close();

  //read original Image
  vtkJPEGReader *rO;
  vtkNEW(rO);
  mafString imageFileOrig="";
  imageFileOrig<<path.c_str();
  imageFileOrig<<"\\";
  imageFileOrig<<name.c_str();
  imageFileOrig<<"_";
  imageFileOrig<<"image";
  imageFileOrig<<m_TestNumber;
  imageFileOrig<<".jpg";
  rO->SetFileName(imageFileOrig.GetCStr());
  rO->Update();

  vtkImageData *imDataOrig = rO->GetOutput();

  //read compared image
  vtkJPEGReader *rC;
  vtkNEW(rC);
  rC->SetFileName(imageFile.GetCStr());
  rC->Update();

  vtkImageData *imDataComp = rC->GetOutput();


  vtkImageMathematics *imageMath = vtkImageMathematics::New();
  imageMath->SetInput1(imDataOrig);
  imageMath->SetInput2(imDataComp);
  imageMath->SetOperationToSubtract();
  imageMath->Update();

  double srR[2] = {-1,1};
  imageMath->GetOutput()->GetPointData()->GetScalars()->GetRange(srR);

  CPPUNIT_ASSERT(srR[0] == 0.0 && srR[1] == 0.0);
  //CPPUNIT_ASSERT(ComparingImagesDetailed(imDataOrig,imDataComp));

  // end visualization control
  vtkDEL(rO);
  vtkDEL(rC);
  vtkDEL(imageMath);

  vtkDEL(w);
  vtkDEL(w2i);
}
//----------------------------------------------------------------------------
void vtkMAFProfilingActorTest::TestPrintSelf()
//----------------------------------------------------------------------------
{
  vtkMAFProfilingActor *actor;
  actor = vtkMAFProfilingActor::New();
  actor->PrintSelf(std::cout, 3);
  actor->Delete();
}