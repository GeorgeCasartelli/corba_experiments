import sys
from omniORB import CORBA
import CosNaming 
import Demo, Demo__POA
import time
import datetime

class Hello_i(Demo__POA.Hello):
    def greet(self, name):
        if name.strip() == "":
            raise Demo.Hello.InvalidName("Empty string! Must provide a name!")
        else:
            return f"Hello, {name}!"

    def add(self, a, b):
        return a+b

    def makePerson(self, name, age):
        return Demo.Person(name, age)

    def slowCall(self, delaySeconds):
        time.sleep(delaySeconds)
        return f"Finally replied after {delaySeconds}"

class Heartbeat_i(Demo__POA.Heartbeat):
    def __init__(self):
        self.resetDelay


    def ping(self):
        time.sleep(self.delay)
        return f"Ping {datetime.datetime.now()}"


    def resetDelay(self):
        self.delay = 0

    def setDelay(self, seconds):
        self.delay = seconds


orb = CORBA.ORB_init(sys.argv + ["-ORBInitRef", "NameService=corbaname::localhost:2809"], CORBA.ORB_ID)
poa = orb.resolve_initial_references("RootPOA")

hello_servant = Hello_i()
hello_obj = hello_servant._this()

heartbeat_servant = Heartbeat_i()
heartbeat_obj = heartbeat_servant._this()


# ior = orb.object_to_string(hello_obj)
# with open("test_py.ior", "w") as f:
#     f.write(ior)

# get naming service
naming_obj = orb.resolve_initial_references("NameService")
naming_context = naming_obj._narrow(CosNaming.NamingContext)

hello_name = [CosNaming.NameComponent("Hello", "")]
naming_context.rebind(hello_name, hello_obj)

heartbeat_name = [CosNaming.NameComponent("Heartbeat", "")]
naming_context.rebind(heartbeat_name, heartbeat_obj)

print("Python server ready. Bound as \"Hello\" in the Naming Service")

poa._get_the_POAManager().activate()
orb.run()