#include <iostream>
#include <filesystem>

#include "model-builder.h"

using namespace std;

int main(int, char**) {
    string path = (string)filesystem::current_path() + "/measurements/model";    
    mb::ModelBuilder modelBuilder(path, mb::Target::AMD);

    modelBuilder.Run(mb::Benchmark::IDLE);

    // Micro Benchmarks ---------------

    modelBuilder.Run(mb::Benchmark::ADD);
    modelBuilder.Run(mb::Benchmark::MULT);
    modelBuilder.Run(mb::Benchmark::TRIAD);
    modelBuilder.Run(mb::Benchmark::COPY);

    modelBuilder.Run(mb::Benchmark::SIN);
    modelBuilder.Run(mb::Benchmark::LOG);
    modelBuilder.Run(mb::Benchmark::SQRT);

    // Tests --------------------------

    modelBuilder.Run(mb::Benchmark::TEST_1);
    modelBuilder.Run(mb::Benchmark::TEST_2);
    modelBuilder.Run(mb::Benchmark::TEST_3);
    modelBuilder.Run(mb::Benchmark::TEST_4);
    modelBuilder.Run(mb::Benchmark::TEST_5);

    return 0;
}
