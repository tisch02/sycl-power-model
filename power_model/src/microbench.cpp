#include "microbench.h"
#include "microbench-papi-wrapper.h"
#include "power-wrappers/microbench-power-wrapper.h"

#include <iostream>
#include <sycl/sycl.hpp>
#include <utility>
#include <map>
#include <chrono>
#include <thread>
#include <fstream>

using namespace mb;
using namespace std;

mb::BenchmarkInfo::BenchmarkInfo(string name){
    Name = name;
}

mb::BenchmarkSuite::BenchmarkSuite(Target target, mb::DataType dataType){
    cout << "SYCL MicroBenchmark Suite!" << endl;
    
    // Create measurement tools
    papi = PapiWrapper(target);
    power = PowerWrapper(10 * 1000);

    // Configure measurement tools
    ConfigureDeviceSelection(0, mb::DeviceType::GPU);

    // Register all available benchmarks based on the data type
    if (dataType == mb::DataType::INT){
        registerBenchmarks<int>();
        datatype_name = "Integer";
    } else if (dataType == mb::DataType::FLOAT) {
        registerBenchmarks<float>();
        datatype_name = "Float";
    } else if (dataType == mb::DataType::DOUBLE) {
        registerBenchmarks<double>();
        datatype_name = "Double";
    }

    cout << "Counting a total number of " << benchmarks.size() << " benchmarks." << endl;
}

template<typename T>
void mb::BenchmarkSuite::registerBenchmarks(){
    registerBenchmark(Benchmark::INFO, &BenchmarkSuite::benchmark_info<T>, "Information");
    registerBenchmark(Benchmark::IDLE, &BenchmarkSuite::benchmark_idle<T>, "Idle");
    registerBenchmark(Benchmark::ADD, &BenchmarkSuite::benchmark_add<T>, "Add");
    registerBenchmark(Benchmark::ADD_BABEL, &BenchmarkSuite::benchmark_add_babel<T>, "Add Babel");
    registerBenchmark(Benchmark::ADD_LOCAL, &BenchmarkSuite::benchmark_add_local<T>, "Add Local");

    registerBenchmark(Benchmark::TRIAD, &BenchmarkSuite::benchmark_triad<T>, "Triad");
    registerBenchmark(Benchmark::COPY, &BenchmarkSuite::benchmark_copy<T>, "Copy");
    registerBenchmark(Benchmark::MULT, &BenchmarkSuite::benchmark_mult<T>, "Multiply");
    registerBenchmark(Benchmark::SIN, &BenchmarkSuite::benchmark_sin<T>, "Sine");
    registerBenchmark(Benchmark::SQRT, &BenchmarkSuite::benchmark_sqrt<T>, "Squareroot");
    registerBenchmark(Benchmark::LOG, &BenchmarkSuite::benchmark_sqrt<T>, "Logarithm");

    registerBenchmark(Benchmark::TEST_1, &BenchmarkSuite::benchmark_test_1<T>, "TEST 1");
    registerBenchmark(Benchmark::TEST_2, &BenchmarkSuite::benchmark_test_2<T>, "TEST 2");
    registerBenchmark(Benchmark::TEST_3, &BenchmarkSuite::benchmark_test_3<T>, "TEST 3");
    registerBenchmark(Benchmark::TEST_4, &BenchmarkSuite::benchmark_test_4<T>, "TEST 4");
    registerBenchmark(Benchmark::TEST_5, &BenchmarkSuite::benchmark_test_5<T>, "TEST 5");
}

std::string mb::BenchmarkSuite::GetBenchmarkName(Benchmark benchmark){
    for (const auto &pair : benchmarks){
        if (pair.first == benchmark){
            return pair.second.second.Name;
        }
    }
    return "";
}

void mb::BenchmarkSuite::Run(Benchmark benchmark, size_t array_size, size_t repetition_count){
    run_configuration_array_size = array_size;
    run_configuration_repetition_count = repetition_count;

    // Find requested benchmark
    for (const auto &pair : benchmarks){
        if (pair.first == benchmark){
            // Extract additional information
            int (mb::BenchmarkSuite::*func)() = pair.second.first;
            BenchmarkInfo info = pair.second.second;
            run_configuration_benchmark_name = info.Name;

            // Run benchmark
            cout << "RUN BENCHMARK: " << info.Name << " (arr: " << run_configuration_array_size << ", n: " << run_configuration_repetition_count << ")" << endl;
            int ret = (*this.*func)();

            // Abort further looping
            return;
        }        
    }
    cout << "The requested benchmark was not found. Benchmark: " << to_string(benchmark) << endl;
    exit(1);
}

void mb::BenchmarkSuite::Print(){
    cout << endl;
    papi.Print();
    power.Print();
    cout << endl;
}

void mb::BenchmarkSuite::WriteCsv(std::string path){
    ifstream f(path.c_str());
    bool exists = f.good();
    
    if (!exists){
        ofstream csv_file;
        csv_file.open(path);
        csv_file << getCsvHeader() << endl;
        csv_file << getCsvLine() << endl;
    } else {        
        ofstream csv_file(path, ios_base::app | ios_base::out);
        csv_file << getCsvLine() << endl;
    }
}

void mb::BenchmarkSuite::WritePowerCsv(std::string path){
    power.WritePowerCsv(path);
}

void mb::BenchmarkSuite::ConfigureDeviceSelection(int offset, DeviceType type){
    deviceOffset = offset;
    deviceType = type;
    papi.ConfigureDevices({deviceOffset});
}

void mb::BenchmarkSuite::ConfigureSleep(int beforeSleep, int afterSleep){
    before_sleep_duration = beforeSleep;
    after_sleep_duratin = afterSleep;
}

std::string mb::BenchmarkSuite::getCsvHeader(){
    string line_str = "benchmark,arr,n,datatype,sleep," + papi.GetCsvHeader() + power.GetCsvHeader();
    line_str.pop_back();
    return line_str;
}

std::string mb::BenchmarkSuite::getCsvLine(){
    string line_str = run_configuration_benchmark_name + "," 
    + std::to_string(run_configuration_array_size) + "," 
    + std::to_string(run_configuration_repetition_count) + "," 
    + datatype_name + ","
    + std::to_string(after_sleep_duratin) + ",";

    line_str += papi.GetCsvLine() + power.GetCsvLine();   
    line_str.pop_back();
    return line_str;
}

void mb::BenchmarkSuite::registerBenchmark(mb::Benchmark type, int (mb::BenchmarkSuite::*func)(), std::string name){
    BenchmarkInfo info(name);
    auto pair = make_pair(func, info);            
    benchmarks.insert({type, pair});
}

void mb::BenchmarkSuite::startMeasuring(){
    std::this_thread::sleep_for(std::chrono::milliseconds(before_sleep_duration));
    papi.Start();
    power.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(after_sleep_duratin));
}

void mb::BenchmarkSuite::stopMeasuring(){
    std::this_thread::sleep_for(std::chrono::milliseconds(after_sleep_duratin));
    papi.Stop();  
    power.Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(before_sleep_duration));
}

template<typename T>
T mb::BenchmarkSuite::getRandom(T min, T max){
    return min + static_cast<T>(rand()) /( static_cast <T> ((T)RAND_MAX/(max - min)));
}

sycl::queue mb::BenchmarkSuite::getQueue(){
    
    list<sycl::device> devices;

    // Filter for device type
    for (auto device : sycl::device::get_devices())
    {
        if (deviceType == mb::DeviceType::GPU && device.is_gpu() || 
            deviceType == mb::DeviceType::CPU && device.is_cpu() || 
            deviceType == mb::DeviceType::ANY)
        {
            devices.push_back(device);
        }
    } 

    // Apply selection offset
    int deviceCount = devices.size();
    if (deviceCount == 0){
        cout << "ERROR: There is no SYCL device for the current configuration." << endl;
        exit(1);
    } else if (deviceOffset >= deviceCount){
        cout << "ERROR: The device offset is too large for the number of available SYCL devices." << endl;
        exit(1);
    }

    // Create queue
    auto d = *next(devices.begin(), deviceOffset);
    return sycl::queue(d);
}

// Benchmarks =================================================================

template<typename T>
int mb::BenchmarkSuite::benchmark_info(){        
    sycl::queue q = getQueue();

    cout << endl << "----=== Information ===----" << endl << endl;
    cout << "Selected device:   " << q.get_device().get_info<sycl::info::device::name>() << endl;
    cout << "Datatype:          " << datatype_name << " (Size: " << sizeof(T) << " bytes)" << endl;
    cout << "Sleep (before):    " << before_sleep_duration << " ms" << endl;
    cout << "Sleep (after):     " << after_sleep_duratin << " ms" << endl;
    cout << endl;

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_idle(){    
    startMeasuring();
    std::this_thread::sleep_for(std::chrono::milliseconds(run_configuration_array_size * 1000)); 
    stopMeasuring();           
    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_add_babel(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);

    T rb = getRandom<T>(1.0, 2.0);
    T rc = getRandom<T>(2.0, 4.0);

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();
    
    startMeasuring();    

    for (size_t rep = 0; rep < run_configuration_repetition_count; rep++)
    {
        q.submit([&](sycl::handler& h){
            h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
                a[i] = b[i] + c[i];
            });
        });
        q.wait();
    }
    
    stopMeasuring();    

    free(a, q);
    free(b, q);
    free(c, q);

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_add(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);

    T rb = getRandom<T>(1.0, 2.0);
    T rc = getRandom<T>(2.0, 4.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();
    
    startMeasuring();    

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = b[i] + c[i];
                a[i] = a[i] + b[i];
                a[i] = a[i] + c[i];

                a[i] = b[i] + c[i];
                a[i] = a[i] + b[i];
                a[i] = a[i] + c[i];

                a[i] = b[i] + c[i];
                a[i] = a[i] + b[i];
                a[i] = a[i] + c[i];
            }
        });
    });
    q.wait();
    
    stopMeasuring();    

    free(a, q);
    free(b, q);
    free(c, q);

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_add_local(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);

    T rb = getRandom<T>(1.0, 2.0);
    T rc = getRandom<T>(2.0, 4.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();
    
    startMeasuring();    

        
    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            T Value1 = 0;
            T Value2 = 0;
            T Value3 = 0;
            T Value = 0; 
            T I1 = b[i];
            T I2 = c[i];

            #pragma unroll
            for (size_t rep = 0; rep < 700000000; rep++){
                Value1 = I1 + I2;
    	        Value3 = I1 - I2;
    	        Value1 += Value2;
    	        Value1 += Value2;
    	        Value2 = Value3 - Value1;
    	        Value1 = Value2 + Value3;
            }
            
            a[i]= Value1 + Value2;
        });
    });
    q.wait();
    
    stopMeasuring();    

    free(a, q);
    free(b, q);
    free(c, q);

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_triad(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);

    T rb = getRandom<T>(1.0, 2.0);
    T rc = getRandom<T>(2.0, 4.0);
    T scalar = getRandom<T>(0.0, 1.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();

    startMeasuring();    

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = b[i] + scalar * c[i];
                a[i] = c[i] + scalar * b[i];

                a[i] = b[i] + scalar * c[i];
                a[i] = c[i] + scalar * b[i];

                a[i] = b[i] + scalar * c[i];
                a[i] = c[i] + scalar * b[i];

                a[i] = b[i] + scalar * c[i];
                a[i] = c[i] + scalar * b[i];

                a[i] = b[i] + scalar * c[i];
                a[i] = c[i] + scalar * b[i];
            }
        });
    });
    q.wait();
    
    stopMeasuring();  
    
    free(a, q);
    free(b, q);
    free(c, q);

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_copy(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);

    T rb = getRandom<T>(1.0, 2.0);
    T rc = getRandom<T>(2.0, 4.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();

    startMeasuring();    

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = b[i];
                a[i] = c[i];

                a[i] = b[i];
                a[i] = c[i];

                a[i] = b[i];
                a[i] = c[i];

                a[i] = b[i];
                a[i] = c[i];

                a[i] = b[i];
                a[i] = c[i];
            }
        });
    });
    q.wait();
    
    stopMeasuring();    

    
    free(a, q);
    free(b, q);
    free(c, q);    

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_mult(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);

    T rb = getRandom<T>(1.0, 2.0);
    T rc = getRandom<T>(2.0, 4.0);
    T scalar = getRandom<T>(0.0, 1.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();

    startMeasuring();    

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = scalar * c[i];
                a[i] = scalar * b[i];

                a[i] = scalar * c[i];
                a[i] = scalar * b[i];

                a[i] = scalar * c[i];
                a[i] = scalar * b[i];
                
                a[i] = scalar * c[i];
                a[i] = scalar * b[i];

                a[i] = scalar * c[i];
                a[i] = scalar * b[i];
            }
        });
    });
    q.wait();
    
    stopMeasuring();  
    
    free(a, q);
    free(b, q);
    free(c, q);

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_sin(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);

    T rb = getRandom<T>(1.0, 2.0);
    T rc = getRandom<T>(2.0, 4.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();
    
    startMeasuring();    

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = sin(b[i]);
                a[i] = sin(c[i]);

                a[i] = sin(b[i]);
                a[i] = sin(c[i]);

                a[i] = sin(b[i]);
                a[i] = sin(c[i]);

                a[i] = sin(b[i]);
                a[i] = sin(c[i]);

                a[i] = sin(b[i]);
                a[i] = sin(c[i]);
            }
        });
    });
    q.wait();
    
    stopMeasuring();    

    free(a, q);
    free(b, q);
    free(c, q);

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_sqrt(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);

    T rb = getRandom<T>(1.0, 2.0);
    T rc = getRandom<T>(2.0, 4.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();
    
    startMeasuring();    

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = sqrt(b[i]);
                a[i] = sqrt(c[i]);

                a[i] = sqrt(b[i]);
                a[i] = sqrt(c[i]);

                a[i] = sqrt(b[i]);
                a[i] = sqrt(c[i]);

                a[i] = sqrt(b[i]);
                a[i] = sqrt(c[i]);

                a[i] = sqrt(b[i]);
                a[i] = sqrt(c[i]);
            }
        });
    });
    q.wait();
    
    stopMeasuring();    

    free(a, q);
    free(b, q);
    free(c, q);

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_log(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);

    T rb = getRandom<T>(1.0, 2.0);
    T rc = getRandom<T>(2.0, 4.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();
    
    startMeasuring();    

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = log(b[i]);
                a[i] = log(c[i]);

                a[i] = log(b[i]);
                a[i] = log(c[i]);

                a[i] = log(b[i]);
                a[i] = log(c[i]);

                a[i] = log(b[i]);
                a[i] = log(c[i]);

                a[i] = log(b[i]);
                a[i] = log(c[i]);
            }
        });
    });
    q.wait();
    
    stopMeasuring();    

    free(a, q);
    free(b, q);
    free(c, q);

    return 0;
}

// Test Kernels =================================================================

template<typename T>
int mb::BenchmarkSuite::benchmark_test_1(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);    

    T rb = getRandom<T>(4.0, 36.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;            
        });
    });
    q.wait();

    startMeasuring();    

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = b[i] + b[i];
                a[i] = a[i] + b[i];
                a[i] = a[i] + sin(b[i]);
            }
        });
    });
    q.wait();
    
    stopMeasuring(); 
    
    free(a, q);
    free(b, q);    

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_test_2(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);    
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);    

    T rb = getRandom<T>(4.0, 36.0);
    T rc = getRandom<T>(4.0, 36.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();

    startMeasuring();

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = b[i];
                a[i] = a[i] + sin(b[i]);
                a[i] = a[i] + sin(b[i]);
                a[i] = a[i] + log(c[i]);
                a[i] = a[i] + sqrt(b[i]);
            }
        });
    });
    q.wait();

    stopMeasuring();
    
    free(a, q);
    free(b, q);    
    free(c, q);    

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_test_3(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);    
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);    

    T rb = getRandom<T>(4.0, 36.0);
    T rc = getRandom<T>(4.0, 36.0);
    T scalar = getRandom<T>(0.0, 1.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();

    startMeasuring();

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = c[i] + scalar * b[i];
                a[i] = a[i] + scalar * b[i];
                a[i] = scalar * sin(b[i]);
                a[i] = scalar * log(b[i]);
            }
        });
    });
    q.wait();

    stopMeasuring();
    
    free(a, q);
    free(b, q);    
    free(c, q);    

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_test_4(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);    
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);   
    T* d = sycl::malloc_shared<T>(run_configuration_array_size, q);    

    T rb = getRandom<T>(1.0, 2.0);
    T rc = getRandom<T>(2.0, 3.0);
    T rd = getRandom<T>(4.0, 10.0);
    T scalar = getRandom<T>(0.0, 1.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();

    startMeasuring();

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = b[i];
                a[i] = a[i] + scalar * b[i];
                a[i] = c[i];
                a[i] = d[i];
                a[i] = a[i] + d[i] * b[i];

                a[i] = a[i] + d[i];
                a[i] = c[i] + d[i];
                a[i] = b[i] + scalar * c[i];
                a[i] = c[i] + scalar * d[i];
                a[i] = c[i];
                a[i] = d[i];

                a[i] = scalar * d[i];
                a[i] = scalar * b[i];
            }
        });
    });
    q.wait();

    stopMeasuring();
    
    free(a, q);
    free(b, q);    
    free(c, q);    
    free(d, q);    

    return 0;
}

template<typename T>
int mb::BenchmarkSuite::benchmark_test_5(){
    sycl::queue q = getQueue();

    T* a = sycl::malloc_shared<T>(run_configuration_array_size, q);
    T* b = sycl::malloc_shared<T>(run_configuration_array_size, q);    
    T* c = sycl::malloc_shared<T>(run_configuration_array_size, q);    

    T rb = getRandom<T>(4.0, 36.0);
    T rc = getRandom<T>(4.0, 36.0);
    T scalar = getRandom<T>(0.0, 1.0);
    T max = (T)run_configuration_repetition_count;

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {
            a[i] = 0.0;
            b[i] = rb;
            c[i] = rc;
        });
    });
    q.wait();

    startMeasuring();

    q.submit([&](sycl::handler& h){
        h.parallel_for(run_configuration_array_size, [=](sycl::id<1> i) {            
            for (size_t rep = 0; rep < max; rep++){
                a[i] = sin(b[i]);
                a[i] = sin(c[i]);
                a[i] = sin(b[i]);

                a[i] = sqrt(c[i]);
                a[i] = sqrt(b[i]);
                a[i] = sqrt(c[i]);
                a[i] = sqrt(b[i]);
                a[i] = sqrt(c[i]);

                a[i] = log(b[i]);
                a[i] = log(c[i]);
                a[i] = log(b[i]);
                a[i] = log(c[i]);
                a[i] = log(b[i]);
                a[i] = log(c[i]);
                a[i] = log(b[i]);
            }
        });
    });
    q.wait();

    stopMeasuring();
    
    free(a, q);
    free(b, q);    
    free(c, q);    

    return 0;
}