#include <jansson.h>
#include "zetaclient.h"

static bool chat_reply_handler(cometd_client_t * client, cometd_message * message) {
	//printf("chat message :\n");
	cometd_dump_message_file(stdout, message);
	//printf("\n");
	return false;
}

int main(int argc, char** argv) {
	if (argc != 7) {
		//printf("Bad number of arguments (%d) . Usage : %s <server> <login> <resource> <businessId> <deploymentId> <password>\n", argc-1, argv[0]);
		exit(1);
	}
	int argn = 1;
	const char * server = argv[argn++];
	const char * login = argv[argn++];
	const char * resource = argv[argn++];
	const char * businessId = argv[argn++];
	const char * deploymentId = argv[argn++];
	const char * password = argv[argn++];
	
	//printf("Starting test program for user %s and resource %s\n", login, resource);
	if (cometd_init()) {
		//printf("cometd_init failed\n");
	} else {
		zeta_handshake_manager_t * hm = zeta_create_mem_handshake_manager(businessId, deploymentId, login, password);
		zeta_client_t * client = zeta_create_client(server, cometd_create_long_polling_transport(), businessId, hm);
		if (client) {
			//printf("client created\n");
			cometd_channel_subscribe(client->cometClient, chat_reply_handler, "/service/eH1O_PpN/WGkA/reply");
			int i = 0;
			if (!zeta_handshake(client)) {

				//printf("cometd_handshake OK\n");
				while (!cometd_main_loop(client->cometClient)) {
					//printf("cometd_main_loop OK\n");
					//if (!(i++ % 10))
					// this was already comented	cometd_send_chat_message(client, "this is a chat message", buddy);
					if (i == 25) {
						//printf("cometd_disconnect called\n");
						cometd_disconnect(client->cometClient);
					}
				}
			} //else
				//printf("cometd_handshake failed\n");
		}
	}
	return 0;
}
