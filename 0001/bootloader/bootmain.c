#if 1
void bootmain(void)
{
  char *p = (char *)0xb8000;
  int i;
  for (i=0;i<12;i++) {
    *p='a';
    p++;
  }
  
}
#else
void bootmain(void)
{
  char *w = "Hellow World!";
  short *p = (short *)0xb8000;
  int i;
  for (i=0;i<12;i++) {
    *p++=w[i];
  }

}

#endif
