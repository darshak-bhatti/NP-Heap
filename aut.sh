#!/bin/bash
IFS=$'\n'
np=0
m_s_o=16384
i=1
for line in $(cat ./fn)
do
    np=$(echo $line | cut -d ',' -f1)
    no=$(echo $line | cut -d ',' -f2)
    ffc=$(echo $line | cut -d ',' -f3)
    number_of_objects=$no
    max_size_of_objects=$m_s_o
    feature_combinations=$ffc
    number_of_processes=$np
    sudo insmod kernel_module/npheap.ko
    sudo chmod 777 /dev/npheap
    echo "Test $i: $number_of_objects $feature_combinations $number_of_processes"
    ./test_cases/benchmark_single $number_of_objects $max_size_of_objects $feature_combinations $number_of_processes
    sleep 10
    number_of_log_files=`ls *log | wc -l`
    if [ $number_of_log_files -eq 0 ]
    then
        echo "fail"
        rm -f *.log
    else
        echo "1"
        cat *.log > trace
        echo "2"
        sort -n -k 3 trace > sorted_trace
        echo "3 . dhan tanananan"
        ./test_cases/validate $number_of_objects $max_size_of_objects $number_of_processes < sorted_trace
        echo "4"
        rm *trace
        rm -f *.log
    fi
    sudo rmmod npheap
    i=$((i+1))
done