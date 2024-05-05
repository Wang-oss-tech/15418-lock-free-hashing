#include <bits/stdc++.h>
#include "LockFreeExtendibleHashTable.h"
#include "LockFreeSortedList.h"

using namespace std;

// lock free extendible hash table implemented using recursive split ordering
// some choices were made for the sake of simplicity. for example, the table only grows and never shrinks.
LockFreeExtendibleHashTable::LockFreeExtendibleHashTable(){
  this->count.store(0); 
  this->size.store(2); 
  buckets = reinterpret_cast<LockFreeSortedList<unsigned int, std::string>::LockFreeSortedListItem **>(calloc(2,8));
  T.insert(0, "dummy value");
  // does this work with atomic?
  buckets[0] = T.headPtr();
}

// this would be faster with a lookup table
unsigned int reverseBits(unsigned int n){
    unsigned int res = 0;
    for (unsigned int i = 0; i < 32; i++) {
      unsigned int b = (n & 1);
      res |= b << (31 - i);
      n >>= 1;
    }
    return res;
}

// this function flips the most significant bit
unsigned int getParent(unsigned int key){
	unsigned int i;
	for (i = 31; i >= 0; i--) {
		if (key & (1 << i)) {
      return key & ~(1 << i);
    }
	}
	return 0;
}

unsigned int LockFreeExtendibleHashTable::makeRegularKey(unsigned int key){
  unsigned int mask = 1 << 31;
  return reverseBits(key | mask);
}

unsigned int LockFreeExtendibleHashTable::makeDummyKey(unsigned int key){
  return reverseBits(key);
}

bool LockFreeExtendibleHashTable::isDummy(unsigned int key){
  return !(key & 1) ;
}

void LockFreeExtendibleHashTable::initializeBucket(unsigned int bucket){
  printf("Initializing bucket %d\n", bucket);
  unsigned int parent = getParent(bucket);
  auto curr_buckets = buckets.load();
  if (curr_buckets[parent] == nullptr){
    initializeBucket(parent);
  }
  auto dummyKey = makeDummyKey(bucket);
  // if insert fails that means another node created dummykey already, but
  // thats ok because we are calling getNode to get the node
  T.insert(dummyKey, "dummy value");
  auto dummyBucket = T.getNode(dummyKey);
  curr_buckets[bucket] = dummyBucket;
  assert(dummyBucket->key == dummyKey);
  this->buckets.store(curr_buckets);
}

bool LockFreeExtendibleHashTable::get(unsigned int key, string *value){
  unsigned int size = this->size.load();
  auto bucket = key % size;
  auto curr_buckets = buckets.load();
  // note that just because current bucket not initalized doesn't mean
  // its not in the table. we might have to split an old bucket to find it
  if (curr_buckets[bucket] == nullptr){
    initializeBucket(bucket);
    // return false;
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

void LockFreeExtendibleHashTable::insert(unsigned int key, string value){
  // printf("INSERTING %d\n", key);
  unsigned int size = this->size.load();
  auto bucket = key % size; 
  // printf("Bucket: %d\n", bucket);
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
  auto c = this->count.fetch_add(1);
  // printf("c: %d\n", c);
  if ((c+1) / csize >= 2){
    // printf("jsakldfjlkads\n");
    this->size.compare_exchange_strong(csize, 2 * csize);
    auto s = this->size.load();
    // printf("SIZE: %d\n", s);
    auto new_b = reinterpret_cast<LockFreeSortedList<unsigned int, std::string>::LockFreeSortedListItem **>(calloc(2 * csize, 8));
    for (int i = 0; i < csize; i++){
      new_b[i] = curr_buckets[i];
    }
    // printf("finished reallocating\n");
    this->buckets.store(new_b);
  }
}

void LockFreeExtendibleHashTable::remove(unsigned int key){
  unsigned int size = this->size.load();
  auto bucket = key % size; 
  auto curr_buckets = buckets.load();
  if (curr_buckets[bucket] == nullptr){
    initializeBucket(bucket);
  }
  // removal unsuccessful, key not inserted
  if (!T.remove(makeRegularKey(key))){
    printf("can't find it\n");
    return;
  }
  this->count.fetch_sub(1);
}

void LockFreeExtendibleHashTable::update(unsigned int key, string value){
  remove(key);
  insert(key, value);
}

void LockFreeExtendibleHashTable::print_table(){
  auto curr_size = this->size.load();
  auto curr_count = this->count.load();
  auto curr_buckets = this->buckets.load();
  char str[100];
  printf("------------------------------------------------------\n");
  printf("PRINTING HASH TABLE: \n");
  printf("Size: %d, Count, %d\n", curr_size, curr_count);
  for(unsigned int i = 0; i < curr_size; i++){
    auto bucket = curr_buckets[i];
    if (bucket == nullptr){
      continue;
    }
    printf("Bucket %d\n", i);
    bucket = bucket->next_ptr;
    // printf("ajsklfjdksla\n");
    auto k = bucket->key;
    // printf("key: %u\n", k);
    while (bucket != nullptr && !isDummy(bucket->key)){
      printf( "Key: %u, Value: %s \n", bucket->key, bucket->value.c_str());
      bucket = bucket->next_ptr;
    }
    // printf("%s", str);
    printf("\n");
  }
  printf("------------------------------------------------------\n");
}

void LockFreeExtendibleHashTable::print_list(){
  auto p = T.headPtr();
  printf("Printing list: \n");
  while (p){
    printf("K: %u, V: %s | ", p->key, p->value.c_str());
    p = p->next_ptr;
  }    
  printf("\n");
}