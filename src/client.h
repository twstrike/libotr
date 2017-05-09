#ifndef OTR4_CLIENT_H
#define OTR4_CLIENT_H

#include "otrv4.h"
#include "list.h"

typedef struct {
	char *recipient;
	otrv4_t *conn;

	//const char* accountname;
	//const char* proto; //???
	//otrl_instag_t their_instance;
	//otrl_instag_t our_instance;
} otr4_conversation_t;

//A client handle messages from/to a sender to/from multiple recipients.
typedef struct {
	otrv4_callbacks_t *callbacks;

	otrv4_keypair_t *keypair;
	list_element_t *conversations;
} otr4_client_t;

otr4_client_t *otr4_client_new(otrv4_keypair_t * keypair);

void otr4_client_free(otr4_client_t * client);

char *otr4_client_query_message(const char *recipient, const char *message,
				otr4_client_t * client);

int
otr4_client_send(char **newmessage, const char *message, const char *recipient,
		 otr4_client_t * client);

int
otr4_client_receive(char **newmessage, char **todisplay, const char *message,
		    const char *recipient, otr4_client_t * client);

int
otr4_client_disconnect(char **newmessage, const char *recipient,
		       otr4_client_t * client);

otr4_conversation_t *otr4_client_get_conversation(int force,
						  const char *recipient,
						  otr4_client_t * client);

int
otr4_client_get_our_fingerprint(otrv4_fingerprint_t fp,
				const otr4_client_t * client);

int otr4_read_privkey_FILEp(otr4_client_t * client, FILE * privf);

#endif
