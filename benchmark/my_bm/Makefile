all: hello_bm hello_bm2 hello_vd

hello_bm: hello_bm.c 
	$(CC) -g -O0 hello_bm.c -o hello_bm -I/usr/local/include -lnpheap

hello_bm2: hello_bm2.c 
	$(CC) -g -O0 hello_bm2.c -o hello_bm2 -I/usr/local/include -lnpheap


hello_vd: hello_vd.c 
	$(CC) -g -O0 hello_vd.c -o hello_vd -I/usr/local/include -lnpheap
	
clean:
	rm -f hello_bm hello_vd hello_bm2
