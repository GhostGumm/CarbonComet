

#include <string.h>
#include <unistd.h>

#include "cometd_internal.h"


int cometd_json_dump_callback_file(const char *buffer, size_t size, void *data) {
	return fwrite(buffer, size, 1, (FILE*)data) < 0; // TODO limit length
}
int cometd_json_dump_callback_buffer(const char *buffer, size_t size, void *data) {
	return cometd_buffer_append((cometd_buffer_t*) data, buffer, size);
}

int cometd_dump_message(void * output, cometd_message * message, json_dump_callback_t callback) {
	json_t * root = json_object();
	json_object_set_new(root, CHANNEL_FIELD, json_string_nocheck(message->channel));
	if (message->id)
		json_object_set_new(root, ID_FIELD, json_string_nocheck(message->id));
	if (message->clientId)
		json_object_set_new(root, CLIENT_ID_FIELD, json_string_nocheck(message->clientId));
	json_object_set_new(root, SUCCESSFUL_FIELD, message->successful ? json_true() : json_false());
	json_object_set_new(root, VERSION_FIELD, json_string_nocheck(message->version));
	if (message->supportedConnectionTypes)
		json_object_set(root, SUPPORTED_CONNECTION_TYPES_FIELD, message->supportedConnectionTypes);
	if (message->connectionType)
		json_object_set_new(root, CONNECTION_TYPE_FIELD, json_string_nocheck(message->connectionType));
	if (message->data)
		json_object_set(root, DATA_FIELD, message->data);
	if (message->ext)
		json_object_set(root, EXT_FIELD, message->ext);
	if (message->advice.present) {
		if (message->advice.sourceAdvice)
			json_object_set_new(root, ADVICE_FIELD, message->advice.sourceAdvice);
		else {
			json_t * advice = json_object();
			if (message->advice.reconnect)
				json_object_set_new(advice, RECONNECT_FIELD, json_string_nocheck(message->advice.reconnect));
			json_object_set_new(advice, INTERVAL_FIELD, json_integer(message->advice.interval));
			json_object_set(root, DATA_FIELD, message->data);
			json_object_set_new(root, ADVICE_FIELD, advice);
		}
	}
	int result = json_dump_callback(root, callback, output, 0);
	json_decref(root);
	return result;
}
int cometd_dump_message_file(FILE * f, cometd_message * message) {
	return cometd_dump_message(f, message, &cometd_json_dump_callback_file);
}
int cometd_dump_message_buffer(cometd_buffer_t* buffer, cometd_message * message) {
	// cometd 3.x always expects arrays of messages
	int errCode = 0;
	errCode |= cometd_json_dump_callback_buffer("[", 1, buffer);
	errCode |= cometd_dump_message(buffer, message, &cometd_json_dump_callback_buffer);
	errCode |= cometd_json_dump_callback_buffer("]", 1, buffer);
	return errCode;
}

int cometd_dump_messages(void * output, cometd_array_t * messageList, json_dump_callback_t callback) {
	CMTD_TRACE_IN
	int first = 1, errCode = 0;
	if (messageList) {
		cometd_message ** array = (cometd_message **)messageList->array;
		errCode |= callback("[", 1, output);
		while(*array && !errCode) {
			if (!first)
				errCode |= callback(",", 1, output);
			errCode |= cometd_dump_message(output, *array++, callback);
			first = 0;
		}
		errCode |= callback("]", 1, output);
	}
	CMTD_RETURN(errCode);
}
int cometd_dump_messages_file(FILE * f, cometd_array_t * messageList) {
	return cometd_dump_messages(f, messageList, &cometd_json_dump_callback_file);
}
int cometd_dump_messages_buffer(cometd_buffer_t* buffer, cometd_array_t * messageList) {
	return cometd_dump_messages(buffer, messageList, &cometd_json_dump_callback_buffer);
}
cometd_message * cometd_parse_message(json_t * source) {
	CMTD_TRACE_IN
	CALLOC(cometd_message, message);
	message->source = source;
	message->channel = json_string_value(json_object_get(source, CHANNEL_FIELD));
	message->clientId = json_string_value(json_object_get(source, CLIENT_ID_FIELD));
	message->id = json_string_value(json_object_get(source, ID_FIELD));
	message->version = json_string_value(json_object_get(source, VERSION_FIELD));
	message->connectionType = json_string_value(json_object_get(source, CONNECTION_TYPE_FIELD));
	message->data = json_object_get(source, DATA_FIELD);
	message->ext = json_object_get(source, EXT_FIELD);
	message->supportedConnectionTypes = json_object_get(source, SUPPORTED_CONNECTION_TYPES_FIELD);
	message->successful = json_is_true(json_object_get(source, SUCCESSFUL_FIELD));
	json_t * advice = json_object_get(source, ADVICE_FIELD);
	if (advice) {
		message->advice.present = true;
		message->advice.reconnect = json_string_value(json_object_get(advice, CONNECTION_TYPE_FIELD));
		message->advice.interval = json_integer_value(json_object_get(advice, INTERVAL_FIELD));
	}
	CMTD_RETURN(message);
}
cometd_array_t * cometd_transport_default_message_parser(const char *buffer, size_t size) {
	CMTD_TRACE_IN
	json_error_t errorCode;
	json_t * parsed = json_loadb(buffer, size, 0, &errorCode);
	if (!parsed) {
		printf("Failed to parse '%.*s' : '%s'\n", (int)size, buffer, errorCode.text);
		CMTD_RETURN(0);
	} else {
		cometd_array_t * list = 0;
		size_t size = json_array_size(parsed);
		if (json_is_array(parsed))
			for (size_t i = 0; i < size; i++)
				cometd_array_add(cometd_parse_message(json_array_get(parsed, i)), &list);
		else
			cometd_array_add(cometd_parse_message(parsed), &list);
		CMTD_RETURN(list);
	}
}
bool cometd_string_startsWith(const char * string, const char * prefix) {
	return strstr(string, prefix) == string;
}
bool cometd_isMeta(const cometd_message * message) {
	return message && cometd_string_startsWith(message->channel, META_PREFIX);
}

cometd_message * cometd_message_create(void) {
	CALLOC(cometd_message, message);
	return message;
}
void cometd_message_free(cometd_message * message) {
	if (message) {
		json_decref(message->source);
		cometd_free_fn(message);
	}
}
/*static void millisleep(int millis) {
	timespec req;
	req.tv_sec = millis / 1000;
	req.tv_nsec = (millis - (req.tv_sec * 1000))*1000;
	nanosleep(&req, 0);
}
void millisleep(unsigned int ms) 
{ 
	struct timeval tm;
	tm.tv_sec  = ms/1000L;
	tm.tv_usec = 1000L*(ms%1000L);
	return select(0,NULL,NULL,NULL,&tm); 
}*/
void millisleep(unsigned int ms)
{
	sleep((ms+999) / 1000);
}
bool cometd_handshake(cometd_client_t * client, json_t * handshakeFields) {
	CMTD_TRACE_IN
	CALLOC(cometd_message, message);
	cometd_client_impl* cli = (cometd_client_impl*)client;
	cli->handshakeFields = handshakeFields;
	message->channel = META_HANDSHAKE;
	message->version = DEFAULT_VERSION;
	message->supportedConnectionTypes = json_array();
	json_array_append_new(message->supportedConnectionTypes, json_string_nocheck(LONG_POLLING));
	// TODO manage returned reference for ext
	json_t * ext = json_object_get(handshakeFields, EXT_FIELD);
	message->ext = ext;
	client->transport->sender(client->transport, message, client, true);
	CMTD_RETURN(false);
}
bool cometd_handshake_message(cometd_client_t * client, json_t * handshakeFields) {
	CMTD_TRACE_IN
	cometd_handshake(client, handshakeFields);
	bool result = client->transport->main(client->transport, client);
	CMTD_RETURN(result);
}

void cometd_publish(cometd_client_t * client, const char * channel, json_t * data) {
	CMTD_TRACE_IN
	CALLOC(cometd_message, message);
	cometd_client_impl* cli = (cometd_client_impl*)client;
	message->channel = channel;
	message->version = DEFAULT_VERSION;
	message->clientId = cli->clientId;
	message->data = data;
	client->transport->sender(client->transport, message, client, false);
	CMTD_TRACE_OUT
}
bool cometd_connect(cometd_client_t * client, cometd_message * msg) {
	CMTD_TRACE_IN
	CALLOC(cometd_message, message);
	cometd_client_impl* cli = (cometd_client_impl*)client;
	message->channel = META_CONNECT;
	message->version = DEFAULT_VERSION;
	message->clientId = cli->clientId;
	message->connectionType = LONG_POLLING;
	if (msg && msg->advice.interval)
		millisleep(msg->advice.interval);
	client->transport->sender(client->transport, message, client, false);
	CMTD_RETURN(false);
}
void cometd_disconnect(cometd_client_t * client) {
	cometd_publish(client, META_DISCONNECT, 0);
}
