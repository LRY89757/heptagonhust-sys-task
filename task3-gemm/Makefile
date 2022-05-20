gemm: main.cpp
	g++ main.cpp -o gemm -fopenmp -mavx -mavx2 -mfma -msse -msse2 -msse3 -mavx512bw -mavx512vl -mavx512f -mavx512cd -mavx512dq -O0 -Wall -Werror -std=c++11 

.PHONY: clean
clean:
	- rm -f gemm

.PHONY: run
run: 
	./gemm

.PHONY: rmdata
rmdata:
	- rm -rf data*	 