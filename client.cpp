#include "test.hh"
#include <iostream>
#include <fstream>
#include <omniORB4/Naming.hh>
#include <omniORB4/omniORB.h>
#include <unistd.h>
#include <chrono>
#include <vector>


int main(int argc, char** argv) {

    // set global timeout. default 60s. set here to 10s
    int argc2 = 3;
    char* argv2[] = { argv[0], (char*)"-ORBclientCallTimeOutPeriod", (char*)"10000" };

    CORBA::ORB_var orb = CORBA::ORB_init(argc2, argv2);

    // resolving naming service
    
    // CORBA::Object_var nsobj = orb->resolve_initial_references("NameService");
    // CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(nsobj);
    // if (CORBA::is_nil(nc)) {
    //     std::cerr << "Failed to narrow NamingContext" << std::endl;
    //     return 1;
    // }

    // // resolve hello interface
    // CosNaming::Name hello_name;
    // hello_name.length(1);
    // hello_name[0].id = CORBA::string_dup("Hello");
    // hello_name[0].kind = CORBA::string_dup("");

    // CosNaming::Name heartbeat_name;
    // heartbeat_name.length(1);
    // heartbeat_name[0].id = CORBA::string_dup("Hello"); // same target, diff name obj
    // heartbeat_name[0].kind = CORBA::string_dup("");

    std::ifstream iorFile("ior_file.ior");
    std::string ior;
    std::getline(iorFile, ior);
    iorFile.close();

    // reference 1 for function calls with lenient timeout
    CORBA::Object_var obj_hello = orb->string_to_object(ior.c_str());
    Demo::Hello_var hello = Demo::Hello::_narrow(obj_hello);
    if (CORBA::is_nil(hello)) {
        std::cerr << "Failed to narrow object reference" << std::endl;
        return 1;
    }
    omniORB::setClientCallTimeout(hello, 6000); // more lenient 6s timeout

    // resolve from same interface, but used only for heartbeat check

    CORBA::Object_var heartbeat_ref = orb->string_to_object(ior.c_str());
    // CORBA::Object_var heartbeat_ref = nc->resolve(hello_name);
    omniORB::setClientCallTimeout(heartbeat_ref, 2000); // shorter (2s), fails faster

    // address check. independent local objects, same remote object
    std::cout << "obj_hello address: " << (void*)obj_hello.in() << std::endl;
    std::cout << "heartbeat_ref address: " << (void*)heartbeat_ref.in() << std::endl;

    CORBA::Boolean same = obj_hello->_is_equivalent(heartbeat_ref);
    std::cout << "Same remote object? " << (same ? "yes" : "no") << std::endl;


    // hello calls checking orb server working for func calls
    try {
        CORBA::String_var result = hello->greet("John");
        std::cout << result << std::endl;
    } catch (const Demo::Hello::InvalidName& ex) {
        std::cerr << "Server rejected name: " << ex.reason << std::endl;
    }

    std::cout << hello->add(5, 7) << std::endl;

    Demo::Person_var person = hello->makePerson("Charlie", 26);
    std::cout << person->name << ", " << person-> age << std::endl;
    std::cout << hello->greet(person->name) << std::endl;



    // --== HEARTBEAT ROUTINE USING _non_existent ==--
    // shorter timeout
    // to show behaviour find server PID and `kill -STOP <pid>` to freeze,
    // then `-CONT` to continue. 
    //
    // After 3 failures, tries to run hello->greet() on original ref
    // also get a timer. should hang for as long as defined previous
    // 
    // Proves independence between two references


    int count = 0;
    int failures = 0;

    while (true) {
        std::cout << "Count: " << count << std::endl;

        try {
            CORBA::Boolean gone = heartbeat_ref->_non_existent();
            if (gone) {
                std::cerr << "Object gone" << std::endl;
            } else {
                std::cout << "Heartbeat strong. Object exists" <<std::endl;
                failures = 0;
            }
        } catch (const CORBA::TIMEOUT&) {
            std::cerr << "Heartbeat timed out after ~2s" << std::endl;
            failures++;
        } 

        if (failures >=3 ) {
            std::cout << "Hearbeat failed >3 times. Testing hello timeout" << std::endl;

            auto start = std::chrono::steady_clock::now();
            try {
                CORBA::String_var result = hello->greet("George");
                std::cout << "Unexpected success: " << result << std::endl;
            } catch (const CORBA::SystemException& ex) {
                auto end = std::chrono::steady_clock::now();
                double elapsed = std::chrono::duration<double>(end - start).count();
                std::cout << ">>> Original 'hello' reference failed after "
                        << elapsed << " seconds (exception: " << ex._name() << ")" << std::endl;
            }
            failures = 0; // reset loop
        }
        sleep(1);
        count++;
    }
}
