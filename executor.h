 
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
      bool dirty; // Dirty Page - CFLRU Integration
      bool cold; // Cold Page - LRU-WSR Integration

    public:
      Page(int pageId); // Page Constructor

      char* getPageContent();
      bool isDirtyPage() const; //CFLRU Integration
      int getPageID() const;
      void setDirtyPage(bool dirty); //CFLRU Integration
      void printAllPageEntries();
      void insertEntryIntoPage(int offset, string entry);
      
      void setColdFlag(bool flag); //LRU-WSR Integration
      bool isCold() const; //LRU-WSR Integration

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
      int instructions_seen;

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

      void prepend(T val, int pageId, bool Disk_Simulation, int bufferMiss) 
      {
          if (current_page_cnt + 1 > page_capacity) { // If we add another page, if we go over count
            printf("We hit page_capacity at: %d!\n", current_page_cnt);
            printf("Total Istructions Seen is: %d!\n", instructions_seen);

            // Dynamic way to calculate eviction amount. If our cache is in a locality/useful for that part of computation, we have low eviction rate
            const double E_MIN = 0.01;  // Minimum eviction rate (1%)
            const double E_MAX = 0.25;  // Maximum eviction rate (25%)
            const double SCALE_FACTOR = 0.25;  // Scaling factor
            double missRate = (bufferMiss/instructions_seen); // Miss rate Buffer Miss / Instructions Seened so far...
            double scaledEvictionRate = SCALE_FACTOR * missRate;
            double evictionRate = std::max(E_MIN, std::min(E_MAX, scaledEvictionRate)); // Capped from 1% - 25% Eviction 
            int pages_to_evict = page_capacity * evictionRate; // Apply our eviction rate to our page capacity
            if (pages_to_evict == 0) { // Guard for 0 page evictions
              pages_to_evict = 1;
            }
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
              // cout << "[DEBUG] Flushing Dirty Page to DB! Page ID: " << pageId << endl;
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
      static const int window_size = 10;

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
      int LRUWSR(); //unused

      static int printBuffer();
      static int printStats();
      static int printBufferStats(Buffer* buffer_instance);

      //******CFLRU METHOD - PAGE REFERENCE *****//
      // Added Disk I/O Operations

      void cflruReferPage(int pageId, bool isWrite) {
        Node<Page>* node = bufferpool_LRU.getPage(pageId);

        if (node == nullptr) {  // Page miss
            buffer_miss++;
            cout << "[CFLRU] Page miss for: " << pageId << ". Total misses: " << buffer_miss << endl;

            // Check capacity before reading in a new page
            if (bufferpool_LRU.current_page_cnt >= max_buffer_size) {
                cout << "[CFLRU] Buffer full. Evicting pages..." << endl;
                cflruEvictPage(); 
            }

            // Simulate disk read and prepend new page
            Page newPage = simulateDiskRead(pageId);
            newPage.setDirtyPage(isWrite); // Mark dirty if it's a write operation
            bufferpool_LRU.prepend(newPage, pageId, simulation_disk, Buffer::buffer_miss);
            bufferpool_LRU.display();
            if (!isWrite) read_io++; // Increment read I/O if it's a read operation
        } else {  // Page hit
            buffer_hit++;
            cout << "[CFLRU] Buffer hit for page: " << pageId << ". Total hits: " << buffer_hit << endl;
            if (isWrite && !node->data.isDirtyPage() && simulation_disk) {
                node->data.setDirtyPage(true);
                cout << "[CFLRU] Page " << pageId << " set to dirty due to write operation." << endl;
            }
            moveFront(node); // Move to front to mark as most recently used
        }
      }

      //******CFLRU METHOD - PAGE EVICTION *****//

      void cflruEvictPage() {
        int evictedCount = 0;
        Node<Page>* current = bufferpool_LRU.tail;

        // First pass: Evict clean pages within the window size limit
        while (current != nullptr && evictedCount < window_size && !current->data.isDirtyPage()) {
            Node<Page>* toDelete = current;
            current = current->prev;
            if (!toDelete->data.isDirtyPage()) {
                removeNode(toDelete);  // Custom method to remove a node from DLL and free memory
                evictedCount++;
                std::cout << "[CFLRU Eviction] Evicting clean page: " << toDelete->data.getPageID() << std::endl;
            }
        }
      }




      //******LRU-WSR METHOD - PAGE EVICTION *****//
      void wsrReferPage(int pageId, bool isWrite) {
        Node<Page>* node = bufferpool_LRU.getPage(pageId);

        if (node == nullptr) { // Page miss - increase the counter
          buffer_miss++;
          std::cout << "Page miss for: " << pageId << std::endl;

        if (bufferpool_LRU.current_page_cnt >= max_buffer_size) {
            wsrEvictPage();
        }

        Page newPage = simulateDiskRead(pageId);
        newPage.setDirtyPage(isWrite);
        bufferpool_LRU.prepend(newPage, pageId, simulation_disk, Buffer::buffer_miss);

        if (!isWrite && simulation_disk) {
            read_io++; // Only when we simulate on disk, read_io is incremented for read operations
        }

        if (isWrite && simulation_disk) {
            write_io++; // Only when we simulate on disk, write_io is incremented for write operations
        }
        } else { // Page hit - increase the counter
          buffer_hit++;
          std::cout << "Buffer hit for page: " << pageId << std::endl;
          if (isWrite && !node->data.isDirtyPage()) {
              node->data.setDirtyPage(true);
              if (simulation_disk) {
                  // write_io only increases when making a clean page dirty involves disk access
                  write_io++; 
              }
          }
          moveFront(node);
      }
  }

      void wsrEvictPage() {
        Node<Page>* current = bufferpool_LRU.tail;
        bool foundEvictable = false;

        // First pass: Try to find a non-cold, dirty page to evict
        while (current != nullptr && !foundEvictable) {
            if (!current->data.isCold() && current->data.isDirtyPage()) {
                std::cout << "[LRU-WSR Eviction] Evicting dirty, non-cold page: " << current->data.getPageID() << std::endl;
                removeNode(current);
                foundEvictable = true;
            }
            current = current->prev;
        }

        // Second pass: If no suitable page found, reset cold flags and evict the least recently used page
        if (!foundEvictable) {
            current = bufferpool_LRU.tail;
            while (current != nullptr) {
                if (current->data.isCold()) {
                    current->data.setColdFlag(false);  // Reset cold flag
                }
                current = current->prev;
            }
            // Evict the least recently used page
            if (bufferpool_LRU.tail != nullptr) {
                std::cout << "[LRU-WSR Eviction] Evicting the least recently used page: " << bufferpool_LRU.tail->data.getPageID() << std::endl;
                removeNode(bufferpool_LRU.tail);
            }
        }
    }




      //******HELPER FUNCTION - REMOVE NODE *****//
      void removeNode(Node<Page>* node) {
        if (node->prev) node->prev->next = node->next;
        if (node->next) node->next->prev = node->prev;
        if (node == bufferpool_LRU.head) bufferpool_LRU.head = node->next;
        if (node == bufferpool_LRU.tail) bufferpool_LRU.tail = node->prev;

        bufferpool_LRU.map.erase(node->data.getPageID());
        delete node;
    }

    //******HELPER FUNCTION - WRITE BACK TO DISK *****//
      void writeBackToDisk(Page& page) {
        cout << "[DEBUG] Writing back dirty page to disk: Page ID: " << page.getPageID() << endl;

        // This is the path to the simulated disk file, set up the fsstream object
        string datFilePath = "./rawdata_database.dat";
        fstream file(datFilePath, ios::in | ios::out | ios::binary);

        if (!file) {
            cerr << "Unable to open file for writing back page to disk." << endl;
            return;
        }

        // Calculate the byte offset in the file where this page's data should be written
        int byteStart = page.getPageID() * 4096; // 4KB page

        // Seek to the correct position in the file
        file.seekp(byteStart, ios::beg);
        if (!file) {
            cerr << "Seek failed while trying to write back dirty page to disk." << endl;
            file.close();
            return;
        }

        // Write the page content back to the disk file
        file.write(page.getPageContent(), 4096); // Writing the entire page content (4096 bytes)
        if (!file) {
            cerr << "Write failed while writing back dirty page to disk." << endl;
        }

        file.close(); // Close the file after writing
    }



      //******HELPER FUNCTION - MOVE PAGE TO FRONT *****//
      void moveFront(Node<Page>* node) {
        if (node != bufferpool_LRU.head) { //If it is not the first page in the buffer
          if (node->prev) node->prev->next = node->next;
          if (node->next) node->next->prev = node->prev;
          if (node == bufferpool_LRU.tail) bufferpool_LRU.tail = node->prev;

          node->next = bufferpool_LRU.head;
          node->prev = nullptr;
          if (bufferpool_LRU.head) {
            bufferpool_LRU.head->prev = node;
          }
          bufferpool_LRU.head = node;
        }
      }

      //******HELPER FUNCTION - SIMULATE DISK READ - borrowed from Zach :) *****//
      Page simulateDiskRead(int pageId) {
        std::string datFilePath = "./rawdata_database.dat";
        std::fstream file(datFilePath, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) {
          std::cerr << "Unable to open file." << std::endl;
          throw std::runtime_error("Disk read simulation failed - could not open file."); 
        }

        int byteStart = pageId * 4096; // 4KB page
        file.seekg(byteStart, std::ios::beg);
        if (!file) {
          file.close();
          std::cerr << "Seek failed. Check if the position is beyond the file size." << std::endl;
          throw std::runtime_error("Disk read simulation failed - seek failed.");
        }

        Page bufferPage(pageId);
        file.read(bufferPage.getPageContent(), 4096); // Read 4KB of data into the buffer
        if (!file) {
          std::cerr << "Read failed. Check if there are enough bytes left in the file. " << std::endl;
          file.close();
          throw std::runtime_error("Disk read simulation failed - read failed.");
        }
        file.close();
        std::cout << "Disk read simulation succeeded for page: " << pageId << std::endl; // Debug print - RGVA
        return bufferPage;
      }
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