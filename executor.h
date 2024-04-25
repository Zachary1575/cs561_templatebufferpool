 
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

      int getPageID() const;
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

      DoublyLinkedList_Hashmap_LRU_Cache(int cap) : head(nullptr), tail(nullptr), current_page_cnt(0), page_capacity(cap) {}
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
      void prepend(T val, int pageId) 
      {
          if (current_page_cnt + 1 > page_capacity) { // If we add another page, if we go over count
            printf("We hit page_capacity at: %d\n!", current_page_cnt);
            // Then we need to perform LRU eviction
          }

          // We add the page.
          printf("Added Page: %d\n", pageId);
          Node<T>* newNode = new Node<T>(val);
          if (head == nullptr) { // Empty list
              head = tail = newNode;
          } else {
              newNode->next = head;
              head->prev = newNode;
              head = newNode;
          }
          map[pageId] = newNode; // Map<Query (as string), Node Reference>, later this is useful for lookups
      }

      // Buffer LOOKUP/ACCESSOR, if successful, it WILL rearrange the Doubly Linked List (as according to LRU Principles)!
      // Returns the Page for processing, otherwise it returns 'nullptr'.
      Page* lookupInBuffer(int pageId) 
      {
        if (getPage(pageId) == nullptr) // Cache Miss
        {
          printf("Cache Missed on Page: %d!\n", pageId);
          return nullptr;
        } 
          else 
        {
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

      // Probably not gonna use this func.
      // Just here just in case...
      // void append(T val) {
      //     Node<T>* newNode = new Node<T>(val);
      //     if (tail == nullptr) { // Empty list
      //         head = tail = newNode;
      //     } else {
      //         tail->next = newNode;
      //         newNode->prev = tail;
      //         tail = newNode;
      //     }
      // }

      // // Probably not gonna use this func.
      // // Just here just in case...
      // void insertAfter(Node<T>* prevNode, T val) {
      //     if (prevNode == nullptr) {
      //         return; // Ideally throw an exception or handle this case
      //     }

      //     Node<T>* newNode = new Node<T>(val);
      //     newNode->next = prevNode->next;
      //     newNode->prev = prevNode;
          
      //     if (prevNode->next == nullptr) { // Inserting at the end
      //         tail = newNode;
      //     } else {
      //         prevNode->next->prev = newNode;
      //     }
      //     prevNode->next = newNode;
      // }

      // void remove(T val) {
      //     Node<T>* currentNode = head;
      //     while (currentNode != nullptr) {
      //         if (currentNode->data == val) {
      //             if (currentNode == head && currentNode == tail) { // Single node case
      //                 head = tail = nullptr;
      //             } else if (currentNode == head) { // Removing the head
      //                 head = head->next;
      //                 head->prev = nullptr;
      //             } else if (currentNode == tail) { // Removing the tail
      //                 tail = tail->prev;
      //                 tail->next = nullptr;
      //             } else { // Removing from middle
      //                 currentNode->prev->next = currentNode->next;
      //                 currentNode->next->prev = currentNode->prev;
      //             }

      //             delete currentNode;
      //             return;
      //         }
      //         currentNode = currentNode->next;
      //     }
      // }

      // Utility Functions
      void display() const {
        Node<T>* currentNode = head;
        while (currentNode != nullptr) {
            std::cout << currentNode->data.getPageData() << " " << "->" << " ";
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
      bool simulation_disk;
      DoublyLinkedList_Hashmap_LRU_Cache<Page> bufferpool_LRU;

      static long max_buffer_size;  //in pages
      
      static Buffer* getBufferInstance(Simulation_Environment* _env);
      static int determineBufferSize(Simulation_Environment* _env);

      static int buffer_hit;
      static int buffer_miss;
      static int read_io;
      static int write_io;

      int LRU();
      int LRUWSR();

      static int printBuffer();
      static int printStats();
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
