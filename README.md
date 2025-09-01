
Clone the repository. Inside the `power_model` directory, creat a `measurements` directory as the default location for results. Inside the `python` directory, create a `out` directory for visualizations.

The project has multiple dependencies like PAPI or AdaptiveCpp. You may use and adapt the Makefile inside the `power_model` directory. 

Inside the `power_model` directory: The `/src/benchmarks.cpp` file shows an example of the low-level API. This can be run with `make microbench`. The `/src/model.cpp` file shows an example of the hight-level API. This can be run with `make microbench`.

Python scripts for visualizations and the model constructions are located in the `python` folder. Depending on available packages, some depedencies have to be installed (numpy, padans, matplotlib). The Makefile can be used to run the scripts. Paths have to be adapted in the source files.


## Preperation on Test System
Run the following commands:
```sh
module add z3/4.12.2 llvm/18.1.8 boost/1.75 openmpi/4.1.1-gcc12.2 rocprofiler-compute/6.3.0

export PATH=$PATH:/opt/rocm-6.3.4/llvm/bin \
    PAPI_ROCM_ROOT=/opt/rocm-6.3.4 \
    PAPI_ROCMSMI_ROOT=/opt/rocm-6.4.0 \
    PAPI_DIR=/var/tmp/papi-kraljic

export PATH=${PAPI_DIR}/bin:$PATH \
    LD_LIBRARY_PATH=${PAPI_DIR}/lib:$LD_LIBRARY_PATH
```