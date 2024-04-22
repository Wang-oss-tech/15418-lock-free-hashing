#include <bits/stdc++.h>
#include <shared_mutex>
#include <stdexcept>
#include <mutex>

using namespace std;

// useful link: https://www.geeksforgeeks.org/extendible-hashing-dynamic-approach-to-dbms/
// coarse grained: every time someone wants to modify they get a lock on the whole directory

class Bucket {
  int local_depth;
  int bucket_size;
  std::map<int, string> elements;
  public:
    Bucket(int local_depth, int bucket_size);
    int size();
    bool get(int key, string* value);
    void insert(int key, string value);
    bool remove(int key);
    void update(int key, string value);
    std::map<int, string> getElements();
    void increaseDepth();
    void decreaseDepth();
    int getLocalDepth();
};

class Directory{
  int global_depth;
  int bucket_size;
  std::vector<Bucket *> buckets;
  public:
    Directory(int bucket_size);
    std::shared_mutex directory_mutex;
    int hash(int key);
    bool get(int key, string* value);
    void insert(int key, string value);
    void remove(int key);
    void update(int key, string value);
    void split(int bucketIdx);
    int getSplitIdx(int bucketIdx);
    void increaseGlobalDepth();
};

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
  int mask = 1 << this->global_depth;
  return key & mask; 
}

bool Directory::get(int key, string *val){
  int hash_key = this->hash(key);
  std::shared_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  Bucket *bucket = this->buckets[hash_key];
  string value;
  if(bucket->get(key, &value)){
    val = &value;
    directory_lock.unlock();
    return true;
  }
  directory_lock.unlock();
  return false;
}

void Directory::insert(int key, string value){
  int hash_key = this->hash(key);
  std::unique_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  Bucket *bucket = this->buckets[hash_key];
  // case 1: bucket is not full
  // just insert. nothing complicated
  if (bucket->size() < this->bucket_size){
    bucket->insert(key, value); 
    directory_lock.unlock();
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
  directory_lock.unlock();
}

void Directory::remove(int key){
  int hash_key = this->hash(key);
  std::unique_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  Bucket *bucket = this->buckets[hash_key]; 
  if (!bucket->remove(key)){
    directory_lock.unlock();
    throw std::invalid_argument("key not found");
  }

  // check to see if we need to recursively merge
  if (bucket->size() != 0){
    directory_lock.unlock();
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
  directory_lock.unlock();
}

void Directory::update(int key, string value){
  int hash_key = this->hash(key);
  std::unique_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  auto bucket = this->buckets[hash_key];
  bucket->update(key, value);
  directory_lock.unlock();
}

int Directory::getSplitIdx(int bucketIdx){
  std::shared_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  int mask = 1 << this->buckets[bucketIdx]->getLocalDepth();
  directory_lock.unlock();
  return bucketIdx ^ mask; 
}

void Directory::increaseGlobalDepth(){
  // all new buckets should point to their split index
  std::unique_lock<std::shared_mutex> directory_lock(this->directory_mutex);
  for (int i = 0; i < (1 << this->global_depth); i ++){
    this->buckets.emplace_back(buckets[i]);
  }
  this->global_depth++;
  directory_lock.unlock();
}