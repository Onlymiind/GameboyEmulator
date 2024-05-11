#!/bin/bash

CONFIGURATION="Debug"

if [ "$#" -eq 1 ]; then
	CONFIGURATION="$1"
fi

cd build && cmake -DCMAKE_BUILD_TYPE=$CONFIGURATION .. && cmake --build .

