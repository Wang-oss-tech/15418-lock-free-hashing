#include "coarse_grained.h"
#include <bits/stdc++.h>
#include <shared_mutex>
#include <stdexcept>
#include <mutex>


// Testing library for a multithreaded environment
#include <vector>
#include <thread>
#include <iostream>
#include <cassert>
#include <iostream>

using namespace std;


void test_inserts(Directory* directory) {
    // Attempt to insert multiple values into the directory
    for (int i = 0; i < 100; ++i) {
        try {
            directory->insert(i, "value" + std::to_string(i));
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
  directory->insert(0, "zero");
  directory->insert(1, "one");
  directory->remove(0);
  directory->update(1, "new");
}

int main() {
    Directory directory(10); // adjust bucket size as needed

    // testing one thread
    std::thread t1(test_one, &directory);
    
    t1.join();
    
    
    
    
    
    
    // std::thread t1(test_inserts, &directory);
    // std::thread t2(test_reads, &directory);
    // std::thread t3(test_deletes, &directory);

    // t1.join();
    // t2.join();
    // t3.join();

    return 0;
}

