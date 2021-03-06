#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>


#include <stdlib.h>
#include <stdio.h>
#include <wslay/wslay.h>
#include <unistd.h>
#include <nettle/base64.h>
#include <nettle/sha.h>
#include <wslay/wslay.h>
#include <string.h>


int	main()
{
  wslay_event_context_ptr		*ctx = (wslay_event_context_ptr *)malloc(sizeof(wslay_event_context_ptr) * 2);
  struct wslay_event_callbacks		*callbacks = (struct wslay_event_callbacks*)malloc(sizeof(struct wslay_event_callbacks) * 2);
  void					*user_data = malloc(sizeof(void*) * 2);
  uint8_t				buf[4096];
  size_t				len = 50;
  int					flags = 0;

  if (wslay_event_context_client_init(ctx, callbacks, user_data) != 0)
    {exit (-1);}
  wslay_event_config_set_max_recv_msg_length(ctx, 300);
  return (0);
  
}
