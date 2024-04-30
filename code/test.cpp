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
    for (int i = 0; i < 50; ++i) {
        try {
            directory->insert(i, "value" + std::to_string(i));
            directory->print_dir();
            printf("\n");

        } catch (const std::exception& e) {
            std::cerr << "Exception during insert: " << e.what() << std::endl;
        }
    }
}

void test_reads(Directory* directory) {
    // Attempt to read values back from the directory
    std::string value;
    for (int i = 0; i < 100; ++i) {
        if (directory->get(i, &value)) {
            assert(value == "value" + std::to_string(i));
        }
    }
}

void test_deletes(Directory* directory) {
    // Attempt to remove values
    for (int i = 0; i < 100; ++i) {
        try {
            directory->remove(i);
        } catch (const std::exception& e) {
            std::cerr << "Exception during remove: " << e.what() << std::endl;
        }
    }
}

void test_one(Directory *directory){
    try{
        // printf("???????\n");
        directory->insert(0, "zero");
        // printf("???????\n");
        // directory->print_dir();
        directory->insert(1, "one");
        directory->remove(0);
        directory->update(1, "new");

        // Prints the directory after operations
        directory->print_dir();
    } catch(const std::exception& e) {
        std::cerr << "Exception during insert: " << e.what() << std::endl;
    } 

}

int main() {
    Directory d = Directory(2); // adjust bucket size as needed
    // testing one thread
    std::thread t1(test_inserts, &d);

    t1.join();
    
    // std::thread t1(test_inserts, &directory);
    // std::thread t2(test_reads, &directory);
    // std::thread t3(test_deletes, &directory);

    // t1.join();
    // t2.join();
    // t3.join();
    return 0;
}

