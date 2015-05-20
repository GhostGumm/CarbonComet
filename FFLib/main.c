#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <nopoll.h>
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

void	nopoll_get_conn()
{
   noPollCtx*	ctx = nopoll_ctx_new();
  
  
  nopoll_log_color_enable(ctx, true);
  nopoll_log_enable(ctx, nopoll_true);
  nopoll_conn_connect_timeout(ctx, 10000l);
  /* noPollConn *conn = nopoll_conn_new(ctx,  "54.171.156.38" , "8080" ,  "m.zpush.ovh:8080" , "ws://m.zpush.ovh:8080/str/strd", NULL, "null"); */
  noPollConn *conn = nopoll_conn_new(ctx,  "10.0.0.103", "8080", NULL, NULL, NULL, "null")
;
  if (!nopoll_conn_is_ok(conn))
    {
      puts("Error connection : Failed to send websocket init");
    }
}


int main(void)
{
  cometd	*cometd = cometd_new();
  JsonNode	*connect = json_node_new(JSON_NODE_OBJECT);
  /*After the new you can enable or disable the Websocket simply do this, enabled by defautl*/
  cometd->config->webSockState = true;

  printf("web sock state = %d", cometd->config->webSockState);
  
  cometd_configure(cometd, COMETDOPT_URL, "http://m.zpush.ovh:8080/str/strd");
  cometd_configure(cometd, COMETDOPT_MAX_BACKOFF, 5000);
  struct _cometd_ext* logger = cometd_ext_logger_new();
  cometd_ext_add(&cometd->exts, logger);
  cometd_connect(cometd);
  connect = cometd_msg_connect_new(cometd);
  cometd_transport_send(cometd, connect);  
  //nopoll_get_conn();
  //sleep(2);
  exit (0);
  cometd_subscribe(cometd, "/service/GmY-HuzW/6sd0/ls", handler);
  cometd_subscribe(cometd, "service/GmY-HuzW/6sd0/updateMeta", handler);
  cometd_subscribe(cometd, "/service/GmY-HuzW/6sd0/ls", handler);
  cometd_subscribe(cometd, "/service/GmY-HuzW/6sd0/newFile", handler);
  cometd_transport_send(cometd, cometd_ping_ls(cometd, "/service/GmY-HuzW/6sd0/ls"));  

  cometd_listen(cometd);
  return 0;
}
