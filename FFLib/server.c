#include <nopoll.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>



  void listener_on_message (noPollCtx * ctx, noPollConn * conn, noPollMsg * msg, noPollPtr * user_data) {
    // print the message (for debugging purposes) and reply
    printf ("Listener received (size: %d, ctx refs: %d): (first %d bytes, fragment: %d)\n",
	    nopoll_msg_get_payload_size (msg),
	    nopoll_ctx_ref_count (ctx), chown, nopoll_msg_is_fragment(msg));

    // reply to the message
    nopoll_conn_send_text (conn, "Message receivedddd", 19);

    return;
  }

int	main()
{

  noPollCtx * ctx = nopoll_ctx_new ();
  if (! ctx) {
    puts("ctx failure");
  }
  
  
    noPollConn * listener = nopoll_listener_new (ctx, "localhost", "8080");
  if (! nopoll_conn_is_ok (listener)) {
    puts("error on listner");
  }
  nopoll_ctx_set_on_msg (ctx, listener_on_message, NULL);
  nopoll_loop_wait (ctx, 0);


}
