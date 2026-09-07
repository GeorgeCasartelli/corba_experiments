import sys
from omniORB import CORBA
import Demo

orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID) # setup ORB runtime

with open("demo.ior") as f: # read ior file
    ior = f.read()
 
obj = orb.string_to_object(ior) # turn string into CORBA::object
hello = obj._narrow(Demo.Hello) # confirm object is a "Hello"

if hello is None:
    print("Failed to narrow obj ref")
    sys.exit(1)


print(hello.greet("George")) # like normal function call, but through server to cpp