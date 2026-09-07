#include "hello.hh"
#include <iostream>
#include <fstream>

class Hello_i : public POA_Demo::Hello {
public:
    char* greet(const char* name) {
        std::string msg = std::string("Hello, ") + name + "!";
        return CORBA::string_dup(msg.c_str());
    }
};

int main(int argc, char** argv) {
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv); // start ORB

    // get POA (portable object adapter) -> turns C++ object into callable
    CORBA::Object_var poa_obj = orb->resolve_initial_references("RootPOA"); 
    PortableServer::POA_var poa = PortableServer::POA::_narrow(poa_obj);
    PortableServer::POAManager_var pman = poa->the_POAManager();

    // create object from IDL interface for Hello
    Hello_i* hello_servant = new Hello_i();
    PortableServer::ObjectId_var oid = poa->activate_object(hello_servant); // register obj w/ POA to be reachable

    // get reference, write to a file
    CORBA::Object_var obj = hello_servant->_this(); // corba obj ref to servant
    CORBA::String_var ior = orb->object_to_string(obj); // convert ref to IOR (ref of how to find obj, host, port ID etc)

    // write to ior file so python client can read
    std::ofstream iorFile("hello.ior");
    iorFile << ior;
    iorFile.close();

    std::cout << "Server ready. IOR written to hello.ior" << std::endl;

    // start serving
    pman->activate(); // put POA into active state. wont process reqs before this
    orb->run(); // blocks forever, handling incoming calls in loop

    hello_servant->_remove_ref();
    return 0;
}