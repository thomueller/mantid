/*WIKI* 


Saves an instrument with RectangularDetectors to an ISAW .DetCal file.

This algorithm will fail on instruments without RectangularDetectors. Additionally, the banks should be named "bankXX" where XX is the bank index.
Other names will fail or create an invalid .DetCal file.



*WIKI*/
#include "MantidAPI/FileProperty.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidDataHandling/SaveIsawDetCal.h"
#include "MantidGeometry/Instrument.h"
#include "MantidGeometry/Instrument/RectangularDetector.h"
#include "MantidKernel/System.h"
#include <fstream>
#include "MantidAPI/Workspace.h"
#include "MantidAPI/ExperimentInfo.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace Mantid::Geometry;

namespace Mantid
{
namespace DataHandling
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(SaveIsawDetCal)
  


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  SaveIsawDetCal::SaveIsawDetCal()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  SaveIsawDetCal::~SaveIsawDetCal()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void SaveIsawDetCal::initDocs()
  {
    this->setWikiSummary("Saves an instrument with RectangularDetectors to an ISAW .DetCal file.");
    this->setOptionalMessage("Saves an instrument with RectangularDetectors to an ISAW .DetCal file.");
  }

  //----------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
   */
  void SaveIsawDetCal::init()
  {
    declareProperty(new WorkspaceProperty<Workspace>("InputWorkspace","",Direction::Input), "An input workspace.");

    std::vector<std::string> exts;
    exts.push_back(".DetCal");

    declareProperty(new FileProperty("Filename", "", FileProperty::Save, exts),
        "Path to an ISAW-style .detcal file to save.");

    declareProperty( "TimeOffset",0.0,"Offsets to be applied to times");
  }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
   */
  void SaveIsawDetCal::exec()
  {
    std::string filename = getPropertyValue("Filename");
   // MatrixWorkspace_sptr ws = getProperty("InputWorkspace");

    Workspace_sptr ws1 = getProperty("InputWorkspace");
    ExperimentInfo_sptr ws = boost::dynamic_pointer_cast<ExperimentInfo>(ws1);

    double T0= getProperty("TimeOffset");
    std::ofstream out;
    out.open( filename.c_str());

    Instrument_const_sptr inst = ws->getInstrument();
    if (!inst) throw std::runtime_error("No instrument in the Workspace. Cannot save DetCal file.");

    double l1; V3D beamline; double beamline_norm; V3D samplePos;
    inst->getInstrumentParameters(l1, beamline, beamline_norm, samplePos);

    out << "6         L1    T0_SHIFT" <<  std::endl;
    out << "7 "<< std::setw( 10 )  ;
    out <<   std::setprecision( 4 ) <<  std::fixed <<  ( l1*100 ) ;
    out << std::setw( 12 ) <<  std::setprecision( 3 ) <<  std::fixed  ;
    // Time offset of 0.00 for now
    out <<std::setw( 12 ) <<  std::setprecision( 4) << T0 <<  std::endl;

    out <<  "4 DETNUM  NROWS  NCOLS   WIDTH   HEIGHT   DEPTH   DETD   CenterX   CenterY   CenterZ    BaseX    BaseY    BaseZ      UpX      UpY      UpZ"
        <<  std::endl;

    // Get all children
    std::vector<IComponent_const_sptr> comps;
    inst->getChildren(comps, true);

    for (size_t i=0; i<comps.size(); i++)
    {
      // Retrieve it
      RectangularDetector_const_sptr det = boost::dynamic_pointer_cast<const RectangularDetector>(comps[i]);
      if (det)
      {
        std::string name = det->getName();
        if (name.size() < 5) continue;
        std::string bank = name.substr(4,name.size()-4);

        // Center of the detector
        V3D center = det->getPos();
        // Distance to center of detector
        double detd = (center - inst->getSample()->getPos()).norm();

        // Base unit vector (along the horizontal, X axis)
        V3D base = det->getAtXY(det->xpixels()-1,0)->getPos() - det->getAtXY(0,0)->getPos();
        base.normalize();
        // Up unit vector (along the vertical, Y axis)
        V3D up = det->getAtXY(0,det->ypixels()-1)->getPos() - det->getAtXY(0,0)->getPos();
        up.normalize();

        // Write the line
        out << "5 "
         << std::setw(6) << std::right << bank << " "
         << std::setw(6) << std::right << det->xpixels() << " "
         << std::setw(6) << std::right << det->ypixels() << " "
         << std::setw(7) << std::right << std::fixed << std::setprecision(4) << 100.0*det->xsize() << " "
         << std::setw(7) << std::right << std::fixed << std::setprecision(4) << 100.0*det->ysize() << " "
         << "  0.2000 "
         << std::setw(6) << std::right << std::fixed << std::setprecision(2) << 100.0*detd << " "
         << std::setw(9) << std::right << std::fixed << std::setprecision(4) << 100.0*center.X() << " "
         << std::setw(9) << std::right << std::fixed << std::setprecision(4) << 100.0*center.Y() << " "
         << std::setw(9) << std::right << std::fixed << std::setprecision(4) << 100.0*center.Z() << " "
         << std::setw(8) << std::right << std::fixed << std::setprecision(5) << base.X() << " "
         << std::setw(8) << std::right << std::fixed << std::setprecision(5) << base.Y() << " "
         << std::setw(8) << std::right << std::fixed << std::setprecision(5) << base.Z() << " "
         << std::setw(8) << std::right << std::fixed << std::setprecision(5) << up.X() << " "
         << std::setw(8) << std::right << std::fixed << std::setprecision(5) << up.Y() << " "
         << std::setw(8) << std::right << std::fixed << std::setprecision(5) << up.Z() << " "
         << std::endl;
      }
    }

    out.close();

  }



} // namespace Mantid
} // namespace DataHandling

