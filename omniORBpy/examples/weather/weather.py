#!/usr/bin/env python
#                           Package   : weather
# weather.py                Created on: 2000/01/06
#			    Author    : Duncan Grisby (dpg1)
#
#    Copyright (C) 2000 AT&T Laboratories Cambridge
#
#  This is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA.
#
# Description:
#   
#   Client to AT&T Labs Cambridge weather service

weatherIOR    = None
CORBAInterval = 1

import Tkinter
import sys, threading, time
import gauge

from omniORB import CORBA
import _GlobalIDL

immediateState  = None
cumulativeState = None

class CORBAThread(threading.Thread):
    def __init__(self, argv, gui):
        global weatherIOR, immediateState, cumulativeState

        self.gui = gui

        threading.Thread.__init__(self)
        self.setDaemon(1)

        self.orb = CORBA.ORB_init(argv, CORBA.ORB_ID)

        self.verbose = 0
        for arg in argv[1:]:
            if arg == "-v":
                self.verbose = 1
            else:
                weatherIOR = arg

        if weatherIOR is None:
            print "You must specify an IOR for the weather server."
            sys.exit(1)

        self.wio = self.orb.string_to_object(weatherIOR)

        print "Acquiring initial weather state..."
        immediateState  = self.wio.immediate()
        cumulativeState = self.wio.cumulative()
        print "...initial state acquired."

        self.gui.update()

    def run(self):
        global cumulativeState, immediateState

        print "CORBA thread started"

        while 1:
            time.sleep(CORBAInterval)
            if self.verbose: print "CORBA calls...",
            sys.stdout.flush()

            self.gui.startCORBA()
            before = time.time()
            immediateState  = self.wio.immediate()
            cumulativeState = self.wio.cumulative()
            after = time.time()
            self.gui.endCORBA()

            if self.verbose:
                print "done in", int((after-before) * 1000000), "microseconds"

            self.gui.update()
            self.gui.callTime(int((after-before)*1000))


class GUI:

    def __init__(self):

        self.root   = Tkinter.Tk()
        self.root.title("AT&T Rooftop Weather")
        self.canvas = Tkinter.Canvas(self.root, width=800, height=500,
                                     background="blue")
        self.canvas.pack()

        self.root.protocol("WM_DELETE_WINDOW", self.root.quit)

        # Make gauges
        self.windDir = gauge.Compass(self.canvas, 100, 100,
                                     title        = "Wind\ndirection",
                                     handcolour   = "green")

        self.wind    = gauge.Gauge(self.canvas, 250, 100,
                                   title        = "Wind speed",
                                   min          = 0,
                                   max          = 100,
                                   unitspertick = 5,
                                   bigticks     = 25,
                                   digitformat  = "%d knots",
                                   handcolour   = "orange")

        self.windAvg = gauge.Gauge(self.canvas, 400, 100,
                                   title        = "Wind speed\n(10 min avg.)",
                                   min          = 0,
                                   max          = 100,
                                   unitspertick = 5,
                                   bigticks     = 25,
                                   digitformat  = "%.1f knots",
                                   handcolour   = "orange")

        self.temp    = gauge.Gauge(self.canvas, 550, 100,
                                   title        = "Temperature",
                                   min          = -10,
                                   max          = 35,
                                   scalevalues  = [-10,0,10,20,30],
                                   unitspertick = 1,
                                   bigticks     = 5,
                                   digitformat  = "%.1f C",
                                   handcolour   = "red")

        self.dewpt   = gauge.Gauge(self.canvas, 700, 100,
                                   title        = "Dew point",
                                   min          = -10,
                                   max          = 35,
                                   scalevalues  = [-10,0,10,20,30],
                                   unitspertick = 1,
                                   bigticks     = 5,
                                   digitformat  = "%.1f C",
                                   handcolour   = "cyan")

        self.press   = gauge.Gauge(self.canvas, 175, 250,
                                   title        = "Pressure\nQNH",
                                   min          = 950,
                                   max          = 1050,
                                   unitspertick = 5,
                                   digitformat  = "%d mB",
                                   handcolour   = "purple")

        self.humid   = gauge.Gauge(self.canvas, 325, 250,
                                   title        = "Humidity",
                                   min          = 0,
                                   max          = 100,
                                   unitspertick = 5,
                                   bigticks     = 25,
                                   digitformat  = "%d %%",
                                   handcolour   = "green")

        self.sun     = gauge.Gauge(self.canvas, 475, 250,
                                   title        = "Sun since\nmidnight",
                                   min          = 0,
                                   max          = 24,
                                   unitspertick = 1,
                                   bigticks     = 6,
                                   digitformat  = "%.2f hours",
                                   handcolour   = "yellow")

        self.rain    = gauge.Gauge(self.canvas, 625, 250,
                                   title        = "Rain since\nmidnight",
                                   min          = 0,
                                   max          = 100,
                                   unitspertick = 5,
                                   bigticks     = 25,
                                   digitformat  = "%.1f mm",
                                   handcolour   = "blue")

        self.clock   = gauge.Clock(self.canvas, 400, 400)

        # CORBA run indicator:
        self.canvas.create_oval(85,385, 115,415, fill="red", outline="black",
                                width=3, tags="corba")
        self.canvas.create_text(120,400, text="CORBA call in progress",
                                anchor="w", fill="white")

        self.canvas.create_text(550,400,
                                text="CORBA invocation time: --- ms",
                                anchor="w", fill="white", tags="call")



    def update(self):
        self.windDir.set(immediateState.windDirection)
        self.wind.   set(immediateState.windSpeed)
        self.windAvg.set(immediateState.rollingWindSpeed)
        self.temp.   set(immediateState.temperature)
        self.dewpt.  set(immediateState.dewpoint)
        self.press.  set(immediateState.pressure)
        self.humid.  set(immediateState.humidity)
        self.sun.    set(cumulativeState.sunshine)
        self.rain.   set(cumulativeState.rainfall)
        self.clock.  set(cumulativeState.end)

    def startCORBA(self):
        self.canvas.itemconfigure("corba", fill="green")

    def endCORBA(self):
        time.sleep(0.1)
        self.canvas.itemconfigure("corba", fill="dark green")

    def callTime(self, ct):
        self.canvas.itemconfigure("call",
                                  text="CORBA invocation time: %d ms" % ct)

    def go(self):
        self.root.mainloop()
        self.root.destroy()


gui = GUI()
ct  = CORBAThread(sys.argv, gui)
ct.start()
gui.go()
