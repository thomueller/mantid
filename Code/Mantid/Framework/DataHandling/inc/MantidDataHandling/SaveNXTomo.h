#ifndef MANTID_DATAHANDLING_SAVENXTOMO_H_
#define MANTID_DATAHANDLING_SAVENXTOMO_H_

//---------------------------------------------------
// Includes
//---------------------------------------------------
//#include "MantidNexus/NexusClasses.h"
//#include <nexus/NeXusFile.hpp>
//#include <nexus/NeXusException.hpp>

namespace Mantid
{
	namespace DataHandling
	{

		/**
		 * Saves a workspace into a NeXus/HDF5 NXTomo file.
     * File format is defined here: http://download.nexusformat.org/sphinx/classes/applications/NXtomo.html
		 *
		 * Required properties:
		 * <ul>
		 * <li> InputWorkspace - The workspace to save. </li>
		 * <li> Filename - The filename for output </li>
		 * </ul>
		 *
		 * @author John R Hill, RAL 
		 * @date 10/09/2014
		 *
		 * This file is part of Mantid.
		 *
		 *   Mantid is free software; you can redistribute it and/or modify
		 *   it under the terms of the GNU General Public License as published by
		 *   the Free Software Foundation; either version 3 of the License, or
		 *   (at your option) any later version.
		 *
		 *   Mantid is distributed in the hope that it will be useful,
		 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
		 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		 *   GNU General Public License for more details.
		 *
		 *   You should have received a copy of the GNU General Public License
		 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
		 *
		 *   File change history is stored at: <https://github.com/mantidproject/mantid>
		 *   Code Documentation is available at: <http://doxygen.mantidproject.org>
		 *
		 */

		class DLLExport SaveNXTomo: public API::Algorithm
		{
		public:
			SaveNXTomo();
			/// Virtual dtor
			virtual ~SaveNXTomo() {}

      /// Algorithm's name for identification overriding a virtual method
			virtual const std::string name() const {	return "SaveNXTomo"; }

			///Summary of algorithms purpose
			virtual const std::string summary() const {return "Writes a MatrixWorkspace to a file in the NXTomo format.";}

			/// Algorithm's version
			virtual int version() const	{	return (1);	}

			/// Algorithm's category for identification
			virtual const std::string category() const { return "DataHandling\\Nexus;DataHandling\\Tomo;Diffraction";	}

		private:			
			/// Initialisation code
			void init();
			/// Execution code
			void exec();

      /// Save all data to file

      /// Save batch of images to the file

			///the number of bins in each histogram, as the histogram must have common bins this shouldn't change
			size_t m_nBins;
			/// The filename of the output file
			std::string m_filename;

			// Some constants to be written for masked values.
			/// Value for data if pixel is masked
			static const double MASK_FLAG;
			/// Value for error if pixel is masked
			static const double MASK_ERROR;
		  /// file format version
		  static const std::string NXTOMO_VER;
		};

	} // namespace DataHandling
} // namespace Mantid

#endif // MANTID_DATAHANDLING_SAVENXTOMO_H_
