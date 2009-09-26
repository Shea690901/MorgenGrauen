// MorgenGrauen MUDlib
//
// shells/magier.c -- magier shell
//
// $Id: hate.c 5350 2006-09-23 10:29:02Z root $

inherit "/std/shells/magier";

static int _query_age()
{
  upd_my_age();
  return 0;
}
