 
#include "parameter.h"

#include <iostream>
#include <vector>

using namespace std;

namespace bufmanager {

  class Buffer {
    // This class maintains specific property of the buffer.
    // You definitely need to modify this part
    // You need to add more variables here for your implementation. For example, currently the bufferpool itself is missing

    private:
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
    static int read(Buffer* buffer_instance, int pageId, int algorithm);
    static int write(Buffer* buffer_instance, int pageId, int algorithm);
    static int search(Buffer* buffer_instance, int pageId);
    static int unpin(Buffer* buffer_instance, int pageId);
  };
}
