# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2017 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#    This file is part of the mantid workbench.
#
#

from __future__ import (absolute_import, division, print_function, unicode_literals)

from matplotlib.backends.backend_qt5 import NavigationToolbar2QT
from qtpy import QtCore, QtGui, QtPrintSupport, QtWidgets

from mantidqt.icons import get_icon


class WorkbenchNavigationToolbar(NavigationToolbar2QT):

    home_clicked = QtCore.Signal()
    sig_display_all_triggered = QtCore.Signal()
    sig_grid_toggle_triggered = QtCore.Signal()
    sig_active_triggered = QtCore.Signal()
    sig_hold_triggered = QtCore.Signal()
    sig_toggle_fit_triggered = QtCore.Signal()
    sig_plot_options_triggered = QtCore.Signal()
    sig_generate_plot_script_file_triggered = QtCore.Signal()
    sig_generate_plot_script_clipboard_triggered = QtCore.Signal()

    toolitems = (
        ('Home', 'Reset original view', 'mdi.home', 'home', None),
        ('Back', 'Back to previous view', 'mdi.arrow-left', 'back', None),
        ('Forward', 'Forward to next view', 'mdi.arrow-right', 'forward', None),
        (None, None, None, None, None),
        ('Pan', 'Pan axes with left mouse, zoom with right', 'mdi.arrow-all', 'pan', False),
        ('Zoom', 'Zoom to rectangle', 'mdi.magnify', 'zoom', False),
        ('Display All', 'Set zoom to display all', 'mdi.arrow-expand-all', 'display_all_clicked',
         None),
        (None, None, None, None, None),
        ('Grid', 'Toggle grid on/off', 'mdi.grid', 'toggle_grid', False),
        ('Save', 'Save the figure', 'mdi.content-save', 'save_figure', None),
        ('Print', 'Print the figure', 'mdi.printer', 'print_figure', None),
        (None, None, None, None, None),
        ('Customize', 'Configure plot options', 'mdi.settings', 'launch_plot_options', None),
        (None, None, None, None, None),
        ('Create Script', 'Generate a script that will recreate the current figure',
         'mdi.script-text-outline', 'generate_plot_script', None),
        (None, None, None, None, None),
        ('Fit', 'Toggle fit browser on/off', None, 'toggle_fit', False),
    )

    def _init_toolbar(self):
        for text, tooltip_text, mdi_icon, callback, checked in self.toolitems:
            if text is None:
                self.addSeparator()
            else:
                if text == 'Create Script':
                    # Add a QMenu under the QToolButton for "Create Script"
                    a = self.addAction(get_icon(mdi_icon), text, lambda: None)
                    # This is the only way I could find of getting hold of the QToolButton object
                    button = [child for child in self.children()
                              if isinstance(child, QtWidgets.QToolButton)][-1]
                    menu = QtWidgets.QMenu("Menu", parent=button)
                    menu.addAction("Script to file",
                                   self.sig_generate_plot_script_file_triggered.emit)
                    menu.addAction("Script to clipboard",
                                   self.sig_generate_plot_script_clipboard_triggered.emit)
                    button.setMenu(menu)
                    button.setPopupMode(QtWidgets.QToolButton.InstantPopup)
                elif mdi_icon:
                    a = self.addAction(get_icon(mdi_icon), text, getattr(self, callback))
                else:
                    a = self.addAction(text, getattr(self, callback))
                self._actions[callback] = a
                if checked is not None:
                    a.setCheckable(True)
                    a.setChecked(checked)
                if tooltip_text is not None:
                    a.setToolTip(tooltip_text)
                if text == 'Home':
                    a.triggered.connect(self.on_home_clicked)

        self.buttons = {}
        # Add the x,y location widget at the right side of the toolbar
        # The stretch factor is 1 which means any resizing of the toolbar
        # will resize this label instead of the buttons.
        if self.coordinates:
            self.locLabel = QtWidgets.QLabel("", self)
            self.locLabel.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignTop)
            self.locLabel.setSizePolicy(
                QtWidgets.QSizePolicy(QtWidgets.Expanding, QtWidgets.QSizePolicy.Ignored))
            labelAction = self.addWidget(self.locLabel)
            labelAction.setVisible(True)

        # reference holder for subplots_adjust window
        self.adj_window = None

        # Adjust icon size or they are too small in PyQt5 by default
        dpi_ratio = QtWidgets.QApplication.instance().desktop().physicalDpiX() / 100
        self.setIconSize(QtCore.QSize(24 * dpi_ratio, 24 * dpi_ratio))

    def display_all_clicked(self):
        self.sig_display_all_triggered.emit()
        self.push_current()

    def launch_plot_options(self):
        self.sig_plot_options_triggered.emit()

    def toggle_grid(self):
        self.sig_grid_toggle_triggered.emit()

    def toggle_fit(self):
        fit_action = self._actions['toggle_fit']
        if fit_action.isChecked():
            if self._actions['zoom'].isChecked():
                self.zoom()
            if self._actions['pan'].isChecked():
                self.pan()
        self.sig_toggle_fit_triggered.emit()

    def trigger_fit_toggle_action(self):
        self._actions['toggle_fit'].trigger()

    def print_figure(self):
        printer = QtPrintSupport.QPrinter(QtPrintSupport.QPrinter.HighResolution)
        printer.setOrientation(QtPrintSupport.QPrinter.Landscape)
        print_dlg = QtPrintSupport.QPrintDialog(printer)
        if print_dlg.exec_() == QtWidgets.QDialog.Accepted:
            painter = QtGui.QPainter(printer)
            page_size = printer.pageRect()
            pixmap = self.canvas.grab().scaled(page_size.width(), page_size.height(),
                                               QtCore.Qt.KeepAspectRatio)
            painter.drawPixmap(0, 0, pixmap)
            painter.end()

    def contextMenuEvent(self, event):
        pass

    def on_home_clicked(self, _):
        self.home_clicked.emit()


class ToolbarStateManager(object):
    """
    An object that lets users check and manipulate the state of the toolbar
    whilst hiding any implementation details.
    """
    def __init__(self, toolbar):
        self._toolbar = toolbar

    def is_zoom_active(self):
        """
        Check if the Zoom button is checked
        """
        return self._toolbar._actions['zoom'].isChecked()

    def is_pan_active(self):
        """
        Check if the Pan button is checked
        """
        return self._toolbar._actions['pan'].isChecked()

    def is_tool_active(self):
        """
        Check if any of the zoom buttons are checked
        """
        return self.is_pan_active() or self.is_zoom_active()

    def toggle_fit_button_checked(self):
        fit_action = self._toolbar._actions['toggle_fit']
        if fit_action.isChecked():
            fit_action.setChecked(False)
        else:
            fit_action.setChecked(True)

    def home_button_connect(self, slot):
        self._toolbar.home_clicked.connect(slot)
