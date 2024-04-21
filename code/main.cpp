#include <bits/stdc++.h>
#include <stdexcept>
using namespace std;

// useful link: https://www.geeksforgeeks.org/extendible-hashing-dynamic-approach-to-dbms/

class Bucket {
  int local_depth;
  int bucket_size;
  std::map<int, string> elements;
  public:
    Bucket(int local_depth, int bucket_size);
    int size();
    void insert(int key, string value);
    void remove(int key);
    void update(int key, string value);
    std::map<int, string> getElements();
    void increaseDepth();
    int getLocalDepth();
};

class Directory{
  int global_depth;
  int bucket_size;
  std::vector<Bucket *> buckets;
  public:
    Directory(int bucket_size);
    int hash(int key);
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

void Bucket::insert(int key, string value){
  if (this->bucket_size == this->elements.size()){
    throw std::invalid_argument("bucket full. can't insert.\n");
  }
  this->elements.insert({key, value});
}

void Bucket::remove(int key){
  auto it = this->elements.find(key);
  if (it == this->elements.end()){
    throw std::invalid_argument("key not in bucket\n");
  }
  this->elements.erase(it);
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

}

void Directory::update(int key, string value){

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