#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "parameter.h"
#include "executor.h"

using namespace std;
using namespace bufmanager;

// ---------------- Buffer ---------------- 

Buffer *Buffer::buffer_instance;
long Buffer::max_buffer_size = 0;
int Buffer::buffer_hit = 0;
int Buffer::buffer_miss = 0;
int Buffer::read_io = 0;
int Buffer::write_io = 0;

Buffer::Buffer(Simulation_Environment *_env)
: bufferpool_LRU(determineBufferSize(_env))
{
  algorithm = _env->algorithm;
  simulation_disk = _env->simulation_on_disk;

  // I keep forgetting what each is each
  cout << "Buffer Size in Pages: " << _env->buffer_size_in_pages << endl;
  cout << "Disk Size in Pages: " << _env->disk_size_in_pages << endl;
  cout << "Entry Size: " << _env->entry_size << endl;

  cout << "Number of Operations: " << _env->num_operations << endl;
  cout << "Percentage Reads: " << _env->perct_reads << '%' << endl;
  cout << "Percentage Writes: " << _env->perct_writes << '%' << endl;

  cout << "Pin Mode: " << ( _env->pin_mode ? "Enabled" : "Disabled") << endl;
  cout << "Verbosity Level: " << _env->verbosity << endl;
  cout << "Algorithm: " << _env->algorithm << endl;

  cout << "Simulation on Disk: " << (_env->simulation_on_disk ? "Yes" : "No") << endl;
}

int Buffer::determineBufferSize(Simulation_Environment* _env) 
{
  if (_env->algorithm == 1) {
      return _env->buffer_size_in_pages;
  } else {
      return 0;  // Example: default or minimum size for other algorithms
  }
}

Buffer *Buffer::getBufferInstance(Simulation_Environment *_env)
{
  if (buffer_instance == 0)
    printf("%s\n", "No Buffer Pool Detected. Creating a New Buffer Pool.");
    buffer_instance = new Buffer(_env);
  return buffer_instance;
}

int Buffer::LRU()
{
  int index = 0;
  
  // Implement LRU
  // This is the LRU algorithm.

  return index;
}

int Buffer::LRUWSR()
{
  // Implement LRUWSR
  return -1;
}

int Buffer::printBuffer()
{
  return -1;
}

int Buffer::printStats()
{
  Simulation_Environment* _env = Simulation_Environment::getInstance();
  cout << "******************************************************" << endl;
  cout << "Printing Stats..." << endl;
  cout << "Number of operations: " << _env->num_operations << endl;
  cout << "Buffer Hit: " << buffer_hit << endl;
  cout << "Buffer Miss: " << buffer_miss << endl;
  cout << "Read IO: " << read_io << endl;
  cout << "Write IO: " << write_io << endl;  
  cout << "Global Clock: " << endl;
  cout << "******************************************************" << endl;
  return 0;
}

// ---------------- Workload Executer ---------------- 

int WorkloadExecutor::search(Buffer* buffer_instance, int pageId)
{
  return -1;
  // Implement Search in the Bufferpool
}

int WorkloadExecutor::read(Buffer* buffer_instance, int pageId, int offset, int algorithm)
{
  printf("\nRead Instruction Issued: (pageID~%d, offset~%d, algorithm~%d)\n", pageId, offset, algorithm);
  
  if (algorithm == 1) { // LRU
    DoublyLinkedList_Hashmap_LRU_Cache<Page> bufferpool = buffer_instance->bufferpool_LRU;
    
    // Search in Hashmap to see if that pageID is made...
    Page* cache_page = bufferpool.lookupInBuffer(pageId);
    if (cache_page == nullptr) // If we yield no such page in the Bufferpool
    {
      // Buffer MISS
      Buffer::buffer_miss++;
      if (buffer_instance->simulation_disk) // DISK SIMULATION
      {
        
      } 
        else // NOT DISK SIMULATION
      {
        Page x = Page(pageId); // Create a dummy page | Here we simulate us going into disk and getting the associated page.
        bufferpool.prepend(x, pageId); // Insert the page into the Buffer pool
      }

      return -1;
    } 
      else 
    {
      // Buffer HIT
      Buffer::buffer_hit++;
      if (buffer_instance->simulation_disk) // DISK SIMULATION
      {

      } 
        else // NOT DISK SIMULATION
      { 
        printf("[NOT DISK SIMULATION] Getting Entry and Pretending to Return Data to Satisfy Query: %d, %d.\n", pageId, offset);
      }
    }
  }

  return -1;
}

int WorkloadExecutor::write(Buffer* buffer_instance, int pageId, int offset, const string new_entry, int algorithm)
{
  printf("\nWrite Instruction Issued: (pageID~%d, offset~%d, entry~%s, algorithm~%d)\n", pageId, offset, new_entry.c_str(), algorithm);

  return 1;
}

int WorkloadExecutor::unpin(Buffer* buffer_instance, int pageId)
{
  // This is optional
  return -1;
}


// ---------------- Page (4KB) ---------------- 
// I asked Papon about this: 
// We can assume that the entry sizes are within 4KB (it won't spill over a page)
// ---- Page Constructor ----
Page::Page(int pageId)
: pageId(pageId), dirty(false)
{
  std::fill_n(pageContent, 4096, '0'); // Fill it up with 0. We know all entries are alphebetical data.
}

// ---- Page Instance Methods ----
void Page::insertEntryIntoPage(int offset, string entry) 
{
  int index = offset;
  for (char c: entry) {
    pageContent[index] = c;
    index++;
  }
}

void Page::printAllPageEntries() 
{
  std::string overall_string = "";
  for (char c: pageContent) {
    if (c != '0') {
      overall_string += c;
    }
  }
  printf("\n\nPAGE %d: %s \n", pageId, overall_string.c_str());
}

int Page::getPageID() const 
{
    return pageId;
}