#ifndef p_PublicH
#define p_PublicH
#define MAXLEN 10240


/*���õ�ȥ���ַ�����β��" ","\r","\n","\t"*/
void CutEndSpace(char *_str);

void CutBeginSpace(char *_str);

/*���õ�ȥ���ַ������е�" ","\r","\n","\t"*/
void CutAllSpace(char *_str);

/*���õ�ȥ���ַ�������ͷ��" ","\r","\n","\t"*/
void CutBothEndsSpace(char *_str);
#endif