AM_CFLAGS = -std=c99 -Wall -Werror --pedantic

lib_LTLIBRARIES = libcometd.la libzetapush.la
libcometd_la_SOURCES = cometd.c http_transport.c comet_message.c comet_buffer.c
libcometd_la_LIBADD = ${libcurl_LIBS} ${libjansson_LIBS}
libzetapush_la_SOURCES = zetaclient.c
libzetapush_la_LIBADD = ${libcurl_LIBS} ${libjansson_LIBS} ${libcometd_LIBS}

bin_PROGRAMS = ccometd zetatest
ccometd_SOURCES = test.c
ccometd_LDADD = libcometd.la ${libcurl_LIBS} ${libjansson_LIBS}
zetatest_SOURCES = ztest.c
zetatest_LDADD = libcometd.la libzetapush.la ${libcurl_LIBS} ${libjansson_LIBS}
