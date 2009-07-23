#!/usr/bin/env python
#                           Package   : weather
# wserver.py                Created on: 2000/01/07
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
#   Fake server for AT&T Labs Cambridge weather service

import Tkinter
import sys, threading, time
import gauge

from omniORB import CORBA
import _GlobalIDL, _GlobalIDL__POA

BGCOLOUR = "dark green"

class weatherInfo_i (_GlobalIDL__POA.weatherInfo):

    def __init__(self, gui):
        self.gui = gui

    def windDirection(self):    return int  (self.gui.windDir.get())
    def windSpeed(self):        return int  (self.gui.wind.   get())
    def rollingWindSpeed(self): return float(self.gui.windAvg.get())
    def temperature(self):      return float(self.gui.temp.   get())
    def humidity(self):         return int  (self.gui.humid.  get())
    def dewpoint(self):         return float(self.gui.dewpt.  get())
    def pressure(self):         return int  (self.gui.press.  get())
    def sunny(self):            return 0
    def rainy(self):            return 0

    def immediate(self):
        return _GlobalIDL.immediateWeatherInfo(self.windDirection(),
                                               self.windSpeed(),
                                               self.rollingWindSpeed(),
                                               self.temperature(),
                                               self.humidity(),
                                               self.dewpoint(),
                                               self.pressure(),
                                               0, 0)
    def cumulative(self):
        return _GlobalIDL.cumulativeWeatherInfo(0,
                                                int  (self.gui.clock.get()),
                                                float(self.gui.sun.  get()),
                                                float(self.gui.rain. get()))

class GUI:

    def __init__(self):

        self.root   = Tkinter.Tk()
        self.root.title("AT&T Weather Creation Service")
        self.canvas = Tkinter.Canvas(self.root, width=800, height=500,
                                     background=BGCOLOUR)
        self.canvas.pack()

        self.root.protocol("WM_DELETE_WINDOW", self.root.quit)

        # Make gauges
        self.windDir = gauge.Compass(self.canvas, 100, 100,
                                     title        = "Wind\ndirection",
                                     handcolour   = "green",
                                     interactive  = 1,
                                     bgcolour     = BGCOLOUR)

        self.wind    = gauge.Gauge(self.canvas, 250, 100,
                                   title        = "Wind speed",
                                   min          = 0,
                                   max          = 100,
                                   unitspertick = 5,
                                   bigticks     = 25,
                                   digitformat  = "%d knots",
                                   handcolour   = "orange",
                                   interactive  = 1,
                                   bgcolour     = BGCOLOUR)

        self.windAvg = gauge.Gauge(self.canvas, 400, 100,
                                   title        = "Wind speed\n(10 min avg.)",
                                   min          = 0,
                                   max          = 100,
                                   unitspertick = 5,
                                   bigticks     = 25,
                                   digitformat  = "%.1f knots",
                                   handcolour   = "orange",
                                   interactive  = 1,
                                   bgcolour     = BGCOLOUR)

        self.temp    = gauge.Gauge(self.canvas, 550, 100,
                                   title        = "Temperature",
                                   min          = -10,
                                   max          = 35,
                                   scalevalues  = [-10,0,10,20,30],
                                   unitspertick = 1,
                                   bigticks     = 5,
                                   digitformat  = "%.1f C",
                                   handcolour   = "red",
                                   interactive  = 1,
                                   bgcolour     = BGCOLOUR)

        self.dewpt   = gauge.Gauge(self.canvas, 700, 100,
                                   title        = "Dew point",
                                   min          = -10,
                                   max          = 35,
                                   scalevalues  = [-10,0,10,20,30],
                                   unitspertick = 1,
                                   bigticks     = 5,
                                   digitformat  = "%.1f C",
                                   handcolour   = "cyan",
                                   interactive  = 1,
                                   bgcolour     = BGCOLOUR)

        self.press   = gauge.Gauge(self.canvas, 175, 250,
                                   title        = "Pressure\nQNH",
                                   min          = 950,
                                   max          = 1050,
                                   unitspertick = 5,
                                   digitformat  = "%d mB",
                                   handcolour   = "purple",
                                   interactive  = 1,
                                   bgcolour     = BGCOLOUR)

        self.humid   = gauge.Gauge(self.canvas, 325, 250,
                                   title        = "Humidity",
                                   min          = 0,
                                   max          = 100,
                                   unitspertick = 5,
                                   bigticks     = 25,
                                   digitformat  = "%d %%",
                                   handcolour   = "green",
                                   interactive  = 1,
                                   bgcolour     = BGCOLOUR)

        self.sun     = gauge.Gauge(self.canvas, 475, 250,
                                   title        = "Sun since\nmidnight",
                                   min          = 0,
                                   max          = 24,
                                   unitspertick = 1,
                                   bigticks     = 6,
                                   digitformat  = "%.2f hours",
                                   handcolour   = "yellow",
                                   interactive  = 1,
                                   bgcolour     = BGCOLOUR)

        self.rain    = gauge.Gauge(self.canvas, 625, 250,
                                   title        = "Rain since\nmidnight",
                                   min          = 0,
                                   max          = 100,
                                   unitspertick = 5,
                                   bigticks     = 25,
                                   digitformat  = "%.1f mm",
                                   handcolour   = "blue",
                                   interactive  = 1,
                                   bgcolour     = BGCOLOUR)

        self.clock   = gauge.Clock(self.canvas, 400, 400,
                                   interactive  = 1,
                                   bgcolour     = BGCOLOUR)

    def go(self):
        self.root.mainloop()
        self.root.destroy()

class TimeUpdater(threading.Thread):
    def __init__(self, gui):
        threading.Thread.__init__(self)
        self.setDaemon(1)
        self.gui = gui

    def run(self):
        last = time.time()
        while 1:
            time.sleep(1)
            now = time.time()
            self.gui.clock.set(self.gui.clock.get() + (now - last))
            last = now


orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
poa = orb.resolve_initial_references("RootPOA")
poa._get_the_POAManager().activate()

gui = GUI()
wis = weatherInfo_i(gui)
wio = wis._this()

print orb.object_to_string(wio)

TimeUpdater(gui).start()

gui.go()
