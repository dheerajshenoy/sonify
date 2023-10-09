from PyQt6.QtWidgets import QGridLayout, QProgressBar, QWidget, QVBoxLayout, QHBoxLayout, QLabel
from PyQt6.QtCore import QTimer

class StatusBar(QWidget):
    def __init__(self, parent = None, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.layout = QGridLayout(parent)
        self.setLayout(self.layout)
        self.setMaximumHeight(20)
    
        # status bar widgets
        self.msgLabel = QLabel()
        self.sb_fileName = QLabel()
        self.sb_fileSize = QLabel()
        self.sb_fileDim = QLabel()
        self.sb_progressbar = QProgressBar()

        self.layout.addWidget(self.msgLabel, 0, 0)
        self.layout.addWidget(self.msgLabel, 0, 1)
        self.layout.addWidget(self.sb_fileName, 0, 2)
        self.layout.addWidget(self.sb_fileSize, 0, 3)
        self.layout.addWidget(self.sb_fileDim, 0, 4)

        self.layout.setColumnStretch(5, 1)

    def setMsg(self, msg, time = 2) -> None:
        self.msgLabel.setText(msg)
        QTimer.singleShot(time * 1000, lambda: self.msgLabel.clear())

    def setFileName(self, fileName) -> None:
        self.sb_fileName.setText(fileName)

    def setFileSize(self, fileSize) -> None:
        self.sb_fileSize.setText(fileSize)

    def setFileDim(self, fileDim) -> None:
        self.sb_fileDim.setText(fileDim)

    def showProgressBar(self) -> None:
        self.sb_progressbar.setHidden(False)

    def hideProgressBar(self) -> None:
        self.sb_progressbar.setHidden(True)

    def ProgressBar(self) -> QProgressBar:
        return self.sb_progressbar
