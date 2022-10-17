#!/bin/bash


gcc src/*.c -o bin/trading -Wall -g -lraylib -lpthread -ldl -lrt -lX11 -Iinc -lm
if [ 0 -eq 0 ];
then
	echo -e "BUILD SUCCEEDED\n"
	cd bin
	./trading
fi
