#pragma once

#include "microbench-papi-wrapper.h"
#include "power-wrappers/microbench-power-wrapper.h"
#include <iostream>
#include <sycl/sycl.hpp>
#include <utility>
#include <map>

namespace mb{
    enum Benchmark {
        INFO,
        IDLE,
        ADD,
        ADD_BABEL,
        ADD_LOCAL,
        TRIAD,
        COPY,
        MULT,
        SIN,
        LOG,
        SQRT,
        TEST_1,
        TEST_2,
        TEST_3,
        TEST_4,
        TEST_5
    };

    enum DeviceType {
        ANY,
        CPU,
        GPU
    };

    enum DataType {
        INT,
        FLOAT,
        DOUBLE
    };

    class BenchmarkInfo{
        public:
            std::string Name;

            BenchmarkInfo(std::string name);
    };

    class BenchmarkSuite{
        public:
            BenchmarkSuite(Target target, DataType dataType = DataType::FLOAT);
            void Run(Benchmark benchmark, size_t array_size = 1, size_t repetition_count = 1);
            void Print();
            void WriteCsv(std::string path);
            void WritePowerCsv(std::string path);
            std::string GetBenchmarkName(Benchmark benchmark);
            void ConfigureDeviceSelection(int deviceOffset, DeviceType deviceType);            
            void ConfigureSleep(int beforeSleep, int afterSleep);

        private:
            std::map<Benchmark, std::pair<int (mb::BenchmarkSuite::*)(), mb::BenchmarkInfo>> benchmarks;
            size_t run_configuration_array_size;
            size_t run_configuration_repetition_count;
            std::string run_configuration_benchmark_name;
            std::string datatype_name;
            
            mb::PapiWrapper papi;
            mb::PowerWrapper power;

            int deviceOffset;
            DeviceType deviceType;

            int before_sleep_duration = 0;
            int after_sleep_duratin = 0;

            void registerBenchmark(mb::Benchmark type, int (mb::BenchmarkSuite::*func)(), std::string name);        
            void startMeasuring();
            void stopMeasuring();
            std::string getCsvHeader();
            std::string getCsvLine();
            sycl::queue getQueue();

            template<typename T>
            T getRandom(T min, T max);

            template<typename T>
            void registerBenchmarks();

            template<typename T>
            int benchmark_info();

            template<typename T>
            int benchmark_idle();            

            template<typename T>
            int benchmark_add();

            template<typename T>
            int benchmark_add_babel();

            template<typename T>
            int benchmark_add_local();

            template<typename T>
            int benchmark_triad();

            template<typename T>
            int benchmark_copy();

            template<typename T>
            int benchmark_mult();
            
            template<typename T>
            int benchmark_sin();

            template<typename T>
            int benchmark_log();

            template<typename T>
            int benchmark_sqrt();

            template<typename T>
            int benchmark_test_1();

            template<typename T>
            int benchmark_test_2();

            template<typename T>
            int benchmark_test_3();

            template<typename T>
            int benchmark_test_4();

            template<typename T>
            int benchmark_test_5();
    };
}
