#include <iostream>
#include <filesystem>

#include "microbench.h"

using namespace std;

int main(int, char**) {
    string path = (string)filesystem::current_path() + "/measurements";

    mb::BenchmarkSuite suite(mb::Target::AMD, mb::DataType::DOUBLE);
    suite.ConfigureDeviceSelection(1, mb::DeviceType::GPU);
    suite.ConfigureSleep(500, 0);
    
    suite.Run(mb::Benchmark::INFO);
    
    suite.Run(mb::Benchmark::ADD, 100000, 10000000);
    suite.WriteCsv(path + "/test.csv");
    suite.WritePowerCsv(path + "/test_power.csv");
    suite.Print();

    //suite.Run(mb::Benchmark::COPY, 100000, 10000000);
    //suite.Print();

    //suite.Run(mb::Benchmark::TRIAD, 100000, 10000000);
    //suite.Print();

    //suite.Run(mb::Benchmark::MULT, 100000, 10000000);
    //suite.Print();

    //suite.Run(mb::Benchmark::SIN, 100000, 10000000);
    //suite.Print();

    //suite.Run(mb::Benchmark::SQRT, 100000, 10000000);
    //suite.Print();

    //suite.Run(mb::Benchmark::LOG, 100000, 10000000);
    //suite.Print();

    //suite.Run(mb::Benchmark::TEST_1, 100000, 10000000);
    //suite.Print();

    //suite.Run(mb::Benchmark::TEST_2, 100000, 10000000);
    //suite.Print();

    //suite.Run(mb::Benchmark::TEST_3, 100000, 10000000);
    //suite.Print();

    //suite.Run(mb::Benchmark::TEST_4, 100000, 10000000);
    //suite.Print();

    //suite.Run(mb::Benchmark::TEST_5, 100000, 10000000);
    //suite.Print();

    return 0;    
}