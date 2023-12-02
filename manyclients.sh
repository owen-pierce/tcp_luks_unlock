#!/bin/bash

for N in {1..150}
do
	./tcp_client &
	sleep .1
done
wait
