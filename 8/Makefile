all: UDP_server UDP_client_flowerbed UDP_client_gardener UDP_client_printer
UDP_server: server.c util.c
	gcc server.c util.c -o UDP_server
UDP_client_flowerbed: client_flowerbed.c util.c
	gcc client_flowerbed.c util.c -o UDP_client_flowerbed
UDP_client_gardener: client_gardener.c util.c
	gcc client_gardener.c util.c -o UDP_client_gardener
UDP_client_printer: client_printer.c util.c
	gcc client_printer.c util.c -o UDP_client_printer