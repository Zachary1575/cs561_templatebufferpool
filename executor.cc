#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <fstream>

#include "parameter.h"
#include "executor.h"

using namespace std;
using namespace bufmanager;

Buffer *Buffer::buffer_instance;
long Buffer::max_buffer_size = 0;
int Buffer::buffer_hit = 0;
int Buffer::buffer_miss = 0;
int Buffer::read_io = 0;
int Buffer::write_io = 0;


Buffer::Buffer(Simulation_Environment *_env)
{
  // initialize accordingly
}

Buffer *Buffer::getBufferInstance(Simulation_Environment *_env)
{
  if (buffer_instance == 0)
    buffer_instance = new Buffer(_env);
  return buffer_instance;
}

int WorkloadExecutor::search(Buffer* buffer_instance, int pageId)
{
  // Implement Search in the Bufferpool
}

int WorkloadExecutor::read(Buffer* buffer_instance, int pageId, int algorithm)
{
  // Implement Read in the Bufferpool

  return 1;
}


int WorkloadExecutor::write(Buffer* buffer_instance, int pageId, int algorithm)
{
  // Implement Write in the Bufferpool

  return 1;
}

int WorkloadExecutor::unpin(Buffer* buffer_instance, int pageId)
{
  // This is optional
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
}