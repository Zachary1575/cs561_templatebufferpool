#include <sstream>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <fstream>

#include "workload_generator.h"
#include "parameter.h"
using namespace std;

int WorkloadGenerator::generateWorkload() {
  
  // This method generates the workload
  // However, this considers pinning is disabled.
  // If you want to enforce pin/unpin, you might need to modify this file

  ofstream workload_file;
  workload_file.open("workload.txt");
  Simulation_Environment* _env = Simulation_Environment::getInstance();
  int pageId;
  int endPageId = _env->disk_size_in_pages*(_env->skewed_data_perct/100);

  //cout << "Printing Workload..." << endl;
  for (long i = 0; i < _env->num_operations; i++) {
    //srand(time(0));

    int typeDecider = rand() % 100;  
    int skewed = rand() % 100;

    if (skewed < _env->skewed_perct)
    {
      pageId = rand() % endPageId;
    }
    else
    {
      pageId = rand() % (_env->disk_size_in_pages - endPageId) + endPageId;
    }


    if(typeDecider < _env->perct_reads)
    {
      workload_file << "R " << pageId << std::endl;
    }
    else 
    {
      workload_file << "W " << pageId << std::endl;
    }
  }
  
  workload_file.close();

  return 1;
}