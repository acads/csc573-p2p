#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "p2p_utils.h"

/* Global definitions */
uint32_t g_total_peers = 0;
uint32_t g_next_peer = 0;
uint32_t g_active_peers = 0;
peer_table g_peers[MAX_PEERS];
pthread_mutex_t g_lock;
struct xyz {
      struct sockaddr_in addr;
      int sock;
}xyz;
void
s_add_peer_tid(uint32_t tid, char *name)
{
   uint16_t i = 0;

   DBG_TRACE_ENTER;
   if (NULL == name) {
      DBG_TRACE_NULL_PTR;
      return;      
   }

   for (i = 0; i < g_active_peers; ++i) {
      if (tid == g_peers[i].tid) {
         /* Peer exists. Just ignore. */
         return;
      }
   }
   g_peers[g_next_peer].tid = pthread_self();
   strncpy(g_peers[g_next_peer].name, name, strlen(name));
   TRACE("***peer %s with tid %u added to system at pos %u***\n",
          g_peers[g_next_peer].name, g_peers[g_next_peer].tid, g_next_peer);
   g_active_peers += 1;
   g_next_peer += 1;
   DBG_TRACE_EXIT;
   return;
}

void
s_del_peer_tid(uint32_t tid)
{
   uint16_t i = 0;
   peer_db *peer = NULL;
 
   DBG_TRACE_ENTER;
   for (i = 0; i < g_active_peers; ++i) {
      if (tid == g_peers[i].tid) {
         TRACE("***peer %s with tid %u deleted from system at pos %u***\n",
                g_peers[i].name, g_peers[i].tid, i);
         peer = get_peer_by_name(g_peers[i].name);
         del_peer(peer);
         g_active_peers -= 1;
         g_next_peer = i;
      }
   }
   DBG_TRACE_EXIT;
   return;
}

void *
s_process_clients(void *socket_data)
{
   struct sockaddr_in ad;
   struct xyz *p;
   p=socket_data;
   int tmp;
   int sock_fd = 0;
   uint32_t bytes_recv = 0;
   uint32_t bytes_sent = 0;
   char send_buf[MAX_BUF_LEN] = "";
   char recv_buf[MAX_BUF_LEN] = "";
   char *op_str = NULL;
   char peer_name[MAX_NAME_LEN] = "";
   char msg_type[MAX_RET_MSG_LEN] = "";
   char ipaddress[16]="";
   tmp = p->sock;
   sock_fd = tmp;
   ad=p->addr;
   
   
   inet_ntop(AF_INET, &p->addr, ipaddress, INET_ADDRSTRLEN);
   printf("\n the ip address is %s\n",ipaddress); 
 

 
   DBG_TRACE("***Thread created with ID: %u***\n", (unsigned int) pthread_self());

   while ((bytes_recv = recv(sock_fd, recv_buf, MAX_BUF_LEN, 0)) > 0)
   {
      LOCK_MUTEX;
      DBG_TRACE_LOCK;
      get_peer_name_for_in_msg(recv_buf, peer_name);
      get_msg_type_for_in_msg(recv_buf, msg_type);      
      TRACE("***Msg type %sreceived from peer %s***\n", msg_type, peer_name);
      TRACE("***client data begin***\n%s***client data end***\n", recv_buf);
      s_add_peer_tid(pthread_self(), peer_name); 
      UNLOCK_MUTEX;
      DBG_TRACE_UNLOCK;

      op_str = s_handle_in_msg(recv_buf,ipaddress);
      memcpy(send_buf, op_str, MAX_BUF_LEN);
      free(op_str);
      if ((bytes_sent = send(sock_fd, send_buf, MAX_BUF_LEN, 0)) < 0)
      {
         perror("Server send() failed");
         goto EXIT_ON_ERROR;
      }
      memset(recv_buf, 0, sizeof(recv_buf));
      memset(send_buf, 0, sizeof(send_buf));
      memset(peer_name, 0, sizeof(peer_name));
      bytes_sent = 0;
      bytes_recv = 0;
      op_str = NULL;
   }

   /* Peer has left the system.
    * Remove all peer related data and close this thread.
    */
   LOCK_MUTEX;
   DBG_TRACE_LOCK;
   s_del_peer_tid(pthread_self());
   close(sock_fd);
   /* TODO: code to exit thread */
   UNLOCK_MUTEX;
   DBG_TRACE_UNLOCK;

EXIT_ON_ERROR:
   close(sock_fd);
   TRACE("***thread with tid %u has exited***\n", (unsigned int) pthread_self());
   pthread_exit(NULL);
}

int
main(int argc, char *argv[])
{
   int serverportnumber = 0;
   int listen_fd = 0;
   int conn_fd = 0;
   struct xyz obj;
   int addrlen=sizeof(struct sockaddr_in);
   struct sockaddr_in serv_addr,cli_addr; 
   pthread_t serv_thread[MAX_PEERS];
   int th;

   if (2 != argc) {
      TRACE("ERROR: Usage: %s <Port-on-which-clients-should-connect-to>\n", argv[0]);
      return -1;
   }
   serverportnumber = atoi(argv[1]);

   memset(&serv_addr, '0', sizeof(serv_addr));
   memset(&cli_addr, '0', sizeof(cli_addr));
   if (pthread_mutex_init(&g_lock, NULL) != 0) {
      perror("Server pthread_mutex_lock() failed"); 
      return -1;
   }
   //printf("\n enter the port number: ");
   //scanf("%d",&serverportnumber);
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   serv_addr.sin_port = htons(serverportnumber); 

   listen_fd = socket(AF_INET, SOCK_STREAM, 0);
   bind(listen_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)); 
   listen(listen_fd, 32); 

   printf("***SERVER LISTENING TO CONNECTIONS***\n");

   while(1)
   {
#if 0
FIXME: DAN to fix this
      if (MAX_PEERS == g_total_peers) {
         /* Our system is running at max capacity. */
         TRACE("Error: Maximum number of active clients in action.\n");
         continue;
      }
#endif
      conn_fd = accept(listen_fd, (struct sockaddr*) &cli_addr, &addrlen); 
      printf("***CONNECTON ACCEPTED***\n");
      obj.addr=cli_addr;
      obj.sock=conn_fd;
      th = pthread_create(&serv_thread[g_total_peers], NULL,
                          &s_process_clients, (void *) &obj);
      g_total_peers += 1;
   }

   printf("***server exiting***\n");
   pthread_mutex_destroy(&g_lock);
   printf("***server exiting***\n");
   return 0;
}

