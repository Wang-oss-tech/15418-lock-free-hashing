#include <bits/stdc++.h>
#include "LockFreeExtendibleHashTable.h"
#include "LockFreeSortedList.h"

using namespace std;

// lock free extendible hash table implemented using recursive split ordering
// some choices were made for the sake of simplicity. for example, the table only grows and never shrinks.
LockFreeExtendibleHashTable::LockFreeExtendibleHashTable(){
  this->count.store(0); 
  this->size.store(0); 
  buckets = reinterpret_cast<LockFreeSortedList<int, std::string>::LockFreeSortedListItem **>(calloc(8,2));
  T.insert(0, "dummy");
  // does this work with atomic?
  buckets[0] = T.headPtr();
}

// bit reversal function taken here:
// https://www.geeksforgeeks.org/reverse-actual-bits-given-number/
// this would be faster with a lookup table
int reverseBits(int n){
    unsigned int rev = 0;
    while (n > 0) {
        rev <<= 1;

        // if current bit is '1'
        if ((n & 1) == 1)
            rev ^= 1;
 
        // bitwise right shift
        // 'n' by 1
        n >>= 1;
    }
    return rev;
}

void LockFreeExtendibleHashTable::initializeBucket(int bucket){

}

int makeRegularKey(int key){
  int mask = 1 << 31;
  return reverseBits(key | mask);
}

int makeDummyKey(int key){
  return reverseBits(key);
}

bool LockFreeExtendibleHashTable::get(int key, string *value){
  int size = this->size.load();
  auto bucket = key & size;
  auto curr_buckets = buckets.load();
  if (curr_buckets[bucket] == nullptr){
    initializeBucket(bucket);
    return false;
  }
  auto rev_key = makeRegularKey(key);
  string V;
  auto found = T.find(rev_key, curr_buckets[bucket], &V);
  if (!found){
    return false;
  }
  *value = V;
  return true;
}

void LockFreeExtendibleHashTable::insert(int key, string value){
  int size = this->size.load();
  auto bucket = key & size; 
  auto curr_buckets = buckets.load();
  if (curr_buckets[bucket] == nullptr){
    initializeBucket(bucket);
  }
  auto rev_key = makeRegularKey(key);
  // insertion unsuccessful for some reason
  if (!T.insert(rev_key, value)){
    return;
  }
  auto csize = this->size.load();
  // change the load factor
  if (this->count.fetch_add(1) / csize > 2){
    this->size.compare_exchange_strong(csize, 2 * csize);
    auto new_b = reinterpret_cast<LockFreeSortedList<int, std::string>::LockFreeSortedListItem **>(calloc(8, 2 * csize));
    this->buckets.store(new_b);
  }
}

void LockFreeExtendibleHashTable::remove(int key){
  
}

void LockFreeExtendibleHashTable::update(int key, string value){
  
}