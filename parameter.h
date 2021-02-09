
#ifndef PARAMETER_H_
#define PARAMETER_H_
 

#include <iostream>

using namespace std;

class Simulation_Environment
{
private:
  Simulation_Environment(); 
  static Simulation_Environment *instance;

public:
  static Simulation_Environment* getInstance();

  int buffer_size_in_pages;   // b
  int disk_size_in_pages;     // n

  int num_operations;    // x
  int perct_reads;       // e
  int perct_writes;      

  float skewed_perct;      //s
  float skewed_data_perct; //d

  int pin_mode;   //p

  int verbosity;         // v
  int algorithm;         // a

};

#endif /*PARAMETER_H_*/

