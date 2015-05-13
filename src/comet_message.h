/**
 * Copyright 2012 Pablo Abreu pulco-citron@sourceforge.net
 */

#ifndef COMETD_MESSAGE_H
#define COMETD_MESSAGE_H

#include <stdbool.h>
#include <stdio.h> // FILE
 // includes stdarg

#include "cometd.h"
#include "comet_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	bool present;
	const char * reconnect;
	int interval;
	json_t * sourceAdvice;// for transport specific fields
	// json_t * hosts; TODO ?
	// bool multipleClients; not needed ?
} cometd_message_advice;

typedef struct {
	const char * channel;
	const char * clientId;
	const char * version;
	const char * minimumVersion;
	const char * connectionType;
	json_t * supportedConnectionTypes;// FIXME array
	cometd_message_advice advice;
	const char * id;
	json_t * data;
	const char * timestamp;// TODO ? 
	bool successful, successfulSet;
	const char * subscription;
	json_t * ext;
	json_t * source;
} cometd_message;

/* cometd_message instances MUST NOT be allocated on the stack, because processing can be deferred and asynchronous */
cometd_message * cometd_message_create(void);
void cometd_message_free(cometd_message * message);

bool cometd_isMeta(const cometd_message * message);

int cometd_dump_message(void * output, cometd_message * message, json_dump_callback_t callback);
int cometd_dump_message_file(FILE * f, cometd_message * message);
int cometd_dump_message_buffer(cometd_buffer_t* buffer, cometd_message * message);

int cometd_dump_messages(void * output, cometd_array_t * messageList, json_dump_callback_t callback);
int cometd_dump_messages_file(FILE * f, cometd_array_t * messageList);
int cometd_dump_messages_buffer(cometd_buffer_t* buffer, cometd_array_t * messageList);

cometd_array_t * comet_message_list_create(cometd_message * firstMessage);

#ifdef __cplusplus
}
#endif

#endif

