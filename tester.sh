#!/bin/bash
for n in {1..12} 
do
  for k in {0..3} 
  do
    echo $n $k 1000000 | ./wys
  done
done