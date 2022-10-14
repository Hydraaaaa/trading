#!/bin/bash


gcc src/main.c -o bin/trading -Wall -g -lraylib -lpthread -ldl -lrt -lX11 -Iinc -lm
