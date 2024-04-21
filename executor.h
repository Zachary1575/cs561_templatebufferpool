 
#include "parameter.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

namespace bufmanager {

  // Page Implementation
  class Page {
    private:
      int pageId;
      int offset;
      string entry;
    public:
      Page(int pageId, int offset, string entry); // Page Constructor

      int getPageID() const;
      int getOffset() const;
      string getEntry() const;
      string getPageData() const;
  };

  // === This can be replaced with a more sophisticated Data Structure ===
  // Then we do a Linked List implementation so we can move onto a Skip List Implementation
  // ---------------------- Linked List ----------------------
  template <typename T>
  class Node {
    public:
        T data;
        Node* next;

        Node(T data) : data(data), next(nullptr) {} // Node Constructor
  };

  template <typename T>
  class LinkedList {
    public:
        Node<T>* head;
        int current_pages;
        int buffer_max_page_cnt;

        LinkedList(int max_buffer_pages) : head(nullptr), current_pages(0), buffer_max_page_cnt(max_buffer_pages) {} // Linked List Constructor
        ~LinkedList(); // Destructor

        void insertFront(T data);
        void insertBack(T data);
        void deleteValue(T data);
        void display() const;
  };

  template <typename T>
  LinkedList<T>::~LinkedList() { // Deconstructor 
      Node<T>* current = head; // Access the head
      while (current != nullptr) {
          Node<T>* next = current->next;
          delete current; // Delete each of the node objects
          current = next;
      }
  }

  template <typename T>
  void LinkedList<T>::insertFront(T data) { // insert front of LL (preffered)
      Node<T>* newNode = new Node<T>(data);
      newNode->next = head;
      head = newNode;
  }

  template <typename T>
  void LinkedList<T>::insertBack(T data) { // Insert back of LL
      Node<T>* newNode = new Node<T>(data);
      if (head == nullptr) {
          head = newNode;
      } else {
          Node<T>* current = head;
          while (current->next != nullptr) {
              current = current->next;
          }
          current->next = newNode;
      }
  }

  template <typename T>
  void LinkedList<T>::deleteValue(T data) { // Deletion value based on **Page** implementation
      Node<T>* current = head;
      Node<T>* previous = nullptr;
      while (current != nullptr && current->data != data) {
          previous = current;
          current = current->next;
      }
      if (current == nullptr) return; // data not found
      if (previous == nullptr) {
          head = current->next;
      } else {
          previous->next = current->next;
      }
      delete current;
  }

  template <typename T>
  void LinkedList<T>::display() const { // Display LL based on **Page** implementation
      Node<T>* current = head;
      while (current != nullptr) {
          std::cout << current->data.getPageData() << " -> ";
          current = current->next;
      }
      std::cout << "null" << std::endl;
  }

  // ------------------------------------------------
  class Buffer {
    private:
      // Change this according to the data structure your using...
      LinkedList<Page> bufferpool;

      Buffer(Simulation_Environment* _env);
      static Buffer* buffer_instance;

    public:
      static long max_buffer_size;  //in pages
      
      static Buffer* getBufferInstance(Simulation_Environment* _env);

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
