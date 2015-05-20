
#include "cometd_internal.h"

#include <curl/multi.h>
#include <unistd.h>


const size_t BUFFER_SIZE = 128 * 1024;// arbitrary;

typedef struct cometd_transport_long_polling {
	cometd_transport_t base;
	CURLM * multiHandle;
	CURL * sendHandle;
	CURL * receiveHandle;
	cometd_buffer_t readBuffer;// 'read' by libcurl from us, which actually means it's a write buffer for us
	cometd_buffer_t writeBuffer;// 'written' by libcurl for us, which actually means it's a read buffer for us
	cometd_array_t * messageQueue;
	cometd_message * prioritaryMessage;
	char * errorBuffer;
	bool inError;
} cometd_transport_long_polling_t;


static size_t long_polling_write_data(void *buffer, size_t size, size_t nmemb, void *userdata) {
	CMTD_TRACE_IN
	size_t actualSize = size * nmemb;
	cometd_buffer_append((cometd_buffer_t*)userdata, buffer, actualSize);
	CMTD_RETURN(actualSize);
}
static size_t long_polling_read_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
	CMTD_TRACE_IN
	size_t result = cometd_buffer_read((cometd_buffer_t*)userdata, ptr, size * nmemb);
	CMTD_TRACE_DEBUG("sending : %.*s\n", (int)result, (char*)ptr)
	CMTD_RETURN(result);
}
static int long_polling_seek(void *instream, curl_off_t offset, int origin) {
	//printf("long_polling_seek\n");
	return cometd_buffer_read_seek((cometd_buffer_t*)instream, offset, origin) ? CURL_SEEKFUNC_CANTSEEK : CURL_SEEKFUNC_OK;
}

static cometd_transport_message_parser defaultParser;

int cometd_http_transport_init(cometd_transport_message_parser p) {
  defaultParser = p; /* This is not good to globa in c */
	return 0;// OK 
}
/** returns true if unsuccessful */
bool cometd_handle_response(cometd_transport_long_polling_t * longTransport, cometd_client_t * client) {
	CMTD_TRACE_IN
	cometd_buffer_t * b = &longTransport->writeBuffer;
	size_t size = cometd_buffer_readable_bytes(b);
	CMTD_TRACE_DEBUG("received : %.*s\n", (int)size, b->buffer)
	cometd_array_t * messageList = longTransport->base.parser(b->buffer, size);
	cometd_buffer_reset(b);
	bool result = 0;
	if (messageList) {
		cometd_message ** array = (cometd_message **)messageList->array;
		while(*array)
			result |= cometd_dispatch_message(client, *array++);
	} else
		result = true;
	CMTD_TRACE_DEBUG("result is %d\n", result)
	CMTD_RETURN(result);
}
/** returns true if unsuccessful */
bool cometd_long_polling_messages_sender(cometd_transport_t* transport, cometd_client_t * client) {
	CMTD_TRACE_IN
	  /*All transport and data needed to long pole is in longTransport structure ptype it for more info */
	cometd_transport_long_polling_t * longTransport = (cometd_transport_long_polling_t*)transport;
	CMTD_TRACE_DEBUG("dumping msgs\n")
	if (longTransport->prioritaryMessage)
		cometd_dump_message_buffer(&longTransport->readBuffer, longTransport->prioritaryMessage);
	else
		cometd_dump_messages_buffer(&longTransport->readBuffer, longTransport->messageQueue);// TODO check errors
	size_t s = cometd_buffer_readable_bytes(&longTransport->readBuffer);
	CMTD_TRACE_DEBUG("setting CURLOPT_POSTFIELDSIZE : %zu \n", s);
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_POSTFIELDSIZE, s);
	
	// TODO remove that :
	CURLcode code = curl_easy_perform(longTransport->sendHandle);
	cometd_buffer_reset(&longTransport->readBuffer);
	if (code) {
		if (!longTransport->inError)// trace only once
			//printf("Curl error %d : '%s'\n", code, longTransport->errorBuffer);
		longTransport->inError = true;
		cometd_client_impl* cli = (cometd_client_impl*)client;
		CMTD_RETURN(!cli->firstHandshakeOk);
	} else {
		if (longTransport->prioritaryMessage) {
			cometd_message_free(longTransport->prioritaryMessage);
			longTransport->prioritaryMessage = 0;
		} else
			cometd_array_free(&longTransport->messageQueue, (cometd_free)&cometd_message_free);
		longTransport->inError = false;
		bool result = cometd_handle_response(longTransport, client);
		CMTD_RETURN(result);
	}
	/*
	int still_running=0;
	
	CMTD_TRACE_DEBUG("start the fetch")
	do {
	    while(curl_multi_perform(longTransport->multiHandle, &still_running) ==
		CURLM_CALL_MULTI_PERFORM);
	} while (still_running);
	*/
	
	// TODO free list
}

bool cometd_long_polling_main(cometd_transport_t* transport, cometd_client_t * client) {
	cometd_transport_long_polling_t * longTransport = (cometd_transport_long_polling_t*)transport;
	if (longTransport->inError) {
		//printf("wait before retry\n");
		sleep(10);
		cometd_client_impl* cli = (cometd_client_impl*)client;
		cometd_handshake(client, cli->handshakeFields);
	}
	return cometd_long_polling_messages_sender(transport, client); /*The title stand for it self we send the message */
}

void cometd_long_polling_message_sender(cometd_transport_t* transport, cometd_message * message, cometd_client_t * client, bool bypassQueue) {
	CMTD_TRACE_IN
	cometd_transport_long_polling_t * longTransport = (cometd_transport_long_polling_t*)transport;
	if (message) {
		if (bypassQueue)
			longTransport->prioritaryMessage = message;
		else
			cometd_array_add(message, &longTransport->messageQueue);
	}
	CMTD_TRACE_OUT
}
void cometd_long_polling_transport_init(cometd_transport_t * transport, const char * serverUrl) {
	CMTD_TRACE_IN
	cometd_transport_long_polling_t * longTransport = (cometd_transport_long_polling_t*)transport;
	curl_global_init(0);
	
	longTransport->multiHandle = curl_multi_init();
	cometd_buffer_create(&longTransport->readBuffer, BUFFER_SIZE);
	cometd_buffer_create(&longTransport->writeBuffer, BUFFER_SIZE);
	
	longTransport->sendHandle = curl_easy_init();
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_URL, serverUrl);
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_POST, 1L);
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_WRITEFUNCTION, &long_polling_write_data);
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_WRITEDATA, &longTransport->writeBuffer);
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_READFUNCTION, &long_polling_read_data);
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_READDATA, &longTransport->readBuffer);
	longTransport->errorBuffer = (char*)cometd_zmalloc_fn(CURL_ERROR_SIZE);
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_ERRORBUFFER, longTransport->errorBuffer);
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_SEEKFUNCTION , &long_polling_seek);
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_SEEKDATA, &longTransport->readBuffer);
	//curl_easy_setopt(longTransport->sendHandle, CURLOPT_VERBOSE, 1L);
	struct curl_slist *headers=NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Connection: keep-alive");
	headers = curl_slist_append(headers, "Origin: Hakim");
	//headers = curl_slist_append(headers, "Origin: Hakim");
	
	// TODO at cleanup time  : curl_slist_free_all(slist);
	
	curl_easy_setopt(longTransport->sendHandle, CURLOPT_HTTPHEADER, headers);
	
	longTransport->receiveHandle = curl_easy_init();
	curl_easy_setopt(longTransport->receiveHandle, CURLOPT_URL, serverUrl);
	
	// curl_multi_add_handle(longTransport->multiHandle, longTransport->sendHandle);

	CMTD_TRACE_OUT
}

cometd_transport_t * cometd_create_long_polling_transport(void) {
	CMTD_TRACE_IN
	CALLOC(cometd_transport_long_polling_t, transport);
	transport->base.parser = defaultParser;
	transport->base.sender = cometd_long_polling_message_sender;
	transport->base.init = cometd_long_polling_transport_init;
	transport->base.main = cometd_long_polling_main;
	CMTD_RETURN( (cometd_transport_t *)transport);
}


