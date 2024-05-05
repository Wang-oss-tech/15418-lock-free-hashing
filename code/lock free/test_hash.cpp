#include "LockFreeExtendibleHashTable.h"
#include <string>

// Testing library for a multithreaded environment
#include <thread>
#include <iostream>
#include <cassert>
#include <iostream>
using namespace std;

void test_inserts(LockFreeExtendibleHashTable* T) {
    printf("Inserting test 1 called \n");
    // Attempt to insert multiple values into the directory
    for (int i = 0; i < 20; ++i) {
        try {
            T->insert(i, "value" + std::to_string(i));
            // printf("\n");
            // T->print_table();

        } catch (const std::exception& e) {
            std::cerr << "Exception during insert: " << i << e.what() << std::endl;
        }
    }
    // directory->print_dir();
}

void test_inserts_2(LockFreeExtendibleHashTable* T) {
    printf("Inserting test 2 called \n");
    // Attempt to insert multiple values into the directory
    for (int i = 20; i < 50; ++i) {
        try {
            T->insert(i, "value" + std::to_string(i));
            // T->print_table();
            // printf("\n");

        } catch (const std::exception& e) {
            std::cerr << "Exception during insert: " << i << e.what() << std::endl;
        }
    }
    // directory->print_dir();
}
void test_many_inserts(){
    printf("Testing many inserts\n");
    auto T = LockFreeExtendibleHashTable(); // adjust bucket size as needed
    auto start = std::chrono::high_resolution_clock::now();

    // Multiple Thread test
    std::thread t1(test_inserts, &T);
    std::thread t2(test_inserts_2, &T);
    // std::thread t3(test_inserts_3, &d);
    printf("finished inserts\n");
    t1.join();
    t2.join();
    // t3.join();

    // T.print_table(); 
    for (int i = 0; i < 30; i++){
        // printf("getting %d: \n", i);
        string v;
        auto b = T.get(i, &v);
        assert(b && !v.compare("value"+std::to_string(i)));
    }
    // for (int i = 100; i < 102; i++){
    //     printf("getting t2: \n");
    //     string v;
    //     auto b = T.get(i, &v);
    //     assert(b && !v.compare("value"+std::to_string(i)));
    // }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = stop-start;
    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    T.print_table(); 
}
int main(){
  test_many_inserts();
  // auto T = LockFreeExtendibleHashTable();
  // for (int i = 0; i < 20; i++){
  //   T.insert(i, std::to_string(i));
  // }
  // // T.insert(1, "str1");
  // // // T.print_table();
  // // T.insert(2, "str2");
  // // // T.print_table();
  // // T.insert(3, "str3");
  // // // T.print_table();
  // // T.insert(5, "str5");
  // // T.insert(7, "str7");
  // // T.print_table();
  // string v;
  // // printf("getting 3\n");
  // T.get(3, &v);
  // printf("Get %d: %s\n", 3, v.c_str());
  // // T.print_table();
  // for (int i = 0; i < 10; i++){
  //   T.remove(i);
  // }
  // T.print_table();
}