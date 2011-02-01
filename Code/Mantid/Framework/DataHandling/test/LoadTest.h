#ifndef LOADTEST_H_
#define LOADTEST_H_

#include <cxxtest/TestSuite.h>
#include "MantidDataHandling/Load.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidAPI/AnalysisDataService.h"

using namespace Mantid::API;
using namespace Mantid::DataObjects;
using namespace Mantid::DataHandling;

class LoadTest : public CxxTest::TestSuite
{
public:

  void testFindLoader()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","IRS38633.raw");
    const std::string outputName("LoadTest_IRS38633raw");
    loader.setPropertyValue("OutputWorkspace", outputName);
    loader.setPropertyValue("FindLoader", "1");
    loader.setRethrows(true);
    TS_ASSERT_THROWS_NOTHING(loader.execute());

    // Did it find the right loader
    TS_ASSERT_EQUALS(loader.getPropertyValue("LoaderName"), "LoadRaw");
    AnalysisDataServiceImpl& dataStore = AnalysisDataService::Instance();
    TS_ASSERT_EQUALS(dataStore.doesExist(outputName), false);
    if( dataStore.doesExist(outputName) )
    {
      AnalysisDataService::Instance().remove(outputName);
    }
  }

  void testRaw()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","IRS38633.raw");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
  }

  void testRaw1()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","HRP37129.s02");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
  }

  void testRawGroup()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","EVS13895.raw");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    WorkspaceGroup_sptr wsg = boost::dynamic_pointer_cast<WorkspaceGroup>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(wsg);
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output_1"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
    AnalysisDataService::Instance().remove("LoadTest_Output_1");
    AnalysisDataService::Instance().remove("LoadTest_Output_2");
    AnalysisDataService::Instance().remove("LoadTest_Output_3");
    AnalysisDataService::Instance().remove("LoadTest_Output_4");
    AnalysisDataService::Instance().remove("LoadTest_Output_5");
    AnalysisDataService::Instance().remove("LoadTest_Output_6");
  }
  
  void testHDF4Nexus()
  {
    // Note that there are no 64-bit HDF4 libraries for Windows.
#ifndef _WIN64
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","emu00006473.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
    #endif
  }

  void testHDF4NexusGroup()
  {
    // Note that there are no 64-bit HDF4 libraries for Windows.
#ifndef _WIN64
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","MUSR00015189.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    WorkspaceGroup_sptr wsg = boost::dynamic_pointer_cast<WorkspaceGroup>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(wsg);
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output_1"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
    AnalysisDataService::Instance().remove("LoadTest_Output_1");
    AnalysisDataService::Instance().remove("LoadTest_Output_2");
#endif
  }
   void testISISNexus()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","LOQ49886.nxs");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
  }

  void testUnknownExt()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","hrpd_new_072_01.cal");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    loader.setRethrows(true);
    TS_ASSERT_THROWS(loader.execute(), std::runtime_error);
    TS_ASSERT( !loader.isExecuted() );
  }

  void testSPE()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","Example.spe");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
  }

  void testAscii()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","AsciiExample.txt");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
  }

  void testSpice2D()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","BioSANS_exp61_scan0004_0001.xml");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
  }
  void testSNSSpec()
  {
     Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","LoadSNSspec.txt");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
  }

  void testGSS()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","gss.txt");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
  }

   void testRKH()
  {
    Load loader;
    loader.initialize();
    loader.setPropertyValue("Filename","DIRECT.041");
    loader.setPropertyValue("OutputWorkspace","LoadTest_Output");
    TS_ASSERT_THROWS_NOTHING(loader.execute());
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(AnalysisDataService::Instance().retrieve("LoadTest_Output"));
    TS_ASSERT(ws);
    AnalysisDataService::Instance().remove("LoadTest_Output");
  }
};

#endif /*LOADTEST_H_*/
