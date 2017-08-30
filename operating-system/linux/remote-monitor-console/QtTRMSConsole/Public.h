#ifndef p_PublicH
#define p_PublicH
#define MAXLEN 10240


/*共用的去掉字符串行尾的" ","\r","\n","\t"*/
void CutEndSpace(char *_str);

void CutBeginSpace(char *_str);

/*共用的去掉字符串整行的" ","\r","\n","\t"*/
void CutAllSpace(char *_str);

/*共用的去掉字符串行两头的" ","\r","\n","\t"*/
void CutBothEndsSpace(char *_str);
#endif