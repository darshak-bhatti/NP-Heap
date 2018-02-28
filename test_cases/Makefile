all: benchmark validate benchmark_single

benchmark: benchmark.c 
	$(CC) -g -O0 benchmark.c -o benchmark -I/usr/local/include -lnpheap
	
validate: validate.c 
	$(CC) -g -O0 validate.c -o validate -lnpheap
	
benchmark_single: benchmark_single.c 
	$(CC) -g -O0 benchmark_single.c -o benchmark_single -I/usr/local/include -lnpheap

clean:
	rm -f benchmark validate benchmark_single
