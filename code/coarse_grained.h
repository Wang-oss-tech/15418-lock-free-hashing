#include <map>
#include <shared_mutex>
#include <string>
#include <stdexcept>
#include <vector>

class Bucket {
  int local_depth;
  int bucket_size;
  std::map<int, std::string> elements;
  public:
    Bucket(int local_depth, int bucket_size);
    int size();
    bool get(int key, std::string* value);
    bool insert(int key, std::string value);
    bool remove(int key);
    void update(int key, std::string value);
    std::map<int, std::string> getElements();
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
    bool get(int key, std::string* value);
    void insert(int key, std::string value);
    void remove(int key);
    void update(int key, std::string value);
    void split(int bucketIdx);
    int getSplitIdx(int bucketIdx);
    void increaseGlobalDepth();
    void print_dir();
};