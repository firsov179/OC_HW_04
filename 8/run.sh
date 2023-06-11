#!/bin/bash
make
./TCP_server 9006 2 &
./TCP_client_flowerbed 127.0.0.0 9006 >flowerbed.out &
./TCP_client_gardener 127.0.0.0 9006 alice 4 >alice.out &
./TCP_client_gardener 127.0.0.0 9006 bob 5 >bob.out &
./TCP_client_printer 127.0.0.0 9006 >printer1.out
./TCP_client_printer 127.0.0.0 9006 >printer2.out
