#include <bits/stdc++.h>
using namespace std;

// useful link: https://www.geeksforgeeks.org/extendible-hashing-dynamic-approach-to-dbms/

class Bucket {
  int local_depth;
  public:
    Bucket();
    void insert(int key, string value);
    void remove(int key);
    void update(int key, string value);
};

class Directory{
  int global_depth;
  std::vector<Bucket *> buckets;
  public:
    Directory();
    void insert(int key, string value);
    void remove(int key);
    void update(int key, string value);
};