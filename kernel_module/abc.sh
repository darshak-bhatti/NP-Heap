sudo rmmod npheap
make clean
make
sudo make install
sudo insmod npheap.ko
sudo chmod 777 /dev/npheap