/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafOpExtractIsosurface.h,v $
  Language:  C++
  Date:      $Date: 2008-03-13 17:07:18 $
  Version:   $Revision: 1.2 $
  Authors:   Paolo Quadrani     Silvano Imboden
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __mafOpExtractIsosurface_H__
#define __mafOpExtractIsosurface_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mmgDialog;
class mmgFloatSlider;
class mafVMESurface;
class mafNode;
class mafRWI;

class vtkActor;
class vtkContourVolumeMapper;
class vtkPolyDataMapper;
class vtkOutlineCornerFilter;
class vtkRenderer;

class mmiExtractIsosurface;
class vtkImageData;
class vtkTexture;
class vtkPolyData;
class vtkVolumeSlicer;
class vtkFixedCutter;
class vtkPlane;
class vtkVolume;

//----------------------------------------------------------------------------
// mafOpExtractIsosurface :
//----------------------------------------------------------------------------
/** This operation generate a surface representing a collection of a selected scalar value of the input volume.
The operation offers a preview of the Iso-Density-Surface and offers also a section of the volume with the 
contour of the surface itself. The user can pick also into the slice with the left mouse button to generate the 
corresponding iso-density-surface.*/
class mafOpExtractIsosurface: public mafOp
{
public:
	mafOpExtractIsosurface(const wxString &label = "Extract Isosurface");
 ~mafOpExtractIsosurface(); 
	
  mafTypeMacro(mafOpExtractIsosurface, mafOp);

  mafOp* Copy();
  void OnEvent(mafEventBase *maf_event);

	/** Return true for the acceptable vme type. */
  bool Accept(mafNode* vme);

	/** Builds operation's interface by calling CreateOpDialog() method. */
  void OpRun();

	/** Execute the operation. */
  void OpDo();

	/** Makes the undo for the operation. */
  void OpUndo();

protected:
  mafVMESurface *m_IsosurfaceVme;   // polydata VME

  mmgDialog		*m_Dialog;
	mafRWI      *m_Rwi;
  vtkRenderer *m_PIPRen;
	double       m_IsoValue;
	double       m_MaxDensity;
	double       m_MinDensity;
	double       m_StepDensity;
  double       m_Slice;
  double       m_SliceMin;
  double       m_SliceMax;
	double       m_SliceStep;
  int          m_ShowSlice;
  int          m_Optimize;
	int					 m_Clean;
	int					 m_Triangulate;
  int          m_Autolod;
  double       m_BoundingBox[6];
  double       m_SliceOrigin[3];
  float        m_SliceXVect[3];
  float        m_SliceYVect[3];

  mmgFloatSlider *m_IsoSlider;
  mmgFloatSlider *m_SliceSlider;
  
  //vtkPolyDataMapper       *m_ContourMapper;
  vtkVolume                *m_ContourActor;
  vtkActor                *m_Box;
  vtkContourVolumeMapper  *m_ContourVolumeMapper; 
  vtkOutlineCornerFilter  *m_OutlineFilter;
  vtkPolyDataMapper       *m_OutlineMapper;

  vtkVolumeSlicer   *m_VolumeSlicer;
  vtkVolumeSlicer   *m_PolydataSlicer;
  vtkImageData      *m_SliceImage;
  vtkTexture        *m_SliceTexture;
  vtkPolyData       *m_Polydata;          // copy of polydata used by slicer
  vtkPolyDataMapper *m_SlicerMapper;
  vtkActor          *m_SlicerActor;
  vtkPolyDataMapper *m_PolydataMapper;
  vtkActor          *m_PolydataActor;
  vtkPlane          *m_CutterPlane;
  vtkFixedCutter    *m_IsosurfaceCutter;

  mmiExtractIsosurface *m_DensityPicker;

	/** 
  Builds operation's interface and visualization pipeline. */
  void CreateOpDialog();

	/** 
  Remove operation's interface. */
  void DeleteOpDialog();

	/** 
  Create the pipeline to generate the isosurface of the volume. */
  void CreateVolumePipeline();

	/** 
  Create the pipeline to generate the slice of the volume. */
  void CreateSlicePipeline();

	/** 
  Extract the isosurface and build the related vme. */
  void ExtractSurface(bool clean=true);

	/** 
  Re-generate the surface according to the new threshold value. */
  void UpdateSurface(bool use_lod = false);

	/** 
  Re-slice the volume according to the new coordinate value. */
  void UpdateSlice();
};
#endif