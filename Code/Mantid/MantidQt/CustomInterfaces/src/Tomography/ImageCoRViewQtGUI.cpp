#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidQtAPI/AlgorithmInputHistory.h"
#include "MantidQtAPI/AlgorithmRunner.h"

#include "MantidQtCustomInterfaces/Tomography/ImageCoRViewQtGUI.h"
#include "MantidQtCustomInterfaces/Tomography/ImageCoRPresenter.h"

using namespace Mantid::API;
using namespace MantidQt::CustomInterfaces;

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>

namespace MantidQt {
namespace CustomInterfaces {

// this would be more like a CustomWidget if it's eventually moved there
const std::string ImageCoRViewQtGUI::m_settingsGroup =
    "CustomInterfaces/ImageCoRView";

ImageCoRViewQtGUI::ImageCoRViewQtGUI(QWidget *parent)
    : QWidget(parent), IImageCoRView(), m_presenter(NULL) {
  initLayout();
}

void ImageCoRViewQtGUI::initLayout() {
  // setup container ui
  m_ui.setupUi(this);

  QList<int> sizes;
  sizes.push_back(700);
  sizes.push_back(100);
  m_ui.splitter_img_horiz->setSizes(sizes);
  m_ui.horizontalScrollBar_img_stack->setEnabled(false);

  setupConnections();

  // presenter that knows how to handle a IImageCoRView should take care
  // of all the logic. Note the view needs to now the concrete presenter here
  m_presenter.reset(new ImageCoRPresenter(this));

  // it will know what compute resources and tools we have available:
  // This view doesn't even know the names of compute resources, etc.
  m_presenter->notify(ImageCoRPresenter::Init);
}

void ImageCoRViewQtGUI::setupConnections() {

  // 'browse' buttons
  connect(m_ui.pushButton_browse_img, SIGNAL(released()), this,
          SLOT(browseImgClicked()));
}

void ImageCoRViewQtGUI::initParams(ImageStackPreParams &params) {
  m_params = params;
}

ImageStackPreParams ImageCoRViewQtGUI::userSelection() const {
  return m_params;
}

void ImageCoRViewQtGUI::browseImgClicked() {
  m_presenter->notify(IImageCoRPresenter::BrowseImgOrStack);
  // get path
  QString fitsStr = QString("Supported formats: FITS, TIFF and PNG "
                            "(*.fits *.fit *.tiff *.tif *.png);;"
                            "FITS, Flexible Image Transport System images "
                            "(*.fits *.fit);;"
                            "TIFF, Tagged Image File Format "
                            "(*.tif *.tiff);;"
                            "PNG, Portable Network Graphics "
                            "(*.png);;"
                            "Other extensions/all files (*.*)");
  QString prevPath =
      MantidQt::API::AlgorithmInputHistory::Instance().getPreviousDirectory();
  QString path(QFileDialog::getExistingDirectory(
      this, tr("Open stack of images"), prevPath, QFileDialog::ShowDirsOnly));
  if (!path.isEmpty()) {
    MantidQt::API::AlgorithmInputHistory::Instance().setPreviousDirectory(path);
  } else {
    return;
  }

  m_stackPath = path.toStdString();
  m_presenter->notify(IImageCoRPresenter::NewImgOrStack);
}

void ImageCoRViewQtGUI::showStack(const std::string & /*path*/) {}

void ImageCoRViewQtGUI::showStack(const Mantid::API::WorkspaceGroup_sptr &wsg) {
  MatrixWorkspace_sptr ws =
      boost::dynamic_pointer_cast<MatrixWorkspace>(wsg->getItem(0));
  if (!ws)
    return;

  const size_t MAXDIM = 2048 * 16;
  size_t width;
  try {
    width = boost::lexical_cast<size_t>(ws->run().getLogData("Axis1")->value());
    // TODO: add a settings option for this (like max mem allocation for
    // images)?
    if (width >= MAXDIM)
      width = MAXDIM;
  } catch (std::exception &e) {
    QMessageBox::critical(this, "Cannot load image",
                          "There was a problem while trying to "
                          "find the width of the image: " +
                              QString::fromStdString(e.what()));
    return;
  }

  size_t height;
  try {
    height =
        boost::lexical_cast<size_t>(ws->run().getLogData("Axis2")->value());
    if (height >= MAXDIM)
      height = MAXDIM;
  } catch (std::exception &e) {
    QMessageBox::critical(this, "Cannot load image",
                          "There was a problem while trying to "
                          "find the height of the image: " +
                              QString::fromStdString(e.what()));
    return;
  }

  std::string name;
  try {
    name = ws->run().getLogData("run_title")->value();
  } catch (std::exception &e) {
    QMessageBox::warning(this, "Cannot load image information",
                         "There was a problem while "
                         " trying to find the name of the image: " +
                             QString::fromStdString(e.what()));
  }

  // images are loaded as 1 histogram == 1 pixel (1 bin per histogram):
  if (height != ws->getNumberHistograms() || width != ws->blocksize()) {
    QMessageBox::critical(
        this, "Image dimensions do not match in the input image workspace",
        "Could not load the expected "
        "number of rows and columns.");
    return;
  }
  // find min and max to scale pixel values
  double min = std::numeric_limits<double>::max(),
         max = std::numeric_limits<double>::min();
  for (size_t i = 0; i < ws->getNumberHistograms(); ++i) {
    for (size_t j = 0; j < ws->blocksize(); ++j) {
      const double &v = ws->readY(i)[j];
      if (v < min)
        min = v;
      if (v > max)
        max = v;
    }
  }
  if (min >= max) {
    QMessageBox::warning(
        this, "Empty image!",
        "The image could be loaded but it contains "
        "effectively no information, all pixels have the same value.");
    // black picture
    QPixmap pix(static_cast<int>(width), static_cast<int>(height));
    pix.fill(QColor(0, 0, 0));
    m_ui.label_img->setPixmap(pix);
    m_ui.label_img->show();
    return;
  }

  // load / transfer image into a QImage
  QImage rawImg(QSize(static_cast<int>(width), static_cast<int>(height)),
                QImage::Format_RGB32);
  const double max_min = max - min;
  const double scaleFactor = 255.0 / max_min;
  for (size_t yi = 0; yi < width; ++yi) {
    for (size_t xi = 0; xi < width; ++xi) {
      const double &v = ws->readY(yi)[xi];
      // color the range min-max in gray scale. To apply different color
      // maps you'd need to use rawImg.setColorTable() or similar.
      const int scaled = static_cast<int>(scaleFactor * (v - min));
      QRgb vRgb = qRgb(scaled, scaled, scaled);
      rawImg.setPixel(static_cast<int>(xi), static_cast<int>(yi), vRgb);
    }
  }

  // paint and show image
  QPainter painter;
  QPixmap pix(static_cast<int>(width), static_cast<int>(height));
  painter.begin(&pix);
  painter.drawImage(0, 0, rawImg);
  painter.end();
  m_ui.label_img->setPixmap(pix);
  m_ui.label_img->show();

  m_ui.label_img_name->setText(QString::fromStdString(name));
}

void ImageCoRViewQtGUI::readSettings() {
  QSettings qs;
  qs.beginGroup(QString::fromStdString(m_settingsGroup));
  restoreGeometry(qs.value("interface-win-geometry").toByteArray());
  qs.endGroup();
}

void ImageCoRViewQtGUI::userWarning(const std::string &err,
                                    const std::string &description) {
  QMessageBox::warning(this, QString::fromStdString(err),
                       QString::fromStdString(description), QMessageBox::Ok,
                       QMessageBox::Ok);
}

void ImageCoRViewQtGUI::userError(const std::string &err,
                                  const std::string &description) {
  QMessageBox::critical(this, QString::fromStdString(err),
                        QString::fromStdString(description), QMessageBox::Ok,
                        QMessageBox::Ok);
}

void ImageCoRViewQtGUI::saveSettings() const {
  QSettings qs;
  qs.beginGroup(QString::fromStdString(m_settingsGroup));

  qs.setValue("interface-win-geometry", saveGeometry());
  qs.endGroup();
}

void ImageCoRViewQtGUI::closeEvent(QCloseEvent *event) {
  m_presenter->notify(IImageCoRPresenter::ShutDown);
  event->accept();
}

} // namespace CustomInterfaces
} // namespace MantidQt
