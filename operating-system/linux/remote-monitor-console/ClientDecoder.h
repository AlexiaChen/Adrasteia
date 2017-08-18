#ifndef CLIENT_DECODER_H
#define CLIENT_DECODER_H

#include "MessageDef.h"

#include <vector>

class ClientDecoder
{
public:
    ClientDecoder(bool &isLogin);
    ClientDecoder(uint8_t* recvBuf,size_t bufLength, bool &isLogin);
    void addRecvBuf(uint8_t* recvBuf, size_t bufLength);
    void clearRecvBufFirst(size_t size);
    bool decodeData(TCP_CLIENT_INFO* clientInfo);
    bool decodeData2(TCP_CLIENT_INFO* clientINfo);
    void decodePackage(TCP_CLIENT_INFO* clientInfo);
    ~ClientDecoder();
private:
    std::vector<uint8_t> m_recv_buffer;

    static const int PACKAGE_HEAD_SIZE = 25;
    bool &m_isLogin;
};

#endif

