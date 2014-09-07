/* ------------ Client code for peer to peer File Transfer ----------- */ 
/* ---- header files ---- */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include "p2p_utils.h"

pthread_mutex_t g_lock;
peer_db *node;  // node defined globally
char os[53]="OS: Red Hat Enterprise Linux Workstation release 6.4"; //msg purposes
struct xyz {
      int psock;
      struct sockaddr_in paddr;
 }xyz;

/* ------------- functions ------------------------------ */

/* Function: registernewrfc
What does it do? - gets the details about the new rfc registered and puts 
it in the form of a string ADD RFC .... and returns that string 

Param: rfc   - integer value of the rfc now got
       port  - port number of the receiving client
       title - title of rfc
       name  - name of the receiving host

returns      -  the string ADD RFC .. to be sent to the server for registering 
*/
  
char *
registernewrfc(int rfc, int port, char *title, char *name) {
   char ab[14]="";
   char *strret=NULL;
   strret=calloc(1,4096);
   char line1[3]  = "\n";
   char xtra1[12] = " P2P/CI 1.0";
   char wordh[7]  = "Host: ";
   char wordport[16] = "";
   char word5[8] = "Title: ";
   printf("\n the rfc number is %d",rfc);
   snprintf(ab,sizeof(ab),"ADD RFC %d", rfc);
   strncpy(strret, ab, strlen(ab)+1);
   strncat(strret, xtra1, strlen(xtra1)+1);
   strncat(strret, line1, strlen(line1)+1);
   strncat(strret, wordh, strlen(wordh)+1);
   strncat(strret, name, strlen(name)+1);
   strncat(strret, line1, strlen(line1)+1);
   snprintf(wordport, sizeof(wordport), "Port: %d",port);
   strncat(strret, wordport, strlen(wordport)+1);
   strncat(strret, line1, strlen(line1)+1);
   strncat(strret, word5, strlen(word5)+1);
   strncat(strret,title, strlen(title)+1);
   strncat(strret, line1, strlen(line1)+1);
   printf(" the string to be sent is %s",strret);
   return strret;
   }
/* ------------------------------------------------------ */

/* ------------- thread declarations -------------------- */
void *
listen_client (void *portno) {
      int listen_sock=0;
      pthread_t tid;
      int recpeer=0;
      int *po1;
      po1= portno;
      int po;
      po=*po1;
      int addrlen=sizeof(struct sockaddr_in);
      char *recbuff1=calloc(1,4096);
      char rfcstr[6]="";
      char *final2=NULL;
      int i=0;
      char ch;
      int n2=0;
      int rfctosend1=0;
      char *sendstr=calloc(1,4096);
      LOCK_MUTEX;
      struct sockaddr_in cliserv_addr, clipeer_addr;
      listen_sock=socket(AF_INET,SOCK_STREAM,0);
      int tmp=0;
      tmp=listen_sock;
      memset(&cliserv_addr,0,sizeof(cliserv_addr));
      memset(&clipeer_addr,0,sizeof(clipeer_addr));
      cliserv_addr.sin_family=AF_INET;
      cliserv_addr.sin_port=htons(po);
      cliserv_addr.sin_addr.s_addr=htons(INADDR_ANY);
         if ((bind(listen_sock, (struct sockaddr*)&cliserv_addr,sizeof(cliserv_addr)))<0)    
         {
            perror("ERROR: Client bind() failed");
         }
      listen(listen_sock,10);
      printf("INFO: Client's server thread is now running.\n");
      printf("INFO: Listening for connections...\n");
         while (1) {
            i=0;
            recpeer=accept(listen_sock, (struct sockaddr *)&clipeer_addr,&addrlen);
            printf("INFO: Connection accepted.\n");
            FILE *fop=NULL;
            while ((n2=read(recpeer,(void*) recbuff1,4096))>0) {
               final2=recbuff1;
               final2=strstr(final2,"GET RFC ");
                  while (final2[8+i]!=' ') {
                     rfcstr[i]=final2[8+i];
                     i++;
                  }
               i=0;
               rfctosend1=atoi(rfcstr);
               
                  while(i<node->total_rfcs) {
                     if(rfctosend1==node->rfc_num[i]) {
                        printf("INFO: RFC %d found.\n", rfctosend1);
                        break;
                     }
                     else 
                        i++;
                   }
                  if (i==(node->total_rfcs)) {
                     printf("INFO: RFC %d not found.\n", rfctosend1);
                     sendstr="404 FILE NOT FOUND";
                     write(recpeer, (void*) sendstr,strlen(sendstr)+1);
                  }    
                  fop=fopen(rfcstr,"r");
	             if (fop==NULL) {
                     perror("File pointer failed");
                  }
               while((ch=fgetc(fop))!=EOF) {
                 
                    if(i==1024) {
                       printf("\n buffer contains %s",sendstr);
                       write(recpeer, (void*) sendstr, strlen(sendstr)+1);
                       memset(sendstr, 0, 4096);
                       sleep(1);
                       i=0;
                    }
                   sendstr[i]=ch;
                   i++;
               }
                       printf("\n buffer contains %s",sendstr);
                       write(recpeer, (void*) sendstr, strlen(sendstr)+1);
 
             UNLOCK_MUTEX;
             close(recpeer);
             fclose(fop); 
          }
       }
    }


int main (int argc, char *argv[]) 
{
   int choice = 0;
   int i,flag,rfcno;
   int clisock=0;
   int n=0;
   int g = 0;
   pthread_t thid;
   uint16_t tmp_port = 0;
   int ser_port;
   char *retstr = calloc(1,4096);
   char *str = calloc(1,4096);
   char ipaddr[16] = "";
   char title[256] = "";
   char recbuff[MAX_NAME_LEN] = "";
   char hostname[256] = "";
   char recv_buf[MAX_BUF_LEN] = "";
   char recv_buffer[MAX_BUF_LEN] ="";
   char ipadres[16]="";
   struct in_addr **addr; 
   struct sockaddr_in serv_addr,cli_ad,sin;
   struct hostent *he;
   memset(&cli_ad,0,sizeof(cli_ad));
   node = calloc(1,sizeof(peer_db));

   if (3 != argc) {
      TRACE("ERROR: Usage: %s <Server-IP> <Server-Port>\n", argv[0]);
      return -1;
   }
   ser_port = atoi(argv[2]);

   g = gethostname(hostname,sizeof(hostname));
   //printf("\n the hostname is %s",hostname);
   he=gethostbyname(hostname);
   bcopy((char *)he->h_addr,(char *)&serv_addr.sin_addr.s_addr,he->h_length);
   addr=(struct in_addr **)he->h_addr_list;
   strcpy(ipadres,inet_ntoa(*addr[0]));
   //printf("\n ip address got by gethostbyname is %s",ipadres);
   strcpy(ipaddr,"Host: ");
   strcpy(ipaddr,inet_ntoa(*addr[0]));
   
   //printf("\nEnter a Host Name:");
   //scanf("%[^\n]s",ipaddr);
   printf("Enter the number of RFC(s) to be initialized for the user: ");
   scanf("%d",&rfcno);
   i=0;
   flag=0;
   node=calloc(1,sizeof(peer_db));

   /* Get the RFC details from the user. */
   while (i < rfcno) {
      printf("Enter RFC number: ");
      scanf("%hu",&node->rfc_num[flag]);
      printf("Enter the Title of the RFC: ");
      scanf(" %[^\n]s",title);
      strcpy(node->rfc_title[flag],title);
      flag++;
      node->total_rfcs = i + 1;
      i += 1;
   }

   printf("Enter the port number on which other peers may contact: ");
   scanf("%hu", &tmp_port);
   node->port = tmp_port;      
   strncpy(node->name, ipaddr, MAX_NAME_LEN);
   printf("\n---Information to be sent to the Server---");
   printf("Name: %s\n", node->name);
   printf("Port Number: %u\n",node->port);
   printf("Total Number of RFC(s): %d\n",node->total_rfcs);
   i=0;

      while ( i<flag) {
         printf("RFC number: %u\n",node->rfc_num[i]);
         printf("RFC Title: %s\n",node->rfc_title[i]);
         i++;
      }

/* ------------------ RUNNING THE BACKGROUND THREAD -------------------- */
     if ((pthread_create(&thid, NULL, &listen_client,(void *) &tmp_port))!=0)
     {
        printf("\n client server thread creation failed");
     }
/* --------------------------------------------------------------------- */


/*-----------------------------------------------------------------------*/
// the intialization to connect to the server socket

     if ((clisock =socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n error: could not create socket");
        return 1;
     }
       
     memset(&serv_addr,0, sizeof(serv_addr));
     serv_addr.sin_family=AF_INET;
     serv_addr.sin_port = htons(ser_port);

        if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <=0) {
                 printf("inet_pton failed");
                 return 1;
        }

        if (connect(clisock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0) {
                 perror("ERROR: Client connect() failed");
                 return -1;
        }

/*------------------------------------------------------------------------*/
    while(1) {
       printf("\nWhat do you want to do ?\n");
       printf("1. Register RFC\n");
       printf("2. Lookup RFC\n");
       printf("3. List RFC\n");
       printf("4. Quit\n");
       printf("Please enter a choice: ");
       scanf("%d",&choice);

         if  (choice==1) {
            char abc[14] = "";
            char portno[6] = "";
            char *line = "\n";
            char *xtra = " P2P/CI 1.0";
            char *word1 = "Host: ";
            char *word2 = "ADD RFC ";
            char word3[16] = "";
            char *word4 = "Title: ";
            int i;

               for (i=0; i<node->total_rfcs; i++) {
         
                  sprintf(abc, "ADD RFC %u", node->rfc_num[i]);
                     if (i==0) {
                        strncpy(retstr, abc, strlen(abc));
                     } else {
                        strncat(retstr, abc, strlen(abc));
                       }
         
                  strncat(retstr, xtra, strlen(xtra));
                  strncat(retstr, line, strlen(line));
         

                  strncat(retstr, word1, strlen(word1));
                  strncat(retstr, node->name, strlen(node->name));
                  strncat(retstr, line, strlen(line));
         
                  snprintf(word3, sizeof(word3), "Port: %u", node->port);
                  strncat(retstr, word3, strlen(word3));
                  strncat(retstr, line, strlen(line));

         
                  strncat(retstr, word4, strlen(word4));
                  strncat(retstr,node->rfc_title[i], strlen(node->rfc_title[i]));
                  strncat(retstr, line, strlen(line));
               }
      
               printf("***data to send start***\n%s\n***data to send end***\n", retstr);
               write(clisock, (void*) retstr, strlen(retstr) + 1);
               sleep(1);
               recv(clisock, recv_buf, MAX_BUF_LEN, 0);
               printf("***server response start***\n%s\n***server response end***", recv_buf);
               memset(&recv_buf, 0, MAX_BUF_LEN);
         } else if (choice ==2) {
            FILE *fptr;
            char givetitle[16]="";
            char lookup[24]="LOOKUP RFC ";
            char lookup2[12]=" P2P/CI 1.0";
            char rfc[6]="";
            char port[6]="";
            char word[7]="Host: ";
            char word1[20]="";
            char line[3]="\n";
            char enterport[6]="";
            char enterip[16]="";
            char entertitle[16]="";
            char enterhost[MAX_NAME_LEN]="";
            int portnum=0;
            int peersock=0;
            int rfcinint=0;
            int num_bytes_recv=0;
            char get[9]="GET RFC ";
            printf("\nEnter the RFC that you want to lookup:");
            scanf("%s",rfc);
            //fgets(rfc,6,stdin);
            rfcinint=atoi(rfc);
            sprintf(word1,"Port: %u",node->port);
            strcpy(retstr,lookup);
            strcat(retstr,rfc);
            strcat(retstr,lookup2);
            strcat(retstr,line);
            strcat(retstr,word);
            strcat(retstr,node->name);
            strcat(retstr,line);
            strcat(retstr,word1);
            printf("\n----  %s\n",retstr);
         // now we need to send this to the peer that has the RFC
            write(clisock, (void*) retstr, strlen(retstr)+1);        
            recv(clisock, recv_buf, MAX_BUF_LEN, 0);         
            printf("***server response start***\n%s\n***server response end***", recv_buf);
            strncpy(recv_buffer,recv_buf,strlen(recv_buf)+1);
               if (recv_buf[0]=='R') {
                  printf("Enter the port number from the received message: ");
                  scanf("%s",enterport);
                  printf("Enter the IP address from the message:");
                  scanf("%s",enterip);
                  struct sockaddr_in sa;
                     while ((inet_pton(AF_INET,enterip, &(sa.sin_addr)))<=0) {
                        printf("Enter the IP address from the message:");
                        scanf("%s",enterip);
                     }
                  printf("Enter the Hostname:");
                  scanf("%s",enterhost);
                  printf("Enter the Title:");
                  scanf("%s",entertitle);                                       
                  portnum=atoi(enterport);
                  cli_ad.sin_port=htons(portnum);
                  cli_ad.sin_family=AF_INET;
                  cli_ad.sin_addr.s_addr=inet_addr(enterip);
                    if ((peersock=socket(AF_INET,SOCK_STREAM,0)),0) {
                       printf("\n Error: could not create socket"); 
                    }
            //if (inet_pton(AF_INET,enterip,&cli_ad.sin_addr)<=0) {
            //   printf("\n Inet_pton failed");
            //   return 1;
            //}
                   if (connect(peersock,(struct sockaddr *)&cli_ad,sizeof(cli_ad))<0) {   
                      printf("\n connect failed");
                   }
            /* now the message to be sent is got */
                   strcpy(retstr,get);
                   strcat(retstr,rfc);
                   strcat(retstr,lookup2);
                   strcat(retstr,line);
                   strcat(retstr,word);
                   strcat(retstr,enterhost);
                   strcat(retstr,line);
                   strcat(retstr,os);
                   printf("Enter the file name to be saved:");
                   scanf("%s",givetitle);       
           /* now we need to send this message */
                   write(peersock, (void*) retstr, strlen(retstr)+1);
                   memset(recv_buf, 0, sizeof(recv_buf));
                   fptr=fopen(givetitle,"w");
                      if (fptr==NULL){
                         perror("file pointer not created");
                      }
           
                        while (1) {
                           num_bytes_recv = recv(peersock, recv_buf, MAX_BUF_LEN,0);
                              if (num_bytes_recv < 0) {
                                 perror("ERROR: Peer recv() failed");
                              } else if (0 == num_bytes_recv) {
                                  break;
                              }

                        printf("**server response start**\n%s\n**server response end**",recv_buf);
                        fprintf(fptr,"%s",recv_buf);
           //printf("\n THE SIZE OF THE BUFFER IS %d",strlen(recv_buf));
                        memset(&recv_buf, 0, MAX_BUF_LEN); 
                        }
                        fclose(fptr);
                     }
                        if (recv_buffer[0]=='R') {
                           memset(&recv_buf, 0, MAX_BUF_LEN);
                           char *stringforsend=NULL;
                           stringforsend=registernewrfc(rfcinint, node->port, entertitle, node->name);
                     /* ---- update the node structure ---- */
                        node->rfc_num[node->total_rfcs]=rfcinint;
                        strncpy(node->rfc_title[node->total_rfcs],entertitle,strlen(entertitle)+1);
                        node->total_rfcs++;
                     /* ---- send to server ---- */
                        write(clisock,(void*) stringforsend, strlen(stringforsend)+1);
                        recv(clisock, recv_buf, MAX_BUF_LEN, 0); 
                        printf("**server response start**\n%s\n**server response end**",recv_buf);
                        memset(&recv_buf, 0, MAX_BUF_LEN);
                        }

         } else if (choice ==3) {
              char list[20]="LIST ALL P2P/CI 1.0";
              char port[6]="";
              char word[7]="Host: ";
              char word1[20]="";
              char line[3]="\n";
              sprintf(word1,"Port: %u",node->port);
              strcpy(retstr,list);
              strcat(retstr,line);
              strcat(retstr,word);
              strcat(retstr,node->name);
              strcat(retstr,line);
              strcat(retstr,word1);
              //printf("\n the returned string is %s",retstr);
              write(clisock, (void*) retstr, strlen(retstr)+1);
              recv(clisock, recv_buf, MAX_BUF_LEN, 0);         
              printf("***server response start***\n%s\n***server response end***", recv_buf);
              memset(&recv_buf, 0, MAX_BUF_LEN);           
              continue;
              } else if (choice==4) {
                   char leave[15]="LEAVING SYSTEM";
                   strncpy(retstr,leave,strlen(leave));
                   printf("\n---- %s",retstr);
                   write(clisock, (void*) retstr, strlen(retstr)+1);
                   recv(clisock, recv_buf, MAX_BUF_LEN, 0);         
                   printf("***server response start***\n%s\n***server response end***", recv_buf);
                   memset(&recv_buf, 0, MAX_BUF_LEN);
                   free(retstr);
                   break;
                     
              }
      sleep(1);
  }
return 1;
}
/* ----------------------------------------------------------------------*/

