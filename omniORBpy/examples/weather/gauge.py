# -*- python -*-
#                           Package   : weather
# gauge.py                  Created on: 2000/01/06
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
#   Pretty circular gauges in Tk

import math, time, string, operator
from Tkinter import *

SCALEFONT = "-*-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*"
DIGITFONT = "-*-lucidatypewriter-*-r-*-*-*-120-*-*-m-*-*-*"
TITLEFONT = "-*-helvetica-regular-r-*-*-*-120-*-*-*-*-*-*"

WINDOWPAD = 5

class Gauge :
    def __init__(self, parentcanvas,
                 x, y,                  # Origin
                 min         = 0,       # Minimum value
                 max         = 100,     # Maximum value
                 initial     = 0,       # Initial value
                 scalevalues = None,    # List, eg. [0, 50, 100]
                 scalearc    = 240,     # Angle of scale in degrees
                 minlocation = 240,     # Angle in degrees of min value
                 reverse     = 0,       # Default clockwise; 1 for anti-
                 radius      = 65,      # Radius in pixels
                 handcolour  = "red",   # Colour of hand
                 ticklen     = 8,       # Length of ticks in pixels
                 bigticks    = None,    # Set big ticks when value % n == 0
                 bigticklen  = 12,      # Length of big ticks
                 unitspertick= 5,       # Units per tick
                 knobradius  = 4,       # Radius of knob in pixels
                 knobcolour  = "black", # Colour of knob
                 scalefont   = SCALEFONT,
                 scalecolour = "black",
                 scalesep    = 20,      # Separation of scale text from rim
                 digitformat = "%d",    # Format of digital display
                 digitfont   = DIGITFONT,
                 digitcolour = "black",
                 digitpos    = 35,      # Position of centre of digital display
                 title       = "",
                 titlefont   = TITLEFONT,
                 titlecolour = "black",
                 titlepos    = 25,      # Position of centre of title
                 linecolour  = "black", # Colour of lines
                 facecolour  = "white", # Colour of gauge face
                 bgcolour    = "blue",
                 interactive = 0):      # Is the gauge interactive

        if scalevalues is None:
            scalevalues = [min, (min + max) / 2, max]

        # Things we need to remember
        self.val           = None
        self.min           = min
        self.max           = max
        self.val           = initial
        self.scalesep      = scalesep
        self.scalevalues   = scalevalues
        self.scalefont     = scalefont
        self.scalecolour   = scalecolour
        self.scalearc_r    = scalearc * math.pi / 180
        self.minlocation_r = minlocation * math.pi / 180
        self.reverse       = reverse
        self.radius        = radius
        self.handcolour    = handcolour
        self.knobradius    = knobradius
        self.digitformat   = digitformat
        self.digitfont     = digitfont
        self.digitcolour   = digitcolour
        self.digitpos      = digitpos
        self.titlefont     = titlefont
        self.titlecolour   = titlecolour
        self.titlepos      = titlepos
        self.interactive   = interactive

        # Polygon used to draw the hand
        self.handPolygon = [(-12,-12), (0,radius-5), (12,-12)]

        # Create a canvas for this gauge
        wwidth       = wheight = 2 * radius + 2 * WINDOWPAD
        tl           = WINDOWPAD
        br           = tl + radius * 2
        centre       = tl + radius
        self.centre  = centre
        self.gcanvas = Canvas(parentcanvas,
                              height=wheight, width=wwidth, bg=bgcolour,
                              borderwidth=0, highlightthickness=0)

        # Draw the gauge and knob
        self.gcanvas.create_oval(tl,tl, br,br,
                                 fill=facecolour, outline=linecolour,
                                 width=3, tags="face")
        self.gcanvas.create_oval(centre - knobradius, centre - knobradius,
                                 centre + knobradius, centre + knobradius,
                                 fill=knobcolour)
        # Ticks
        tick = min
        while tick <= max:
            a  = self.angle(tick)
            sa =  math.sin(a)
            ca = -math.cos(a)

            if bigticks:
                if tick % bigticks == 0:
                    len = bigticklen
                else:
                    len = ticklen
            elif tick in scalevalues:
                len = bigticklen
            else:
                len = ticklen

            xof = (radius - len) * sa
            xot = radius         * sa
            yof = (radius - len) * ca
            yot = radius         * ca

            self.gcanvas.create_line(xof+centre, yof+centre,
                                     xot+centre, yot+centre,
                                     fill=linecolour, width=1)

            tick = tick + unitspertick

        # Title
        self.gcanvas.create_text(centre, centre - titlepos,
                                 anchor=CENTER, fill=titlecolour,
                                 font=titlefont, text=title,
                                 justify=CENTER, tags="title")

        # Scale and hand
        self.drawScale()
        self.drawHand()

        # Put gauge into parent canvas
        parentcanvas.create_window(x, y, width=wwidth, height=wheight,
                                   anchor=CENTER, window=self.gcanvas)

    def drawScale(self):
        scalerad = self.radius - self.scalesep
        centre   = self.centre
        for val in self.scalevalues:
            a = self.angle(val)

            self.gcanvas.create_text(centre + scalerad *  math.sin(a),
                                     centre + scalerad * -math.cos(a),
                                     anchor=CENTER, fill=self.scalecolour,
                                     font=self.scalefont, text=str(val))

    def drawHand(self):
        a          = self.angle(self.val)
        centre     = self.centre
        hp         = self.rotatePoints(self.handPolygon, a)
        hp         = self.translatePoints(hp, centre, centre)
        hl         = self.flattenPoints(hp)

        self.hand  = self.gcanvas.create_polygon(hl, fill=self.handcolour,
                                                 tags="hand")

        self.digit = self.gcanvas.create_text(centre, centre + self.digitpos,
                                              anchor=CENTER,
                                              fill=self.digitcolour,
                                              font=self.digitfont,
                                              text=self.digitformat % self.val,
                                              tags="digit")

        self.gcanvas.tag_raise(self.hand, "face")

        if self.interactive:
            self.gcanvas.tag_bind(self.hand, "<B1-Motion>", self.motionhandler)


    def set(self, value):
        """Set value of gauge"""

        if self.val == value:
            return

        self.val = value
        a        = self.angle(value)
        centre   = self.centre
        hp       = self.rotatePoints(self.handPolygon, a)
        hp       = self.translatePoints(hp, centre, centre)
        hl       = self.flattenPoints(hp)

        apply(self.gcanvas.coords, [self.hand] + hl)
        self.gcanvas.itemconfigure(self.digit, text=self.digitformat % value)


    def get(self):
        """Get value of gauge"""
        return self.val

    def motionhandler(self, event):
        x = event.x - self.centre
        y = event.y - self.centre
        a = math.atan2(x,-y) - self.minlocation_r

        while a < 0: a = a + 2 * math.pi

        frac = a / self.scalearc_r

        if frac <= 1.0:
            if self.reverse:
                self.set(self.max - (self.max - self.min) * frac)
            else:
                self.set(self.min + (self.max - self.min) * frac)

    def angle(self, value):
        """Convert value to an angle in radians"""

        if self.reverse == 1:
            value = self.max - value

        if value < self.min:
            a = self.minlocation_r
        elif value > self.max:
            a = self.minlocation_r + self.scalearc_r
        else:
            a = self.minlocation_r + \
                self.scalearc_r * (value - self.min) / (self.max - self.min)

        while a > 2*math.pi:
            a = a - 2 * math.pi

        return a

    def rotatePoint(self, point, theta):
        """Rotate a point clockwise about the origin by an angle in radians"""

        x,y = point
        r   = math.sqrt(x*x + y*y)
        a   = math.atan2(y,x) - theta

        return (r * math.cos(a), r * -math.sin(a))

    def rotatePoints(self, points, theta):
        return map(lambda p, t=theta, rp=self.rotatePoint: rp(p, t), points)

    def translatePoints(self, points, x, y):
        return map(lambda p, x=x, y=y: (p[0]+x,p[1]+y), points)

    def flattenPoints(self, points):
        return list(reduce(operator.add, points))


class Compass(Gauge):
    def __init__(self, parentcanvas,
                 x, y,                  # Origin
                 radius      = 65,      # Radius in pixels
                 handcolour  = "red",   # Colour of hand
                 ticklen     = 8,       # Length of ticks in pixels
                 bigticklen  = 12,      # Length of big ticks
                 knobradius  = 4,       # Radius of knob in pixels
                 knobcolour  = "black", # Colour of knob
                 scalefont   = SCALEFONT,
                 scalecolour = "black",
                 scalesep    = 20,      # Separation of scale text from rim
                 digitfont   = DIGITFONT,
                 digitcolour = "black",
                 digitpos    = 25,      # Position of centre of digital display
                 title       = "",
                 titlefont   = TITLEFONT,
                 titlecolour = "black",
                 titlepos    = 25,      # Position of centre of title
                 linecolour  = "black", # Colour of lines
                 facecolour  = "white", # Colour of gauge face
                 bgcolour    = "blue",
                 interactive = 0):

        Gauge.__init__(self, parentcanvas, x, y, initial      = 0,
                       min         = 0,          max          = 360,
                       minlocation = 0,          scalearc     = 360,
                       radius      = radius,     handcolour   = handcolour,
                       ticklen     = ticklen,    bigticks     = 45,
                       bigticklen  = bigticklen, unitspertick = 15,
                       knobradius  = knobradius, knobcolour   = knobcolour,
                       scalefont   = scalefont,  scalecolour  = scalecolour,
                       scalesep    = scalesep,   digitformat  = "%d deg",
                       digitfont   = digitfont,  digitcolour  = digitcolour,
                       digitpos    = digitpos,   title        = title,
                       titlefont   = titlefont,  titlecolour  = titlecolour,
                       titlepos    = titlepos,   linecolour   = linecolour,
                       facecolour  = facecolour, bgcolour     = bgcolour,
                       interactive = interactive)

    def drawScale(self):
        scalerad = self.radius - self.scalesep
        centre   = self.centre
        for val,txt in [(0,"N"), (90,"E"), (180,"S"), (270,"W")]:
            a = self.angle(val)

            self.gcanvas.create_text(centre + scalerad *  math.sin(a),
                                     centre + scalerad * -math.cos(a),
                                     anchor=CENTER, fill=self.scalecolour,
                                     font=self.scalefont, text=txt)


class Clock(Gauge):
    def __init__(self, parentcanvas,
                 x, y,                  # Origin
                 initial     = time.time(),
                 reverse     = 0,
                 radius      = 65,      # Radius in pixels
                 handcolour  = "red",   # Colour of hand
                 ticklen     = 8,       # Length of ticks in pixels
                 bigticklen  = 12,      # Length of big ticks
                 knobradius  = 4,       # Radius of knob in pixels
                 knobcolour  = "black", # Colour of knob
                 scalefont   = SCALEFONT,
                 scalecolour = "black",
                 scalesep    = 20,      # Separation of scale text from rim
                 digitfont   = DIGITFONT,
                 digitcolour = "black",
                 digitpos    = 25,      # Position of centre of digital display
                 title       = "",
                 titlefont   = TITLEFONT,
                 titlecolour = "black",
                 titlepos    = 25,      # Position of centre of title
                 linecolour  = "black", # Colour of lines
                 facecolour  = "white", # Colour of gauge face
                 bgcolour    = "blue",
                 interactive = 0):

        Gauge.__init__(self, parentcanvas, x, y,
                       initial     = initial,    reverse      = reverse,
                       min         = 0,          max          = 360,
                       minlocation = 0,          scalearc     = 360,
                       radius      = radius,     handcolour   = handcolour,
                       ticklen     = ticklen,    bigticks     = 90,
                       bigticklen  = bigticklen, unitspertick = 30,
                       knobradius  = knobradius, knobcolour   = knobcolour,
                       scalefont   = scalefont,  scalecolour  = scalecolour,
                       scalesep    = scalesep,   digitformat  = None,
                       digitfont   = digitfont,  digitcolour  = digitcolour,
                       digitpos    = digitpos,   title        = "",
                       titlefont   = titlefont,  titlecolour  = titlecolour,
                       titlepos    = titlepos,   linecolour   = linecolour,
                       facecolour  = facecolour, bgcolour     = bgcolour,
                       interactive = interactive)

        self.hm   = None
        self.date = None

    def drawScale(self):
        scalerad = self.radius - self.scalesep
        centre   = self.centre
        for val,txt in [(0,"12"), (90,"3"), (180,"6"), (270,"9")]:
            a = self.angle(val)

            self.gcanvas.create_text(centre + scalerad *  math.sin(a),
                                     centre + scalerad * -math.cos(a),
                                     anchor=CENTER, fill=self.scalecolour,
                                     font=self.scalefont, text=txt)

    def drawHand(self):
        tt = time.gmtime(self.val)
        hm = string.lower(time.strftime("%I:%M %p", tt))
        if hm[0] == "0":
            hm = hm[1:]
        
        self.hm = hm
        hours   = tt[3] % 12
        mins    = tt[4]
        ha      = self.angle(hours * 30 + mins / 2)
        ma      = self.angle(mins  * 6)
        mradius = self.radius - 5
        hradius = mradius * .75
        centre  = self.centre

        self.hand_h = self.gcanvas.create_line(centre, centre,
                                               centre + hradius * math.sin(ha),
                                               centre + hradius *-math.cos(ha),
                                               fill = self.handcolour,
                                               width = 5,
                                               arrow = "last", tags = "hand")

        self.hand_m = self.gcanvas.create_line(centre, centre,
                                               centre + mradius * math.sin(ma),
                                               centre + mradius *-math.cos(ma),
                                               fill = self.handcolour,
                                               width = 5,
                                               arrow = "last", tags = "hand")

        self.digit = self.gcanvas.create_text(centre, centre + self.digitpos,
                                              anchor=CENTER,
                                              fill=self.digitcolour,
                                              font=self.digitfont,
                                              text=hm,
                                              tags="digit")

        self.gcanvas.tag_raise("hand", "face")

        if self.interactive:
            self.gcanvas.tag_bind(self.hand_h,
                                  "<B1-Motion>", self.motionhandler_h)
            self.gcanvas.tag_bind(self.hand_m,
                                  "<B1-Motion>", self.motionhandler_m)

        date = time.strftime("%d %b %Y", tt)
        if date[0] == "0":
            date = date[1:]

        self.gcanvas.itemconfigure("title", text=date)


    def set(self, value):
        """Set time. value is seconds since Unix epoch"""

        self.val = value
        tt       = time.gmtime(value)
        hm       = string.lower(time.strftime("%I:%M %p", tt))

        if hm[0] == "0": hm = hm[1:]
        
        if self.hm == hm:
            return

        self.hm = hm
        hours   = tt[3] % 12
        mins    = tt[4]
        ha      = self.angle(hours * 30 + mins / 2)
        ma      = self.angle(mins  * 6)
        mradius = self.radius - 5
        hradius = mradius * .75
        centre  = self.centre

        self.gcanvas.coords(self.hand_h,
                            centre, centre,
                            centre + hradius *  math.sin(ha),
                            centre + hradius * -math.cos(ha))

        self.gcanvas.coords(self.hand_m,
                            centre, centre,
                            centre + mradius *  math.sin(ma),
                            centre + mradius * -math.cos(ma))

        self.gcanvas.itemconfigure(self.digit, text=hm)

        date = time.strftime("%d %b %Y", tt)
        if date[0] == "0":
            date = date[1:]

        if self.date == date:
            return

        self.gcanvas.itemconfigure("title", text=date)


    def motionhandler_h(self, event):
        x = event.x - self.centre
        y = event.y - self.centre
        a = math.atan2(x,-y)

        while a < 0: a = a + 2 * math.pi

        nh = int((a / (2*math.pi)) * 12 + 0.5) % 12
        if self.reverse: nh = (12 - nh) % 12

        tt = time.gmtime(self.val)
        tl = list(tt)
        oh = tl[3]

        if oh == 0:
            if nh == 11:
                tl[2] = tl[2] - 1
                tl[3] = 23
            else:
                tl[3] = nh

        elif oh <= 10:
            tl[3] = nh

        elif oh == 11:
            if nh == 0:
                tl[3] = 12
            else:
                tl[3] = nh

        elif oh == 12:
            if nh == 11:
                tl[3] = 11
            else:
                tl[3] = nh + 12

        elif oh <= 22:
            tl[3] = nh + 12

        else: # oh == 23
            if nh == 0:
                tl[2] = tl[2] + 1
                tl[3] = 0
            else:
                tl[3] = nh + 12

        self.set(time.mktime(tl))


    def motionhandler_m(self, event):
        x = event.x - self.centre
        y = event.y - self.centre
        a = math.atan2(x,-y)

        while a < 0: a = a + 2 * math.pi

        mins = int((a / (2*math.pi)) * 60 + 0.5) % 60
        if self.reverse: mins = (60 - mins) % 60

        tl   = list(time.gmtime(self.val))

        if   tl[4] > 55 and mins < 5:  tl[3] = tl[3] + 1
        elif tl[4] < 5  and mins > 55: tl[3] = tl[3] - 1

        tl[4] = mins
        self.set(time.mktime(tl))
