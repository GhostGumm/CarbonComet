
#include <jansson.h>
#include "cometd.h"

#define PRESENCE_FIELD "presence"
#define RESOURCE_FIELD "resource"
#define UID_FIELD "uid"
#define PRESENCE_ANNOUNCE_CHANNEL "/service/presence/announce"
#define PRESENCE_REQUEST_CHANNEL "/service/presence/request"
#define CHAT_CHANNEL "/service/districhat/chat"
#define SENDER_FIELD "sender"
#define CONTENT_FIELD "content"
#define TARGETS_FIELD "targets"

static bool cometd_presence_announce_callback(cometd_client_t * client, cometd_message * message) {
	printf("Announce : presence %s for resource %s and uid %s \n",
		json_string_value(json_object_get(message->data, PRESENCE_FIELD)),
		json_string_value(json_object_get(message->data, RESOURCE_FIELD)),
		json_string_value(json_object_get(message->data, UID_FIELD)));
	return false;
}
static bool cometd_presence_request_callback(cometd_client_t * client, cometd_message * message) {
	printf("presence requested \n");
	json_t * data = json_object();
	json_object_set_new(data, PRESENCE_FIELD, json_string_nocheck("ON"));
	cometd_publish(client, PRESENCE_ANNOUNCE_CHANNEL, data);
	return false;
}
static bool cometd_chat_callback(cometd_client_t * client, cometd_message * message) {
	printf("chat received from '%s' : '%s'\n", json_string_value(json_object_get(message->data, SENDER_FIELD)), json_string_value(json_object_get(message->data, CONTENT_FIELD)));
	return false;
}
static void cometd_send_chat_message(cometd_client_t * client, const char * message, const char * target) {
	json_t * data = json_object();
	json_object_set_new(data, CONTENT_FIELD, json_string_nocheck(message));
	json_t * targets = json_array();
	json_array_append_new(targets, json_string_nocheck(target));
	json_object_set_new(data, TARGETS_FIELD, targets);
	cometd_publish(client, CHAT_CHANNEL, data);
}
int main(int argc, char** argv) {
	if (argc != 5) {
		printf("Bad number of arguments (%d) . Usage : %s <server> <login> <resource> <buddy>\n", argc-1, argv[0]);
		exit(1);
	}
	int argn = 1;
	const char * server = argv[argn++];
	const char * login = argv[argn++];
	const char * resource = argv[argn++];
	const char * buddy = argv[argn++];
	
	printf("Starting test program for user %s and resource %s\n", login, resource);
	if (cometd_init()) {
		printf("cometd_init failed\n");
	} else {
		cometd_client_t * client = cometd_create_client(server, cometd_create_long_polling_transport());
		if (client) {
			printf("client created\n");
			
			json_t * handshakeFields = json_object();
			json_t * ext = json_object();
			json_t * auth = json_object();
			json_t * data = json_object();
			json_object_set_new(data, "login", json_string_nocheck(login));
			json_object_set_new(data, "password", json_string_nocheck("password"));
			json_object_set_new(auth, "data", data);
			json_object_set_new(auth, "action", json_string_nocheck("authenticate"));
			json_object_set_new(auth, "type", json_string_nocheck("filestub"));
			json_object_set_new(auth, "resource", json_string_nocheck(resource));
			json_object_set_new(auth, "version", json_string_nocheck("none"));
			json_object_set_new(ext, "authentication", auth);
			json_object_set_new(handshakeFields, "ext", ext);
			
			cometd_channel_subscribe(client, &cometd_presence_announce_callback, PRESENCE_ANNOUNCE_CHANNEL);
			cometd_channel_subscribe(client, &cometd_presence_request_callback, PRESENCE_REQUEST_CHANNEL);
			cometd_channel_subscribe(client, &cometd_chat_callback, CHAT_CHANNEL);
			int i = 0;
			if (!cometd_handshake(client, handshakeFields)) {
				printf("cometd_handshake OK\n");
				while (!cometd_main_loop(client)) {
					//printf("cometd_main_loop OK\n");
					if (!(i++ % 10))
						cometd_send_chat_message(client, "this is a chat message", buddy);
					if (i == 25) {
						printf("cometd_disconnect called\n");
						cometd_disconnect(client);
					}
				}
			} else
				printf("cometd_handshake failed\n");
		}
	}
	return 0;
}
