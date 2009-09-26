// MorgenGrauen MUDlib
//
// external.c -- external udp requests
//
// $Id: external.c 6081 2006-10-23 14:12:34Z Zesstra $

void _receive_udp(string message)
{
  int idx;

  if(previous_object() != find_object("secure/master"))
    return;

  if((idx = strstr(message, ":")) > 0)
      catch(("/p/daemon/udp/"+lower_case(message[0..idx-1]))->_receive_udp(message));
}
