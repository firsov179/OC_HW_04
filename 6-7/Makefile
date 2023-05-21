all: TCP_server TCP_client_flowerbed TCP_client_gardener TCP_client_printer
TCP_server: server.c util.c
	gcc server.c util.c -o TCP_server
TCP_client_flowerbed: client_flowerbed.c util.c
	gcc client_flowerbed.c util.c -o TCP_client_flowerbed
TCP_client_gardener: client_gardener.c util.c
	gcc client_gardener.c util.c -o TCP_client_gardener
TCP_client_printer: client_printer.c util.c
	gcc client_printer.c util.c -o TCP_client_printer