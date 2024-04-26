#include <bits/stdc++.h>
#include <stdexcept>
#include <mutex>
#include "fine_grained.h"

using namespace std;

// useful link: https://www.geeksforgeeks.org/extendible-hashing-dynamic-approach-to-dbms/

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

void Bucket::insert(int key, string value){
  if (this->bucket_size == this->elements.size()){
    throw std::invalid_argument("bucket full. can't insert.\n");
  }
  this->elements.insert({key, value});
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
  this->buckets.push_back(new Bucket(1, bucket_size));
  this->buckets.push_back(new Bucket(1, bucket_size));
}

int Directory::hash(int key){
  std::unique_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  int mask = 1 << this->global_depth;
  return key & mask; 
}

bool Directory::get(int key, string *val){
  std::shared_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  int hash_key = this->hash(key);
  Bucket *bucket = this->buckets[hash_key];
  std::shared_lock<std::shared_mutex> bucket_lock(bucket->bucket_mutex);
  directory_lock.unlock();
  string value;
  if(bucket->get(key, &value)){
    val = &value;
    return true;
  }
  return false;
}

void Directory::insert(int key, string value){
  int hash_key = this->hash(key);
  Bucket *bucket = this->buckets[hash_key];
  // case 1: bucket is not full
  // just insert. nothing complicated
  if (bucket->size() < this->bucket_size){
    bucket->insert(key, value); 
    return;
  }

  // case 2: bucket is full and local depth < global depth.
  // insert with bucket splitting
  bool stop_splitting = false;
  while(!stop_splitting){
    // need to do directory expansion
    if (bucket->getLocalDepth() == this->global_depth){
      this->increaseGlobalDepth();
    }
    // increase depths of this bucket and split
    bucket->increaseDepth();
    this->buckets[this->getSplitIdx(hash_key)] = new Bucket(bucket->getLocalDepth(), this->bucket_size);

    // rehash all elements in this bucket
    auto elems = bucket->getElements();
    for (auto it = elems.begin(); it != elems.end(); it++){
      auto k = it->first;
      auto v = it->second;
      this->insert(k, v);
    }
    // insert the current element. if still can't insert, need to keep splitting
    Bucket *new_bucket = this->buckets[this->hash(key)];
    if (new_bucket->size() < this->bucket_size){
      new_bucket->insert(key, value);
      stop_splitting = true; 
    }
    else { // need to keep splitting
      bucket = new_bucket;

    }
  }

}

void Directory::remove(int key){
  int hash_key = this->hash(key);
  Bucket *bucket = this->buckets[hash_key]; 
  if (!bucket->remove(key)){
    throw std::invalid_argument("key not found");
  }

  // check to see if we need to recursively merge
  if (bucket->size() != 0){
    return;
  }
  bucket->decreaseDepth();

  // new bucket should be the minimum
  auto idx = hash_key;
  bool stop_merging = false;
  while (!stop_merging){
    auto split_idx = this->getSplitIdx(idx);
    auto new_bucket = this->buckets[split_idx];

    // if both the main bucket and split bucket are empty, we merge them together
    if (new_bucket->size() == 0 && new_bucket->getLocalDepth() == bucket->getLocalDepth()){
      new_bucket->decreaseDepth();
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
      this->global_depth --;
    }
  }
}

void Directory::update(int key, string value){
  int hash_key = this->hash(key);
  auto bucket = this->buckets[hash_key];
  bucket->update(key, value);
}

int Directory::getSplitIdx(int bucketIdx){
  int mask = 1 << this->buckets[bucketIdx]->getLocalDepth();
  return bucketIdx ^ mask; 
}

void Directory::increaseGlobalDepth(){
  // all new buckets should point to their split index
  for (int i = 0; i < (1 << this->global_depth); i ++){
    this->buckets.emplace_back(buckets[i]);
  }
  this->global_depth++;
}