import sys
import matplotlib
matplotlib.use('Qt5Agg')
from PyQt6 import QtWidgets
from PyQt6.QtCore import (QCoreApplication, Qt, QFileInfo, pyqtSignal as Signal, pyqtSlot as Slot)
from PyQt6.QtGui import QAction, QIcon
from PyQt6.QtWidgets import (QComboBox, QFileDialog, QLabel, QLineEdit, QPushButton, QStatusBar,
                             QWidget, QApplication, QMainWindow, QToolBar, QSplitter, QVBoxLayout,
                             QHBoxLayout, QGridLayout, QMenu, QMenuBar, QProgressBar
                             )
from PyQt6.QtCore import QThread
from matplotlib import pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
from matplotlib.animation import FuncAnimation
from PIL import Image
import numpy as np
import pandas as pd
import cv2
import sounddevice as sd
import random as rand
import scipy as sp
from scipy.io import wavfile

import resources

# Global Variables
IMAGE_DIR = "images/"

# duration = int(len(self.song) / self.SAMPLE_RATE)

class PlayAudio(QThread):
    def __init__(self, audio, SR):
        super().__init__()
        self.audio = audio
        self.SR = SR

    def run(self):
        sd.play(self.audio, self.SR, loop = False)

    def stop(self):
        sd.stop()
        self.exit()

# Main Window class
class MainWindow(QMainWindow):
    music_signal = Signal(int)

    def __init__(self, parent = None):
        super().__init__()
        self.HandleVars()
        self.InitGUI()
        self.show()
    
    def HandleVars(self):
        self.is_music_playing = False
        self._img = None

    def ConvertStoHMS(self, seconds):
        min, sec = divmod(seconds, 60)
        hour, min = divmod(min, 60)
        if hour == 0:
            return '%02d:%02d' % (min, sec)
        elif min == 0:
            return '%02d' % (sec)

        return '%d:%02d:%02d' % (hour, min, sec)

    # Function for creating the image pane
    def CreateImagePane(self):
        self.InitCanvas()
        self.canvasLayout = QVBoxLayout()
        self.canvas.setLayout(self.canvasLayout)

    # Function for creating the drawer
    def CreateDrawer(self):

        # Drawer Widget
        self.drawer = QWidget()

        self.drawerLayout = QGridLayout()
        self.traverseLabel = QLabel("Traversing")
        self.traverseLabel.setToolTip("Method of moving through the image")

        # Travers Combo Box
        self.traverseComboBox = QComboBox()

        self.traverseComboBox.addItem("Left to Right")
        self.traverseComboBox.addItem("Right to Left")
        self.traverseComboBox.addItem("Top to Botton")
        self.traverseComboBox.addItem("Bottom to Top")
        self.traverseComboBox.addItem("Radial")

        # self.drawerLayout.addWidget(QLabel("Drawer"))
        self.drawer.setLayout(self.drawerLayout)

        # Map Label
        self.mapLabel = QLabel("Mapping")
        self.mapLabel.setToolTip("Method of mapping the feature of image to parameters of sound")

        # Map Combo Box
        self.mapComboBox = QComboBox()
        self.mapComboBox.addItem("Color to Frequency")

        # Sample Rate Combo Box
        self.sampleRateLabel = QLabel("Sample Rate")
        self.sampleRateComboBox = QComboBox()

        self.sampleRateComboBox.addItems(["22050 Hz", "44100 Hz", "48000 Hz"])
        
        # Note time duration box
        
        self.noteDurationLabel = QLabel("Note duration")
        self.noteDurationBox = QLineEdit()
        self.noteDurationBox.setPlaceholderText(str(0.1))

        # Scale Combo Box
        
        self.scaleLabel = QLabel("Scale")

        self.scaleComboBox = QComboBox()
        self.scaleComboBox.addItems(["AEOLIAN", "BLUES", "PHYRIGIAN", "CHROMATIC", "DORIAN", "HARMONIC_MINOR", "LYDIAN",
                                     "MAJOR", "MELODIC_MINOR", "MINOR", "MIXOLYDIAN", "NATURAL_MINOR", "PENTATONIC"])
            
        # Sonify Button

        self.sonifyButton = QPushButton(QIcon(":/icons/sonify.png"), " Sonify")
        self.sonifyButton.setToolTip("Sonify")
        self.sonifyButton.setDisabled(True)
        # self.sonifyButton.clicked.connect(self.Sonify)
        self.sonifyButton.clicked.connect(self.genOtherScale)

        # Play Button
        self.playButton = QPushButton(QIcon(":/icons/play.png"), " Play")
        self.playButton.setToolTip("Play")
        self.playButton.setEnabled(False)
        self.playButton.clicked.connect(self.Play)

        # Duration Label

        self.durationLabelText = QLabel("Duration: ")
        self.durationLabel = QLabel("")

        # Drawer Layout
        self.drawerLayout.addWidget(self.traverseLabel, 0, 0)
        self.drawerLayout.addWidget(self.traverseComboBox, 0, 1)
        self.drawerLayout.addWidget(self.mapLabel, 1, 0)
        self.drawerLayout.addWidget(self.mapComboBox, 1, 1)
        self.drawerLayout.addWidget(self.sampleRateLabel, 2, 0)
        self.drawerLayout.addWidget(self.sampleRateComboBox, 2, 1)
        self.drawerLayout.addWidget(self.noteDurationLabel, 3, 0)
        self.drawerLayout.addWidget(self.noteDurationBox, 3, 1)
        self.drawerLayout.addWidget(self.scaleLabel, 4, 0)
        self.drawerLayout.addWidget(self.scaleComboBox, 4, 1)

        self.drawerBtnsLayout = QHBoxLayout()
        self.drawerBtnsLayout.addWidget(self.sonifyButton)
        self.drawerBtnsLayout.addWidget(self.playButton)
        self.drawerLayout.addLayout(self.drawerBtnsLayout, 5, 0, 1, 2)

        self.drawerLayout.setRowStretch(10, 1)

        self.drawerLayout.addWidget(self.durationLabelText, 6, 0, Qt.AlignmentFlag.AlignHCenter)
        self.drawerLayout.addWidget(self.durationLabel, 6, 1, Qt.AlignmentFlag.AlignHCenter)

        # Drawer Max Width
        self.drawer.setMinimumWidth(self.drawer.minimumWidth())
        self.drawer.setMaximumWidth(300)
    
    def Download(self):
        file = QFileDialog().getSaveFileName()
        if file != "":
            wavfile.write(file[0], self.SAMPLE_RATE, self.song)
        else:
            self.Msg("Please select a file to save", 5)

    # Exit Function
    def Exit(self):
        # TODO: handle ask on save
        QCoreApplication.exit(0)

    # Function to handle opening files
    def FileOpen(self, fileName = None):
        if fileName is None:
            self.file = QFileDialog.getOpenFileName(self, "Open File",
                                                    filter = "Image Files (*.jpg *.jpeg *.png *.tiff *.hdf5 *.fits)")[0]
            if self.file == "":
                self.Msg("Please select an image", 10)
                return
        else:
            self.file = fileName
        
        self.Msg("Loading Image", 2)
        self.ReadImage()
        self.Msg("Image Loaded", 5)

    def FileSize(self):
        return str(QFileInfo(self.file).size())

    def Hue2freq(self, h,scale_freqs):
        thresholds = [26 , 52 , 78 , 104,  128 , 154 , 180]
        note = scale_freqs[0]
        if (h <= thresholds[0]):
            note = scale_freqs[0]
        elif (h > thresholds[0]) & (h <= thresholds[1]):
            note = scale_freqs[1]
        elif (h > thresholds[1]) & (h <= thresholds[2]):
            note = scale_freqs[2]
        elif (h > thresholds[2]) & (h <= thresholds[3]):
            note = scale_freqs[3]
        elif (h > thresholds[3]) & (h <= thresholds[4]):    
            note = scale_freqs[4]
        elif (h > thresholds[4]) & (h <= thresholds[5]):
            note = scale_freqs[5]
        elif (h > thresholds[5]) & (h <= thresholds[6]):
            note = scale_freqs[6]
        else:
            note = scale_freqs[0]
        return note

    # Initialisation function for the GUI
    def InitGUI(self):
        self.mainLayout = QVBoxLayout()
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self.mainWidget = QWidget()
        self.mainWidget.setLayout(self.mainLayout)

        self.InitMenubar()
        self.InitToolbar()
        self.InitMainWidget()
        self.InitStatusBar()

        self.setCentralWidget(self.mainWidget)

    # Initialisation function for the canvas
    def InitCanvas(self):
        self.figure, self.ax = plt.subplots(2, 1, figsize = (16, 9), gridspec_kw={'height_ratios': [5, 1]})
        self.canvas = FigureCanvasQTAgg(self.figure)

        # self.ax[0].set_axis_off()
        self.ax[1].set_axis_off()

    # Initialisation function for the menubar
    def InitMenubar(self):
        self.menubar = QMenuBar()
        self.fileMenu = QMenu("&File")
        self.editMenu = QMenu("&Edit")
        self.viewMenu = QMenu("&View")
        self.helpMenu = QMenu("&Help")

        self.menubar.addMenu(self.fileMenu)
        self.menubar.addMenu(self.editMenu)
        self.menubar.addMenu(self.viewMenu)
        self.menubar.addMenu(self.helpMenu)

        # File Menu Buttons

        self.action_file_open = QAction(QIcon(":/icons/open.png"), "Open", self)
        self.action_file_open.setShortcut("Ctrl+O")

        self.action_file_exit = QAction(QIcon(":/icons/exit.png"), "Exit", self)

        self.action_file_open.triggered.connect(lambda: self.FileOpen(None))
        self.action_file_exit.triggered.connect(self.Exit)

        self.fileMenu.addAction(self.action_file_open)
        self.fileMenu.addAction(self.action_file_exit)

        # Edit Menu Buttons

        self.action_edit_prefs = QAction(QIcon(":icons/prefs.png"), "Preferences", self)

        self.editMenu.addAction(self.action_edit_prefs)

        # View Menu Buttons

        self.action_view_overlay = QMenu("Overlay", self)
        self.action_view_overlay.setIcon(QIcon(":/icons/overlay.png"))
        
        self.action_view_overlay_line = QAction("Line", self)
        self.action_view_overlay_line.setChecked(True)
        self.action_view_overlay_line.setCheckable(True)

        self.action_view_overlay_sound_graph = QAction("Sound Graph", self)
        self.action_view_overlay_sound_graph.setChecked(True)
        self.action_view_overlay_sound_graph.setCheckable(True)

        self.action_view_overlay.addAction(self.action_view_overlay_line)
        self.action_view_overlay.addAction(self.action_view_overlay_sound_graph)
    
        self.viewMenu.addMenu(self.action_view_overlay)

        # Help Menu
        
        self.action_help_about_sonify = QAction("About Sonify", self)

        self.helpMenu.addAction(self.action_help_about_sonify)

        self.setMenuBar(self.menubar)

    def InitToolbar(self):
        self.toolbar = QToolBar()

        self.mainLayout.addWidget(self.toolbar)

        self.toolbar_open = QAction(QIcon(":/icons/open.png"), "Open", self)
        self.toolbar_open.triggered.connect(lambda: self.FileOpen(None))

        self.toolbar_selection = QAction(QIcon(":/icons/select.png"), "Selection", self)
        # self.toolbar_selection.triggered.connect(self.Selection)

        self.toolbar_download = QAction(QIcon(":/icons/save.png"), "Download", self)
        self.toolbar_download.triggered.connect(self.Download)
        self.toolbar_download.setDisabled(True)


        self.toolbar.addAction(self.toolbar_open)
        self.toolbar.addAction(self.toolbar_selection)
        self.toolbar.addAction(self.toolbar_download)


    # Initialisation function for the statusbar
    def InitStatusBar(self):
        self.statusbar = QStatusBar()
        self.statusbar_layout = QHBoxLayout()
        self.setStatusBar(self.statusbar)
        self.mainLayout.addWidget(self.statusbar)

        self.statusbar.setMaximumHeight(20)

        self.status_fileName = QLabel("File Name")
        self.status_fileSize = QLabel("File Size")
        self.status_fileDim = QLabel("File Dimension")
        self.statusbar.addPermanentWidget(self.status_fileName, 0)
        self.statusbar.addPermanentWidget(self.status_fileSize, 0)
        self.statusbar.addPermanentWidget(self.status_fileDim, 0)

        self.Msg("Hello World....", 5)

    # Initialisation function for the main widget
    def InitMainWidget(self):
        self.mainSplitter = QSplitter(orientation = Qt.Orientation.Horizontal)
        # self.mainSplitter.setSizePolicy(QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.MinimumExpanding, QtWidgets.QSizePolicy.MinimumExpanding))

        self.CreateImagePane()
        self.CreateDrawer()

        self.mainSplitter.addWidget(self.drawer)
        self.mainSplitter.addWidget(self.canvas)
        self.mainLayout.addWidget(self.mainSplitter)

    #Function that handles the play button click
    def Play(self):
        if self.is_music_playing:
            self.is_music_playing = False
            self.worker.stop()

            self.playButton.setIcon(QIcon(":/icons/play.png"))
            self.playButton.setText(" Play")
            return

        self.is_music_playing = True
        self.playButton.setIcon(QIcon(":/icons/stop.png"))
        self.playButton.setText(" Stop")
        self.worker = PlayAudio(self.song, self.SAMPLE_RATE)
        self.worker.start()

        # self.MoveHorizLine()
            

        # duration = int(len(self.song) / self.SAMPLE_RATE)
        # refreshPeriod = 100
        # f = np.arange(0, self.img_width, 5)
        # ani = FuncAnimation(self.figure, self.animateHorizLine, frames = f,
        #                     interval = 1/duration * self.img_width * self.T, blit = True, repeat = False)
        # # ani = FuncAnimation(self.figure, self.animateSinWave, init_func= self.INIT, frames = self.t, interval = 50, blit = True)
        # self.UpdateCanvas()

    def INIT(self):
        self.wave, = self.ax[1].plot([], [], '.b', lw = 2)
        self.wave.set_data([], [])
        return self.wave,

    def Stop(self):
        self.worker.stop()
        self.is_music_playing = False

        #self.worker.progress.connect(self.updateMusic)
        #self.worker.finished.connect(self.audioFinished)

    # Function for reading the image
    def ReadImage(self):
        self.img = Image.open(self.file)
        self.imghsv = cv2.cvtColor(np.array(self.img), cv2.COLOR_BGR2HSV)
        # self.ax.imshow(self.imghsv)
        if self._img is None:
            self._img = self.ax[0].imshow(self.img)
        else:
            self._img.set_data(self.img)
        self.UpdateCanvas()
        # Update the statusbar info
        self.status_fileName.setText(self.file)
        self.status_fileSize.setText(self.FileSize())
        self.status_fileDim.setText(str(self.imghsv.shape))
        self.sonifyButton.setEnabled(True)

    # Function that handles the sonification
    def Sonify(self):
        self.progressbar_sonify = QProgressBar()

        self.img_height, self.img_width, self.img_nlayers = self.imghsv.shape
        self.hues = []
        self.MapHorizontal_LR(5, 5)

        self.hues = pd.DataFrame(self.hues, columns= ["hues"])

        #Define frequencies that make up A-Harmonic Minor Scale
        scale_freqs = [220.00, 246.94 ,261.63, 293.66, 329.63, 349.23, 415.30] 
        self.hues['notes'] = self.hues.apply(lambda row : self.Hue2freq(row['hues'],scale_freqs), axis = 1)

        self.frequencies = self.hues['notes'].to_numpy()

        self.song = np.array([])
        self.SAMPLE_RATE = int(self.sampleRateComboBox.currentText().split("Hz")[0])
        self.T = 0.1
        self.t = np.linspace(0, self.T, int(self.T * self.SAMPLE_RATE), endpoint = False)

        amp = 0.5
        
        self.npixels = len(self.frequencies)

        self.statusbar.addPermanentWidget(self.progressbar_sonify)

        # for i in range(self.npixels):
        #     self.progressbar_sonify.setValue(int(i / self.npixels * 100))
        #     note = amp * np.sin(2 * np.pi * self.frequencies[i] * self.t)
        #     self.song = np.concatenate([self.song, note])

        octaves = np.array([0.5, 1, 2, 3, 4, 5, 6, 7, 8])

        for i in range(self.npixels):
            self.progressbar_sonify.setValue(int(i / self.npixels * 100))
            octave = rand.choice(octaves)
            note = amp * np.sin(2 * np.pi * octave * self.frequencies[i] * self.t)
            self.song = np.concatenate([self.song, note])


        duration = int(len(self.song) / self.SAMPLE_RATE)

        self.statusbar.removeWidget(self.progressbar_sonify)

        self.playButton.setEnabled(True)

    # Helper function for showing message in the statusbar
    def Msg(self, msg = None, t = 1):
        self.statusbar.showMessage(msg, t * 1000)
    
    # TODO: Overlay line on the image while playing audio
    # def MoveHorizLine(self):
    #     self.vl = self.ax[0].axvline(0, ls = '-', color = 'r', lw = 1, zorder = 10)
    #     self.UpdateCanvas()
    #
    # def animateHorizLine(self, i):
    #     self.vl.set_xdata([i, i])
    #     return self.vl,

    # TODO: Overlay sin wave on the image while playing audio
    # def animateSinWave(self, vl, i):
    #     self.wave.set_data(self.t, y)
    #     return self.wave,

    # IMAGE MAPPINGS:

    # Horizontal Left to Right Mapping
    def Traverse_Horizontal_LR(self, skipw, skiph):
        for j in range(0, self.img_width, skipw):
            for i in range(0, self.img_height, skiph):
                hue = self.imghsv[i][j][0]
                self.hues.append(hue)

    def Traverse_Horizontal_RL(self, skipw, skiph):
        for j in range(self.img_width, 0, -skipw):
            for i in range(0, self.img_height, skiph):
                hue = self.imghsv[i][j][0]
                self.hues.append(hue)

    def Traverse_Vertical_BT(self, skipw, skiph):
        pass

    def Traverse_Vertical_TB(self, skipw, skiph):
        pass

    def Traverse_Stack_BU(self, skipw, skiph):
        pass

    def Traverse_Stack_TD(self, skipw, skiph):
        pass

    # Piano notes function
    def Get_piano_notes(self):   
        # White keys are in Uppercase and black keys (sharps) are in lowercase
        octave = ['C', 'c', 'D', 'd', 'E', 'F', 'f', 'G', 'g', 'A', 'a', 'B'] 
        base_freq = 440 #Frequency of Note A4
        keys = np.array([x+str(y) for y in range(0,9) for x in octave])
        # Trim to standard 88 keys
        start = np.where(keys == 'A0')[0][0]
        end = np.where(keys == 'C8')[0][0]
        keys = keys[start:end+1]

        note_freqs = dict(zip(keys, [2**((n+1-49)/12)*base_freq for n in range(len(keys))]))
        note_freqs[''] = 0.0 # stop

        return note_freqs

    # Helper function to update canvas
    def UpdateCanvas(self):
        self.canvas.draw()

    def genOtherScale(self):
        note_freqs = self.Get_piano_notes()
        scale_intervals = ['A','a','B','C','c','D','d','E','F','f','G','g']
        
        key = 'a'
        #Find index of desired key
        index = scale_intervals.index(key)

        #Redefine scale interval so that scale intervals begins with whichKey
        new_scale = scale_intervals[index:12] + scale_intervals[:index]
    
        whichScale = self.scaleComboBox.currentText()

        if whichScale == 'AEOLIAN':
            scale = [0, 2, 3, 5, 7, 8, 10]
        elif whichScale == 'BLUES':
            scale = [0, 2, 3, 4, 5, 7, 9, 10, 11]
        elif whichScale == 'PHYRIGIAN':
            scale = [0, 1, 3, 5, 7, 8, 10]
        elif whichScale == 'CHROMATIC':
            scale = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
        elif whichScale == 'DORIAN':
            scale = [0, 2, 3, 5, 7, 9, 10]
        elif whichScale == 'HARMONIC_MINOR':
            scale = [0, 2, 3, 5, 7, 8, 11]
        elif whichScale == 'LYDIAN':
            scale = [0, 2, 4, 6, 7, 9, 11]
        elif whichScale == 'MAJOR':
            scale = [0, 2, 4, 5, 7, 9, 11]
        elif whichScale == 'MELODIC_MINOR':
            scale = [0, 2, 3, 5, 7, 8, 9, 10, 11]
        elif whichScale == 'MINOR':    
            scale = [0, 2, 3, 5, 7, 8, 10]
        elif whichScale == 'MIXOLYDIAN':     
            scale = [0, 2, 4, 5, 7, 9, 10]
        elif whichScale == 'NATURAL_MINOR':   
            scale = [0, 2, 3, 5, 7, 8, 10]
        elif whichScale == 'PENTATONIC':    
            scale = [0, 2, 4, 7, 9]

        harmony_select = {'U0' : 1,
                          'ST' : 16/15,
                          'M2' : 9/8,
                          'm3' : 6/5,
                          'M3' : 5/4,
                          'P4' : 4/3,
                          'DT' : 45/32,
                          'P5' : 3/2,
                          'm6': 8/5,
                          'M6': 5/3,
                          'm7': 9/5,
                          'M7': 15/8,
                          'O8': 2
                          }

        #Get length of scale (i.e., how many notes in scale)
        nNotes = len(scale)

        #Initialize arrays
        freqs = []
        #harmony = []
        #harmony_val = harmony_select[makeHarmony]
        for i in range(nNotes):
            note = new_scale[scale[i]] + str(3)
            freqToAdd = note_freqs[note]
            freqs.append(freqToAdd)
            #harmony.append(harmony_val*freqToAdd)

        self.hues = []
        self.img_height, self.img_width, self.img_nlayers = self.imghsv.shape

        # Handle traversing of image
    
        match self.traverseComboBox.currentText():
            case "Left to Right":
                self.Traverse_Horizontal_LR(5, 5)
            case "Right to Left":
                self.Traverse_Horizontal_RL(5, 5)
            case "Top to Bottom":
                self.Traverse_Vertical_TB(5, 5)
            case "Bottom to Top":
                self.Traverse_Vertical_BT(5, 5)

        self.hues = pd.DataFrame(self.hues, columns= ["hues"])
        self.hues['notes'] = self.hues.apply(lambda row : self.Hue2freq(row['hues'], freqs), axis = 1)

        self.frequencies = self.hues['notes'].to_numpy()

        self.song = np.array([]) 
        self.SAMPLE_RATE = int(self.sampleRateComboBox.currentText().split("Hz")[0]) # sample rate
        d = self.noteDurationBox.text()
        if d != "":
            self.T = float(d)
        else:
            self.T = 0.1

        self.t = np.linspace(0, self.T, int(self.T*self.SAMPLE_RATE), endpoint=False) # time variable
        #nPixels = int(len(frequencies))#All pixels in image
        nPixels = int(len(self.frequencies))
        
        self.progressbar_sonify = QProgressBar()
        self.statusbar.addPermanentWidget(self.progressbar_sonify)

        octaves = np.array([0.5, 1, 2, 3, 4, 5])

        for i in range(nPixels):
            self.progressbar_sonify.setValue(int(i / nPixels * 100))
            octave = rand.choice(octaves)
            note = 0.5 * np.sin(2 * np.pi * octave * self.frequencies[i] * self.t)
            self.song = np.concatenate([self.song, note])

        duration = int(len(self.song) / self.SAMPLE_RATE)

        self.durationLabel.setText(str(self.ConvertStoHMS(duration)))

        self.statusbar.removeWidget(self.progressbar_sonify)
        self.playButton.setEnabled(True)
        self.toolbar_download.setEnabled(True)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = MainWindow()
    app.exec()
