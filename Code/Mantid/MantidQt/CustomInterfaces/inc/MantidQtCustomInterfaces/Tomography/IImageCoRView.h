#ifndef MANTIDQTCUSTOMINTERFACES_TOMOGRAPHY_IIMAGECORVIEW_H_
#define MANTIDQTCUSTOMINTERFACES_TOMOGRAPHY_IIMAGECORVIEW_H_

#include "MantidAPI/WorkspaceGroup_fwd.h"
#include "MantidQtCustomInterfaces/Tomography/ImageStackPreParams.h"

namespace MantidQt {
namespace CustomInterfaces {

/**
Widget to handle the selection of the center of rotation, region of
interest, region for normalization, etc. from an image or stack of
images. This is the abstract base class / interface for the view of
this widget (in the sense of the MVP pattern).  The name ImageCoR
refers to the Center-of-Rotation, which is the most basic parameter
that users can select via this widget. This class is Qt-free. Qt
specific functionality and dependencies are added in a class derived
from this.

Copyright &copy; 2015 ISIS Rutherford Appleton Laboratory, NScD
Oak Ridge National Laboratory & European Spallation Source

This file is part of Mantid.

Mantid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Mantid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

File change history is stored at: <https://github.com/mantidproject/mantid>
Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class IImageCoRView {

public:
  IImageCoRView(){};
  virtual ~IImageCoRView(){};

  /**
   * Sets the user selection. This should guarantee that all widgets
   * are updated (including spin boxes, image, slider through the
   * image stack, etc.
   *
   * @param tools identifiers of the tools that can or could be run.
   * Order matters
   *
   */
  virtual void initParams(ImageStackPreParams &params) = 0;

  /**
   * Provides the current user selection.
   *
   * @return parameters as set/edited by the user.
   */
  virtual ImageStackPreParams userSelection() const = 0;

  /**
   * Path to a stack of images that the user has requested to
   * display. The path would be expected to point to a recognized
   * directory structure (sample/dark/white) or image file (as a
   * particular case).
   *
   * @return directory of file path as a string
   */
  virtual std::string stackPath() const = 0;

  /**
   * Display a special case of stack of images: individual image, from
   * a path to a recognized directory structure (sample/dark/white) or
   * image format. Here recognized format means something that is
   * supported natively by the widgets library, in practice
   * Qt. Normally you can expect that .tiff and .png images are
   * supported.
   *
   * @param path path to the stack (directory) or individual image file.
   */
  virtual void showStack(const std::string &path) = 0;

  /**
   * Display a stack of images (or individual image as a particular
   * case), from a workspace group containing matrix workspaces. It
   * assumes that the workspace contains an image in the form in which
   * LoadFITS loads FITS images (or spectrum per row, all of them with
   * the same number of data points (columns)).
   *
   * @param ws Workspace group where every workspace is a FITS or
   * similar image that has been loaded with LoadFITS or similar
   * algorithm.
   */
  virtual void showStack(const Mantid::API::WorkspaceGroup_sptr &ws) = 0;

  /**
   * Display a warning to the user (for example as a pop-up window).
   *
   * @param warn warning title, should be short and would normally be
   * shown as the title of the window or a big banner.
   *
   * @param description longer, free form description of the issue.
   */
  virtual void userWarning(const std::string &warn,
                           const std::string &description) = 0;

  /**
   * Display an error message (for example as a pop-up window).
   *
   * @param err Error title, should be short and would normally be
   * shown as the title of the window or a big banner.
   *
   * @param description longer, free form description of the issue.
   */
  virtual void userError(const std::string &err,
                         const std::string &description) = 0;

  /**
   * Save settings (normally when closing this widget).
   */
  virtual void saveSettings() const = 0;
};

} // namespace CustomInterfaces
} // namespace MantidQt

#endif // MANTIDQTCUSTOMINTERFACES_TOMOGRAPHY_IIMAGECORVIEW_H_
