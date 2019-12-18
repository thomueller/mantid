# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
"""
List of workspaces for Plotting
"""
from qtpy.QtGui import QStandardItemModel, QStandardItem


class DNSPlotListModel(QStandardItemModel):
    """
    QT Model to store DNS mantid workspace names for plotting
    """

    def get_checked_item_names(self):
        return [item.text() for item in self.get_items() if item.checkState()]

    def get_items(self):
        return [self.item(i) for i in range(self.rowCount())]

    def get_names(self):
        return [self.item(i).text() for i in range(self.rowCount())]

    def get_texts(self):
        return [item.text() for item in self.get_items()]

    def uncheck_items(self):
        for item in self.get_items():
            item.setCheckState(0)

    def set_items(self, items):
        self.clear()
        for dataset in items:
            item = QStandardItem(dataset)
            item.setCheckable(True)
            item.setEditable(False)
            item.setTristate(False)
            self.appendRow(item)

    def get_checked_item_numbers(self):
        numbers = []
        for i in range(self.rowCount()):
            if self.item(i).checkState():
                numbers.append(i)
        return numbers

    def down(self):
        """
        checking the item below the the previously checked item
        taking care of borders
        """
        numbers = self.get_checked_item_numbers()
        if len(numbers) == 1:
            i = numbers[0]
        else:
            self.uncheck_items()
            i = -1
        if i < self.rowCount() - 1:
            if i >= 0:
                self.item(i).setCheckState(0)
            self.item(i + 1).setCheckState(2)

    def up(self):
        """
        checking the item above the previously checked item
        taking care of borders
        """
        numbers = self.get_checked_item_numbers()
        if len(numbers) == 1:
            i = numbers[0]
        else:
            self.uncheck_items()
            i = self.rowCount()
        if i > 0:
            if i <= self.rowCount() - 1:
                self.item(i).setCheckState(0)
            self.item(i - 1).setCheckState(2)

    def check_first(self):
        self.item(0).setCheckState(2)

    def check_seperated(self):
        """
        Cheing data speparated using sum rules
        """
        self.uncheck_items()
        items = self.get_items()
        for item in items:
            text = item.text()
            if (text.endswith('_magnetic')
                    or text.endswith('_spin_incoh')
                    or text.endswith('_nuclear_coh')):
                item.setCheckState(2)

    def check_raw(self):
        """
        Checking the processed but not seperated data
        """
        self.uncheck_items()
        items = self.get_items()
        for item in items:
            text = item.text()
            if text.endswith('_sf') or text.endswith('_nsf'):
                item.setCheckState(2)
