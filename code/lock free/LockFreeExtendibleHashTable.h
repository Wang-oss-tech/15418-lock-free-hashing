#include "LockFreeSortedList.h"
#include <string>
#include <atomic>

class LockFreeExtendibleHashTable {
public:
  LockFreeExtendibleHashTable();
  bool get(int key, std::string* value);
  void insert(int key, std::string value);
  void remove(int key);
  void update(int key, std::string value);
private:
  // global variables 
  std::atomic<LockFreeSortedList<int, std::string>::LockFreeSortedListItem **>buckets; // array of pointers to list node items
  LockFreeSortedList<int, std::string> T; // linked list where all elements are stored
  std::atomic<int> count; // # elements in table
  std::atomic<int> size; // size of table
  // private functions
  void initializeBucket(int bucket);
  int makeRegularKey(int key);
  int makeDummyKey(int key);
};