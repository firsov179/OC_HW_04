#!/bin/bash
make
./TCP_server 9009 &
./TCP_client_flowerbed 127.0.0.0 9009 >flowerbed.out &
./TCP_client_gardener 127.0.0.0 9009 alice 4 >alice.out &
./TCP_client_gardener 127.0.0.0 9009 bob 5 >bob.out &
./TCP_client_printer 127.0.0.0 9009 >printer.out
