#include "../../includes/cometd.h"
#include <stdio.h>
#include <stdlib.h>

static void print_log(const char* type, JsonNode* node);
static void logger_incoming(const cometd* h, JsonNode* msg);
static void logger_outgoing(const cometd* h, JsonNode* msg);

struct _cometd_ext* cometd_ext_logger_new(void)
{
  struct _cometd_ext* logger = cometd_ext_new();
  logger->incoming = logger_incoming;
  logger->outgoing = logger_outgoing;
  return logger;
}


void print_log(const char* type, JsonNode* node)
{
  char* channel = cometd_msg_channel(node);
  char* json = cometd_json_node2str(node);

  puts(type);
  puts(" ");
  puts(channel);
  puts(": ");
  puts(json);
  puts("\n\n");

  free(json);
  free(channel);

   return;
}

void
logger_incoming(const cometd* h, JsonNode* msg)
{
  print_log("incoming", msg);
}

void
logger_outgoing(const cometd* h, JsonNode* msg)
{
  print_log("outgoing", msg);
}
