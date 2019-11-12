
#include "adapi_def.h"
#include "md5.h"

#define MD5_FUNCTION
typedef struct tagMD5HANDLE
{
    BYTE   szBuffer[64];        //input buffer
    UINT32 nCount[2];            //number of bits, modulo 2^64 (lsb first)
    UINT32 nMD5[4];
}MD5HANDLE;

static void Init_MD5HANDLE(MD5HANDLE *pMd5Handle)
{
    memset(pMd5Handle->szBuffer,0,64);
    pMd5Handle->nCount[0]=0;
    pMd5Handle->nCount[1]=0;

    pMd5Handle->nMD5[0] = MD5_INIT_STATE_0;
    pMd5Handle->nMD5[1] = MD5_INIT_STATE_1;
    pMd5Handle->nMD5[2] = MD5_INIT_STATE_2;
    pMd5Handle->nMD5[3] = MD5_INIT_STATE_3;
}
static  void DWordToByte(BYTE* Output, UINT32* Input, UINT nLength )
{


    //transfer the data by shifting and copying
    UINT i = 0;
    UINT j = 0;
    for ( ; j < nLength; i++, j += 4) 
    {
        Output[j] =   (BYTE)(Input[i] & 0xff);
        Output[j+1] = (BYTE)((Input[i] >> 8) & 0xff);
        Output[j+2] = (BYTE)((Input[i] >> 16) & 0xff);
        Output[j+3] = (BYTE)((Input[i] >> 24) & 0xff);
    }
}
static void ByteToDWord(UINT32* Output, BYTE* Input, UINT nLength)
{


    //initialisations
    UINT i=0;    //index to Output array
    UINT j=0;    //index to Input array

    //transfer the data by shifting and copying
    for ( ; j < nLength; i++, j += 4)
    {
        Output[i] = (UINT32)Input[j]            | 
                    (UINT32)Input[j+1] << 8    | 
                    (UINT32)Input[j+2] << 16 | 
                    (UINT32)Input[j+3] << 24;
    }
}

#ifdef MD5_FUNCTION

static UINT32 RotateLeft(UINT32 x, int n)
{
    return (x << n) | (x >> (32-n));
}

static void FF( UINT32 *pA, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T)
{
    UINT32 F = (B & C) | (~B & D);
    (*pA) += F + X + T;
    (*pA) = RotateLeft((*pA), S);
    (*pA) += B;
}

static void GG( UINT32 *pA, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T)
{
    UINT32 G = (B & D) | (C & ~D);
    (*pA) += G + X + T;
    (*pA) = RotateLeft((*pA), S);
    (*pA) += B;
}


static void HH( UINT32 *pA, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T)
{
    UINT32 H = (B ^ C ^ D);
    (*pA) += H + X + T;
    (*pA) = RotateLeft((*pA), S);
    (*pA) += B;
}



static  void II( UINT32 *pA, UINT32 B, UINT32 C, UINT32 D, UINT32 X, UINT32 S, UINT32 T)
{
    UINT32 I = (C ^ (B | ~D));
    (*pA) += I + X + T;
    (*pA) = RotateLeft((*pA), S);
    (*pA) += B;
}
#else

#define FF( A, B, C, D, X, S, T) \
{   \
    UINT32 F = ((B) & (C)) | (~(B) & (D));  \
    (A) += (F) + (X) + (T); \
    (A)=ROTATE32_LEFT((A),(S)); \
    (A) += (B); \
}
#define GG( A, B, C, D, X, S, T) \
{   \
    UINT32 G = ((B) & (D)) | ((C) & ~(D));      \
    (A) += (G) + (X) + (T);     \
    (A)=ROTATE32_LEFT((A),(S)); \
    (A) += (B);     \
}
#define HH( A, B, C, D, X, S, T) \
{       \
    UINT32 H = ((B) ^ (C) ^ (D));       \
    (A) += (H) + (X) + (T);     \
    (A)=ROTATE32_LEFT((A),(S)); \
    (A) += (B);     \
}
#define II( A, B, C, D, X, S, T) \
{       \
    UINT32 I = ((C) ^ ((B) | ~(D)));       \
    (A) +=  (I) + (X) + (T);     \
    (A)=ROTATE32_LEFT((A),(S)); \
    (A) += (B);     \
}

#endif

static void Transform(MD5HANDLE *pMd5Handle,BYTE Block[64])
{
    //initialise local data with current checksum
    UINT32 a = pMd5Handle->nMD5[0];
    UINT32 b = pMd5Handle->nMD5[1];
    UINT32 c = pMd5Handle->nMD5[2];
    UINT32 d = pMd5Handle->nMD5[3];

    //copy BYTES from input 'Block' to an array of ULONGS 'X'
    UINT32 X[16];
    ByteToDWord( X, Block, 64 );

#ifdef MD5_FUNCTION
    //Perform Round 1 of the transformation
    FF (&a, b, c, d, X[ 0], MD5_S11, MD5_T01); 
    FF (&d, a, b, c, X[ 1], MD5_S12, MD5_T02); 
    FF (&c, d, a, b, X[ 2], MD5_S13, MD5_T03); 
    FF (&b, c, d, a, X[ 3], MD5_S14, MD5_T04); 
    FF (&a, b, c, d, X[ 4], MD5_S11, MD5_T05); 
    FF (&d, a, b, c, X[ 5], MD5_S12, MD5_T06); 
    FF (&c, d, a, b, X[ 6], MD5_S13, MD5_T07); 
    FF (&b, c, d, a, X[ 7], MD5_S14, MD5_T08); 
    FF (&a, b, c, d, X[ 8], MD5_S11, MD5_T09); 
    FF (&d, a, b, c, X[ 9], MD5_S12, MD5_T10); 
    FF (&c, d, a, b, X[10], MD5_S13, MD5_T11); 
    FF (&b, c, d, a, X[11], MD5_S14, MD5_T12); 
    FF (&a, b, c, d, X[12], MD5_S11, MD5_T13); 
    FF (&d, a, b, c, X[13], MD5_S12, MD5_T14); 
    FF (&c, d, a, b, X[14], MD5_S13, MD5_T15); 
    FF (&b, c, d, a, X[15], MD5_S14, MD5_T16); 

    //Perform Round 2 of the transformation
    GG (&a, b, c, d, X[ 1], MD5_S21, MD5_T17); 
    GG (&d, a, b, c, X[ 6], MD5_S22, MD5_T18); 
    GG (&c, d, a, b, X[11], MD5_S23, MD5_T19); 
    GG (&b, c, d, a, X[ 0], MD5_S24, MD5_T20); 
    GG (&a, b, c, d, X[ 5], MD5_S21, MD5_T21); 
    GG (&d, a, b, c, X[10], MD5_S22, MD5_T22); 
    GG (&c, d, a, b, X[15], MD5_S23, MD5_T23); 
    GG (&b, c, d, a, X[ 4], MD5_S24, MD5_T24); 
    GG (&a, b, c, d, X[ 9], MD5_S21, MD5_T25); 
    GG (&d, a, b, c, X[14], MD5_S22, MD5_T26); 
    GG (&c, d, a, b, X[ 3], MD5_S23, MD5_T27); 
    GG (&b, c, d, a, X[ 8], MD5_S24, MD5_T28); 
    GG (&a, b, c, d, X[13], MD5_S21, MD5_T29); 
    GG (&d, a, b, c, X[ 2], MD5_S22, MD5_T30); 
    GG (&c, d, a, b, X[ 7], MD5_S23, MD5_T31); 
    GG (&b, c, d, a, X[12], MD5_S24, MD5_T32); 

    //Perform Round 3 of the transformation
    HH (&a, b, c, d, X[ 5], MD5_S31, MD5_T33); 
    HH (&d, a, b, c, X[ 8], MD5_S32, MD5_T34); 
    HH (&c, d, a, b, X[11], MD5_S33, MD5_T35); 
    HH (&b, c, d, a, X[14], MD5_S34, MD5_T36); 
    HH (&a, b, c, d, X[ 1], MD5_S31, MD5_T37); 
    HH (&d, a, b, c, X[ 4], MD5_S32, MD5_T38); 
    HH (&c, d, a, b, X[ 7], MD5_S33, MD5_T39); 
    HH (&b, c, d, a, X[10], MD5_S34, MD5_T40); 
    HH (&a, b, c, d, X[13], MD5_S31, MD5_T41); 
    HH (&d, a, b, c, X[ 0], MD5_S32, MD5_T42); 
    HH (&c, d, a, b, X[ 3], MD5_S33, MD5_T43); 
    HH (&b, c, d, a, X[ 6], MD5_S34, MD5_T44); 
    HH (&a, b, c, d, X[ 9], MD5_S31, MD5_T45); 
    HH (&d, a, b, c, X[12], MD5_S32, MD5_T46); 
    HH (&c, d, a, b, X[15], MD5_S33, MD5_T47); 
    HH (&b, c, d, a, X[ 2], MD5_S34, MD5_T48); 

    //Perform Round 4 of the transformation
    II (&a, b, c, d, X[ 0], MD5_S41, MD5_T49); 
    II (&d, a, b, c, X[ 7], MD5_S42, MD5_T50); 
    II (&c, d, a, b, X[14], MD5_S43, MD5_T51); 
    II (&b, c, d, a, X[ 5], MD5_S44, MD5_T52); 
    II (&a, b, c, d, X[12], MD5_S41, MD5_T53); 
    II (&d, a, b, c, X[ 3], MD5_S42, MD5_T54); 
    II (&c, d, a, b, X[10], MD5_S43, MD5_T55); 
    II (&b, c, d, a, X[ 1], MD5_S44, MD5_T56); 
    II (&a, b, c, d, X[ 8], MD5_S41, MD5_T57); 
    II (&d, a, b, c, X[15], MD5_S42, MD5_T58); 
    II (&c, d, a, b, X[ 6], MD5_S43, MD5_T59); 
    II (&b, c, d, a, X[13], MD5_S44, MD5_T60); 
    II (&a, b, c, d, X[ 4], MD5_S41, MD5_T61); 
    II (&d, a, b, c, X[11], MD5_S42, MD5_T62); 
    II (&c, d, a, b, X[ 2], MD5_S43, MD5_T63); 
    II (&b, c, d, a, X[ 9], MD5_S44, MD5_T64); 
#else
	//Perform Round 1 of the transformation
	FF (a, b, c, d, X[ 0], MD5_S11, MD5_T01); 
	FF (d, a, b, c, X[ 1], MD5_S12, MD5_T02); 
	FF (c, d, a, b, X[ 2], MD5_S13, MD5_T03); 
	FF (b, c, d, a, X[ 3], MD5_S14, MD5_T04); 
	FF (a, b, c, d, X[ 4], MD5_S11, MD5_T05); 
	FF (d, a, b, c, X[ 5], MD5_S12, MD5_T06); 
	FF (c, d, a, b, X[ 6], MD5_S13, MD5_T07); 
	FF (b, c, d, a, X[ 7], MD5_S14, MD5_T08); 
	FF (a, b, c, d, X[ 8], MD5_S11, MD5_T09); 
	FF (d, a, b, c, X[ 9], MD5_S12, MD5_T10); 
	FF (c, d, a, b, X[10], MD5_S13, MD5_T11); 
	FF (b, c, d, a, X[11], MD5_S14, MD5_T12); 
	FF (a, b, c, d, X[12], MD5_S11, MD5_T13); 
	FF (d, a, b, c, X[13], MD5_S12, MD5_T14); 
	FF (c, d, a, b, X[14], MD5_S13, MD5_T15); 
	FF (b, c, d, a, X[15], MD5_S14, MD5_T16); 

	//Perform Round 2 of the transformation
	GG (a, b, c, d, X[ 1], MD5_S21, MD5_T17); 
	GG (d, a, b, c, X[ 6], MD5_S22, MD5_T18); 
	GG (c, d, a, b, X[11], MD5_S23, MD5_T19); 
	GG (b, c, d, a, X[ 0], MD5_S24, MD5_T20); 
	GG (a, b, c, d, X[ 5], MD5_S21, MD5_T21); 
	GG (d, a, b, c, X[10], MD5_S22, MD5_T22); 
	GG (c, d, a, b, X[15], MD5_S23, MD5_T23); 
	GG (b, c, d, a, X[ 4], MD5_S24, MD5_T24); 
	GG (a, b, c, d, X[ 9], MD5_S21, MD5_T25); 
	GG (d, a, b, c, X[14], MD5_S22, MD5_T26); 
	GG (c, d, a, b, X[ 3], MD5_S23, MD5_T27); 
	GG (b, c, d, a, X[ 8], MD5_S24, MD5_T28); 
	GG (a, b, c, d, X[13], MD5_S21, MD5_T29); 
	GG (d, a, b, c, X[ 2], MD5_S22, MD5_T30); 
	GG (c, d, a, b, X[ 7], MD5_S23, MD5_T31); 
	GG (b, c, d, a, X[12], MD5_S24, MD5_T32); 

	//Perform Round 3 of the transformation
	HH (a, b, c, d, X[ 5], MD5_S31, MD5_T33); 
	HH (d, a, b, c, X[ 8], MD5_S32, MD5_T34); 
	HH (c, d, a, b, X[11], MD5_S33, MD5_T35); 
	HH (b, c, d, a, X[14], MD5_S34, MD5_T36); 
	HH (a, b, c, d, X[ 1], MD5_S31, MD5_T37); 
	HH (d, a, b, c, X[ 4], MD5_S32, MD5_T38); 
	HH (c, d, a, b, X[ 7], MD5_S33, MD5_T39); 
	HH (b, c, d, a, X[10], MD5_S34, MD5_T40); 
	HH (a, b, c, d, X[13], MD5_S31, MD5_T41); 
	HH (d, a, b, c, X[ 0], MD5_S32, MD5_T42); 
	HH (c, d, a, b, X[ 3], MD5_S33, MD5_T43); 
	HH (b, c, d, a, X[ 6], MD5_S34, MD5_T44); 
	HH (a, b, c, d, X[ 9], MD5_S31, MD5_T45); 
	HH (d, a, b, c, X[12], MD5_S32, MD5_T46); 
	HH (c, d, a, b, X[15], MD5_S33, MD5_T47); 
	HH (b, c, d, a, X[ 2], MD5_S34, MD5_T48); 

	//Perform Round 4 of the transformation
	II (a, b, c, d, X[ 0], MD5_S41, MD5_T49); 
	II (d, a, b, c, X[ 7], MD5_S42, MD5_T50); 
	II (c, d, a, b, X[14], MD5_S43, MD5_T51); 
	II (b, c, d, a, X[ 5], MD5_S44, MD5_T52); 
	II (a, b, c, d, X[12], MD5_S41, MD5_T53); 
	II (d, a, b, c, X[ 3], MD5_S42, MD5_T54); 
	II (c, d, a, b, X[10], MD5_S43, MD5_T55); 
	II (b, c, d, a, X[ 1], MD5_S44, MD5_T56); 
	II (a, b, c, d, X[ 8], MD5_S41, MD5_T57); 
	II (d, a, b, c, X[15], MD5_S42, MD5_T58); 
	II (c, d, a, b, X[ 6], MD5_S43, MD5_T59); 
	II (b, c, d, a, X[13], MD5_S44, MD5_T60); 
	II (a, b, c, d, X[ 4], MD5_S41, MD5_T61); 
	II (d, a, b, c, X[11], MD5_S42, MD5_T62); 
	II (c, d, a, b, X[ 2], MD5_S43, MD5_T63); 
	II (b, c, d, a, X[ 9], MD5_S44, MD5_T64); 
    //add the transformed values to the current checksum
#endif
    pMd5Handle->nMD5[0] += a;
    pMd5Handle->nMD5[1] += b;
    pMd5Handle->nMD5[2] += c;
    pMd5Handle->nMD5[3] += d;
}
static void Update(MD5HANDLE *pMd5Handle, BYTE* Input,    UINT nInputLen )
{
    //Compute number of bytes mod 64
    UINT i=0;
    UINT nPartLen=0;
    UINT uTmp=0;
    UINT nIndex; 

    uTmp=pMd5Handle->nCount[0] >> 3;
    nIndex = (UINT)(uTmp & 0x3F);

    //Update number of bits
    uTmp=nInputLen << 3;
    if ( ( pMd5Handle->nCount[0] += uTmp )  <  uTmp )
    {
        pMd5Handle->nCount[1]++;
    }
    uTmp=nInputLen >> 29;
    pMd5Handle->nCount[1] += uTmp;

    //Transform as many times as possible.
    i=0;        
    nPartLen = 64 - nIndex;
    if (nInputLen >= nPartLen)     
    {
        memcpy( &pMd5Handle->szBuffer[nIndex], Input, nPartLen );
        Transform(pMd5Handle, pMd5Handle->szBuffer );
        for (i = nPartLen; i + 63 < nInputLen; i += 64) 
        {
            Transform(pMd5Handle, &Input[i] );
        }
        nIndex = 0;
    } 
    else 
    {
        i = 0;
    }

    // Buffer remaining input
    memcpy( &pMd5Handle->szBuffer[nIndex], &Input[i], nInputLen-i);
}
static int Final(MD5HANDLE *pMd5Handle,char *szMd5Out34,int nMd5OutLen)
{

    int nRet=0;
    int i=0;
    const int nMD5Size = 16;
    unsigned char szMD5[ 16 ];
    UINT32 nIndex;
    UINT32 nPadLen;
    UINT32 uTmp;
    //Save number of bits
    BYTE Bits[8];
    if(!pMd5Handle||!szMd5Out34)return 0;
    DWordToByte( Bits, pMd5Handle->nCount, 8 );

    //Pad out to 56 mod 64.
    uTmp=pMd5Handle->nCount[0] >> 3;
    nIndex = (UINT32)(uTmp & 0x3f);
    nPadLen = (nIndex < 56) ? (56 - nIndex) : (120 - nIndex);
    Update(pMd5Handle, PADDING, nPadLen );

    //Append length (before padding)
    Update(pMd5Handle,  Bits, 8 );

    //Store final state in 'szMD5'

    DWordToByte( szMD5, pMd5Handle->nMD5, nMD5Size );

   
    //Convert the hexadecimal checksum to a CString
    nRet=0;
    for (i=0; i < nMD5Size; i++) 
    {
        if((nMd5OutLen-nRet)<=2)return nRet;

        if (szMD5[i] == 0) 
        {
             nRet+=strcpyn_stop(szMd5Out34+nRet,"00",3,0);
            
        }
        else if (szMD5[i] <= 15)     
        {

            sprintf(szMd5Out34+nRet,"0%x",szMD5[i]);
            nRet+=strlenx(szMd5Out34+nRet);
 
        }
        else 
        {

            sprintf(szMd5Out34+nRet,"%x",szMD5[i]);
            nRet+=strlenx(szMd5Out34+nRet);

        }

    }

    return nRet;
}

int MYAPI GetMD5(const void *pData ,int nDataLen,char *szMd5Out,int nMd5OutLen)
{
    
    MD5HANDLE Md5Handle;
    if(!pData||!szMd5Out)return -1;
    Init_MD5HANDLE(&Md5Handle);

    if(nDataLen==-1)nDataLen=strlenx((char*)pData);
    Update(&Md5Handle,(BYTE*)pData,(UINT)nDataLen);
    return Final(&Md5Handle,szMd5Out,nMd5OutLen);
}

int MYAPI GetMD5File(const char *szFile,char *szMd5Out,int nMd5OutLen)
{
    int nLength = 0;
    MD5HANDLE Md5Handle;
    FILE_HANDLE hFile;
    BYTE Buffer[SIZE_32KB];
    const int nBufferSize = SIZE_32KB;
    if(!szFile||!szMd5Out)
    {
        return -1;
    }
    Init_MD5HANDLE(&Md5Handle);
    hFile=FileOpenXX(szFile,0);
    if(hFile==FILE_HANDLE_INVALID)
    {
        return -1;
    }

    while ((nLength = FileReadXX(hFile, Buffer, nBufferSize )) > 0 )
    {
        Update(&Md5Handle, Buffer, nLength );
    }

    FileCloseXX(hFile);
    return Final(&Md5Handle,szMd5Out,nMd5OutLen);
}
