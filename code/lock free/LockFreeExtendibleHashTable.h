#include "LockFreeSortedList.h"
#include <string>
#include <atomic>

class LockFreeExtendibleHashTable {
public:
  LockFreeExtendibleHashTable();
  bool get(unsigned int key, std::string* value);
  void insert(unsigned int key, std::string value);
  void remove(unsigned int key);
  void update(unsigned int key, std::string value);
  void print_table();
  void print_list();
private:
  // global variables 
  std::atomic<LockFreeSortedList<unsigned int, std::string>::LockFreeSortedListItem **>buckets; // array of pointers to list node items
  LockFreeSortedList<unsigned int, std::string> T; // linked list where all elements are stored
  std::atomic<unsigned int> count; // # elements in table
  std::atomic<unsigned int> size; // size of table
  // private functions
  void initializeBucket(unsigned int bucket);
  unsigned int makeRegularKey(unsigned int key);
  unsigned int makeDummyKey(unsigned int key);
  bool isDummy(unsigned int key);
};