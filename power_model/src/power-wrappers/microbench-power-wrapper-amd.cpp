#include "microbench-power-wrapper.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

#include <rocm_smi/rocm_smi.h>

using namespace mb;
using namespace std;

mb::PowerWrapper::PowerWrapper(){};

mb::PowerWrapper::PowerWrapper(int interval){
    cout << "Power Wrapper for AMD!" << endl;

    loop_interval = interval;

    handleReturn(rsmi_init(0));

    // Get the number of devices
    uint32_t num_devices;
    handleReturn(rsmi_num_monitor_devices(&num_devices));
    device_count = num_devices;

    // Initialize data storage
    power_measurement = new uint64_t[device_count]();
    energy_measurement = new float[device_count]();
    loop_power_values = new std::list<uint64_t>[device_count]();
}

void mb::PowerWrapper::Start(){
    loop_cancel = false;            
    for (int i = 0; i < device_count; i++)
        loop_power_values[i].clear();                  
    loop_timestamp_values.clear();
    
    addMeasurement();
    loop_thread = thread(&PowerWrapper::loop, this);
}

void mb::PowerWrapper::Stop(){
    loop_cancel = true;
    loop_thread.join();

    addMeasurement();
    calculateDuration();
    calculateEnergy();
}

void mb::PowerWrapper::Print(){
    cout << "POWER COUNTERS: Measured for " << measurement_duration << " s and collected " << loop_timestamp_values.size() << " samples!" << endl;
    for (int i = 0; i < device_count; i++){
        float energy_per_second = energy_measurement[i] / measurement_duration;

        std::cout << "\tENERGY:device=" << i << ": " << energy_measurement[i] << " J => " << energy_per_second << " J/s" << std::endl;                
    }
}

string mb::PowerWrapper::GetCsvHeader(){
    string line_str = "duration,samples,";
    
    for (int i = 0; i < device_count; i++){
        line_str += "ENERGY:device=" + to_string(i) + "," + "ENERGY_PER_SECOND:device=" + to_string(i) + ",";          
    }

    return line_str;
}

string mb::PowerWrapper::GetCsvLine(){           
    string line_str = to_string(measurement_duration) + "," + to_string(loop_timestamp_values.size()) + ",";
    
    for (int i = 0; i < device_count; i++){
        float energy_per_second = energy_measurement[i] / measurement_duration;
        line_str += to_string(energy_measurement[i]) + "," + to_string(energy_per_second) + ",";          
    }

    return line_str;
}

void mb::PowerWrapper::WritePowerCsv(std::string path){
    ofstream csv_file;
    csv_file.open(path);

    // Input header
    string header = "id,timestamp,";
    for (size_t i = 0; i < device_count; i++){
        header += "power:device=" + to_string(i) + ",";
    }
    header.pop_back();
    csv_file << header << endl;

    // Input lines
    int loop_count = loop_timestamp_values.size();
    for (int j = 0; j < loop_count; j++){

        auto t = *next(loop_timestamp_values.begin(), j);
        string line = to_string(j) + "," + to_string(t) + ",";
        
        for (int i = 0; i < device_count; i++){
            int p = *next(loop_power_values[i].begin(), j);
            line += to_string(p) + ",";
        }
        line.pop_back();
        csv_file << line << endl;
    }

    csv_file.close();
}

void mb::PowerWrapper::loop(){
    if (!loop_cancel){
        int duration = loop_interval - loopCallback();                
        this_thread::sleep_for(std::chrono::microseconds(duration));                
        loop();
    }
}

int mb::PowerWrapper::loopCallback(){            
    auto start = chrono::high_resolution_clock::now().time_since_epoch().count();
    addMeasurement();    
    auto stop = chrono::high_resolution_clock::now().time_since_epoch().count();            
    return (stop - start) / 1000;
}

void mb::PowerWrapper::addMeasurement(){
    measurePower();
    auto timestamp = chrono::high_resolution_clock::now().time_since_epoch().count();

    // Store measurement results
    for (int i = 0; i < device_count; i++){
        loop_power_values[i].push_back(power_measurement[i]);
    }
    loop_timestamp_values.push_back((long)timestamp);
}

void mb::PowerWrapper::calculateDuration(){
    long t1 = loop_timestamp_values.front() / 1000000;
    long t2 = loop_timestamp_values.back()  / 1000000;
    measurement_duration = (float)(t2 - t1) / 1000;
}

void mb::PowerWrapper::calculateEnergy(){
    int loop_count = loop_timestamp_values.size();

    for (int i = 0; i < device_count; i++)
    {
        float device_energy = 0.0;
        
        for (int j = 0; j < loop_count - 1; j++)
        {
            int p1 = *next(loop_power_values[i].begin(), j) / 1000000;
            int p2 = *next(loop_power_values[i].begin(), j + 1) / 1000000;

            int t1 = *next(loop_timestamp_values.begin(), j) / 1000000;
            int t2 = *next(loop_timestamp_values.begin(), j + 1) / 1000000;

            float time_dif = (float)(t2 - t1) / 1000;
            int abs_dif = abs(p2 - p1);
            int min_val = p1 < p2 ? p1 : p2;

            device_energy += time_dif * min_val + (time_dif * abs_dif) / 2; 
        }
        energy_measurement[i] = device_energy;
    }
}

void mb::PowerWrapper::measurePower(){
    RSMI_POWER_TYPE* type = new RSMI_POWER_TYPE[device_count]();
    for (int i = 0; i < device_count; i++){        
        handleReturn(rsmi_dev_power_get(i, &power_measurement[i], &type[i]));
    }
}

void mb::PowerWrapper::handleReturn(int retval){
    if (retval == RSMI_STATUS_NOT_SUPPORTED){
        cout << "ROCm SMI Error: Status not supported!" << endl;
        exit(1);
    } else if (retval == RSMI_STATUS_INVALID_ARGS){
        cout << "ROCm SMI Error: Status invalid arguments!" << endl;
        exit(1);
    } else if (retval == RSMI_STATUS_UNEXPECTED_DATA){
        cout << "ROCm SMI Error: Status unexpected data!" << endl;
        exit(1);
    }  else if (retval != RSMI_STATUS_SUCCESS) {
        cout << "ROCm SMI Error: Number " << retval << endl;
        exit(1);
    } 
}