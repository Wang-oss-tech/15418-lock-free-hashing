#include "fine_grained.h"
#include <bits/stdc++.h>


// Testing library for a multithreaded environment
#include <string>
#include <thread>
#include <iostream>
#include <cassert>
#include <iostream>

using namespace std;


void test_inserts(Directory* directory) {
    printf("Inserting test 1 called \n");
    // Attempt to insert multiple values into the directory
    for (int i = 0; i < 10; ++i) {
        try {
            directory->insert(i, "value" + std::to_string(i));
            // printf("\n");

        } catch (const std::exception& e) {
            std::cerr << "Exception during insert fn 1: " << i << e.what() << std::endl;
        }
    }
    directory->print_dir();
}

void test_inserts_2(Directory* directory) {
    printf("Inserting test 2 called \n");
    // Attempt to insert multiple values into the directory
    for (int i = 4; i < 8; ++i) {
        try {
            directory->insert(i, "value" + std::to_string(i));
            // printf("\n");

        } catch (const std::exception& e) {
            std::cerr << "Exception during insert fn 2: " << i << e.what() << std::endl;
        }
    }
    // directory->print_dir();
}

void test_inserts_3(Directory* directory) {
    printf("Inserting test 3 called \n");
    // Attempt to insert multiple values into the directory
    for (int i = 1500; i < 3000; ++i) {
        try {
            directory->insert(i, "value" + std::to_string(i));
            // printf("\n");

        } catch (const std::exception& e) {
            std::cerr << "Exception during insert: " << e.what() << std::endl;
        }
    }
    // directory->print_dir();
}



void test_merge(Directory* directory){
    directory->insert(0, "value0");
    directory->insert(1, "value1");
    directory->insert(3, "value3");
    directory->insert(5, "value5");
    directory->insert(7, "value7");
    directory->print_dir();
    printf("DONE INSERTING\n\n");
    directory->remove(1);
    directory->remove(3);
    directory->remove(5);
    // expect a merge on 7
    directory->remove(7);
    directory->print_dir();
}

void test_reads(Directory* directory) {
    printf("Reading test called\n\n");
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
    printf("\nDeleting test 1 called\n\n");
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

void test_deletes_2(Directory* directory){
    printf("\n Deleting test 2 called\n\n");
    // Attempt to remove multiple values from the directory
    for (int i = 20; i < 25; ++i) {
        try {
            directory->insert(i, "value" + std::to_string(i));
            // printf("\n");

        } catch (const std::exception& e) {
            std::cerr << "Exception during insert: " << e.what() << std::endl;
        }
    }
}

void test_deletes_3(Directory* directory){
    printf("\n Deleting test 3 called\n\n");
    // Attempt to remove multiple values from the directory
    for (int i = 30; i < 35; ++i) {
        try {
            directory->insert(i, "value" + std::to_string(i));
            printf("\n");

        } catch (const std::exception& e) {
            std::cerr << "Exception during insert: " << e.what() << std::endl;
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

void test_many_inserts(){
    printf("Testing many inserts\n");
    Directory d = Directory(2); // adjust bucket size as needed
    auto start = std::chrono::high_resolution_clock::now();

    // Multiple Thread test
    std::thread t1(test_inserts, &d);
    std::thread t2(test_inserts_2, &d);
    // std::thread t3(test_inserts_3, &d);

    t1.join();
    t2.join();
    // t3.join();

    for (int i = 0; i < 9; i++){
        string v;
        auto b = d.get(i, &v);
        assert(b && !v.compare("value"+std::to_string(i)));
    }
    // for (int i = 100; i < 110; i++){
    //     string v;
    //     auto b = d.get(i, &v);
    //     assert(b && !v.compare("value"+std::to_string(i)));
    // }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = stop-start;
    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    d.print_dir(); 
}

int main() {
    // setting bucket size to 2 and setting directory
    test_many_inserts();
    // Directory d = Directory(2); // adjust bucket size as needed
    
    // test_inserts(&d);
    
    // auto start = std::chrono::high_resolution_clock::now();
    // // testing one thread
    // // std::thread t1(test_sequential, &d);


    // // Multiple Thread test
    // std::thread t1(test_inserts, &d);
    // std::thread t2(test_inserts_2, &d);
    // std::thread t3(test_inserts_3, &d);

    // t1.join();
    // t2.join();
    // t3.join();

    // auto stop = std::chrono::high_resolution_clock::now();
    // auto duration = stop-start;
    // cout << "Time taken by function: "
    //      << duration.count() << " microseconds" << endl;

    // d.print_dir();

    return 0;
}

