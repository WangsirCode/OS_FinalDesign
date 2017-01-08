#!/bin/bash
for ((i=10; i<=100;i=i+10));do
    for((j=10000;j<=1000000000;j=j*10));do
        echo "$i个线程(进程)，累加到$j"
        time ./multisum $i $j
    done
done
