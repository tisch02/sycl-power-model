#include "microbench-papi-wrapper.h"
#include <iostream>
#include <papi.h>
#include <list>

using namespace mb;
using namespace std;

mb::PapiWrapper::PapiWrapper(){}

mb::PapiWrapper::PapiWrapper(Target t, list<int> device_ids, int event_set){
    target = t;
    event_set_selection = event_set;
    
    if (target == Target::AMD) initAmd();
    else exit(1);

    ConfigureDevices(device_ids);
    initPapi();
}

void mb::PapiWrapper::Start() {    
    initPapi();
    handleReturn(PAPI_start(host_eventset));
    handleReturn(PAPI_start(device_eventset));
}

void mb::PapiWrapper::Stop() {
    // cout << "Stop" << endl;
    handleReturn(PAPI_stop(host_eventset, host_counters));
    handleReturn(PAPI_stop(device_eventset, device_counters));
    handleReturn(PAPI_cleanup_eventset(host_eventset));
    handleReturn(PAPI_cleanup_eventset(device_eventset));
}

void mb::PapiWrapper::Print(){
    // Print Host Counter to Console            
    cout << "HOST COUNTERS:" << endl;
    int i = 0;
    for (int const& event : host_events) {
        cout << "\t" << getEventName(event) << ": " << host_counters[i] << endl;       
        i += 1; 
    }

    // Print Device Counter to Console
    cout << "DEVICE COUNTERS:" << endl;
    i = 0;
    for (string const& event : device_events) {
        cout << "\t" << event << ": " << device_counters[i] << endl;       
        i += 1; 
    }
}

string mb::PapiWrapper::GetCsvHeader(){
    string line_str = "";
    
    for (int const& event : host_events) {
        line_str += getEventName(event) + ",";
    }

    // Print Device Counter to Console    
    for (string const& event : device_events) {
        line_str += event + ",";
    }

    return line_str;
}

string mb::PapiWrapper::GetCsvLine(){           
    string line_str = "";

    int i = 0;
    for (int const& event : host_events) {
        line_str += to_string(host_counters[i]) + ",";        
        i += 1; 
    }

    i = 0;
    for (string const& event : device_events) {
        line_str += to_string(device_counters[i]) + ",";                
        i += 1; 
    }

    return line_str;
}

void mb::PapiWrapper::ConfigureDevices(std::list<int> device_ids){
    devices = device_ids;

    // Check if event set is available
    if (event_set_selection >= device_event_sets.size()){
        cout << "ERROR: There is no event set available for the selection!" << endl;
        exit(1);
    }
    
    // Create events based on selection
    device_events.clear();
    list<string> event_bases = *next(device_event_sets.begin(), event_set_selection);
    for (int device : devices){
        for (string event_base : event_bases) {
            auto ev = event_base + ":device=" + to_string(device);            
            device_events.push_back(ev);
        }
    }
}

void mb::PapiWrapper::initAmd(){
    cout << "PAPI Wrapper for AMD!" << endl;

    device_event_sets = {
        {
            "rocm:::SQC_DCACHE_HITS",
            "rocm:::SQC_DCACHE_MISSES",
            "rocm:::TCC_HIT_sum",            
            "rocm:::TCC_MISS_sum",
            "rocm:::SQ_INSTS",
            "rocm:::SQ_INSTS_VALU",
            "rocm:::SQ_INSTS_MFMA",
            "rocm:::SQ_INSTS_SALU"
        },
        {
            "rocm:::SQC_DCACHE_REQ",
            "rocm:::SQC_DCACHE_HITS",
            "rocm:::SQC_DCACHE_MISSES",
            "rocm:::SQC_DCACHE_MISSES_DUPLICATE",
            "rocm:::SQC_DCACHE_ATOMIC",            
            "rocm:::TCC_HIT_sum",            
            "rocm:::TCC_MISS_sum",
            "rocm:::TCC_READ_sum",
            "rocm:::TCC_REQ_sum"
        },
        {
            "rocm:::SQC_DCACHE_REQ_READ_1",
            "rocm:::SQC_DCACHE_REQ_READ_2",
            "rocm:::SQC_DCACHE_REQ_READ_4",
            "rocm:::SQC_DCACHE_REQ_READ_8", 
            "rocm:::SQC_DCACHE_REQ_READ_16",    
        },
        {
            "rocm:::SQ_INSTS",
            "rocm:::SQ_INSTS_VALU",
            "rocm:::SQ_INSTS_MFMA",
            "rocm:::SQ_INSTS_VMEM_WR",
            "rocm:::SQ_INSTS_VMEM_RD",
            "rocm:::SQ_INSTS_SALU",            
            "rocm:::SQ_INST_LEVEL_VMEM",
            "rocm:::SQ_INST_LEVEL_SMEM",
            "rocm:::MemUnitBusy",
            "rocm:::MemWrites32B",
            "rocm:::Wavefronts",
            "rocm:::WRITE_REQ_32B",
            "rocm:::WriteSize"
        },
        {
            "rocm:::SQ_INSTS_SMEM",
            "rocm:::SQC_ICACHE_MISSES",

        }
    };
}

void mb::PapiWrapper::initPapi(){
    // Initialize events
    handleReturn(PAPI_library_init(PAPI_VER_CURRENT));
    
    initHostEventset();    
    initDeviceEventset();
}

void mb::PapiWrapper::initHostEventset(){
    // Initialize event set
    host_eventset = PAPI_NULL;
    handleReturn(PAPI_create_eventset(&host_eventset));

    // Define host events
    host_events = {
        PAPI_TOT_INS,
        PAPI_FP_INS,
        PAPI_BR_INS,
        PAPI_VEC_INS,
        PAPI_TOT_CYC
    };

    // Add events to set
    for (int const& event : host_events) {
        handleReturn(PAPI_add_event(host_eventset, event));
    }
    
    // Allocate space for results
    host_counters = new long_long[host_events.size()]();
}

void mb::PapiWrapper::initDeviceEventset(){
    // Initialize event set
    device_eventset = PAPI_NULL;
    handleReturn(PAPI_create_eventset(&device_eventset));

    // Add events to set
    for (string const& event : device_events) {        
        handleReturn(PAPI_add_named_event(device_eventset, event.c_str()));                        
    }
    
    // Allocate space for results
    device_counters = new long_long[device_events.size()]();
}

void mb::PapiWrapper::handleReturn(int retval){
    if (!(retval == PAPI_OK || retval == PAPI_VER_CURRENT)){
        std::cout << "PAPI Error: " << retval << std::endl;
        exit(1);          
    }                
}

std::string mb::PapiWrapper::getEventName(int event){
    char name[PAPI_MAX_STR_LEN];
    handleReturn(PAPI_event_code_to_name(event, name));
    return std::string(name);
}
