 
#include "parameter.h"

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map> 

using namespace std;

namespace bufmanager {

  // ------------------------------------------------ Page Implementation ------------------------------------------------
  // > Each Offset is an index within a page
  // > More to come later...
  class Page {
    private:
      int pageId;
      char pageContent[4096];
      bool dirty;

    public:
      Page(int pageId); // Page Constructor

      char* getPageContent();
      bool isDirtyPage() const;
      int getPageID() const;
      void setDirtyPage(bool dirty);
      void printAllPageEntries();
      void insertEntryIntoPage(int offset, string entry);
  };
  // ------------------------------------------------ Doubly Linked List & Hashmap for LRU ------------------------------------------------
  // EACH PAGE IS CONSIDERED AS A NODE.
  // Downside of this structure is that it takes lots of memory.
  // To ADD data to the Cache: Takes O(1) as we add the page to the very beginning of the Doubly Linked List and Hashmap (add memory reference)
  // To LOOKUP/ACCESS data in the Cache: Takes O(1) as we just need to look at a hashmap to look up the memory reference
  // To EVICT/DELETE the Cache: Takes O(k) where 'k' is how many pages to delete or nodes. This is reflected on both the Doubly Linked List and Hashmap
  // To REORDER the cache: Takes O(1) as we just look up the node, we then severe the next and prev while connecting the both nodes and then just preappend

  // Memory that this takes is O(n)...

  template<typename T>
  class Node { // Each Node should hold a page
    public:
      T data; 
      Node<T>* prev;
      Node<T>* next;

      // Constructor
      Node(T val) : data(val), prev(nullptr), next(nullptr) {}
  };


  template<typename T>
  class DoublyLinkedList_Hashmap_LRU_Cache {
    public:
      Node<T>* head;
      Node<T>* tail;
      unordered_map<int, Node<T>*> map; // The hashmap for fast lookups
      int current_page_cnt;
      int page_capacity;
      int algorithm_eviction_count;

      DoublyLinkedList_Hashmap_LRU_Cache(int cap) : head(nullptr), tail(nullptr), current_page_cnt(0), page_capacity(cap), algorithm_eviction_count(0) {}
      ~DoublyLinkedList_Hashmap_LRU_Cache() { clear();}

      void clear() {
          Node<T>* currentNode = head;
          while (currentNode != nullptr) {
              Node<T>* nextNode = currentNode->next;
              delete currentNode;
              currentNode = nextNode;
          }
          head = tail = nullptr;
      }

      // ====== Hash Map Funcs. ======
      string getPageString(string query) 
      {
        try 
        {
          return map.at(query)->data.getPageData();
        } catch (const out_of_range &e) 
        {
          return "Key_Not_Found_in_Cache!";
        }
      }

      Node<T>* getPage(int pageId) 
      {
        try {
          return map.at(pageId);
        } catch (const out_of_range &e) {
          return nullptr; // Return a Null PTR
        }
      }

      // ====== Doubly Linked List Funcs. ======
      // Adds data to the doubly linked list, O(1) append
      // HOWEVER, this function also evicts if we hit the cap.
      // THIS IS A PAGE LEVEL ADD, A NODE IS A PAGE! (val -> PAGE in DB)
      void prepend(T val, int pageId, bool Disk_Simulation) 
      {
          if (current_page_cnt + 1 > page_capacity) { // If we add another page, if we go over count
            printf("We hit page_capacity at: %d!\n", current_page_cnt);
            // Then we need to perform LRU eviction
            // We need to take cache hit rate, cache miss rate, then evict 1- 50% of the total capacity
            int pages_to_evict = page_capacity * 0.25; // For now lets pretend we evict 25% of our cache (alot!)
            if (pages_to_evict == 0) {
              pages_to_evict = 1;
            }
            printf("[LRU] We are evicting this number of pages: %d!\n", pages_to_evict);
            deleteLastXPages(pages_to_evict, Disk_Simulation);
            current_page_cnt = current_page_cnt - pages_to_evict;
            algorithm_eviction_count++;
          }

          // We add the page.
          Node<T>* newNode = new Node<T>(val);
          if (head == nullptr) { // Empty list
              head = tail = newNode;
          } else {
              newNode->next = head;
              head->prev = newNode;
              head = newNode;
          }
          map[pageId] = newNode; // Map<Query (as string), Node Reference>, later this is useful for lookups
          current_page_cnt++;
      }

      // Buffer LOOKUP/ACCESSOR, if successful, it WILL rearrange the Doubly Linked List (as according to LRU Principles)!
      // Returns the Page for processing, otherwise it returns 'nullptr'.
      Page* lookupInBuffer(int pageId) 
      {
        if (getPage(pageId) == nullptr) // Cache Miss
        {
          printf("[DEBUG] Cache Missed on Page: %d!\n", pageId);
          return nullptr;
        } 
          else 
        {
          printf("[DEBUG] Cache Hit on Page: %d!\n", pageId);
          Node<Page>* node = getPage(pageId);
          Page* bufferPage = &(node->data);
          // Then we need to rearrange the node as it was just accessed
          if (node == head) { // Node is already at the front, no need to move
            return bufferPage;
          }
          // Detach node from its current position
          if (node->prev) { 
              node->prev->next = node->next;
          }
          if (node->next) {
              node->next->prev = node->prev;
          }
          if (node == tail) { // If node is the tail, update the tail
              tail = node->prev;
          }

          // Attach node at the front of the list
          node->next = head;
          node->prev = nullptr;
          if (head) {
              head->prev = node;
          }
          head = node;

          // If the list was empty (shouldn't be the case here, but good to check)
          if (tail == nullptr) {
              tail = node;
          }

          return bufferPage;
        }
      }

      void deleteLastXPages(int x, bool Disk_Simulation) {
        if (x <= 0) { // Sanity Check & Safety: Nothing to delete if x is zero or negative
            return; 
        }

        int count = 0;
        while (tail != nullptr && count < x) {
            Node<T>* nodeToDelete = tail;
            int pageId = nodeToDelete->data.getPageID(); // Assuming Node<T> stores data that has getPageID()

            if (Disk_Simulation && (nodeToDelete->data.isDirtyPage())) {
              cout << "[DEBUG] Flushing Dirty Page to DB! Page ID: " << pageId << endl;
              // We index based off of pageID and then write the page in the .dat file.
              string datFilePath = "./rawdata_database.dat";
              std::fstream file(datFilePath, std::ios::in | std::ios::out | std::ios::binary);
              if (!file) 
              {
                std::cerr << "Unable to open file." << std::endl;
                return;
              }
              int byteStart = pageId * 4096; // 4KB page
              file.seekg(byteStart, std::ios::beg);

              if (!file) 
              {
                  std::cerr << "Seek failed. Check if the position is beyond the file size." << std::endl;
                  file.close();
                  return;
              }

              char* buffer = nodeToDelete->data.getPageContent();
              for (int i = 0; i < 4096; ++i) { // 4KB of 4096 bytes
                  if (buffer[i] != '0') {  // Write only non-zero bytes
                      // Move the file position pointer to the correct position for writing
                      file.seekp(byteStart + i, std::ios::beg);
                      if (!file) {
                          std::cerr << "Seek failed when trying to write." << std::endl;
                          file.close();
                          return;
                      }

                      // Write the byte back to the file
                      file.write(&buffer[i], 1);
                      if (!file) {
                          std::cerr << "Write failed." << std::endl;
                          file.close();
                          return;
                      }
                  }
              }

              // Close the file
              file.close();
            }
            map.erase(pageId); // Delete the hashmap entry

            tail = tail->prev;
            if (tail != nullptr) {
                tail->next = nullptr;
            } else {
                head = nullptr; // If tail is nullptr now, it means we've deleted the last node, hence list is empty
            }
            delete nodeToDelete; // Free the memory allocated to the node
            count++;
            current_page_cnt--;
        }
      }

      // Utility Functions
      // This function displays the sequence of pages in the node
      void display() const {
        Node<T>* currentNode = head;
        while (currentNode != nullptr) {
            std::cout << currentNode->data.getPageID() << " " << "->" << " ";
            currentNode = currentNode->next;
        }
        std::cout << "null" << std::endl;
      }
  };

  // --------------------------------------------------------------------------------------------------------------------
  class Buffer {
    private:
      Buffer(Simulation_Environment* _env);
      static Buffer* buffer_instance;

    public:
      int algorithm;
      int entry_size;
      bool simulation_disk;
      DoublyLinkedList_Hashmap_LRU_Cache<Page> bufferpool_LRU;

      static long max_buffer_size;  //in pages
      
      static Buffer* getBufferInstance(Simulation_Environment* _env);
      static int determineBufferSize(Simulation_Environment* _env);

      static int buffer_hit;
      static int buffer_miss;
      static int read_io;
      static int write_io;

      void LRU(int x);
      int LRUWSR();

      static int printBuffer();
      static int printStats();
      static int printBufferStats(Buffer* buffer_instance);
  };

  class Disk {
    private: 
    public:
  };


  class WorkloadExecutor {
    private:
    public:
      static int read(Buffer* buffer_instance, int pageId, int offset, int algorithm);
      static int write(Buffer* buffer_instance, int pageId, int offset, const string new_entry, int algorithm);
      static int search(Buffer* buffer_instance, int pageId);
      static int unpin(Buffer* buffer_instance, int pageId);
  };
}
