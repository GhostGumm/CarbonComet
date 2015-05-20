#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <syslog.h>

#include <signal.h>

#include <libwebsockets.h>

int force_exit = 0;

static int
my_protocol_callback(struct libwebsocket_context *context,
		     struct libwebsocket *wsi,
		     enum libwebsocket_callback_reasons reason,
		     void *user, void *in, size_t len)
{
  int n, m;
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 +
		    LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];


  switch (reason) {

  case LWS_CALLBACK_ESTABLISHED:
    printf("New Connection\n");
    break;

  case LWS_CALLBACK_SERVER_WRITEABLE:
    break;

  case LWS_CALLBACK_RECEIVE:

    printf("%s\n",in);

    break;

  case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
    break;

  default:
    break;
  }

  return 0;
}
//**************************************************************************//
/* list of supported protocols and callbacks */
//**************************************************************************//
static struct libwebsocket_protocols protocols[] = {
  /* first protocol must always be HTTP handler */

  {
    "my-protocol",
    my_protocol_callback,
    30,
    10,
  },
  { NULL, NULL, 0, 0 } /* terminator */
};
//**************************************************************************//
void sighandler(int sig)
{
  force_exit = 1;
}
//**************************************************************************//
//**************************************************************************//
int main(int argc, char **argv)
{
  int n = 0;
  struct libwebsocket_context *context;
  int opts = 0;
  const char *iface = NULL;
  int syslog_options = LOG_PID | LOG_PERROR;
  unsigned int oldus = 0;
  struct lws_context_creation_info info;

  int debug_level = 7;

  memset(&info, 0, sizeof info);
  info.port = 9999;


  signal(SIGINT, sighandler);

  /* we will only try to log things according to our debug_level */
  setlogmask(LOG_UPTO (LOG_DEBUG));
  openlog("lwsts", syslog_options, LOG_DAEMON);

  /* tell the library what debug level to emit and to send it to syslog */
  lws_set_log_level(debug_level, lwsl_emit_syslog);

  info.iface = iface;
  info.protocols = protocols;
  info.extensions = libwebsocket_get_internal_extensions();
  info.ssl_cert_filepath = NULL;
  info.ssl_private_key_filepath = NULL;

  info.gid = -1;
  info.uid = -1;
  info.options = opts;

  context = libwebsocket_create_context(&info);
  if (context == NULL) {
    lwsl_err("libwebsocket init failed\n");
    return -1;
  }

  n = 0;
  while (n >= 0 && !force_exit) {

    n = libwebsocket_service(context, 50);

  };//while n>=0


  libwebsocket_context_destroy(context);

  lwsl_notice("libwebsockets-test-server exited cleanly\n");

  closelog();

  return 0;
}//main#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <syslog.h>

#include <signal.h>

#include <libwebsockets.h>

int force_exit = 0;

static int
my_protocol_callback(struct libwebsocket_context *context,
		     struct libwebsocket *wsi,
		     enum libwebsocket_callback_reasons reason,
		     void *user, void *in, size_t len)
{
  int n, m;
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 +
		    LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];


  switch (reason) {

  case LWS_CALLBACK_ESTABLISHED:
    printf("New Connection\n");
    break;

  case LWS_CALLBACK_SERVER_WRITEABLE:
    break;

  case LWS_CALLBACK_RECEIVE:

    printf("%s\n",in);

    break;

  case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
    break;

  default:
    break;
  }

  return 0;
}
//**************************************************************************//
/* list of supported protocols and callbacks */
//**************************************************************************//
static struct libwebsocket_protocols protocols[] = {
  /* first protocol must always be HTTP handler */

  {
    "my-protocol",
    my_protocol_callback,
    30,
    10,
  },
  { NULL, NULL, 0, 0 } /* terminator */
};
//**************************************************************************//
void sighandler(int sig)
{
  force_exit = 1;
}
//**************************************************************************//
//**************************************************************************//
int main(int argc, char **argv)
{
  int n = 0;
  struct libwebsocket_context *context;
  int opts = 0;
  const char *iface = NULL;
  int syslog_options = LOG_PID | LOG_PERROR;
  unsigned int oldus = 0;
  struct lws_context_creation_info info;

  int debug_level = 7;

  memset(&info, 0, sizeof info);
  info.port = 9999;


  signal(SIGINT, sighandler);

  /* we will only try to log things according to our debug_level */
  setlogmask(LOG_UPTO (LOG_DEBUG));
  openlog("lwsts", syslog_options, LOG_DAEMON);

  /* tell the library what debug level to emit and to send it to syslog */
  lws_set_log_level(debug_level, lwsl_emit_syslog);

  info.iface = iface;
  info.protocols = protocols;
  info.extensions = libwebsocket_get_internal_extensions();
  info.ssl_cert_filepath = NULL;
  info.ssl_private_key_filepath = NULL;

  info.gid = -1;
  info.uid = -1;
  info.options = opts;

  context = libwebsocket_create_context(&info);
  if (context == NULL) {
    lwsl_err("libwebsocket init failed\n");
    return -1;
  }

  n = 0;
  while (n >= 0 && !force_exit) {

    n = libwebsocket_service(context, 50);

  };//while n>=0


  libwebsocket_context_destroy(context);

  lwsl_notice("libwebsockets-test-server exited cleanly\n");

  closelog();

  return 0;
}//main
