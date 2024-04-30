#include "coarse_grained.h"
#include <bits/stdc++.h>


// Testing library for a multithreaded environment
#include <thread>
#include <iostream>
#include <cassert>
#include <iostream>

using namespace std;


void test_inserts(Directory* directory) {
    // Attempt to insert multiple values into the directory
    for (int i = 0; i < 10; ++i) {
        try {
            directory->insert(i, "value" + std::to_string(i));
            printf("\n");

        } catch (const std::exception& e) {
            std::cerr << "Exception during insert: " << e.what() << std::endl;
        }
    }
    directory->print_dir();
}

void test_reads(Directory* directory) {
    // Attempt to read values back from the directory
    std::string value;
    for (int i = 0; i < 10; ++i) {
        printf("checking %d\n", i);
        if (directory->get(i, &value)) {
            printf("value: %s\n", value.c_str());
            assert(value == "value" + std::to_string(i));
        }
    }
}

void test_deletes(Directory* directory) {
    // Attempt to remove values
    for (int i = 0; i < 10; ++i) {
        try {
            printf("removing %d\n", i);
            directory->remove(i);
            directory->print_dir();
        } catch (const std::exception& e) {
            std::cerr << "Exception during remove: " << e.what() << std::endl;
        }
    }
}

void test_sequential(Directory *directory){
    try{
       test_inserts(directory);
       test_reads(directory);
       test_deletes(directory); 
    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    } 

}

int main() {
    Directory d = Directory(2); // adjust bucket size as needed
    // testing one thread
    std::thread t1(test_sequential, &d);

    
    // std::thread t1(test_inserts, &d);
    // std::thread t2(test_reads, &d);
    // std::thread t3(test_deletes, &d);

    t1.join();
    // t1.join();
    // t2.join();
    // t3.join();
    return 0;
}

