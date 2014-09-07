/* server.h */

/* P2P TCP server include file */

#ifndef SERVER_H /* SERVER_H */
#define SERVER_H /* SERVER_H */

#define uint16_t unsigned short
#define uint32_t unsigned int
#define bool unsigned char
#define TRUE 1
#define FALSE 0
//#define DBG_TRACE printf
//#define DBG_TRACE_ENTER printf("---INFO: Entering %s---\n", __func__)
//#define DBG_TRACE_EXIT printf("---INFO: Exiting %s---\n", __func__)
//#define DBG_TRACE_ERROR_EXIT printf("---INFO: Error Exiting %s---\n", __func__)
#define TRACE printf
#define DBG_TRACE //
#define DBG_TRACE_ENTER // 
#define DBG_TRACE_EXIT //
#define DBG_TRACE_ERROR_EXIT //
#define DBG_TRACE_NULL_PTR printf("null pointer: %s %d %s\n", __FILE__, __LINE__, __func__)
#define DBG_TRACE_LOCK printf("mutex locked at: %d %s %s\n", __LINE__, __FILE__, __func__)
#define DBG_TRACE_UNLOCK printf("mutex unlocked at: %d %s %s\n", __LINE__, __FILE__, __func__)
#define LOCK_MUTEX pthread_mutex_lock(&g_lock)
#define UNLOCK_MUTEX pthread_mutex_unlock(&g_lock)
#define free(x) if (x) free(x)

#define SERVER_PORT 7734
#define MAX_PEERS 1024
#define MAX_BUF_LEN 10000
#define MAX_NAME_LEN 256
#define MAX_RFC_PER_PEER 20
#define MAX_RET_MSG_LEN 20

typedef struct peer_db {
   char     name[MAX_NAME_LEN];
   uint16_t port;
   uint16_t total_rfcs;
   uint16_t rfc_num[MAX_RFC_PER_PEER];
   char     rfc_title[MAX_RFC_PER_PEER][MAX_NAME_LEN];
   char     ip[16];
   struct peer_db *next;
} peer_db;

typedef struct peer_table_t {
   uint32_t tid;
   char name[MAX_NAME_LEN];
} peer_table;

/* Globals */
extern uint32_t g_total_peers;
extern uint32_t g_active_peers;
extern uint32_t g_total_rfcs;
extern peer_db *g_first_peer;
extern peer_db *g_last_peer;

/* Server util routines */
uint16_t s_extract_msg_type(char *in_msg);
char *s_handle_in_msg(char *in_msg, char *ipadd);
char *s_handle_add_msg(char *in_msg, char *str2);
char *s_handle_lookup_msg(char *in_msg);
char *s_handle_list_msg(void);
void s_handle_leave_msg(char *peer_name);
void s_init(void);

/* Peer database utils */
void init_peer_db(void);
bool add_peer(peer_db *peer);
bool add_new_rfc_for_peer(peer_db *peer, peer_db *existing);
void del_peer(peer_db *peer);
peer_db *get_peer_by_name(char *peer_name);
bool is_rfc_present_for_peer(char *peer_name, uint16_t rfc);
peer_db *get_first_peer(void);
peer_db *get_next_peer(peer_db *peer);
peer_db *get_last_peer(void);
void get_peer_name_for_in_msg(char *in_msg, char *peer_name);
peer_db *get_second_to_last_peer(void);
uint32_t get_total_peers(void);
uint32_t get_total_rfc_for_peer(peer_db *peer);
char *get_rfcs_for_peer(peer_db *peer);
void get_msg_type_for_in_msg(char *in_msg, char *msg_type);

/* thread functions */
void *listen_client(void *portno);
void *filesend(void *obj);

#endif /* SERVER_H */
