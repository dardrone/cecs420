#!/bin/sh
# This is a comment

rm *.txt
make clean
make
./commonwords files/in1.txt files/in2.txt student_out.txt

if diff -w student_out.txt files/my_out.txt; then
    echo Success-----------------------------------------------Success
else
    echo Fail--------------------------------------------------Fail
fi
