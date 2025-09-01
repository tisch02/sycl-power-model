#include <iostream>
#include <filesystem>

#include "model-builder.h"

using namespace mb;
using namespace std;

mb::RunInfo::RunInfo(size_t repetitions, size_t start, size_t step, size_t step_count, size_t kernel_repetitions){
    Repetitions = repetitions;
    Start = start;
    Step = step;
    StepCount = step_count;
    KernelRepetitions = kernel_repetitions;
}

string mb::ModelBuilder::createPath(string base, string name){
    string path = base + "/" + name;
    filesystem::create_directories(path);
    return path;
}

void mb::ModelBuilder::registerRun(mb::Benchmark benchmark, size_t repetitions, size_t start, size_t step, size_t step_count, size_t kernel_repetitions){
    RunInfo info(repetitions, start, step, step_count, kernel_repetitions);
    runs.insert({benchmark, info});
}

void  mb::ModelBuilder::registerRuns(){
    registerRun(mb::Benchmark::IDLE, 5, 10, 0, 1, 1);

    // Micro Benchmarks ---------------

    registerRun(mb::Benchmark::ADD, 10, 100000, 50000, 8, 7000000);
    registerRun(mb::Benchmark::MULT, 10, 100000, 50000, 8, 7000000);    
    registerRun(mb::Benchmark::COPY, 10, 100000, 50000, 8, 10000000);
    registerRun(mb::Benchmark::TRIAD, 10, 100000, 50000, 8, 10000000);

    registerRun(mb::Benchmark::LOG, 10, 100000, 50000, 8, 5000000);
    registerRun(mb::Benchmark::SQRT, 10, 100000, 50000, 8, 5000000);
    registerRun(mb::Benchmark::SIN, 10, 100000, 50000, 8, 3000000);

    // Tests --------------------------

    registerRun(mb::Benchmark::TEST_1, 10, 150000, 75000, 10, 10000000);
    registerRun(mb::Benchmark::TEST_2, 10, 100000, 50000, 10, 10000000);
    registerRun(mb::Benchmark::TEST_3, 10, 100000, 50000, 10, 10000000);
    registerRun(mb::Benchmark::TEST_4, 10, 100000, 50000, 10, 5000000);
    registerRun(mb::Benchmark::TEST_5, 10, 100000, 50000, 10, 2000000);
}

mb::ModelBuilder::ModelBuilder(string p_model_path, mb::Target p_target, mb::DataType p_data_type, mb::DeviceType p_device_type, int p_device_offset){
    cout << "SYCL Model Builder!" << endl;
    
    model_path = p_model_path;
    target = p_target;
    data_type = p_data_type;
    device_type = p_device_type;
    device_offset = p_device_offset;

    registerRuns();
    cout << "Counting a total number of " << runs.size() << " run configurations." << endl;

    // Prepare directory
    filesystem::remove_all(model_path);
    filesystem::create_directories(model_path);
}

void mb::ModelBuilder::Run(mb::Benchmark benchmark){
    // Configure benchmark suite
    BenchmarkSuite suite(target, data_type);
    suite.ConfigureDeviceSelection(device_offset, device_type);
    suite.ConfigureSleep(500, 0);
    
    // Find requested benchmark
    RunInfo info = runs.find(benchmark)->second;
    string name = suite.GetBenchmarkName(benchmark);

    // Execute with requested repetitions
    string benchmark_path = createPath(model_path, name);
    for (size_t i = 0; i < info.StepCount; i++){
        size_t arr = info.Start + info.Step * i;
        string run_path = createPath(benchmark_path, "run_" + to_string(i));

        for (size_t i = 0; i < info.Repetitions; i++){
            suite.Run(benchmark, arr, info.KernelRepetitions);
            suite.WritePowerCsv(run_path + "/power_" + to_string(i) + ".csv");
            suite.WriteCsv(run_path + "/counter.csv");
        }
    }
}