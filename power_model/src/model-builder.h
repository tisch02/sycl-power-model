#pragma once

#include <iostream>
#include <map>

#include "microbench.h"

namespace mb{
    class RunInfo{
        public: 
            size_t Repetitions;
            size_t Start;
            size_t Step;
            size_t StepCount;
            size_t KernelRepetitions;

            RunInfo(size_t repetitions, size_t start, size_t step, size_t step_count, size_t kernel_repetitions);
    };

    class ModelBuilder{
        public:
            ModelBuilder(std::string p_model_path, mb::Target p_target, mb::DataType p_data_type = mb::DataType::DOUBLE, mb::DeviceType p_device_type = mb::DeviceType::GPU, int p_device_offset = 0);
            void Run(mb::Benchmark benchmark);
        
        private:
            std::string model_path;
            mb::Target target;
            mb::DataType data_type;
            mb::DeviceType device_type; 
            int device_offset;
            std::map<mb::Benchmark, RunInfo> runs;

            std::string createPath(std::string base, std::string name);
            void registerRun(mb::Benchmark benchmark, size_t repetitions, size_t start, size_t step, size_t step_count, size_t kernel_repetitions);
            void registerRuns();

    };
}