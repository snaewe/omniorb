#!/usr/bin/env python

# gameServer.py

import sys, threading, time, Queue
import CORBA, PortableServer, CosNaming
import TicTacToe, TicTacToe__POA

SCAVENGER_INTERVAL = 30

class GameFactory_i (TicTacToe__POA.GameFactory):

    def __init__(self, poa):
        # Lists of games and iterators, and a lock to protect access
        # to them.
        self.games     = []
        self.iterators = {}
        self.lock      = threading.Lock()
        self.poa       = poa

        # Create a POA for the GameIterators. Shares the POAManager of
        # this object. The POA uses the default policies of TRANSIENT,
        # SYSTEM_ID, UNIQUE_ID, RETAIN, NO_IMPLICIT_ACTIVATION,
        # USE_ACTIVE_OBJECT_MAP_ONLY, ORB_CTRL_MODEL.

        self.iterator_poa = poa.create_POA("IterPOA", None, [])
        self.iterator_poa._get_the_POAManager().activate()

        self.iterator_scavenger = IteratorScavenger(self)

        print "GameFactory_i created."

    def newGame(self, name):
        # Create a POA for the game and its associated objects.
        # Default policies are suitable. Having one POA per game makes
        # it easy to deactivate all objects associated with a game.
        try:
            game_poa = self.poa.create_POA("Game-" + name, None, [])

        except PortableServer.POA.AdapterAlreadyExists:
            raise TicTacToe.GameFactory.NameInUse()

        # Create Game servant object
        gservant = Game_i(self, name, game_poa)

        # Activate it
        gid = game_poa.activate_object(gservant)

        # Get the object reference
        gobj = game_poa.id_to_reference(gid)

        # Activate the POA
        game_poa._get_the_POAManager().activate()
        
        # Add to our list of games
        self.lock.acquire()
        self.games.append((name, gservant, gobj))
        self.lock.release()

        # Return the object reference
        return gobj

    def listGames(self, how_many):
        self.lock.acquire()
        front = self.games[:int(how_many)]
        rest  = self.games[int(how_many):]
        self.lock.release()

        # Create list of GameInfo structures to return
        ret = map(lambda g: TicTacToe.GameInfo(g[0], g[2]), front)

        # Create iterator if necessary
        if rest:
            iter = GameIterator_i(self, self.iterator_poa, rest)
            iid  = self.iterator_poa.activate_object(iter)
            iobj = self.iterator_poa.id_to_reference(iid)
            self.lock.acquire()
            self.iterators[iid] = iter
            self.lock.release()
        else:
            iobj = None # Nil object reference

        return (ret, iobj)

    def _removeGame(self, name):
        self.lock.acquire()
        for i in range(len(self.games)):
            if self.games[i][0] == name:
                del self.games[i]
                break
        self.lock.release()

    def _removeIterator(self, iid):
        self.lock.acquire()
        del self.iterators[iid]
        self.lock.release()

class GameIterator_i (TicTacToe__POA.GameIterator):

    def __init__(self, factory, poa, games):
        self.factory = factory
        self.poa     = poa
        self.games   = games
        self.tick    = 1 # Tick for time-out garbage collection
        print "GameIterator_i created."

    def __del__(self):
        print "GameIterator_i deleted."

    def next_n(self, how_many):
        self.tick  = 1
        front      = self.games[:int(how_many)]
        self.games = self.games[int(how_many):]

        # Convert internal representation to GameInfo sequence
        ret = map(lambda g: TicTacToe.GameInfo(g[0], g[2]), front)

        if self.games:
            more = 1
        else:
            more = 0

        return (ret, more)

    def destroy(self):
        id = self.poa.servant_to_id(self)
        self.factory._removeIterator(id)
        self.poa.deactivate_object(id)

class IteratorScavenger (threading.Thread):
    def __init__(self, factory):
        threading.Thread.__init__(self)
        self.setDaemon(1)
        self.factory = factory
        self.start()

    def run(self):
        print "Iterator scavenger running..."

        lock      = self.factory.lock
        iterators = self.factory.iterators
        poa       = self.factory.iterator_poa
        manager   = poa._get_the_POAManager()

        while 1:
            time.sleep(SCAVENGER_INTERVAL)

            print "Scavenging dead iterators..."

            # Bonus points for spotting why we hold requests...
            manager.hold_requests(1)
            lock.acquire()

            for id, iter in iterators.items():
                if iter.tick == 1:
                    iter.tick = 0
                else:
                    del iterators[id]
                    poa.deactivate_object(id)

                # This del drops the last reference to the iterator so
                # it can be collected immediately. Without it, the
                # Python servant object stays around until the next
                # time around the loop.
                del iter

            lock.release()
            manager.activate()

class Game_i (TicTacToe__POA.Game):

    def __init__(self, factory, name, poa):
        self.factory = factory
        self.name    = name
        self.poa     = poa
        self.lock    = threading.Lock()

        n = TicTacToe.Nobody

        self.players = 0
        self.state   = [[n,n,n],
                        [n,n,n],
                        [n,n,n]]

        self.p_noughts         = None
        self.p_crosses         = None
        self.whose_go          = TicTacToe.Nobody
        self.spectators        = []
        self.spectatorNotifier = SpectatorNotifier(self.spectators, self.lock)

        print "Game_i created."

    def __del__(self):
        print "Game_i deleted."

    def _get_name(self):
        return self.name

    def _get_players(self):
        return self.players

    def _get_state(self):
        return self.state

    def joinGame(self, player):
        try:
            self.lock.acquire()
            
            if self.players == 2:
                raise TicTacToe.Game.CannotJoin()

            if self.players == 0:
                ptype = TicTacToe.Nought
                self.p_noughts = player
            else:
                ptype = TicTacToe.Cross
                self.p_crosses = player

                # Notify the noughts player that it's their go
                try:
                    self.whose_go = TicTacToe.Nought
                    self.p_noughts.yourGo(self.state)
                except (CORBA.COMM_FAILURE, CORBA.OBJECT_NOT_EXIST), ex:
                    print "Lost contact with player"
                    self.kill()

            # Create a GameController
            gc   = GameController_i(self, ptype)
            id   = self.poa.activate_object(gc)
            gobj = self.poa.id_to_reference(id)

            self.players = self.players + 1

        finally:
            self.lock.release()

        return (gobj, ptype)

    def watchGame(self, spectator):
        self.lock.acquire()
        cookie = len(self.spectators)
        self.spectators.append(spectator)
        self.lock.release()
        return cookie, self.state

    def unwatchGame(self, cookie):
        cookie = int(cookie)
        self.lock.acquire()
        if len(self.spectators) > cookie:
            self.spectators[cookie] = None
        self.lock.release()

    def kill(self):
        self.factory._removeGame(self.name)

        if self.p_noughts:
            try:
                self.p_noughts.gameAborted()
            except CORBA.SystemException, ex:
                print "System exception contacting noughts player"

        if self.p_crosses:
            try:
                self.p_crosses.gameAborted()
            except CORBA.SystemException, ex:
                print "System exception contacting crosses player"

        self.spectatorNotifier.gameAborted()

        self.poa.destroy(1,0)

        print "Game killed"

    
    def _play(self, x, y, ptype):
        """Real implementation of GameController::play()"""

        if self.whose_go != ptype:
            raise TicTacToe.GameController.NotYourGo()

        if x < 0 or x > 2 or y < 0 or y > 2:
            raise TicTacToe.GameController.InvalidCoordinates()

        if self.state[x][y] != TicTacToe.Nobody:
            raise TicTacToe.GameController.SquareOccupied()

        self.state[x][y] = ptype

        w = self._checkForWinner()

        try:
            if w is not None:
                print "Winner:", w
                self.p_noughts.end(self.state, w)
                self.p_crosses.end(self.state, w)
                self.spectatorNotifier.end(self.state, w)

                # Kill ourselves
                self.factory._removeGame(self.name)
                self.poa.destroy(1,0)
            else:
                # Tell opponent it's their go
                if ptype == TicTacToe.Nought:
                    self.whose_go = TicTacToe.Cross
                    self.p_crosses.yourGo(self.state)
                else:
                    self.whose_go = TicTacToe.Nought
                    self.p_noughts.yourGo(self.state)

                self.spectatorNotifier.update(self.state)

        except (CORBA.COMM_FAILURE, CORBA.OBJECT_NOT_EXIST), ex:
            print "Lost contact with player!"
            self.kill()

        return self.state

    def _checkForWinner(self):
        """If there is a winner, return the winning player's type. If
        the game is a tie, return Nobody, otherwise return None."""

        # Rows
        for i in range(3):
            if self.state[i][0] == self.state[i][1] and \
               self.state[i][1] == self.state[i][2] and \
               self.state[i][0] != TicTacToe.Nobody:
                return self.state[i][0]

        # Columns
        for i in range(3):
            if self.state[0][i] == self.state[1][i] and \
               self.state[1][i] == self.state[2][i] and \
               self.state[0][i] != TicTacToe.Nobody:
                return self.state[0][i]

        # Top-left to bottom-right
        if self.state[0][0] == self.state[1][1] and \
           self.state[1][1] == self.state[2][2] and \
           self.state[0][0] != TicTacToe.Nobody:
            return self.state[0][0]

        # Bottom-left to top-right
        if self.state[0][2] == self.state[1][1] and \
           self.state[1][1] == self.state[2][0] and \
           self.state[0][2] != TicTacToe.Nobody:
            return self.state[0][2]

        # Return None if the game is not full
        for i in range(3):
            for j in range(3):
                if self.state[i][j] == TicTacToe.Nobody:
                    return None

        # It's a draw
        return TicTacToe.Nobody
    
class SpectatorNotifier (threading.Thread):

    # This thread is used to notify all the spectators about changes
    # in the game state. Since there is only one thread, one errant
    # spectator can hold up all the others. A proper event or
    # notification service should make more effort to contact clients
    # concurrently. No matter what happens, the players can't be held
    # up.
    #
    # The implementation uses a simple work queue, which could
    # potentially get backed-up. Ideally, items on the queue should be
    # thrown out if they have been waiting too long.

    def __init__(self, spectators, lock):
        threading.Thread.__init__(self)
        self.setDaemon(1)
        self.spectators = spectators
        self.lock       = lock
        self.queue      = Queue.Queue(0)
        self.start()

    def run(self):
        print "SpectatorNotifier running..."

        while 1:
            method, args = self.queue.get()

            print "Notifying:", method

            try:
                self.lock.acquire()
                for i in range(len(self.spectators)):
                    spec = self.spectators[i]
                    if spec:
                        try:
                            apply(getattr(spec, method), args)

                        except (CORBA.COMM_FAILURE,
                                CORBA.OBJECT_NOT_EXIST), ex:
                            print "Spectator lost"
                            self.spectators[i] = None
            finally:
                self.lock.release()

    def update(self, state):
        s = (state[0][:], state[1][:], state[2][:])
        self.queue.put(("update", (s,)))

    def end(self, state, winner):
        self.queue.put(("end", (state, winner)))

    def gameAborted(self):
        self.queue.put(("gameAborted", ()))

class GameController_i (TicTacToe__POA.GameController):

    def __init__(self, game, ptype):
        self.game  = game
        self.ptype = ptype
        print "GameController_i created."

    def __del__(self):
        print "GameController_i deleted."
    
    def play(self, x, y):
        return self.game._play(x, y, self.ptype)

def main(argv):

    print "Game Server starting..."

    orb = CORBA.ORB_init(argv, CORBA.ORB_ID)
    poa = orb.resolve_initial_references("RootPOA")

    poa._get_the_POAManager().activate()

    gf_impl = GameFactory_i(poa)
    gf_id   = poa.activate_object(gf_impl)
    gf_obj  = poa.id_to_reference(gf_id)

    print orb.object_to_string(gf_obj)

    # Bind the GameFactory into the Naming service. This code is
    # paranoid about checking all the things which could go wrong.
    # Normally, you would assume something this fundamental works, and
    # just die with uncaught exceptions if it didn't.
    try:
        nameRoot = orb.resolve_initial_references("NameService")
        nameRoot = nameRoot._narrow(CosNaming.NamingContext)
        if nameRoot is None:
            print "NameService narrow failed!"
            sys.exit(1)

    except CORBA.ORB.InvalidName, ex:
        # This should never happen, since "NameService" is always a
        # valid name, even if it hadn't been configured.

        print "Got an InvalidName exception when resolving NameService!"
        sys.exit(1)

    except CORBA.NO_RESOURCES, ex:
        print "No NameService configured!"
        sys.exit(1)

    except CORBA.SystemException, ex:
        print "System exception trying to resolve and narrow NameService!"
        print ex
        sys.exit(1)

    # Create a new context named "tutorial"
    try:
        name = [CosNaming.NameComponent("tutorial", "")]
        tutorialContext = nameRoot.bind_new_context(name)

    except CosNaming.NamingContext.AlreadyBound, ex:
        # There is already a context named "tutorial", so we resolve
        # that.
        print 'Reusing "tutorial" naming context.'

        tutorialContext = nameRoot.resolve(name)
        tutorialContext = tutorialContext._narrow(CosNaming.NamingContext)

        if tutorialContext is None:
            # Oh dear -- the thing called "tutorial" isn't a
            # NamingContext. We could replace it, but it's safer to
            # bail out.
            print 'The name "tutorial" is already bound in the NameService.'
            sys.exit(1)

    # Bind the GameServer into the "tutorial" context. Use rebind() to
    # replace an existing entry is there is one.
    tutorialContext.rebind([CosNaming.NameComponent("GameFactory","")], gf_obj)

    print "GameFactory bound in NameService."

    orb.run()

if __name__ == "__main__":
    main(sys.argv)
