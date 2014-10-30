#!/bin/sh
# This is a comment

rm *.txt
make clean
make

./cpusched results/in.txt sout_fcfs.txt FCFS
./cpusched results/in.txt sout_srtf_10.txt SRTF 10
./cpusched results/in.txt sout_srtf.txt SRTF

if diff -w sout_fcfs.txt results/out_fcfs.txt; then
    echo Success-----------------------------------------------Success
else
    echo Fail--------------------------------------------------Fail
fi

if diff -w sout_srtf.txt results/out_srtf.txt; then
    echo Success-----------------------------------------------Success
else
    echo Fail--------------------------------------------------Fail
fi

if diff -w sout_srtf_10.txt results/out_srtf_10.txt; then
    echo Success-----------------------------------------------Success
else
    echo Fail--------------------------------------------------Fail
fi
