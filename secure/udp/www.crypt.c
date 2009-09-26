// MorgenGrauen MUDlib
//
// www.crypt.c -- Cryptologic functionality to conceal path names
//
// $Id: www.crypt.c 5386 2006-09-23 10:29:38Z root $

#pragma strong_types
#pragma combine_strings

string _crypt(string str, string tab, string passwd)
{
  int i;
  i = strlen(str);
  while(i--)
    str[i]= tab[(member(tab, str[i]) 
          + member(tab, passwd[i % strlen(passwd)])) % strlen(tab)];
  return str;
}

string _decrypt(string str, string tab, string passwd)
{
  int i, tmp;
  i = strlen(str);
  while(i--) {
    if ((tmp=(member(tab, str[i]) - member(tab, passwd[i%strlen(passwd)])))<0)
      tmp += strlen(tab);
    str[i] = tab[tmp];
  }
  return str;
}
