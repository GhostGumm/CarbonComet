#include <stdlib.h>
#include <search.h>
#include <string.h>

#include "cometd.h"
#include "cometd_internal.h"


cometd_malloc cometd_malloc_fn = &malloc;
cometd_free cometd_free_fn = &free;

void * cometd_zmalloc_fn(size_t size) {
	void * result = cometd_malloc_fn(size);
	if (result)
		memset(result, 0, size);
	return result;
}

static int compare_subs(const void * s1, const void * s2) {
	if (!s1)
		return s2 ? -1 : 0;
	if (!s2)
		return 1;
	cometd_client_subscription* sub1 = (cometd_client_subscription*)s1;
	cometd_client_subscription* sub2 = (cometd_client_subscription*)s2;
	return strcmp(sub1->channel, sub2->channel);
}

void cometd_set_alloc_funcs(cometd_malloc malloc_fn, cometd_free free_fn) {
	cometd_malloc_fn = malloc_fn;
	cometd_free_fn = free_fn;
}

void onHandshakeRequestSent(cometd_client_t * client) {
	if (client->state == UNCONNECTED)
		client->state = CONNECTING;
}
void onTimeout(cometd_client_t * client) {
	client->state = UNCONNECTED;
}
void onSuccessfulConnect(cometd_client_t * client) {
	if (client->state == CONNECTING)
		client->state = CONNECTED;
}

void onDisconnectRequest(cometd_client_t * client) {
	client->state = UNCONNECTED;
}
int cometd_init(void) {/* Here its to init the transport structure but it is bad to use globals specilay for a library */
	return cometd_http_transport_init(&cometd_transport_default_message_parser);
}
static bool cometd_handshake_callback(cometd_client_t * client, cometd_message * message) {
	CMTD_TRACE_IN
	cometd_client_impl* cli = (cometd_client_impl*)client;
	//printf("received ");
	//cometd_dump_message_file(stdout, message);
	//this was already comented printf("\n");
	if (message->clientId) {
		cli->firstHandshakeOk = true;
		CMTD_TRACE_DEBUG("message->clientId received %s\n", message->clientId)
		size_t size = strlen(message->clientId) + 1;
		char * b = cometd_malloc_fn(size);
		memcpy(b, message->clientId, size);
		cli->clientId = b;
	}
	CMTD_TRACE_DEBUG("message->successful : %d\n", message->successful)
	if (message->successful) {
		cometd_connect(client, message);
		bool result = cometd_main_loop(client);
		CMTD_RETURN(result)
	}
	CMTD_TRACE_DEBUG("cometd_handshake_callback FAILED\n")
	CMTD_RETURN(true)
}
static bool cometd_disconnect_callback(cometd_client_t * client, cometd_message * message) {
	//printf("disconnected\n");
	return true;
}
cometd_client_t * cometd_create_client(const char * serverUrl, cometd_transport_t * transport) {
	CMTD_TRACE_IN
	  /* Here the client is finalizing its initializations to perform a handshake, as well as a Meta_Connect */
	  CALLOC(cometd_client_impl, client); /*Here as well the client struct seems to be given as a global variable,hummm....*/
	client->base.transport = transport;
	client->subscriptions = 0;
	transport->serverUrl =  serverUrl;
	transport->init(transport, serverUrl);
	//for (int i=0;i<50;i++)
	cometd_channel_subscribe(&client->base, &cometd_handshake_callback, META_HANDSHAKE);
	cometd_channel_subscribe(&client->base, &cometd_connect, META_CONNECT);
	cometd_channel_subscribe(&client->base, &cometd_disconnect_callback, META_DISCONNECT);
	CMTD_RETURN( &client->base);
}
static cometd_client_subscription* cometd_find_subscription(cometd_client_t * client, const char * channel) {
	if (!client || !channel)
		return 0;
	cometd_client_impl* cli = (cometd_client_impl*)client;
	cometd_client_subscription test;
	test.channel = channel;
	void ** found = tfind(&test, &cli->subscriptions, &compare_subs);
	return found ? *(cometd_client_subscription**)found : 0;
}
/** returns true if unsuccessful */
bool cometd_dispatch_message(cometd_client_t * client, cometd_message * message) {
	CMTD_TRACE_IN
	if (message && (cometd_isMeta(message) || !(message->successful && !message->data))) {
		CMTD_TRACE_DEBUG("A message to dispatch on channel %s\n", message->channel)
		cometd_client_subscription* sub = cometd_find_subscription(client, message->channel);
		if (sub && sub->callbacks) {
			CMTD_TRACE_DEBUG("Found callback(s) for channel %s\n", message->channel)
			bool result = false;
			cometd_subscription_callback * array = (cometd_subscription_callback *)sub->callbacks->array;
			while (*array) {
				result |= (*array)(client, message);
				array++;
			}
			CMTD_TRACE_DEBUG("result is %d\n", result)
			CMTD_RETURN(result);
		} else {
			//printf("No callback for channel %s\n", message->channel);
		}
	}
	CMTD_RETURN(false);
}
static bool cometd_channel_needs_meta_subscribe(const char * channel) {
	return !cometd_string_startsWith(channel, SERVICE_PREFIX) && ! cometd_string_startsWith(channel, META_PREFIX);
}
void cometd_channel_subscribe(cometd_client_t * client, cometd_subscription_callback callback, const char * channel) {
	CMTD_TRACE_IN
	cometd_client_impl* cli = (cometd_client_impl*)client;
	cometd_client_subscription* sub = cometd_find_subscription(client, channel);/*Here we iterate ont the client with the channel name to chekc if we aren't already subscribed */
	if (!sub) {
		sub = cometd_zmalloc_fn(sizeof(cometd_client_subscription));
		sub->channel = channel;
		tsearch(sub, &cli->subscriptions, &compare_subs);// TODO check error
	}
	cometd_array_add((void*)(unsigned long int)callback, &sub->callbacks);
	if (cometd_channel_needs_meta_subscribe(channel)) {
		printf("TODO : /meta/subscribe for public channels if it's really needed");
	}
	CMTD_TRACE_DEBUG("Added a callback for channel %s\n", channel)
	CMTD_TRACE_OUT
}
bool cometd_main_loop(cometd_client_t * client) {
  CMTD_TRACE_IN /* perform a longpoling connection */
	bool result = client->transport->main(client->transport, client);
	CMTD_TRACE_DEBUG("result is %d\n", result)
	CMTD_RETURN(result);
}
