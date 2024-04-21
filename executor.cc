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
{
  printf("%s\n", "Creating Buffer (Linked List)...");

  cout << "Buffer Size in Pages: " << _env->buffer_size_in_pages << endl;
  cout << "Disk Size in Pages: " << _env->disk_size_in_pages << endl;
  cout << "Entry Size: " << _env->entry_size << endl;

  cout << "Number of Operations: " << _env->num_operations << endl;
  cout << "Percentage Reads: " << _env->perct_reads << '%' << endl;
  cout << "Percentage Writes: " << _env->perct_writes << '%' << endl;

  cout << "Skewed Percentage: " << _env->skewed_perct << endl;
  cout << "Skewed Data Percentage: " << _env->skewed_data_perct << endl;

  cout << "Pin Mode: " << ( _env->pin_mode ? "Enabled" : "Disabled") << endl;
  cout << "Verbosity Level: " << _env->verbosity << endl;
  cout << "Algorithm: " << _env->algorithm << endl;

  cout << "Simulation on Disk: " << (_env->simulation_on_disk ? "Yes" : "No") << endl;


  LinkedList<Page> buffer_LL(_env->buffer_size_in_pages);
  bufferpool = buffer_LL;
  printf("%s\n", "Done!");

  // Page p1 = Page(1,123, "asdads");
  // Page p2 = Page(2,456, "kajgsd");
  // Page p3 = Page(3,789, "dflighdo");

  // buffer_LL.insertFront(p1);
  // buffer_LL.insertFront(p2);
  // buffer_LL.insertFront(p3);

  // buffer_LL.display();  // "null", Should be null
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
  //printf("\nRead Instruction Issued: (pageID~%d, offset~%d, algorithm~%d)\n", pageId, offset, algorithm);

  return -1;
}


int WorkloadExecutor::write(Buffer* buffer_instance, int pageId, int offset, const string new_entry, int algorithm)
{
  //printf("\nWrite Instruction Issued: (pageID~%d, offset~%d, entry~%s, algorithm~%d)\n", pageId, offset, new_entry.c_str(), algorithm);

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
Page::Page(int pageId, int offset, string entry) 
: pageId(pageId), offset(offset), entry(entry)
{} // Unless you need to something special, we can keep this blank

// ---- Page Instance Methods ----
std::string Page::getPageData() const {
  ostringstream oss;
  oss << "PageID: " << pageId << ", Offset: " << offset << ", Entry Data:" << entry;
  std::string result = oss.str();
  return result;
}

int Page::getPageID() const 
{
    return pageId;
}
int Page::getOffset() const 
{
    return offset;
}
std::string Page::getEntry() const 
{
    return entry;
}