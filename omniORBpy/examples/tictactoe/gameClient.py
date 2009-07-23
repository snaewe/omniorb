#!/usr/bin/env python

# gameClient.py

import sys, threading
import CORBA, PortableServer
import TicTacToe, TicTacToe__POA

from Tkinter import *

class GameBrowser :

    """This class implements a top-level user interface to the game
    player. It lists the games currently running in the GameFactory.
    The user can choose to create new games, and join, watch or kill
    existing games."""

    def __init__(self, orb, poa, gameFactory):
        self.orb         = orb
        self.poa         = poa
        self.gameFactory = gameFactory
        self.initGui()
        self.getGameList()
        print "GameBrowser initialised"

    def initGui(self):
        """Initialise the Tk objects for the GUI"""

        self.master = Tk()
        self.master.title("Game Client")
        self.master.resizable(0,0)

        frame = Frame(self.master)

        # List box and scrollbar
        listframe    = Frame(frame)
        scrollbar    = Scrollbar(listframe, orient=VERTICAL)
        self.listbox = Listbox(listframe, exportselection = 0,
                               width = 30, height = 20,
                               yscrollcommand = scrollbar.set)

        scrollbar.config(command = self.listbox.yview)
        self.listbox.pack(side=LEFT, fill=BOTH, expand=1)
        scrollbar.pack(side=RIGHT, fill=Y)

        self.listbox.bind("<ButtonRelease-1>", self.selectGame)

        listframe.grid(row=0, column=0, rowspan=6)

        # Padding
        Frame(frame, width=20).grid(row=0, column=1, rowspan=6)

        # Buttons
        newbutton    = Button(frame, text="New game",   command=self.newGame)
        joinbutton   = Button(frame, text="Join game",  command=self.joinGame)
        watchbutton  = Button(frame, text="Watch game", command=self.watchGame)
        killbutton   = Button(frame, text="Kill game",  command=self.killGame)
        updatebutton = Button(frame, text="Update list",command=self.update)
        quitbutton   = Button(frame, text="Quit",       command=frame.quit)

        newbutton   .config(width=15)
        joinbutton  .config(width=15)
        watchbutton .config(width=15)
        killbutton  .config(width=15)
        updatebutton.config(width=15)
        quitbutton  .config(width=15)

        self.newbutton = newbutton
        newbutton.bind("<ButtonRelease-1>", self.setNewButtonPosition)

        newbutton   .grid(row=0, column=2)
        joinbutton  .grid(row=1, column=2)
        watchbutton .grid(row=2, column=2)
        killbutton  .grid(row=3, column=2)
        updatebutton.grid(row=4, column=2)
        quitbutton  .grid(row=5, column=2)

        self.newGameDialogue = None

        # Padding at bottom
        Frame(frame, height=10).grid(row=6, columnspan=3)

        # Status bar
        self.statusbar = Label(self.master,
                               text="", bd=1, relief=SUNKEN, anchor=W)
        self.statusbar.pack(side=BOTTOM, fill=X)

        frame.pack(side=TOP)

    def getGameList(self):
        """Get the list of games from the GameFactory, and populate
        the Listbox in the GUI"""

        # To make life interesting, we get the game information
        # structures one at a time from the server. It would be far
        # more sensible to get them many at a time.

        self.gameList = []
        self.listbox.delete(0,END)

        try:
            seq, iterator = self.gameFactory.listGames(0)
        except CORBA.SystemException, ex:
            print "System exception contacting GameFactory:"
            print "  ", CORBA.id(ex), ex
            return

        if len(seq) > 0:
            print "listGames() did not return an empty sequence as it should"

        if iterator is None:
            print "No games in the GameFactory"
            return

        try:
            more = 1
            while more:
                seq, more = iterator.next_n(1)

                for info in seq:
                    # seq should only ever have one element, but loop
                    # to be safe
                    self.gameList.append(info)
                    self.listbox.insert(END,info.name)

            iterator.destroy()

        except CORBA.SystemException, ex:
            print "System exception contacting GameIterator:"
            print "  ", CORBA.id(ex), ex

    def statusMessage(self, msg):
        self.statusbar.config(text = msg)

    def selectGame(self, evt):
        selection = self.listbox.curselection()

        if selection == (): return

        index = int(selection[0])
        info  = self.gameList[index]

        try:
            players = info.obj._get_players()
            if players == 0:
                msg = "no players yet"
            elif players == 1:
                msg = "one player waiting"
            else:
                msg = "game in progress"

        except CORBA.SystemException, ex:
            print "System exception contacting Game:"
            print "  ", CORBA.id(ex), ex
            msg = "error contacting Game object"

        self.statusMessage("%s: %s" % (info.name, msg))

    def setNewButtonPosition(self, evt):
        self._new_x = self.master.winfo_x() + self.newbutton.winfo_x() + evt.x
        self._new_y = self.master.winfo_y() + self.newbutton.winfo_y() + evt.y

    def newGame(self):
        if self.newGameDialogue:
            self.newGameDialogue.destroy()

        self.newGameDialogue = toplevel = Toplevel(self.master)
        toplevel.transient()
        toplevel.title("New game...")
        toplevel.geometry("+%d+%d" % (self._new_x, self._new_y))

        Label(toplevel, text="Enter name for new game").pack()

        entry = Entry(toplevel)
        entry.pack()
        entry.focus()

        entry.bind("<Key-Return>", self.newGameEntered)

    def newGameEntered(self, evt):
        name = evt.widget.get()
        self.newGameDialogue.destroy()
        self.newGameDialogue = None

        if name == "":
            self.statusMessage("You must give a non-empty name")
            return

        try:
            game = self.gameFactory.newGame(name)

        except TicTacToe.GameFactory.NameInUse:
            self.statusMessage("Game name in use")
            return

        except CORBA.SystemException, ex:
            print "System exception trying to create new game:"
            print "  ", CORBA.id(ex), ex
            self.statusMessage("System exception trying to create new game")
            return

        self.getGameList()

    def joinGame(self):
        selection = self.listbox.curselection()
        if selection == (): return
        
        index = int(selection[0])
        info  = self.gameList[index]

        pi = Player_i(self.master, info.name)
        id = poa.activate_object(pi)
        po = poa.id_to_reference(id)
        try:
            controller, type = info.obj.joinGame(po)
            if type == TicTacToe.Nought:
                stype = "noughts"
            else:
                stype = "crosses"

            pi.go(info.obj, controller, stype)

            self.statusMessage("%s: joined game as %s" % (info.name, stype))

        except TicTacToe.Game.CannotJoin, ex:
            poa.deactivate_object(id)
            self.statusMessage("%s: cannot join game" % info.name)

        except CORBA.SystemException, ex:
            poa.deactivate_object(id)
            print "System exception trying to join game:"
            print "  ", CORBA.id(ex), ex
            self.statusMessage("%s: system exception contacting game" % \
                               info.name)
            self.getGameList()

    def watchGame(self):
        selection = self.listbox.curselection()
        if selection == (): return
        
        index = int(selection[0])
        info  = self.gameList[index]

        si = Spectator_i(self.master, info.name)
        id = poa.activate_object(si)
        so = poa.id_to_reference(id)
        try:
            cookie, state = info.obj.watchGame(so)
            si.go(info.obj, cookie, state)

            self.statusMessage("Watching %s" % info.name)

        except CORBA.SystemException, ex:
            poa.deactivate_object(id)
            print "System exception trying to watch game:"
            print "  ", CORBA.id(ex), ex
            self.statusMessage("%s: system exception contacting game" % \
                               info.name)
            self.getGameList()

    def update(self):
        self.getGameList()

    def killGame(self):
        selection = self.listbox.curselection()
        if selection == (): return
        
        index = int(selection[0])
        info  = self.gameList[index]

        try:
            info.obj.kill()
            msg = "killed"

        except CORBA.SystemException, ex:
            print "System exception trying to kill game:"
            print "  ", CORBA.id(ex), ex
            msg = "error contacting object"

        self.statusMessage("%s: %s" % (info.name, msg))
        self.getGameList()

class Player_i (TicTacToe__POA.Player):

    def __init__(self, master, name):
        self.master = master
        self.name   = name
        print "Player_i created"

    def __del__(self):
        print "Player_i deleted"

    # CORBA methods
    def yourGo(self, state):
        self.drawState(state)
        self.statusMessage("Your go")

    def end(self, state, winner):
        self.drawState(state)
        if winner == TicTacToe.Nought:
            self.statusMessage("Noughts wins")
        elif winner == TicTacToe.Cross:
            self.statusMessage("Crosses wins")
        else:
            self.statusMessage("It's a draw")
        self.toplevel = None

    def gameAborted(self):
        self.statusMessage("Game aborted!")
        self.toplevel = None

    # Implementation details
    def go(self, game, controller, type):
        self.game       = game
        self.controller = controller
        self.type       = type

        self.toplevel   = Toplevel(self.master)
        self.toplevel.title("%s (%s)" % (self.name, type))

        self.canvas = Canvas(self.toplevel, width=300, height=300)
        self.canvas.pack()

        self.canvas.create_line(100,   0, 100, 300, width=5)
        self.canvas.create_line(200,   0, 200, 300, width=5)
        self.canvas.create_line(0,   100, 300, 100, width=5)
        self.canvas.create_line(0,   200, 300, 200, width=5)

        self.canvas.bind("<ButtonRelease-1>", self.click)
        self.toplevel.bind("<Destroy>", self.close)

        self.statusbar = Label(self.toplevel,
                               text="", bd=1, relief=SUNKEN, anchor=W)
        self.statusbar.pack(side=BOTTOM, fill=X)

    def statusMessage(self, msg):
        if self.toplevel:
            self.statusbar.config(text = msg)

    def click(self, evt):
        x = evt.x / 100
        y = evt.y / 100
        try:
            self.statusMessage("Waiting for other player...")
            state = self.controller.play(x, y)
            self.drawState(state)

        except TicTacToe.GameController.SquareOccupied:
            self.statusMessage("Square already occupied")

        except TicTacToe.GameController.NotYourGo:
            self.statusMessage("Not your go")

        except TicTacToe.GameController.InvalidCoordinates:
            self.statusMessage("Eek!  Invalid coordinates")

        except CORBA.SystemException:
            print "System exception trying to contact GameController:"
            print "  ", CORBA.id(ex), ex
            self.statusMessage("System exception contacting GameController!")

    def close(self, evt):
        if self.toplevel:
            self.toplevel = None
            try:
                self.game.kill()
            except CORBA.SystemException, ex:
                print "System exception trying to kill game:"
                print "  ", CORBA.id(ex), ex
                
            id = poa.servant_to_id(self)
            poa.deactivate_object(id)

    def drawNought(self, x, y):
        cx = x * 100 + 20
        cy = y * 100 + 20
        self.canvas.create_oval(cx, cy, cx+60, cy+60,
                                outline="darkgreen", width=5)

    def drawCross(self, x, y):
        cx = x * 100 + 30
        cy = y * 100 + 30
        self.canvas.create_line(cx, cy, cx+40, cy+40,
                                fill="darkred", width=5)
        self.canvas.create_line(cx, cy+40, cx+40, cy,
                                fill="darkred", width=5)

    def drawState(self, state):
        for i in range(3):
            for j in range(3):
                if state[i][j] == TicTacToe.Nought:
                    self.drawNought(i, j)
                elif state[i][j] == TicTacToe.Cross:
                    self.drawCross(i, j)

class Spectator_i (TicTacToe__POA.Spectator):

    def __init__(self, master, name):
        self.master = master
        self.name   = name
        print "Spectator_i created"

    def __del__(self):
        print "Spectator_i deleted"

    # CORBA methods
    def update(self, state):
        self.drawState(state)

    def end(self, state, winner):
        self.drawState(state)
        if winner == TicTacToe.Nought:
            self.statusMessage("Noughts wins")
        elif winner == TicTacToe.Cross:
            self.statusMessage("Crosses wins")
        else:
            self.statusMessage("It's a draw")
        self.toplevel = None

    def gameAborted(self):
        self.statusMessage("Game aborted!")
        self.toplevel = None

    # Implementation details
    def go(self, game, cookie, state):
        self.game   = game
        self.cookie = cookie

        self.toplevel = Toplevel(self.master)
        self.toplevel.title("Watching %s" % self.name)

        self.canvas = Canvas(self.toplevel, width=300, height=300)
        self.canvas.pack()

        self.canvas.create_line(100,   0, 100, 300, width=5)
        self.canvas.create_line(200,   0, 200, 300, width=5)
        self.canvas.create_line(0,   100, 300, 100, width=5)
        self.canvas.create_line(0,   200, 300, 200, width=5)

        self.toplevel.bind("<Destroy>", self.close)

        self.statusbar = Label(self.toplevel,
                               text="", bd=1, relief=SUNKEN, anchor=W)
        self.statusbar.pack(side=BOTTOM, fill=X)
        self.drawState(state)

    def statusMessage(self, msg):
        self.statusbar.config(text = msg)

    def close(self, evt):
        if self.toplevel:
            self.toplevel = None
            try:
                self.game.unwatchGame(self.cookie)
            except CORBA.SystemException, ex:
                print "System exception trying to unwatch game:"
                print "  ", CORBA.id(ex), ex
                
            id = poa.servant_to_id(self)
            poa.deactivate_object(id)

    def drawNought(self, x, y):
        cx = x * 100 + 20
        cy = y * 100 + 20
        self.canvas.create_oval(cx, cy, cx+60, cy+60,
                                outline="darkgreen", width=5)

    def drawCross(self, x, y):
        cx = x * 100 + 30
        cy = y * 100 + 30
        self.canvas.create_line(cx, cy, cx+40, cy+40,
                                fill="darkred", width=5)
        self.canvas.create_line(cx, cy+40, cx+40, cy,
                                fill="darkred", width=5)

    def drawState(self, state):
        for i in range(3):
            for j in range(3):
                if state[i][j] == TicTacToe.Nought:
                    self.drawNought(i, j)
                elif state[i][j] == TicTacToe.Cross:
                    self.drawCross(i, j)


orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
poa = orb.resolve_initial_references("RootPOA")
poa._get_the_POAManager().activate()

# Get the GameFactory reference using a corbaname URI. On a pre-CORBA
# 2.4 ORB, this would have to explicitly contact the naming service.
try:
    gameFactory = orb.string_to_object("corbaname:rir:#tutorial/GameFactory")
    gameFactory = gameFactory._narrow(TicTacToe.GameFactory)

except CORBA.BAD_PARAM, ex:
    # string_to_object throws BAD_PARAM if the name cannot be resolved
    print "Cannot find the GameFactory in the naming service."
    sys.exit(1)

except CORBA.SystemException, ex:
    # This might happen if the naming service is dead, or the narrow
    # tries to contact the object and it is not there.

    print "CORBA system exception trying to get the GameFactory reference:"
    print "  ", CORBA.id(ex), ex
    sys.exit(1)

# Start the game browser
browser = GameBrowser(orb, poa, gameFactory)

# Run the Tk mainloop in a separate thread

def tkloop():
    browser.master.mainloop()
    print "Shutting down the ORB..."
    orb.shutdown(0)
    
threading.Thread(target=tkloop).start()

# Run the ORB main loop (not necessary with omniORBpy, but may be
# necessary with other ORBs. According to the CORBA specification,
# orb.run() must be given the main thread.
orb.run()
