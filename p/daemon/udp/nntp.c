// INEWS.C -- International News

#pragma strong_types
#pragma combine_strings

#include "/secure/config.h"
#include <daemon.h>
#include <news.h>

#define DEBUG(str) tell_object(find_player("hate"), sprintf("DEBUG: %O\n",str))

void create()
{
  seteuid(getuid());
}

void post(mixed message) {
  string text;

  message[M_BOARD] = "de.alt.test";

  text = "From: "+lower_case(message[M_WRITER])+"@mg.mud.de\n"
         "Newsgroups: "+message[M_BOARD]+"\n"
	 "Subject: "+message[M_TITLE]+"\n"
	 "\n"+message[M_MESSAGE];

  tell_object(find_player("hate"), sprintf("%O", text));
  send_udp(UDPSERV,4123, "NNTP_POST|"+text);
}
