#!/usr/bin/python
# coding=UTF-8
import pygst
pygst.require("0.10")
import gst
import pygtk
import gtk

#CODIGO MODIFICADO DE http://www.jonobacon.org/?p=750 y http://www.25fps48khz.net/node/16, y http://www.jejik.com/articles/2007/01/streaming_audio_over_tcp_with_python-gstreamer/

class Main:
    def __init__(self):
        #Creación de los objetos de la GUI
        self.window = gtk.Window()
        self.vbox = gtk.VBox()
        self.da = gtk.DrawingArea()
        self.bb = gtk.HButtonBox()
        self.da.set_size_request(300,150)
        self.playButton = gtk.Button(stock='gtk-media-play')
        self.playButton.connect("clicked", self.OnPlay)
        self.stopButton = gtk.Button(stock='gtk-media-stop')
        self.stopButton.connect("clicked", self.OnStop)
        self.decrButton = gtk.Button(label="+Grave")  #       
        self.decrButton.connect("clicked", self.OnDecrease)
        self.incrButton = gtk.Button(label="+Agudo")  #
        self.incrButton.connect("clicked", self.OnIncrease)
        self.quitButton = gtk.Button(stock='gtk-quit')
        self.quitButton.connect("clicked", self.OnQuit)
        self.vbox.pack_start(self.da)
        self.bb.add(self.playButton)
        self.bb.add(self.stopButton)
        self.bb.add(self.decrButton) #
        self.bb.add(self.incrButton) #
        self.bb.add(self.quitButton)
        self.vbox.pack_start(self.bb)
        self.window.add(self.vbox)
        # Creación del pipeline de GStreamer
        self.pipeline = gst.Pipeline("mypipeline")
        
        # Obtención de fuente de vídeo
        self.videotestsrc = gst.element_factory_make("videotestsrc", "video")
        # Inclusión en el pipeline
        self.pipeline.add(self.videotestsrc)
        # Obtención del destino del vídeo
        self.sink = gst.element_factory_make("xvimagesink", "sink")
        # Inclusión en el pipeline
        self.pipeline.add(self.sink)
        
        # Enlazado del orígen de vídeo con el destino - xv
        self.videotestsrc.link(self.sink)
        
        #AUDIO
        self.audiopipeline = gst.Pipeline("audiopipeline")

        self.audiotestsrc = gst.element_factory_make("audiotestsrc", "audio")
        self.audiofreq = 200
        self.audiotestsrc.set_property("freq", self.audiofreq)
        self.audiopipeline.add(self.audiotestsrc)

        self.audiosink = gst.element_factory_make("alsasink", "sink")
        self.audiopipeline.add(self.audiosink)
        self.audiotestsrc.link(self.audiosink)
        
        self.window.show_all()
    
    def OnPlay(self, widget):
        print "play"
        # Hacemos que la salida de video se muestre en nuestro DrawingArea
        self.sink.set_xwindow_id(self.da.window.xid)
        self.pipeline.set_state(gst.STATE_PLAYING)
        self.audiopipeline.set_state(gst.STATE_PLAYING)
    
    def OnStop(self, widget):
        print "stop"
        self.pipeline.set_state(gst.STATE_READY)
        self.audiopipeline.set_state(gst.STATE_READY)
    
    def OnIncrease (self, widget):
        print "increase"
        self.audiofreq += 100
        self.audiotestsrc.set_property("freq", self.audiofreq)

    def OnDecrease (self, widget):
        print "decrease"
        self.audiofreq -= 100
        if (self.audiofreq<0):
            self.audiofreq = 0;
        self.audiotestsrc.set_property("freq", self.audiofreq)
            
    def OnQuit(self, widget):
        gtk.main_quit()
    
start=Main()
gtk.main() 
