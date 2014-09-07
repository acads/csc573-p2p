/* p2p_utils.c */

/* This file implements all the utility routines that are required by
 * the P2P TCP server-client. It includes client message processing and data
 * structure updates.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "p2p_utils.h"

/* Global definitions */
uint32_t g_total_peers;
uint32_t g_active_peers;
uint32_t g_total_rfcs;
peer_db *g_first_peer = NULL;
peer_db *g_last_peer = NULL;

const char *g_add_msg = "ADD RFC ";
const char *g_list_msg = "LIST ";
const char *g_lookup_msg = "LOOKUP ";
const char *g_unknown_msg = "UNKNOWN ";
const char *g_ok_msg = "200 OK";
const char *g_not_found_msg = "404 Not Found";
char *g_bad_request_msg = "400 Bad Request";

void
extract_message(char *msg_str)
{
   return;
}

/* Function: extract message

Descripion: this function will find out which method we need to use
these will lead to either register, list or lookup
Params
str --> the intiial string that the client sent to the server
*/
int 
match(char *haystack, const char *needle)
{
   int j = 0;		
   int i = 0;
   int counter = 0;

   for (i = 0; i < strlen(haystack); i++) {
      if (haystack[i] == needle[j]) {
         if (j == strlen(needle) - 1) {
            counter++;
            j = 0;
            continue;
         }
      } else {
         j = 0;
         continue;
      }
      j++;
   }
   return counter;
}

void
s_init(void)
{
   g_total_peers = 0;
   g_active_peers = 0;
   g_total_rfcs = 0;
   g_first_peer = NULL;
   g_last_peer= NULL;
   return;
}

uint16_t
s_extract_msg_type(char *in_msg)
{
   return;
}

char *
s_handle_lookup_msg(char *str)
{
   DBG_TRACE_ENTER;
   int i = 0;
   int rfnum = 0;
   int count = 0;
   char str1[4] = "RFC ";
   char str2[6] = " HOST ";
   char *retstr = calloc(1, MAX_NAME_LEN);	
   char rfc[6] = "";
   char title[MAX_NAME_LEN] = "";
   char portno[9] = "";
   peer_db *root = NULL;
   peer_db *node = NULL;
 
   DBG_TRACE_ENTER;
   root = get_first_peer();
   node = root;
   str = strstr(str, g_lookup_msg);

   while (str[11 + i] != ' ') {
      rfc[i] = str[11 + i];
      ++i;
   }
   rfnum = atoi(rfc);

// now that we have the port number we can use that to search for the rfc

   while (node) {
      i = 0;
      while (i < node->total_rfcs) {
         if (node->rfc_num[i] == rfnum) {
            strncat(retstr, "RFC ", strlen("RFC "));
            strncat(retstr, rfc, strlen(rfc));
            strncat(retstr, " ", 1);

            strncat(retstr, node->rfc_title[i], strlen(node->rfc_title[i]));
            strncat(retstr, " ", 1);
            strncat(retstr, node->name, strlen(node->name));
            strncat(retstr, " ", 1);

            sprintf(portno, "%u", node->port);
            strncat(retstr, portno, strlen(portno));
            strncat(retstr," ",1);
            strncat(retstr,node->ip,strlen(node->ip));
            DBG_TRACE_EXIT;
            return retstr;
         } else 
            i++;
      }
      node=(peer_db *) node->next;
      i = 0;
   }
   memcpy(retstr, g_not_found_msg, MAX_RET_MSG_LEN);
   return retstr;
}

char *
s_handle_add_msg(char *str,char *str2)
{
   char *temp = NULL;
   int k = 0;
   char *final=NULL;
   char *final1 = calloc(1, strlen(str) + 1);
   final1=str;
   uint16_t port = 0;
   char portno[6] = "";	
   char rfc[5] = "";
   char ipaddr[MAX_NAME_LEN] = "";
   char title[256] = "";
   int j = 0;
   int count1 = 0;
   peer_db *peer = NULL;

   DBG_TRACE_ENTER;   

   temp = calloc(1, MAX_RET_MSG_LEN);
   count1 = match(final1, "ADD RFC");
   while (j < count1) { 
      int i = 0;
      memset(rfc, 0, sizeof(rfc));
      memset(ipaddr, 0, sizeof(ipaddr));
      memset(title, 0, sizeof(title));
      peer = calloc(1, sizeof(peer_db));

      final1=strstr(final1, "ADD RFC");
      while (final1[8+i]!=' ') {
         rfc[i]=final1[8+i];
         i++; 
      }
      peer->rfc_num[0] = atoi(rfc);
      peer->total_rfcs = 1;

      // port number is got now get ip address
      final1=strstr(final1, "Host: ");
      i = 0;
      while (final1[6 + i] != '\n') {
        ipaddr[i] = final1[6 + i];
        i++; 
      }
      strncpy(peer->name, ipaddr, strlen(ipaddr));

      // rfc is got now port number
      final1=strstr(final1, "Port: ");
      i = 0;
      while (final1[6 + i] != '\n') {
         portno[i] = final1[6 + i];
         i++; 
      }
      port=atoi(portno);			
      peer->port=port;

      // ipaddress is got
      final1=strstr(final1, "Title: ");
      i = 0;
      while (final1[7 + i] != '\n') {
        title[i] = final1[7 + i];
        i++; 
      }
      strncpy(peer->rfc_title[0], title, strlen(title));
      strncpy(peer->ip,str2,strlen(str2));
      // title is got			
      i=0;
      final=final1;
      j++;

      if (add_peer(peer))
         memcpy(temp, g_ok_msg, MAX_RET_MSG_LEN);
      else
         memcpy(temp, g_bad_request_msg, MAX_RET_MSG_LEN);
   }
   DBG_TRACE_EXIT;
   return temp;
}

/* char *
 * s_handle_list_msg(void)
 *
 * Returns a neat char string which has all the details of all the peers and its
 * associated RFCs. 
 *
 * Params:
 * void 
 *
 * Returns:
 * char *: pointer to the string containing the peer/RFC details.
 *
 * Note: The caller has to free the returned string!
 */
char *
s_handle_list_msg(void)
{
   peer_db *peer = NULL;
   char tmp[1000] = "";
   char rfc_buf[5000] = "";
   char *peer_buf = NULL;
   uint32_t i = 0;

   DBG_TRACE_ENTER;
   peer_buf = calloc(1, 10000);
   if (NULL == (peer = get_first_peer())) {
      strncpy(peer_buf, "no active peers in system.\n", 100);
      return peer_buf;
   }
   
   while (peer) {
      for (i = 0; i < peer->total_rfcs; ++i) {
         snprintf(tmp, sizeof(tmp), "RFC: %u\nRFC Title: %s\n", 
                  peer->rfc_num[i],peer->rfc_title[i]);
         strcat(rfc_buf, tmp);          
         memset(tmp, 0, sizeof(tmp));
      }
      snprintf(tmp, sizeof(tmp), "IP address: %s\nPeer Name: %s\nPeer Port: %u\nTotal RFCs: %u\n%s\n",peer->ip,peer->name, peer->port, peer->total_rfcs, rfc_buf);
      strcat(peer_buf, tmp);
      memset(tmp, 0, sizeof(tmp));
      memset(rfc_buf, 0, sizeof(rfc_buf));
      peer = peer->next;
   }
   DBG_TRACE_EXIT;
   return peer_buf;
}

/* Name: s_handle_leave_msg
 *
 * Description: removes the data associated with a peer which has just 
 *              left the system
 *
 * Params:
 * peer_name - name of the peer which has just left the system
 *
 * Returns: nothing
 * 
 * Note:
 */
void
s_handle_leave_msg(char *peer_name)
{
   peer_db *peer = NULL;

   DBG_TRACE_ENTER;
   if (NULL == peer_name) {
      DBG_TRACE_NULL_PTR;
      goto CLEANUP_ON_ERROR;
   }
 
   peer = get_peer_by_name(peer_name);
   if (peer)
      del_peer(peer);
   DBG_TRACE_EXIT;  
   return;

CLEANUP_ON_ERROR:
   DBG_TRACE_ERROR_EXIT;
   return;
}

/* Name: s_handle_in_msg
 *
 * Description: Identifies the incoming request type and calls the appropriate
 *              msg handler.
 *
 * Params:
 * in_msg: Pointer to the incoming msg
 *
 * Returns: Pointer to the msg that has to be sent back to client
 * 
 * Note: Caller should free the memory pointed by the returned pointer.
 */
char *
s_handle_in_msg(char *in_msg,char *ipadd)
{
   if (NULL == in_msg) {
      DBG_TRACE_NULL_PTR;
      return;
   }

   if (match(in_msg, g_add_msg)) {
      return s_handle_add_msg(in_msg,ipadd);
   } else if (match(in_msg, g_list_msg)) {
      return s_handle_list_msg();
   } else if (match(in_msg, g_lookup_msg)) {
      return s_handle_lookup_msg(in_msg);
   } else {
      char *temp = NULL;

      temp = calloc(1, MAX_RET_MSG_LEN);
      memcpy(temp, g_bad_request_msg, MAX_RET_MSG_LEN);
      return temp;
   }
}

void
get_msg_type_for_in_msg(char *in_msg, char *msg_type)
{
   if ((NULL == in_msg) || (NULL == msg_type)) {
      DBG_TRACE_NULL_PTR;
      return;     
   }

   if (match(in_msg, g_add_msg)) {
      memcpy(msg_type, g_add_msg, MAX_RET_MSG_LEN);
   } else if (match(in_msg, g_list_msg)) {
      memcpy(msg_type, g_list_msg, MAX_RET_MSG_LEN);
   } else if (match(in_msg, g_lookup_msg)) {
      memcpy(msg_type, g_lookup_msg, MAX_RET_MSG_LEN);
   } else {
      memcpy(msg_type, g_unknown_msg, MAX_RET_MSG_LEN);
   }
}

/* Peer databas util routines */

/* Name: get_peer_by_name
 *
 * Description: returns a pointer to the peer in peer_db list if the given 
 * peer_name is present on the list.
 *
 * Params:
 * peer_name: name of the peer to be searched.
 *
 * Returns:
 * peer_db *: pointer to the peer if found; null otherwise.
 *
 * Note: 
 */
peer_db *
get_peer_by_name(char *peer_name)
{
   peer_db *peer = NULL;
   uint32_t name_len = 0;

   DBG_TRACE_ENTER;
   if (NULL == peer_name) {
      DBG_TRACE_NULL_PTR;
      goto CLEANUP_ON_ERROR;
   }
   
   name_len = strlen(peer_name);
   peer = get_first_peer();
   if (NULL == peer) {
      /* no peers; return */
      DBG_TRACE("Oops! No peers :(\n");
      return NULL;
   }
   while (peer) {
      DBG_TRACE("Searching for peer: %s\n", peer->name);
      if (0 == strncmp(peer_name, peer->name, name_len)) {
         /* got our guy; return his pointer */
         DBG_TRACE("Peer %s found.\n", peer->name);
         return peer;
      }
      peer = peer->next;
   }
   DBG_TRACE_EXIT;
   return NULL;

CLEANUP_ON_ERROR:
   DBG_TRACE_ERROR_EXIT;
   return NULL;   
}

bool
is_rfc_present_for_peer(char *peer_name, uint16_t rfc)
{
   uint16_t i = 0;
   peer_db *peer = 0;

   if (NULL == peer_name) {
      DBG_TRACE_NULL_PTR;
      goto CLEANUP_ON_ERROR;
   }

   peer = get_peer_by_name(peer_name);
   if (NULL == peer) {
      /* new peer */
      return FALSE;
   }

   for (i = 0; i < peer->total_rfcs; ++i) {
      if (rfc == peer->rfc_num[i]) {
         printf("INFO: %s has rfc %d already\n", peer->name, peer->rfc_num[i]);
         return TRUE;
      }
   }
   return FALSE;

CLEANUP_ON_ERROR:
   DBG_TRACE_ERROR_EXIT;
   return FALSE;
}

/* Name: 
 * add_new_rfc_for_peer
 *
 * Description: 
 * adds a new RFC for an existing peer
 *
 * Params:
 * peer - pointer to the incoming peer with new RFC
 * existing - poiter to the exiting peer which has to be updated
 *
 * Returns:
 * TRUE if RFC addition is successful; FALSE otherwise
 * 
 * Note:
 */
bool
add_new_rfc_for_peer(peer_db *peer, peer_db *existing)
{
   uint16_t pos = 0;
   DBG_TRACE_ENTER;
   if (NULL == peer) {
      DBG_TRACE_NULL_PTR;
      goto CLEANUP_ON_ERROR;
   }
  
   if (MAX_RFC_PER_PEER == peer->total_rfcs) {
      return FALSE;
   }
   if (is_rfc_present_for_peer(peer->name, peer->rfc_num[0])) {
      /* adding the same RFC again; ignore*/
      return FALSE;
   }
   pos = existing->total_rfcs;
   existing->rfc_num[pos] = peer->rfc_num[0];
   strncpy(existing->rfc_title[pos], peer->rfc_title[0], MAX_NAME_LEN);
   existing->total_rfcs += 1;
   DBG_TRACE("rfc %d is added to peer %s as %d rfc.\n", existing->rfc_num[pos], 
                     existing->name, existing->total_rfcs);
   free(peer);

   DBG_TRACE_EXIT;
   return TRUE;

CLEANUP_ON_ERROR:
   DBG_TRACE_ERROR_EXIT;
   return FALSE;
}

/* Name: add_peer
 *
 * Description: adds a new peer to the system
 *
 * Params:
 * peer - pointer to the incoming new peer
 *
 * Returns:
 * TRUE if peer addition is successful; FALSE otherwise
 * 
 * Note:
 */
bool
add_peer(peer_db *peer)
{
   peer_db *existing = NULL;

   DBG_TRACE_ENTER;
   if (NULL == peer) {
      DBG_TRACE_NULL_PTR;
      goto CLEANUP_ON_ERROR;
   }

   if (NULL == get_first_peer()) {
      /* first peer */
      g_first_peer = g_last_peer = peer;
      g_first_peer->next = NULL;
      DBG_TRACE("peer %s added in postion 1.\n", peer->name);
   } else if (NULL != (existing = get_peer_by_name(peer->name))) {
      return (add_new_rfc_for_peer(peer, existing));
   } else if (get_last_peer()) {
      /* add to the end of the list */
      g_last_peer->next = peer;
      g_last_peer = peer;
      g_total_peers += 1;
      g_active_peers += 1;
      DBG_TRACE("peer %s added to the end.\n", peer->name);
   }
   DBG_TRACE_EXIT;
   return TRUE;

CLEANUP_ON_ERROR:
   return FALSE;
}

/* Name: del_peer
 *
 * Description:deletes the given peer from the system if present
 *
 * Params:
 * peer - pointer to the peer to be deleted
 *
 * Returns: nothing
 *
 * Note:
 */
void
del_peer(peer_db *peer)
{
   peer_db *i_peer = NULL;
   peer_db *prev = NULL;
   uint16_t name_len = 0;

   DBG_TRACE_ENTER;
   if (NULL == peer) {
      DBG_TRACE_NULL_PTR;
      goto CLEANUP_ON_ERROR;
   }

   if (NULL == g_first_peer) {
      /* No peers at all. Just return. */
      return;
   }

   name_len = strlen(peer->name);
   i_peer = get_first_peer();

   if (peer == get_first_peer()) {
      /* first node is the victim */
      DBG_TRACE("peer %s deleted from the beginning.\n", peer->name);
      prev = i_peer->next;
      g_first_peer = prev;
      free(i_peer);
      return;
   } else if (peer == get_last_peer()) {
      /* last node is the victim */
      DBG_TRACE("peer %s deleted from the end.\n", peer->name); 
      prev = get_second_to_last_peer();
      prev->next = NULL;
      i_peer = g_last_peer;
      g_last_peer = prev;
      free(i_peer);
      return;
   }

   while (i_peer) {
      if (0 == strncmp(peer->name, i_peer->name, MAX_NAME_LEN)) {
         /* i_peer is the victim; take him out! */
         DBG_TRACE("peer %s deleted from the middle.\n", peer->name);
         prev->next = i_peer->next;
         free(i_peer);
         break;
      }
      prev = i_peer;
      i_peer = i_peer->next;
   }
   DBG_TRACE_EXIT;
   return;

CLEANUP_ON_ERROR:
   return;
}

/* Name: get_peer_name_for_in_msg
 *
 * Description: Returns the name of the peer by parsing the given in_msg 
 *
 * Params: Pointer to the incoming message
 *
 * Returns: Fills-in the incoming peer_name pointer with the peer name
 *
 * Note: The caller should allocate sufficient memory for peer_name to hold 
 *       the entire name of the peer.
 */
void
get_peer_name_for_in_msg(char *in_msg, char *peer_name)
{
   uint16_t i = 0;
   char *tmp = NULL;

   DBG_TRACE_ENTER;
   if ((NULL == in_msg) || (NULL == peer_name)) {
      DBG_TRACE_NULL_PTR;
      DBG_TRACE_ERROR_EXIT;
      return;
   }

   tmp = strstr(in_msg, "Host: ");
   if (!tmp)
      return;

   while (tmp[6 + i] != '\n') {
      peer_name[i] = tmp[6 + i];
      i += 1;
   }
   DBG_TRACE_EXIT;
   return;
}

/* Name: get_first_peer
 *
 * Description: returns the first peer in the list
 *
 * Params: none
 *
 * Returns: pointer to the fist peer in the peers list
 *
 * Note:
 */
peer_db
*get_first_peer(void)
{
   return g_first_peer;
}

/* Name: get_last_peer
 *
 * Description: returns the last peer in the list
 *
 * Params: none
 *
 * Returns: pointer to the last peer in the peers list
 *
 * Note:
 */
peer_db *
get_last_peer(void)
{
   return g_last_peer;
}

/* Name: get_second_to_last_peer
 *
 * Description: returns the second to last peer in the list
 *
 * Params: none
 *
 * Returns: pointer to the second to last peer in the peers list
 *
 * Note:
 */
peer_db
*get_second_to_last_peer(void)
{
   peer_db *peer = NULL;

   DBG_TRACE_ENTER;
   peer = get_first_peer();      
   if (NULL == peer->next) {
      /* only one node in list */
      return peer;
   }
   while(NULL != peer->next->next)
      peer = peer->next;

   DBG_TRACE_EXIT;
   return peer;
}

/* Name: get_total_rfcs_for_peer
 *
 * Description: returns the total # of RFCs that the given peer has
 *
 * Params:
 * peer_name: name of the peer whose RFC count is required
 *
 * Returns: the total # of RFCs owned by the peer
 *
 * Note:
 */
uint32_t
get_total_rfcs_for_peer(char *peer_name)
{
   peer_db *peer = NULL;

   if (NULL == peer_name) {
      DBG_TRACE_NULL_PTR;
      goto CLEANUP_ON_ERROR;
   }

   peer = get_peer_by_name(peer_name);
   if (peer)
      return peer->total_rfcs;
   return 0;

CLEANUP_ON_ERROR:
   DBG_TRACE_ERROR_EXIT;
   return 0;
}

