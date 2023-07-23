#!/bin/bash

for i in $(ls ./tests/)
do
    echo "out${i:4}"
done