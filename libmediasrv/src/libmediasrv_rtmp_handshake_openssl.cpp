
#if defined(_RTMP_OPENSSL_)

#include "libmediasrv_def.h"

#include "rtmp_openssl/crypto.h"
#if defined(_RTMP_SUPPORT_)
#define RTMP_SIG_SIZE 1536



#define HTTP_HEADERS_SERVER_US "ETS Mediasrv,f117cdh@gmail.com"
#define HTTP_HEADERS_SERVER_US_LEN 30
/////////////////////////////////////////////////////////////////////////////////
// 68bytes FMS key which is used to sign the sever packet.
static const uint8_t genuineFMSKey[] = {
    0x47, 0x65, 0x6e, 0x75, 0x69, 0x6e, 0x65, 0x20,
    0x41, 0x64, 0x6f, 0x62, 0x65, 0x20, 0x46, 0x6c,
    0x61, 0x73, 0x68, 0x20, 0x4d, 0x65, 0x64, 0x69,
    0x61, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72,
    0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Media Server 001
    0xf0, 0xee, 0xc2, 0x4a, 0x80, 0x68, 0xbe, 0xe8,
    0x2e, 0x00, 0xd0, 0xd1, 0x02, 0x9e, 0x7e, 0x57,
    0x6e, 0xec, 0x5d, 0x2d, 0x29, 0x80, 0x6f, 0xab,
    0x93, 0xb8, 0xe6, 0x36, 0xcf, 0xeb, 0x31, 0xae
}; // 68

// 62bytes FP key which is used to sign the client packet.
static const uint8_t GenuineFPKey[] = {
    0x47, 0x65, 0x6E, 0x75, 0x69, 0x6E, 0x65, 0x20,
    0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x46, 0x6C,
    0x61, 0x73, 0x68, 0x20, 0x50, 0x6C, 0x61, 0x79,
    0x65, 0x72, 0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Player 001
    0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8,
    0x2E, 0x00, 0xD0, 0xD1, 0x02, 0x9E, 0x7E, 0x57,
    0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
    0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
}; // 62



static uint32_t __GetDHOffset0(uint8_t *pBuffer)
{
    uint32_t offset = pBuffer[1532] + pBuffer[1533] + pBuffer[1534] + pBuffer[1535];
    offset = offset % 632;
    offset = offset + 772;
    if (offset + 128 >= 1536) {
        return 0;
    }
    return offset;
}

static uint32_t __GetDHOffset1(uint8_t *pBuffer)
{
    uint32_t offset = pBuffer[768] + pBuffer[769] + pBuffer[770] + pBuffer[771];
    offset = offset % 632;
    offset = offset + 8;
    if (offset + 128 >= 1536) {
        return 0;
    }
    return offset;
}

uint32_t GetDHOffset(uint8_t *pBuffer, uint8_t schemeNumber) {
    switch (schemeNumber) {
    case 0:
    {
        return __GetDHOffset0(pBuffer);
    }
    case 1:
    {
        return __GetDHOffset1(pBuffer);
    }
    default:
    {
        return __GetDHOffset0(pBuffer);
    }
    }
}


static uint32_t __GetDigestOffset0(uint8_t *pBuffer)
{
    uint32_t offset = pBuffer[8] + pBuffer[9] + pBuffer[10] + pBuffer[11];
    offset = offset % 728;
    offset = offset + 12;
    if (offset + 32 >= 1536) {
        return 0;
    }
    return offset;
}

static uint32_t __GetDigestOffset1(uint8_t *pBuffer)
{
    uint32_t offset = pBuffer[772] + pBuffer[773] + pBuffer[774] + pBuffer[775];
    offset = offset % 728;
    offset = offset + 776;
    if (offset + 32 >= 1536)
    {
        return 0;
    }
    return offset;
}


static uint32_t GetDigestOffset(uint8_t *pBuffer, uint8_t schemeNumber)
{
    switch (schemeNumber)
    {
    case 0:
    {
        return __GetDigestOffset0(pBuffer);
    }
    case 1:
    {
        return __GetDigestOffset1(pBuffer);
    }
    default:
    {

        return __GetDigestOffset0(pBuffer);
    }
    }
}
static BOOL ValidateClientScheme(UINT8 *pBuffer, UINT8 scheme)
{

    uint32_t clientDigestOffset = GetDigestOffset(pBuffer, scheme);

    uint8_t pTempBuffer[1536];
    memcpy(pTempBuffer, pBuffer, clientDigestOffset);
    memcpy(pTempBuffer + clientDigestOffset, pBuffer + clientDigestOffset + 32,
        1536 - clientDigestOffset - 32);

    uint8_t pTempHash[512];
    DHWrapper::HMACsha256(pTempBuffer, 1536 - 32, GenuineFPKey, 30, pTempHash);

    int result = memcmp(pTempHash, pBuffer + clientDigestOffset, 32);


    return result == 0;
}

class  CRTMPEncrypt :
    public CObj
{

public:
    _CPP_UTIL_DYNAMIC(CRTMPEncrypt);
    _CPP_UTIL_CLASSNAME(CRTMPEncrypt);
    _CPP_UTIL_QUERYOBJ(CObj);
    CRTMPEncrypt()
    {

        memset(&m_KeyIn, 0, sizeof(m_KeyIn));
        memset(&m_KeyOut, 0, sizeof(m_KeyOut));
    }
    ~CRTMPEncrypt()
    {

    }

public:
    RC4_KEY m_KeyIn;
    RC4_KEY m_KeyOut;
};

void CRTMPNetworkSrv::InitHandshake()
{
    static BOOL  _bInitHandshake = FALSE;
    if (_bInitHandshake)
    {
        return;
    }
    _bInitHandshake = TRUE;
}
int CRTMPNetworkSrv::OnReceivedHandshake1(CRTMPConnContext *pContext, const unsigned char * dataIn, int length)
{
    static unsigned char _data[1537];

   // length = FileReadEx("D:\\flash\\1\\flashinput.bin", 0, _data, sizeof(_data));
   // dataIn = _data;

    if (length != RTMP_SIG_SIZE + 1)
    {
        return -1;
    }

    if (pContext->m_ctxRtmp == NULL)
    {
        LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s: m_ctxRtmp == NULL\n", __FUNCTION__);
        return -1;
    }
    CRTMPState *state = &(pContext->m_ctxRtmp->m_state);
    CRTMPEncrypt *obEnc = NULL;

    uint8_t handshakeType = dataIn[0];
    BOOL encrypted = FALSE;


    if (handshakeType == 3)
    {
        encrypted = FALSE;
    }
    else if (handshakeType == 6 || handshakeType == 8)
    {
        encrypted = TRUE;
    }
    else
    {
        LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s: Unknown version %02x\n", __FUNCTION__, handshakeType);
        return -1;
    }


    state->m_handshake.m_type = handshakeType;
    state->m_handshake.m_bEncrypted = encrypted;

    UINT8 *pInputBuffer = (uint8_t *)dataIn + 1;
    UINT8 _usedScheme = 0;
    if (ValidateClientScheme(pInputBuffer, 0))
    {
        _usedScheme = 0;
    }
    else if (ValidateClientScheme(pInputBuffer, 1))
    {
        _usedScheme = 1;
    }
    else
    {
        return CRTMPNetAsync::OnReceivedHandshake1(pContext, dataIn, length);
    }
    UINT8 _pOutputBuffer[3072];

    //timestamp
    EHTONLP(_pOutputBuffer, (uint32_t)time(NULL));

    //version
    EHTONLP(_pOutputBuffer + 4, (uint32_t)0x00000000);

    //generate random data
    for (uint32_t i = 8; i < 3072; i++) {
        _pOutputBuffer[i] = rand() % 256;
    }
    for (uint32_t i = 0; i < 10; i++) {
        uint32_t index = rand() % (3072 - HTTP_HEADERS_SERVER_US_LEN);
        memcpy(_pOutputBuffer + index, HTTP_HEADERS_SERVER_US, HTTP_HEADERS_SERVER_US_LEN);
    }


    memset(_pOutputBuffer, 0, sizeof(_pOutputBuffer));
    //**** FIRST 1536 bytes from server response ****//
    //compute DH key position
    uint32_t serverDHOffset = GetDHOffset(_pOutputBuffer, _usedScheme);
    uint32_t clientDHOffset = GetDHOffset(pInputBuffer, _usedScheme);


    //generate DH key
    DHWrapper dhWrapper(1024);

    if (!dhWrapper.Initialize()) {
        return -1;
    }


    if (!dhWrapper.CreateSharedKey(pInputBuffer + clientDHOffset, 128)) {
        return -1;
    }

    if (!dhWrapper.CopyPublicKey(_pOutputBuffer + serverDHOffset, 128)) {
        return -1;
    }


    if (encrypted) {
        uint8_t secretKey[128];
        if (!dhWrapper.CopySharedKey(secretKey, sizeof(secretKey))) {
            return -1;
        }

        if (pContext->m_objEncrypt != NULL)
        {
            pContext->m_objEncrypt->ReleaseObj();
        }
        pContext->m_objEncrypt = obEnc;


        DHWrapper::InitRC4Encryption(
            secretKey,
            (uint8_t*)& pInputBuffer[clientDHOffset],
            (uint8_t*)& _pOutputBuffer[serverDHOffset],
            &obEnc->m_KeyIn,
            &obEnc->m_KeyOut);

        //bring the keys to correct cursor
        uint8_t data[1536];
        RC4(&obEnc->m_KeyIn, 1536, data, data);
        RC4(&obEnc->m_KeyOut , 1536, data, data);
    }

    //generate the digest
    uint32_t serverDigestOffset = GetDigestOffset(_pOutputBuffer, _usedScheme);

    uint8_t *pTempBuffer = new uint8_t[1536 - 32];
    memcpy(pTempBuffer, _pOutputBuffer, serverDigestOffset);
    memcpy(pTempBuffer + serverDigestOffset, _pOutputBuffer + serverDigestOffset + 32,
        1536 - serverDigestOffset - 32);

    uint8_t *pTempHash = new uint8_t[512];
    DHWrapper::HMACsha256(pTempBuffer, 1536 - 32, genuineFMSKey, 36, pTempHash);

    //put the digest in place
    memcpy(_pOutputBuffer + serverDigestOffset, pTempHash, 32);


    //cleanup
    delete[] pTempBuffer;
    delete[] pTempHash;


    //**** SECOND 1536 bytes from server response ****//
    //Compute the chalange index from the initial client request
    uint32_t clientDigestOffset = GetDigestOffset(pInputBuffer, _usedScheme);

    //compute the key
    pTempHash = new uint8_t[512];
    DHWrapper::HMACsha256(pInputBuffer + clientDigestOffset, //pData
        32, //dataLength
        genuineFMSKey, //key
        68, //keyLength
        pTempHash //pResult
        );

    //generate the hash
    uint8_t *pLastHash = new uint8_t[512];
    DHWrapper::HMACsha256(_pOutputBuffer + 1536, //pData
        1536 - 32, //dataLength
        pTempHash, //key
        32, //keyLength
        pLastHash //pResult
        );

    //put the hash where it belongs
    memcpy(_pOutputBuffer + 1536 * 2 - 32, pLastHash, 32);
 

    //cleanup
    delete[] pTempHash;
    delete[] pLastHash;
    //***** DONE BUILDING THE RESPONSE ***//




    CObjNetIOBufferDynamic *ioBuf = CObjNetIOBufferDynamic::CreateObj(sizeof(_pOutputBuffer) + 1, SIZE_1KB);
    if (ioBuf == NULL)
    {
        LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d failed CObjNetIOBufferDynamic::CreateObj\n", __FUNCTION__, __LINE__);
        return -1;
    }
    ioBuf->m_pBuf[0] = state->m_handshake.m_type;


    memcpy(ioBuf->m_pBuf + 1, _pOutputBuffer, sizeof(_pOutputBuffer));
    ioBuf->m_nDataLen = sizeof(_pOutputBuffer) + 1;

  //  FileOverWriteX("D:\\flash\\1\\flashoutput2.bin", ioBuf->m_pBuf+1, ioBuf->m_nDataLen-1);


    if (pContext->m_type == CRTMPCtx::ConnTypeRTMPT)
    {
        CObjNetIOBuffer *ioHeader = pContext->SendHttpContentLength(ioBuf->m_nDataLen);
        if (ioHeader == NULL)
        {
            ioBuf->ReleaseObj();
            LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d failed CreateObj SendHttpContentLength\n", __FUNCTION__, __LINE__);
            return -1;
        }


        if (!pContext->SendIoBuffer(ioHeader))
        {
            ioHeader->ReleaseObj();
            ioBuf->ReleaseObj();
            LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d failed SendIoBuffer\n", __FUNCTION__, __LINE__);
            return -1;
        }

        pContext->DecHttpRequests();
    }

    if (!pContext->SendIoBuffer(ioBuf))
    {
        ioBuf->ReleaseObj();
        LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d failed SendIoBuffer\n", __FUNCTION__, __LINE__);
        return -1;
    }


    return 0;
}

int CRTMPNetworkSrv::OnReceivedHandshake2(CRTMPConnContext *pContext, const unsigned char * data, int length)
{
    return 0;
}

#endif

#endif























