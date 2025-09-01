#include "microbench-power-wrapper.h"
#include <iostream>

using namespace mb;
using namespace std;

mb::PowerWrapper::PowerWrapper(){
    cout << "Power Wrapper for NVIDIA" << endl;
}

void mb::PowerWrapper::Start(){
    cout << "Start" << endl;
}

void mb::PowerWrapper::Stop(){
    cout << "End" << endl;
}