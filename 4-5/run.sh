#!/bin/bash
make
./TCP_server 7007 &
./TCP_client_flowerbed 127.0.0.0 7007 >flowerbed.out &
./TCP_client_gardener 127.0.0.0 7007 alice 4 >alice.out &
./TCP_client_gardener 127.0.0.0 7007 bob 5 >bob.out
