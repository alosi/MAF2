/*=========================================================================

 Program: MAF2
 Module: vtkMAFVolumeResampleTest
 Authors: Stefano Perticoni
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __CPP_UNIT_MMOVOLUMERESAMPLETEST_H__
#define __CPP_UNIT_MMOVOLUMERESAMPLETEST_H__

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

#include "vtkDataSet.h"
#include "vtkTransform.h"

class vtkMAFVolumeResampleTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( vtkMAFVolumeResampleTest );
  CPPUNIT_TEST(TestResample);
  CPPUNIT_TEST(TestSetGetVolumeOrigin);
  CPPUNIT_TEST(TestSetGetVolumeAxisX);
  CPPUNIT_TEST(TestSetGetVolumeAxisY);
  CPPUNIT_TEST(TestSetGetWindow);
  CPPUNIT_TEST(TestSetGetLevel);

  CPPUNIT_TEST_SUITE_END();

  protected:
    
    void TestResample();
    void TestSetGetVolumeOrigin();
    void TestSetGetVolumeAxisX();
    void TestSetGetVolumeAxisY();
    void TestSetGetWindow();
    void TestSetGetLevel();

    void WriteVTKDatasetToFile( vtkDataSet * outputVolumeVTKData, const char *outputFilename );
    void TestResampleInternal( const char *inFileName , const char *outVTKFileName );

    void PrintDouble6( ostream& os, double array[6], const char *logMessage = NULL );
    void PrintDouble3( ostream& os, double array[3], const char *logMessage = NULL);
    void PrintInt3( ostream& os, int array[3], const char *logMessage = NULL );
};


int
main( int argc, char* argv[] )
{
  // Create the event manager and test controller
  CPPUNIT_NS::TestResult controller;

  // Add a listener that colllects test result
  CPPUNIT_NS::TestResultCollector result;
  controller.addListener( &result );        

  // Add a listener that print dots as test run.
  CPPUNIT_NS::BriefTestProgressListener progress;
  controller.addListener( &progress );      

  // Add the top suite to the test runner
  CPPUNIT_NS::TestRunner runner;
  runner.addTest( vtkMAFVolumeResampleTest::suite());
  runner.run( controller );

  // Print test in a compiler compatible format.
  CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
  outputter.write(); 

  return result.wasSuccessful() ? 0 : 1;
}

#endif
