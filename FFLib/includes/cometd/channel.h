#ifndef COMETD_CHANNEL_H
#define COMETD_CHANNEL_H

#include "../cometd.h"
#include <glib.h>


gboolean cometd_channel_is_wildcard(const char* channel);
GList*   cometd_channel_matches(const char* channel);
void     cometd_channel_matches_free(GList* matches);

#endif
