Note: The executalbles are UNIX compatible only.

To compile
----------
$ make all
rm -f p2p_*.o
gcc -g -pthread p2p_server.c p2p_utils.c -o e_server
gcc -g -pthread p2p_client.c p2p_utils.c -o e_client

To run the server
----------------
$ ./e_server 
ERROR: Usage: ./e_server <Port-on-which-clients-should-connect-to>

$ ./e_server 9999

To run the client
----------------
$ ./e_client 
ERROR: Usage: ./e_client <Server-IP> <Server-Port>

$ ./e_client 152.1.142.88 9999

