#!/bin/bash
make
./UDP_server 127.0.0.0 7010 &
./UDP_client_flowerbed 127.0.0.0 7010 >flowerbed.out &
./UDP_client_gardener 127.0.0.0 7010 alice 4 >alice.out &
./UDP_client_gardener 127.0.0.0 7010 bob 5 >bob.out &
./UDP_client_printer 127.0.0.0 7010 >printer.out &
./UDP_client_printer 127.0.0.0 7010 >printer1.out
