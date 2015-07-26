// MorgenGrauen MUDlib
//
// www.crypt.c -- Cryptologic functionality to conceal path names
//
// $Id: www.crypt.c 8755 2014-04-26 13:13:40Z Zesstra $

#pragma strong_types
#pragma combine_strings

string _crypt(string str, string tab, string passwd)
{
  int i;
  i = sizeof(str);
  while(i--)
    str[i]= tab[(member(tab, str[i]) 
          + member(tab, passwd[i % sizeof(passwd)])) % sizeof(tab)];
  return str;
}

string _decrypt(string str, string tab, string passwd)
{
  int i, tmp;
  i = sizeof(str);
  while(i--) {
    if ((tmp=(member(tab, str[i]) - member(tab, passwd[i%sizeof(passwd)])))<0)
      tmp += sizeof(tab);
    str[i] = tab[tmp];
  }
  return str;
}
