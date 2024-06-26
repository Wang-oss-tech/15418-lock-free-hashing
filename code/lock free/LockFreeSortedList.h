// This library was found from here: 
//https://github.com/beneschtech/LockFreeSortedList/blob/main/LockFreeSortedList.h
// modifications:
// - find, insert, and remove take in a starting pointer and end after key out of range
// - modified it to be in increasing order
// - unique keys. only one value per key

#ifndef LOCKFREESORTEDLIST_H
#define LOCKFREESORTEDLIST_H
#include <atomic>
#include <cstdio>
#include <vector>

template <class K,class v>
class LockFreeSortedList
{
public:
  LockFreeSortedList():head(nullptr),m_size(0),m_transcount(0) {};
  ~LockFreeSortedList() {clear();}
  struct LockFreeSortedListItem
  {
    K key;
    v value;
    std::atomic<struct LockFreeSortedListItem *> next_ptr;
    LockFreeSortedListItem(K mkey,v mvalue):key(mkey),value(mvalue),next_ptr(nullptr){}
  };

  /**
   * @brief size number of items in list
   * @return number of items in list
   */
  unsigned int size() {
    return m_size.load();
  }

  /**
   * @brief transactionCount
   * @return number of transactions this collection has undergone
   */
  unsigned long transactionCount()
  {
    return m_transcount.load();
  }

  /**
   * @brief clear - clears the list
   */
  void clear() {
    struct LockFreeSortedListItem *p = head.load(),*n = nullptr;
    head = nullptr;
    while (p)
    {
      n = p->next_ptr;
      p->next_ptr = nullptr;
      delete p;
      p = n;
      if (!p) // At end, lets see if new stuff came in
      {
        p = head.load();
        head = nullptr;
      }
    }
    m_size.store(0);
    m_transcount.store(0);
  }

  /**
   * @brief findRange find a range of values given a key range
   * @param begin beginning value of key to search for <= 0 means no beginning
   * @param end ending value of key to search for <= 0 means no end
   * @return a vector of const pointers to values inside the list
   */
  bool find(K key, LockFreeSortedListItem *start, v *val, bool *scanNeeded = nullptr)
  {
    m_transcount.fetch_add(1);
    struct LockFreeSortedListItem *p = start;


    while (p)
    {
      K ik = p->key;
      // sorted so can't be in list
      if (ik > key){
        return false;
      }
      if (ik == key)
      {
        v rba(p->value);
        *val = rba;
        return true;
      } 
      p = p->next_ptr;
    }
    return false;
  }

  /**
   * @brief insert creates a new unique node in the list
   * @param key
   * @param value
   * @return true if inserted false if not
   */
  bool insert(K key,v value)
  {
    m_transcount.fetch_add(1);
    struct LockFreeSortedListItem *p = head.load();
    bool success = false;
    if (!p) // new list
    {
      head.store(new LockFreeSortedListItem(key,value));
      m_size.fetch_add(1);
      return true;
    } else if (p->key > key) { // new head node candidate
      struct LockFreeSortedListItem *newItem = new LockFreeSortedListItem(key,value);
      newItem->next_ptr = p;
      if (!head.compare_exchange_strong(p,newItem))
      {
        delete newItem;
        return insert(key,value); // eventually it will work
      }
      m_size.fetch_add(1);
      return true;
    }
    // Walk down to find value to stick it between
    while (!success)
    {
      struct LockFreeSortedListItem *n = nullptr;
      while (p)
      {
        if (p->key == key && p->value == value)
          return false;
        n = p->next_ptr;
        if (!n)
          break;
        if (n->key > key) // Next pointer is higher, so this pointer is the one to insert after
          break;
        p = n;
      }
      struct LockFreeSortedListItem *newItem = new LockFreeSortedListItem(key,value);
      newItem->next_ptr.store(n);
      // We have our new object set up pointing to the next one
      // If its changed, we just throw it away and try again.
      // compare exchange will do it atomically only if nothing has changed
      if (!p->next_ptr.compare_exchange_strong(n,newItem))
      {
        delete newItem;
        p = head.load();
      } else {
        success = true;
        m_size.fetch_add(1);
      }
    }
    return success;
  }

  /**
   * @brief remove remove a node with key and value combination
   * @param key
   * @param hints - an optional vector of pointers to internal items to try starting the search at
   */
  bool remove(K key,LockFreeSortedListItem *hints = nullptr)
  {
    m_transcount.fetch_add(1);
    struct LockFreeSortedListItem *p = head.load();
    struct LockFreeSortedListItem *n = nullptr,*i = nullptr;
    bool found = false;

    // Test for head node needing removed
    if (!found && p && p->key == key)
    {
      n = p->next_ptr;
      if (head.compare_exchange_strong(p,n))
      {
        n = p;
        found = true;
        m_size.fetch_sub(1);
      }
    }

    // Loop till we find it
    while (p && !found)
    {
      n = p->next_ptr;
      if (!n || n->key > key)
      {
        return false; // Never found and at end of list, return
      }
      if (n->key == key)
      {
        // We found it, n points to the item to remove, p points to previous
        if (!p->next_ptr.compare_exchange_strong(n,n->next_ptr)) // Item changed
        {
          p = head.load(); // start again
          continue;
        } else {
          found = true;
          m_size.fetch_sub(1);
          break; // Exchange happened, n is isolated node now
        }
      }
      p = n;
    }
    if (found){
      delete n;
      return true;
    }
    return false;
  }

  struct LockFreeSortedListItem *getNode(int key){
    m_transcount.fetch_add(1);
    struct LockFreeSortedListItem *p = head;

    while (p)
    {
      K ik = p->key;
      // sorted so can't be in list
      if (ik > key){
        return nullptr;
      }
      if (ik == key)
      {
        v rba(p->value);
        return p;
      } 
      p = p->next_ptr;
    }
    return nullptr;
  }

  // void print_list(){
  //   // m_transcount.fetch_add(1);
  //   struct LockFreeSortedListItem *p = head;
  //   printf("Printing list: \n");
  //   while (p)
  //   {
  //     K ik = p->key;
  //     v iv = p->value;
  //     printf("K: %d, v)
  //     // sorted so can't be in list
  //     if (ik > key){
  //       return nullptr;
  //     }
  //     if (ik == key)
  //     {
  //       v rba(p->value);
  //       return p;
  //     } 
  //     p = p->next_ptr;
  //   }
  //   return nullptr;
  // }

  struct LockFreeSortedListItem *headPtr() { return head; }

private:
  std::atomic<LockFreeSortedListItem *> head;
  std::atomic<unsigned int> m_size;
  std::atomic<unsigned long> m_transcount;
};
#endif // LOCKFREESORTEDLIST_H