#include <bits/stdc++.h>
#include "fine_grained.h"
#include <shared_mutex>
#include <stdexcept>
#include <stdio.h>
#include <mutex>

using namespace std;

// useful link: https://www.geeksforgeeks.org/extendible-hashing-dynamic-approach-to-dbms/
// coarse grained: every time someone wants to modify they get a lock on the whole directory

Bucket::Bucket(int local_depth, int bucket_size){
  this->local_depth = local_depth;
  this->bucket_size = bucket_size;
}

int Bucket::size(){
  return this->elements.size();
}

bool Bucket::get(int key, string *value){
  auto it = this->elements.find(key);
  if (it == this->elements.end()){
    return false;
  }
  *value = it->second;
  return true;
}

bool Bucket::insert(int key, string value){
  if (this->bucket_size == this->elements.size()){
    return false;
  }
  auto it = this->elements.find(key);
  // already in bucket
  if (it != this->elements.end()){
    printf("%d already in\n", key);
    return false;
  }
  this->elements.insert({key, value});
  return true;
}

bool Bucket::remove(int key){
  auto it = this->elements.find(key);
  if (it == this->elements.end()){
    return false;
  }
  this->elements.erase(it);
  return true;
}

void Bucket::update(int key, string value){
  auto it = this->elements.find(key);
  if (it == this->elements.end()){
    throw std::invalid_argument("key not in bucket\n");
  }
  this->elements[key] = value;
}

void Bucket::increaseDepth(){
  this->local_depth++;
}

void Bucket::decreaseDepth(){
  this->local_depth--;
}

int Bucket::getLocalDepth(){
  return this->local_depth;
}

std::map<int, string> Bucket::getElements(){
  return this->elements;
}

// create a new directory
Directory::Directory(int bucket_size){
  this->global_depth = 1;
  this->bucket_size = bucket_size;
  this->buckets.push_back(new Bucket(1, bucket_size));
  this->buckets.push_back(new Bucket(1, bucket_size));
}

int Directory::hash(int key){
  int mask = (1 << this->global_depth)-1;
  return key & mask; 
}

bool Directory::get(int key, string *val){
  int hash_key = this->hash(key);
  std::shared_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  Bucket *bucket = this->buckets[hash_key];
  std::shared_lock<std::shared_mutex> bucket_lock(bucket->bucket_mutex);
  directory_lock.unlock();
  string value;
  if(bucket->get(key, &value)){
    *val = value;
    return true;
  }
  return false;
}

void Directory::insert(int key, string value){
  // printf("global: %d", this->global_depth);
  printf("INSERT: %d, hash: %d\n", key, this->hash(key));
  std::shared_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  printf("%d acquired insert lock\n", key);
  // printf("hash key: %d\n", hash_key);
  int hash_key = this->hash(key);
  Bucket *bucket = this->buckets[hash_key];
  std::unique_lock<std::shared_mutex> bucket_lock(bucket->bucket_mutex);
  printf("%d acquired bucket lock\n", key);
  string v;
  if (bucket->get(key, &value)){
    printf("%d got\n", key);
    return;
  }
  // case 1: bucket is not full
  // just insert. nothing complicated
  if (bucket->size() < this->bucket_size){
    directory_lock.unlock();
    // printf("bucket not full\n");
    printf("KEY %d inserted into %d\n\n", key, hash_key);
    bucket->insert(key, value); 
    // printf("inserted into bucket\n");
    // directory_lock.unlock();
    return;
  }

  bucket_lock.unlock();
  directory_lock.unlock();
  // case 2: bucket is full and local depth < global depth.
  // insert with bucket splitting
  printf("%d trying to acquire unique lock\n", key);
  std::unique_lock<std::shared_mutex> unique_directory_lock(this->directory_mutex);
  bucket_lock.lock();
  // recheck in case table was modified while it was unlocked
  if (bucket->get(key, &value)){
    printf("%d got\n", key);
    return;
  }
  bool stop_splitting = false;
  while(!stop_splitting){
    printf("splitting %d\n", key); 
    // need to do directory expansion
    if (bucket->getLocalDepth() == this->global_depth){
      // directory_lock.lock();
      this->increaseGlobalDepth();
      // directory_lock.unlock();
      // printf("increased global depth\n");
    }
    // increase depths of this bucket and split
    bucket->increaseDepth();
    // printf("hash: %d, splitidx: %d\n", hash_key, this->getSplitIdx(hash_key));
    // directory_lock.lock();
    this->buckets[this->getSplitIdx(hash_key)] = new Bucket(bucket->getLocalDepth(), this->bucket_size);
    // acquire lock on new bucket 
    // std::unique_lock<std::shared_mutex> new_bucket_lock(this->buckets[this->getSplitIdx(hash_key)]->bucket_mutex);
    // rehash all elements in this bucket
    // printf("rehashing\n");
    auto elems = bucket->getElements();
    for (auto it = elems.begin(); it != elems.end(); it++){
      auto k = it->first;
      auto v = it->second;
      bucket->remove(k);
      Bucket *curr_bucket = this->buckets[this->hash(k)];
      curr_bucket->insert(k, v);
    }
    // insert the current element. if still can't insert, need to keep splitting
    Bucket *new_bucket = this->buckets[this->hash(key)];
    if (new_bucket->insert(key, value)){
      // printf("KEY %d inserted into %d\n", key, this->hash(key));
      stop_splitting = true; 
    }
    else { // need to keep splitting
      bucket = new_bucket;

    }
    // new_bucket_lock.unlock();
    // directory_lock.unlock();
  }
  printf("%d done \n", key);
  // directory_lock.unlock();
}

void Directory::remove(int key){
  std::unique_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  int hash_key = this->hash(key);
  Bucket *bucket = this->buckets[hash_key]; 
  std::unique_lock<std::shared_mutex> bucket_lock(bucket->bucket_mutex);
  directory_lock.unlock();
  if (!bucket->remove(key)){
    directory_lock.unlock();
    throw std::invalid_argument("key not found");
  }

  // check to see if we need to recursively merge
  if (bucket->size() != 0){
    directory_lock.unlock();
    return;
  }

  auto idx = hash_key;
  bool stop_merging = false;
  directory_lock.lock();
  while (!stop_merging){
    auto split_idx = this->getSplitIdx(idx);
    auto new_bucket = this->buckets[split_idx];

    // if both the main bucket and split bucket are empty, we merge them together
    if (new_bucket->size() == 0 && new_bucket->getLocalDepth() == bucket->getLocalDepth()){
      // printf("merging buckets %d and %d\n", idx, split_idx);
      new_bucket->decreaseDepth();
      bucket->decreaseDepth();
      // change all things pointing to same bucket to new one
      auto idx_inc = 1 << bucket->getLocalDepth();
      for (int i = idx; i >= 0; i -= idx_inc){
        this->buckets[i] = new_bucket;
      }
      for (int i = idx; i < 1 << this->global_depth; i += idx_inc){
        this->buckets[i] = new_bucket;
      }
    } 
    else{
      stop_merging = true;
    }

    // check if we can decrease depth
    bool can_decrease = true;
    for (auto &b : this->buckets){
      if (b->getLocalDepth() == this->global_depth){
        can_decrease = false;
      }
    }
    if (can_decrease){
      if (this->global_depth == 1){
        break;
      }
      this->global_depth --;
      for (int i = 0; i < (1 << this->global_depth); i++){
        this->buckets.pop_back();
      }
      printf("decreasing global depth to %d:\n", this->global_depth);
    }
  }
}

void Directory::update(int key, string value){
  int hash_key = this->hash(key);
  std::unique_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  auto bucket = this->buckets[hash_key];
  std::unique_lock<std::shared_mutex> bucket_lock(bucket->bucket_mutex);
  directory_lock.unlock();
  bucket->update(key, value);
}

int Directory::getSplitIdx(int bucketIdx){
  // std::shared_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  int mask = 1 << (this->buckets[bucketIdx]->getLocalDepth()-1);
  // directory_lock.unlock();
  return bucketIdx ^ mask; 
}

void Directory::increaseGlobalDepth(){
  // all new buckets should point to their split index
  // std::unique_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  for (int i = 0; i < (1 << this->global_depth); i ++){
    this->buckets.emplace_back(buckets[i]);
  }
  this->global_depth++;
  // printf("NEW GLOBAL: %d\n", this->global_depth);
  // directory_lock.unlock();
}

void Directory::print_dir() {
    char str[100]; // Adjust the size according to your needs
    printf("Printing Directory \n\n");
    printf("GLOBAL DEPTH: %d\n", this->global_depth);
    for (int i = 0; i < buckets.size(); i++) {
        Bucket *bucket = this->buckets[i];
        printf("BUCKET %d, depth %d:\n ", i, bucket->getLocalDepth());
        std::map<int, string> elems = bucket->getElements();
        for (auto it = elems.begin(); it != elems.end(); ++it) {
            sprintf(str, "  Key: %d, Value: %s \n", it->first, it->second.c_str());
            printf("%s", str);
        }
        printf("\n");
    }
}