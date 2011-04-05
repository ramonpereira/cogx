
import os, sys
from PyQt4 import QtCore, QtGui

class ICustomEditorBase:
    # @param data see CTreeItem.data()
    def setEditData(self, data): pass

    # @return see CTreeItem.setData()
    def editData(self): pass

class CTextEditor(QtGui.QLineEdit, ICustomEditorBase):
    def __init__(self, parent = None):
        QtGui.QLineEdit.__init__(self, parent)
        self._type = QtCore.QVariant('')

    # @param data see CTreeItem.data()
    def setEditData(self, data):
        ldata = data.toList()
        self._type = ldata[0]
        prop = ldata[1].toPyObject()

        if prop.validation != None:
            if prop.validation == "int":
                self.setValidator(QtGui.QIntValidator(self))
            elif prop.validation == "double":
                self.setValidator(QtGui.QDoubleValidator(self))
            # XXX elif type(prop.validation) == funcref: create validator that calls funcref()

        v = prop.value if prop.value != None else ""
        if type(v) != type(""):
            v = "%s" % v

        self.setText(v)

    # @return see CTreeItem.setData()
    def editData(self):
        return QtCore.QVariant([self._type, self.text()])

class CStringItemEditor(QtGui.QComboBox, ICustomEditorBase):
    def __init__(self, parent = None):
        QtGui.QComboBox.__init__(self, parent)
        self._type = QtCore.QVariant('')
        self.items = []

    def setEditData(self, data):
        ldata = data.toList()
        self._type = ldata[0]
        prop = ldata[1].toPyObject()
        if prop.items != None: self.items = prop.items

        for it in self.items:
            self.addItem(it)

        v = prop.value if prop.value != None else ""
        i = self.findText(v)
        self.setCurrentIndex(i)

    # @return see CTreeItem.setData()
    def editData(self):
        return QtCore.QVariant([self._type, "%s" % self.currentText()])


# XXX Currently supports only files in CB with history; sometimes one would want
# to edit the filename instead of selecting it; history might not make sense in
# some cases; edit button may not be desired (eg. for binary files)
# This info should probably be carried by the property defined by the server
# (edit=true, history=false, editbutton=false).
# If the filename can be edited, then the formatter should not be used.
class CFilenameEditor(QtGui.QWidget, ICustomEditorBase):
    def __init__(self, parent = None):
        self._type = QtCore.QVariant('')
        self.filter = "All files (*)"
        self.formatter = lambda x: x

        QtGui.QWidget.__init__(self, parent)
        self.setAutoFillBackground(True)
        horz = QtGui.QHBoxLayout(self)
        horz.setContentsMargins(0, 0, -1, 0)

        self.e = QtGui.QLineEdit(self)
        #self.cb = QtGui.QComboBox(self)
        #self.cb.setEditable(True)
        #self.cb.setLineEdit(self.e)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(4)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.e.sizePolicy().hasHeightForWidth())
        self.e.setSizePolicy(sizePolicy)
        horz.addWidget(self.e)
        # Pass the focus to the editor on start. Also necessary for QToolButton-s to work.
        self.setFocusProxy(self.e)

        #TODO: Combo box instead of QLineEdit
        #self.connect(self.e, QtCore.SIGNAL("currentIndexChanged(int)"), self.onComboIndexChanged)

        #self.b = QtGui.QPushButton(self)
        self.b = QtGui.QToolButton(self)
        self.b.setText('...')
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        self.b.setSizePolicy(sizePolicy)
        horz.addWidget(self.b)
        self.connect(self.b, QtCore.SIGNAL("clicked(bool)"), self.onBrowseForFile)

        #self.be = QtGui.QPushButton(self)
        self.be = QtGui.QToolButton(self)
        self.be.setText('Edit')
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Minimum, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        self.be.setSizePolicy(sizePolicy)
        horz.addWidget(self.be)
        self.connect(self.be, QtCore.SIGNAL("clicked(bool)"), self.onEditFile)

    def onBrowseForFile(self):
        qfd = QtGui.QFileDialog
        fn = qfd.getOpenFileName(self, self.e.text(), "", self.filter)
        if fn != None and len(fn) > 1:
            #fn = self.makeConfigFileRelPath(fn)
            #self._ComboBox_AddMru(self.ui.playerConfigCmbx,
            #        self.makeConfigFileDisplay(fn), QtCore.QVariant(fn))
            self.e.setText(fn)

    def onEditFile(self):
        qfd = QtGui.QFileDialog
        fn = qfd.getOpenFileName(self, self.e.text(), "", self.filter)
        if fn != None and len(fn) > 1:
            # TODO: a call into castcontrol to edit(fn)
            pass

    def onComboIndexChanged(self, index):
        if index < 1: return
        #fn = self._playerConfig
        #self._ComboBox_SelectMru(self.ui.playerConfigCmbx, index,
        #        self.makeConfigFileDisplay(fn), QtCore.QVariant(fn))

    def setEditData(self, data):
        ldata = data.toList()
        self._type = ldata[0]
        prop = ldata[1].toPyObject()
        v = prop.value if prop.value != None else ""
        self.e.setText(v)
        self.filter = prop.filter
        if len(ldata) > 2:
            self.formatter = ldata[2].toPyObject()

    def editData(self):
        return QtCore.QVariant([self._type, self.e.text()])

