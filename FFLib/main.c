#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cometd.h"

static int handler(const cometd* h, JsonNode* msg)
{
  return COMETD_SUCCESS;
}

JsonNode*   cometd_ping_ls(cometd* h, char *target)
{
  JsonObject	*adviceObject = json_object_new();
  JsonNode	*adviceNode = json_node_new(JSON_NODE_OBJECT);
  JsonNode	*adviceRoot = json_node_init_object(adviceNode, adviceObject);

  adviceObject = json_node_get_object(adviceRoot);

  json_object_set_string_member(adviceObject, "folder", "/");

  
  JsonNode*	root = json_node_new(JSON_NODE_OBJECT);
  JsonObject*	obj  = json_object_new();
  gint64	seed = ++(h->conn->msg_id_seed);

  char*  connection_type = cometd_current_transport(h)->name;
  
  json_object_set_int_member   (obj, COMETD_MSG_ID_FIELD,      seed);
  json_object_set_string_member(obj, COMETD_MSG_CHANNEL_FIELD, target);
  json_object_set_string_member(obj, "connectionType",         connection_type);
  json_object_set_member(obj, "data", adviceRoot);
  json_object_set_string_member(obj, "clientId",               cometd_conn_client_id(h->conn));
 
  json_node_take_object(root, obj);

  return(root);
}

int main(void)
{
  cometd	*cometd = cometd_new();
  JsonNode	*connect = json_node_new(JSON_NODE_OBJECT);

  cometd_configure(cometd, COMETDOPT_URL, "http://m.zpush.ovh:8080/str/strd");
  cometd_configure(cometd, COMETDOPT_MAX_BACKOFF, 5000);
  struct _cometd_ext* logger = cometd_ext_logger_new();
  cometd_ext_add(&cometd->exts, logger);
  cometd_connect(cometd);
  connect = cometd_msg_connect_new(cometd);
  cometd_transport_send(cometd, connect);  
  cometd_subscribe(cometd, "/service/GmY-HuzW/6sd0/ls", handler);
  //cometd_subscribe(cometd, "service/GmY-HuzW/6sd0/updateMeta", handler);
  //cometd_subscribe(cometd, "/service/GmY-HuzW/6sd0/ls", handler);
  //cometd_subscribe(cometd, "/service/GmY-HuzW/6sd0/newFile", handler);
  cometd_transport_send(cometd, cometd_ping_ls(cometd, "/service/GmY-HuzW/6sd0/ls"));  

  cometd_listen(cometd);
  return 0;
}
