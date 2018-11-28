#!/bin/bash
for i in {1..8}
do
    time ./a.out $i 100
    time ./a.out $i 2000
done
