#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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

nopoll_bool test_sending_and_check_echo (noPollConn * conn, const char * label, const char * msg)
{
  char  buffer[1024];
  int   length = strlen (msg);
  int   bytes_read;

  /* wait for the reply */
  while (nopoll_true) {
    if (nopoll_conn_is_ready (conn))
      break;
    nopoll_sleep (10000);
  } /* end if */

  /* send content text(utf-8) */
  printf ("%s: sending content..\n", label);
  if (nopoll_conn_send_text (conn, msg, length) != length) {
    printf ("ERROR: Expected to find proper send operation..\n");
    return nopoll_false;
  }

  /* wait for the reply (try to read 1024, blocking and with a 3 seconds timeout) */
  bytes_read = nopoll_conn_read (conn, buffer, length, nopoll_true, 3000);
  if (bytes_read > 0)
    buffer[bytes_read] = 0;

  if (bytes_read != length) {
    printf ("ERROR: expected to find 14 bytes but found %d..\n", bytes_read);
    return nopoll_false;
  } /* end if */

  /* check content received */
  if (! nopoll_cmp (buffer, msg)) {
    printf ("ERROR: expected to find message 'This is a test' but something different was received: '%s'..\n",
	    buffer);
    return nopoll_false;
  } /* end if */

  printf ("%s: received reply and echo matches..\n", label);

  /* return that we sent and received the echo reply */
  return nopoll_true;
}

noPollCtx * create_ctx (void) {

  /* create a context */
  noPollCtx * ctx = nopoll_ctx_new ();
  nopoll_log_enable (ctx, nopoll_true);
  nopoll_log_color_enable (ctx, true);
  return ctx;
}

void*          webSocketTest()
{

  noPollConn     * conn;
  noPollCtx      * ctx;

  /* init context */
  ctx = create_ctx ();

  /* create connection */
  conn = nopoll_conn_new (ctx, "echo.websocket.org", "80", NULL, NULL, NULL, NULL);
  if (! nopoll_conn_is_ok (conn)) {
    printf ("ERROR: Expected to find proper client connection status, but found error..\n");
    return nopoll_false;
  } /* end if */

  /* check test */
  if (! test_sending_and_check_echo (conn, "Test 26", "This is a test"))
    return nopoll_false;

  /* close the connection */
  nopoll_conn_close (conn);

  /* release context */
  nopoll_ctx_unref (ctx);

  return nopoll_true;
}


void*          webSocketery()
{
  noPollCtx			*ctx = nopoll_ctx_new();
  noPollMsg			*msg;
  noPollRole			*role = nopoll_conn_role(NOPOLL_ROLE_UNKNOWN);
  struct noPollHandshake	*fetch;
  noPollConnOpts		*opts;

  /* Comment the log lines to disable loging Debug Warnings and Critical errors (Better not)*/
  //nopoll_log_color_enable(ctx, true);
  //nopoll_log_enable(ctx, nopoll_true);
  /* Initializing the cookie options */
  opts = nopoll_conn_opts_new();

  if (!ctx)
    puts("error ctx is nill");
 
  //To add Cookies use this method below
  /* nopoll_conn_opts_set_cookie(opts, "BAYEUX_BROWSER=56a9-mchhnynonz6ji8a6hs1sh49; JSESSIONID=8gz8e00htqrl15vcm3o9yi95f"); */

  // Websocketery Works for mtgox and others servers but not for m.zpush.ovh it keeps rejecting me for an unknown fucking 400 error ! Use Methods below to connect to server, a working example is provided
  //nopoll_conn_new(ctx, ip, port, host, get, protocols, origin)    nopoll_conn_new_opts(ctx, opts, ip, port, host, get, protocols, origin)// 
  

  //noPollConn *conn = nopoll_conn_new(ctx , "54.171.156.38" ,"80" ,"m.zpush.ovh:8080" ,"ws://m.zpush.ovh:8080/str/strd" ,NULL, "Authorize");
  noPollConn *conn = nopoll_conn_new(ctx, "54.238.149.121", "80", "websocket.mtgox.com", "/mtgox", NULL, "chat");
  
  if (!nopoll_conn_wait_until_connection_ready(conn, 50) )
    { puts("nopoll_conn failed, timeout"); return (0);}
  if (nopoll_conn_send_text (conn, "hello how are you doing, do we connect ?", 40) != 40)
    {puts("send text just failed...."); return(0);}
  else
    {
      while (! nopoll_conn_is_ready (conn)) {

	if (! nopoll_conn_is_ok (conn)) {
	  printf ("ERROR (4.1 jkd412): expected to find proper connection handshake finished, but found connection is broken: session=%d, errno=%d : %s..\n",
		  (int) nopoll_conn_socket (conn), errno, strerror (errno));
	  return nopoll_false;
	} /* end if */

	/* wait a bit 10ms */
	nopoll_sleep (10000);
      } /* end if */
      nopoll_conn_close (conn);

      /* finish */
      nopoll_ctx_unref (ctx);

      puts("nopoll conn sucess");
      while (nopoll_true)
	{
	  if (nopoll_conn_is_ready(conn))
	    {
	      puts("break");
	      break;
	    }
	  nopoll_sleep(10000);
	}
	  msg = nopoll_conn_get_msg(conn);
	  if (msg)
	    printf("Msg received = %s\n", nopoll_msg_get_payload(msg));
	    
	  if (!nopoll_conn_is_ok(conn))
	    {
	      puts("------------ Connection Dead ----------------");
	      return nopoll_false;
	    }
	
    }
    nopoll_ctx_unref(ctx);
    return (0);
}


int main(void)
{
  cometd			*cometd = cometd_new();
  JsonNode			*connect = json_node_new(JSON_NODE_OBJECT);
  
  /*After the new you can enable or disable the Websocket simply do this, disabled by defautl*/
  cometd->config->webSockState = true;
  
  
  if (cometd->config->webSockState)
    return(webSocketTest());
  
  cometd_configure(cometd, COMETDOPT_URL, "http://m.zpush.ovh:8080/str/strd");
  cometd_configure(cometd, COMETDOPT_MAX_BACKOFF, 5000);
  struct _cometd_ext* logger = cometd_ext_logger_new();
  cometd_ext_add(&cometd->exts, logger);
  cometd_connect(cometd);
  connect = cometd_msg_connect_new(cometd);
  cometd_transport_send(cometd, connect);


  //cometd_subscribe(cometd, "/service/GmY-HuzW/6sd0/echo", handler);
  cometd_transport_send(cometd, cometd_ping_ls(cometd, "/service/GmY-HuzW/6sd0/echo"));
  //cometd_subscribe(cometd, "/service/GmY-HuzW/6sd0/ls", handler);
  //cometd_subscribe(cometd, "service/GmY-HuzW/6sd0/updateMeta", handler);
  //cometd_subscribe(cometd, "/service/GmY-HuzW/6sd0/ls", handler);
  //cometd_subscribe(cometd, "/service/GmY-HuzW/6sd0/newFile", handler);

  
  cometd_listen(cometd);
  return 0;
}
