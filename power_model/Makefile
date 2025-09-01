# Variables for compilation
acpp-source-path = ~/shared/AdaptiveCpp
acpp-build-path = /var/tmp/adaptive-cpp-kraljic

babel-source-path = ~/ts/llvm-ee-project/BabelStream
babel-build-path = /var/tmp/babel-stream-kraljic

papi-source-path = ~/shared/papi
papi-build-path = /var/tmp/papi-kraljic

# LLVM variables
llvm-path = /opt/rocm-6.3.4/llvm 
lld-path = /opt/rocm-6.3.4/llvm/bin/ld.lld
llc-path = /opt/rocm-6.3.4/llvm/bin/llc
opt-path = /opt/rocm-6.3.4/llvm/bin/opt
hip-path = /opt/rocm-6.3.4/lib/cmake/hip
clang = /opt/rocm-6.3.4/llvm/bin/amdclang
clangpp = /opt/rocm-6.3.4/llvm/bin/amdclang++

# Others
boost = /lib64
acpp-target = hip:gfx942
rocm = /opt/rocm-6.3.4

# Variables for usage
acpp = $(acpp-build-path)/bin/acpp
out-dir = /tmp

# Build PAPI ==================================================================
compile-papi:
	mkdir -p $(papi-build-path)
	cd $(papi-source-path)/src && ./configure --prefix=$(papi-build-path) --with-components="rocm rocm_smi"
	cd $(papi-source-path)/src && make && make install
	
test-papi:	
	$(papi-build-path)/bin/papi_component_avail


# Build AdaptiveCpp ===========================================================
remove-acpp:
	rm -f -r $(acpp-source-path)/build
	rm -f -r $(acpp-build-path)

compile-acpp: remove-acpp
	mkdir -p $(acpp-build-path)
	mkdir -p $(acpp-source-path)/build

	cmake -DCMAKE_INSTALL_PREFIX=$(acpp-build-path) -DHIP_DIR=$(hip-path) -DWITH_ROCM_BACKEND=ON -DBOOST_ROOT=$(boost) -DCMAKE_C_COMPILER=$(clang) -DCMAKE_CXX_COMPILER=$(clangpp) -DACPP_LLD_PATH=$(lld-path) -DACPP_OPT_PATH=$(opt-path) -DACPP_LLC_PATH=$(llc-path) -B$(acpp-source-path)/build -S$(acpp-source-path)
	make -C $(acpp-source-path)/build install

test-acpp: clean-test-acpp compile-test-acpp run-test-acpp
test-acpp-target: clean-test-acpp compile-test-acpp-target run-test-acpp

compile-test-acpp:
	$(acpp) -o $(out-dir)/sycl-hello-world.o -O3 sycl-hello-world.cpp

compile-test-acpp-target:
	$(acpp) -o $(out-dir)/sycl-hello-world.o -O3 --acpp-targets=$(acpp-target) sycl-hello-world.cpp

run-test-acpp:
	$(out-dir)/sycl-hello-world.o

clean-test-acpp:
	rm -f $(out-dir)/sycl-hello-world.o

# Build BabelStream ===========================================================
babel-stream: build-babel-stream run-babel-stream

build-babel-stream:
	mkdir -p $(babel-build-path)	
	cmake -B $(babel-build-path) -H. -S $(babel-source-path) \
		-DMODEL=sycl2020-usm \
		-DSYCL_COMPILER=HIPSYCL \
		-DSYCL_COMPILER_DIR=$(acpp-build-path)
	cmake --build $(babel-build-path)

run-babel-stream:
	$(babel-build-path)/sycl2020-usm-stream

# Compile with ACPP ===========================================================
# "file"		Name of the file to compile, 		e.g. filename=test.cpp
# "jit" 		Use JIT compilation, 				e.g. jit=true
# "run"			Run the program after compilation, 	e.g. run=true
# "O" 			Optimization level, default O=3, 	e.g. O=0

O = 3
papi-conf = -I$(papi-build-path)/include -L$(papi-build-path)/lib -lpapi
rocm-smi = /opt/rocm-6.4.0
rocm-smi-conf = -I$(rocm-smi)/include -L$(rocm-smi)/lib -lrocm_smi64
compile:
ifeq ($(jit),true)
	$(acpp) -o $(out-dir)/$(file).out -O$(O) $(papi-conf) $(file)
else
	$(acpp) -o $(out-dir)/$(file).out -O$(O) --acpp-targets=$(acpp-target) $(papi-conf) $(rocm-smi-conf) -I/src -L/src -lpthread $(file)
endif
	
ifeq ($(run),true)
	$(out-dir)/$(file).out
else
	echo Output: $(out-dir)/$(file).out
endif

# =============================================================================
cpp-files = src/microbench-papi-wrapper.cpp src/microbench.cpp src/model-builder.cpp

# Compile Microbench ==========================================================
microbench-src = benchmarks
microbench-target = amd

microbench: microbench-compile microbench-run

microbench-compile:
	$(acpp) -o $(out-dir)/$(microbench-src).out -O3 --acpp-targets=$(acpp-target) $(papi-conf) $(rocm-smi-conf) -lpthread $(cpp-files) src/benchmarks.cpp src/power-wrappers/microbench-power-wrapper-$(microbench-target).cpp

microbench-run:
	/$(out-dir)/$(microbench-src).out

# Compile Model ==========================================================
model-src = model
model-target = amd

model: model-compile model-run

model-compile:
	$(acpp) -o $(out-dir)/$(model-src).out -O3 --acpp-targets=$(acpp-target) $(papi-conf) $(rocm-smi-conf) -lpthread $(cpp-files) src/model.cpp src/power-wrappers/microbench-power-wrapper-$(model-target).cpp

model-run:
	/$(out-dir)/$(model-src).out





