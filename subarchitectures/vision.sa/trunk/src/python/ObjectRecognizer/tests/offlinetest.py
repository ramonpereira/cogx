# vim:fileencoding=utf-8 sw=4 ts=8 et:vim
import os, sys, time
import numpy as np
import opencv.cv as cv
import opencv.highgui as hg
import opencv.adaptors as cvada

import pymodulepaths
import ObjectRecognizer.mods.capture as captr
import ObjectRecognizer.main as main

class CViewer:
    def __init__(self):
        self.winCamera = "Camera"
        self.winMatch = "Matching view"
        self.viewIndex = 0

        # self.capture = captr.CLoopback1394Capture(device=2)
        # self.capture = captr.CCameraCapture(device=2, size=(1024, 768))
        # self.capture = captr.CCameraCapture(device=0, size=(1024, 768), framerate=7.5)
        self.capture = captr.CCameraCapture(device=301, size=(640, 480), framerate=7.5)
        # self.capture.setSize((640, 480))
        # self.capture.setFrameRate(15)
        # self.capture.setBrightness(0.6)

        self.currentFrame = None

    def recognize(self):
        img = np.array(cvada.Ipl2NumPy(self.currentFrame))
        self.viewIndex += 1
        hg.cvSaveImage("xdata/t%04d.jpg" % self.viewIndex, img)
        print "------------------------------ Test %04d" % self.viewIndex
        res = main.findMatchingObject(img)
        print res

    def loadModels(self):
        # main.Manager.addModel('TwEarlGrey', '/home/mmarko/Documents/doc/Devel/CogX/code/apps/xdata/models/TwEarlGrey')
        # mdir = '../apps/xdata/models'
        mdir = '/media/truecrypt1/Devel/CogX/DATA/xdata/models'
        main.Manager.addModel('CvetMetaTea', mdir)
        main.Manager.addModel('ShelcoreCube', mdir)
        main.Manager.addModel('SwGreenTea', mdir)
        main.Manager.addModel('TwEarlGrey', mdir)

    def interactive(self):
        hg.cvNamedWindow(self.winCamera, hg.CV_WINDOW_AUTOSIZE)
        self.capture.start()
        self.currentFrame = self.capture.grabFrame(copy=True)

        while 1:
            self.currentFrame = self.capture.grabFrame(copy=True)
            if self.currentFrame is None: continue
            canvas = captr.copyFrame(self.currentFrame)
            hg.cvShowImage(self.winCamera, canvas)

            k = hg.cvWaitKey(10)
            if k == '\x1b': break
            elif k == ' ': self.recognize()


    def captureImages(self):
        hg.cvNamedWindow(self.winCamera, hg.CV_WINDOW_AUTOSIZE)
        self.capture.start()
        self.currentFrame = self.capture.grabFrame(copy=True)

        while 1:
            self.currentFrame = self.capture.grabFrame(copy=True)
            canvas = captr.copyFrame(self.currentFrame)
            hg.cvShowImage(self.winCamera, canvas)
            k = hg.cvWaitKey(20)
            if k == '\x1b': break
            elif k == ' ':
                self.viewIndex += 1
                hg.cvSaveImage("xdata/t%04d.png" % self.viewIndex, canvas)

    def processImages(self):
        hg.cvNamedWindow(self.winCamera, hg.CV_WINDOW_AUTOSIZE)
        tdir = "20090707_clutter"
        tdir = "20090707_sgm"
        # tdir = "20090707_quick"
        for fn in sorted(os.listdir(tdir)):
            if not fn.endswith(".png"): continue
            img = hg.cvLoadImage("%s/%s" % (tdir, fn))
            self.currentFrame = img
            hg.cvShowImage(self.winCamera, img)
            k = hg.cvWaitKey(50)
            self.recognize()
            if k == 0: k = hg.cvWaitKey(10)
            if k == '\x1b': break

def mymain():
    App = CViewer()
    App.loadModels()
    # App.processImages()
    # App.captureImages()
    App.interactive()

if __name__ == "__main__": mymain()


