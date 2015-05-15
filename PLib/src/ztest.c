#include <jansson.h>
#include "zetaclient.h"

static bool chat_reply_handler(cometd_client_t * client, cometd_message * message) {
	printf("chat message :\n");
	cometd_dump_message_file(stdout, message);
	printf("\n");
	return false;
}	
int main(int argc, char** argv) {
	if (argc != 7) {
		printf("Bad number of arguments (%d) . Usage : %s <server> <login> <resource> <businessId> <deploymentId> <password>\n", argc-1, argv[0]);
		exit(1);
	}
	/* Init : Few initializations from the standard entry, maybe calling a method to put that into a structure will seem smoother*/
	int argn = 1;
	const char * server = argv[argn++];
	const char * login = argv[argn++];
	const char * resource = argv[argn++];
	const char * businessId = argv[argn++];
	const char * deploymentId = argv[argn++];
	const char * password = argv[argn++];
	/* Init */	
	//printf("Starting test program for user %s and resource %s\n", login, resource);	
	if (cometd_init()) { /* it initses a cometd_http_transport_init structure, but it is bad to  allways return 0 :8 */
	  printf("cometd_init failed\n");/*it will never fail */
	} else { /* Here we populate the handshake manager as well as the array inside the data field inside the Handshake manager*/
		zeta_handshake_manager_t * hm = zeta_create_mem_handshake_manager(businessId, deploymentId, login, password);

		/* Here we init the transport structure with the Url and the send receive... function pointers */
		/* The client structure barely wrap all of the info needed to know what channel did we subscribe in, and makes the requests : perform a ptype on the structure to see */
		zeta_client_t * client = zeta_create_client(server, cometd_create_long_polling_transport(), businessId, hm);
		/* Here is the master loop */
		if (client) {
		  printf("client created\n");
		  /* Here it add a callback for our subscription channels   */
		  cometd_channel_subscribe(client->cometClient, chat_reply_handler, "/service/eH1O_PpN/WGkA/reply");
		  int i = 0;
		  if (!zeta_handshake(client))/* Here to handshakes */ {
		    printf("cometd_handshake OK\n");
		    while (!cometd_main_loop(client->cometClient)) { //Here we perform the handshake
		      //printf("cometd_main_loop OK\n");
		      //if (!(i++ % 10))
		      // this was already comented	cometd_send_chat_message(client, "this is a chat message", buddy);
		      if (i == 25) {
			printf("cometd_disconnect called\n");
			cometd_disconnect(client->cometClient);
		      }
		    }
		  } else
		    printf("cometd_handshake failed\n");
		}
	}
	return 0;
}

//http://m.zpush.ovh:8080/str/strd test2 zetaTuto GmY-HuzW KZyH password 
