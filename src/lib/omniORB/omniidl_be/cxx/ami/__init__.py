
from omniidl_be.cxx.ami import main, ami

def __init__():
    print "AMI init called"
    # Register all the predefined things we need
    ami.register_Messaging_ExceptionHolder()
    ami.register_Messaging_ReplyHandler()


def init_hooks():
    main.__init_hooks__()
    pass



