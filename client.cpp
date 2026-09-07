#include "test.hh"
#include <iostream>
#include <fstream>
#include <omniORB4/Naming.hh>
#include <omniORB4/omniORB.h>
#include <unistd.h>
#include <chrono>

int main(int argc, char** argv) {
    int argc2 = 3;
    char* argv2[] = { argv[0], (char*)"-ORBclientCallTimeOutPeriod", (char*)"10000" };
    // CORBA::ORB_var orb = CORBA::ORB_init(argc, argv); // start ORB
    CORBA::ORB_var orb = CORBA::ORB_init(argc2, argv2);

    // std::ifstream iorFile;

    // iorFile.open("test_py.ior");
    // std::string text;
    // if (iorFile.is_open()) {
        
    //     std::getline(iorFile, text);

    //     // std::cout << text << std::endl;
    //     iorFile.close();
    // }
    // else {
    //     std::cout << "ERROR: File not open" << std::endl;
    // }

    // CORBA::Object_var obj = orb->string_to_object(text.c_str());
    
    CORBA::Object_var nsobj = orb->resolve_initial_references("NameService");
    CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(nsobj);

    if (CORBA::is_nil(nc)) {
        std::cerr << "Failed to narrow NamingContext" << std::endl;
        return 1;
    }

    // resolve hello
    CosNaming::Name name_hello;
    name_hello.length(1);
    name_hello[0].id = CORBA::string_dup("Hello");
    name_hello[0].kind = CORBA::string_dup("");

    CORBA::Object_var obj_hello;
    try {
        obj_hello = nc->resolve(name_hello);
    } catch (const CosNaming::NamingContext::NotFound&) {
        std::cerr << "Name 'Hello' not found in Naming Service" << std::endl;
        return 1;
    }

    

    // resolve heartbeat
    // CosNaming::Name name_heartbeat;
    // name_heartbeat.length(1);
    // name_heartbeat[0].id = CORBA::string_dup("Heartbeat");
    // name_heartbeat[0].kind = CORBA::string_dup("");
    
    // CORBA::Object_var obj_heartbeat;
    // try {
    //     obj_heartbeat = nc->resolve(name_heartbeat);
    // } catch (const CosNaming::NamingContext::NotFound&) {
    //     std::cerr << "Name 'Heartbeat' not found in Naming Service" << std::endl;
    //     return 1;
    // }
    
    Demo::Hello_var hello = Demo::Hello::_narrow(obj_hello);
    omniORB::setClientCallTimeout(hello, 6000);

    if (CORBA::is_nil(hello)) {
        std::cerr << "Failed to narrow object reference" << std::endl;
        return 1;
    }

    CORBA::Object_var heartbeat_ref = nc->resolve(name_hello);
    // CORBA::Object_var heartbeat_ref = CORBA::Object::_duplicate(hello);
    omniORB::setClientCallTimeout(heartbeat_ref, 2000);

    // Demo::Heartbeat_var heartbeat = Demo::Heartbeat::_narrow(obj_heartbeat);
    // omniORB::setClientCallTimeout(heartbeat, 2000);
    // if (CORBA::is_nil(heartbeat)) {
    //     std::cerr << "Failed to narrow heartbeat object reference" << std::endl;
    //     return 1;
    // }

    // hello calls
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

    // --== dedicate heartbeat routine ==--
    // int count = 0;
    // bool delaySet = false;
    // heartbeat->resetDelay();
    
    // while (true) {
    //     std::cout<<"Count: " << count << std::endl;
    //     if (count > 10 && !delaySet) {
    //         try {
    //             heartbeat->setDelay(5);
    //         } catch (const CORBA::SystemException&) {
    //             std::cerr << "setDelay call failed" << std::endl;
    //         }
    //         delaySet = true;
            
    //     }
    //     try {
    //         CORBA::String_var result = heartbeat->ping();
    //         std::cout << "Heartbeat OK: " << result << std::endl;
    //     } catch (const CORBA::TIMEOUT&) {
    //         std::cerr << "Heartbeat TIMEOUT!" << std::endl;
    //         break;
    //     } catch (const CORBA::TRANSIENT&) {
    //         std::cerr << "Call timed out (older style TRANSIENT)!" << std::endl;
    //     } catch (const CORBA::COMM_FAILURE&) {
    //         std::cerr << "Heartbeat COMM_FAILURE" << std::endl;
    //     }

    //     sleep(1);
    //     count++;
    // }


    // --== HEARTBEAT ROUTINE USING _non_existent ==--
    
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
            std::cerr << "Heartbeat timed out after 2s" << std::endl;
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
        }
        sleep(1);
        count++;
    }
}
