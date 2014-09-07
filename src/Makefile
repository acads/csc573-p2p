p2p-app: p2p_server.c p2p_client.c p2p_utils.c 
	gcc -g -pthread p2p_server.c p2p_utils.c -o e_server
	gcc -g -pthread p2p_client.c p2p_utils.c -o e_client

p2p_server: server.c server-util1.c
	gcc -g -pthread p2p_server.c p2p_utils.c -o p2p_server

p2p_client: client_final1.c server-util1.c
	gcc -g -pthread p2p_client.c p2p_utils.c -o p2p_client

.PHONY: clean

clean:
	rm -f p2p_*.o

all: clean p2p-app clean
