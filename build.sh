#!/bin/bash
cmake -B build .
cmake --build build

mkdir -p output/
cp build/bin/diffusion-exp output/diffusion-exp
rm -rf output/python_stuff
mkdir -p output
cp -r python_stuff output/
