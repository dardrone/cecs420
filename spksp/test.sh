#!/bin/sh
# This is a comment

rm *.out
make clean
make
echo
./spksp commandFile.txt 1 | sort > student_out1.out

if diff -w student_out1.out correct_output/correct_output.txt; then
    echo Success-----------------------------------------------Success
else
    echo Fail--------------------------------------------------Fail
fi
