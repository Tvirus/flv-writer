#include "FLVWriter.h"


/* file */
#define FLV_TAG_TYPE_AUDIO    8
#define FLV_TAG_TYPE_VIDEO    9
#define FLV_TAG_TYPE_SCRIPT  18


/* video */
#define FLV_V_FRAME_TYPE_I  1
#define FLV_V_FRAME_TYPE_P  2

#define FLV_V_CODECID_AVC   7

#define FLV_AVCPACKET_TYPE_SPS    0
#define FLV_AVCPACKET_TYPE_NALU   1



/* audio */
#define FLV_A_FORMATE_AAC        10

#define FLV_A_SAMPLE_RATE_5K     0
#define FLV_A_SAMPLE_RATE_11K    1
#define FLV_A_SAMPLE_RATE_22K    2
#define FLV_A_SAMPLE_RATE_44K    3

#define FLV_A_SAMPLE_SIZE_8b     0
#define FLV_A_SAMPLE_SIZE_16b    1

#define FLV_A_SOUND_TYPE_MONO    0
#define FLV_A_SOUND_TYPE_STEREO  1

#define FLV_AACPACKET_TYPE_HEADER  0
#define FLV_AACPACKET_TYPE_RAW     1


/* script */
#define FLV_STRING_METADATA         "onMetaData"
#define FLV_STRING_METADATA_LEN     (sizeof(FLV_STRING_METADATA) - 1)
#define FLV_STRING_DURATION         "duration"
#define FLV_STRING_DURATION_LEN     (sizeof(FLV_STRING_DURATION) - 1)
#define FLV_STRING_WIDTH            "width"
#define FLV_STRING_WIDTH_LEN        (sizeof(FLV_STRING_WIDTH) - 1)
#define FLV_STRING_HEIGHT           "height"
#define FLV_STRING_HEIGHT_LEN       (sizeof(FLV_STRING_HEIGHT) - 1)
#define FLV_STRING_FRAMERATE        "framerate"
#define FLV_STRING_FRAMERATE_LEN    (sizeof(FLV_STRING_FRAMERATE) - 1)
#define FLV_STRING_VCODECID         "videocodecid"
#define FLV_STRING_VCODECID_LEN     (sizeof(FLV_STRING_VCODECID) - 1)
#define FLV_STRING_VDATARATE        "videodatarate"
#define FLV_STRING_VDATARATE_LEN    (sizeof(FLV_STRING_VDATARATE) - 1)
#define FLV_STRING_ADATARATE        "audiodatarate"
#define FLV_STRING_ADATARATE_LEN    (sizeof(FLV_STRING_ADATARATE) - 1)
#define FLV_STRING_ASAMPLERATE      "audiosamplerate"
#define FLV_STRING_ASAMPLERATE_LEN  (sizeof(FLV_STRING_ASAMPLERATE) - 1)
#define FLV_STRING_ASAMPLESIZE      "audiosamplesize"
#define FLV_STRING_ASAMPLESIZE_LEN  (sizeof(FLV_STRING_ASAMPLESIZE) - 1)
#define FLV_STRING_STEREO           "stereo"
#define FLV_STRING_STEREO_LEN       (sizeof(FLV_STRING_STEREO) - 1)
#define FLV_STRING_ACODECID         "audiocodecid"
#define FLV_STRING_ACODECID_LEN     (sizeof(FLV_STRING_ACODECID) - 1)

#define FLV_SCRIPT_VALUE_TYPE_NUM     0
#define FLV_SCRIPT_VALUE_TYPE_BOOL    1
#define FLV_SCRIPT_VALUE_TYPE_STRING  2
#define FLV_SCRIPT_VALUE_TYPE_OBJECT  3





#define WRITE_BUF(pDest, DestSize, pSrc, SrcSize) \
do \
{ \
    if ((SrcSize) > DestSize) \
        return -1; \
    memcpy((void*)pDest, (void*)(pSrc), (size_t)(SrcSize)); \
    pDest = ((u8*)pDest) + (SrcSize); \
    DestSize -= (SrcSize); \
}while(0)

#define WRITE_STRING(pDest, DestSize, pSrc, SrcSize) \
do \
{ \
    if ((SrcSize) + 2 > DestSize) \
        return -1; \
    ((u8*)pDest)[0] = 0xff & (((u16)(SrcSize)) >> 8); \
    ((u8*)pDest)[1] = 0xff &  ((u16)(SrcSize)); \
    pDest = ((u8*)pDest) + 2; \
    DestSize -= 2; \
    memcpy((void*)pDest, (void*)(pSrc), (size_t)(SrcSize)); \
    pDest = ((u8*)pDest) + (SrcSize); \
    DestSize -= (SrcSize); \
}while(0)

/* 自动转成大端 */
#define WRITE_U32(pDest, Size, Value) \
do \
{ \
    if (4 > Size) \
        return -1; \
    ((u8*)pDest)[0] = 0xff & (((u32)(Value)) >> 24); \
    ((u8*)pDest)[1] = 0xff & (((u32)(Value)) >> 16); \
    ((u8*)pDest)[2] = 0xff & (((u32)(Value)) >>  8); \
    ((u8*)pDest)[3] = 0xff &  ((u32)(Value)); \
    pDest = ((u8*)pDest) + 4; \
    Size -= 4; \
}while(0)

#define WRITE_U24(pDest, Size, Value) \
do \
{ \
    if (3 > Size) \
        return -1; \
    ((u8*)(pDest))[0] = 0xff & (((u32)(Value)) >> 16); \
    ((u8*)(pDest))[1] = 0xff & (((u32)(Value)) >>  8); \
    ((u8*)(pDest))[2] = 0xff &  ((u32)(Value)); \
    pDest = ((u8*)pDest) + 3; \
    Size -= 3; \
}while(0)

#define WRITE_U16(pDest, Size, Value) \
do \
{ \
    if (2 > Size) \
        return -1; \
    ((u8*)(pDest))[0] = 0xff & (((u16)(Value)) >> 8); \
    ((u8*)(pDest))[1] = 0xff &  ((u16)(Value)); \
    pDest = ((u8*)pDest) + 2; \
    Size -= 2; \
}while(0)

#define WRITE_U8(pDest, Size, Value) \
do \
{ \
    if (1 > Size) \
        return -1; \
    ((u8*)pDest)[0] = (u8)(Value); \
    pDest = ((u8*)pDest) + 1; \
    Size -= 1; \
}while(0)

#define WRITE_DOUBLE(pDest, Size, Var) \
do \
{ \
    if (8 > Size) \
        return -1; \
    ((u8*)(pDest))[0] = 0xff & ((*(u64*)(&(Var))) >> 56); \
    ((u8*)(pDest))[1] = 0xff & ((*(u64*)(&(Var))) >> 48); \
    ((u8*)(pDest))[2] = 0xff & ((*(u64*)(&(Var))) >> 40); \
    ((u8*)(pDest))[3] = 0xff & ((*(u64*)(&(Var))) >> 32); \
    ((u8*)(pDest))[4] = 0xff & ((*(u64*)(&(Var))) >> 24); \
    ((u8*)(pDest))[5] = 0xff & ((*(u64*)(&(Var))) >> 16); \
    ((u8*)(pDest))[6] = 0xff & ((*(u64*)(&(Var))) >>  8); \
    ((u8*)(pDest))[7] = 0xff &  (*(u64*)(&(Var))); \
    pDest = ((u8*)pDest) + 8; \
    Size -= 8; \
}while(0)


#pragma pack (1)

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
    typedef struct
    {
        u8 aSignature[3]; /* FLV */
        u8 Version; /* 1 */

        u8 rsv1:5; /* 0 */
        u8 TypeFlagAudio:1; /* 是否有音频数据 */
        u8 rsv2:1; /* 0 */
        u8 TypeFlagVideo:1; /* 是否有视频数据 */

        u8 aDataOffset[4]; /* 这个头的长度, 9 */

    }T_FileHeader;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
    typedef struct
    {
        u8 aSignature[3]; /* FLV */
        u8 Version; /* 1 */

        u8 TypeFlagVideo:1; /* 是否有视频数据 */
        u8 rsv2:1; /* 0 */
        u8 TypeFlagAudio:1; /* 是否有音频数据 */
        u8 rsv1:5; /* 0 */

        u8 aDataOffset[4]; /* 这个头的长度, 9 */

    }T_FileHeader;
#else
    #error  need define __BYTE_ORDER__
#endif


#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
    typedef struct
    {
        u8 rsv:2; /* 0 */
        u8 Filter:1;  /* 是否需要解密等预处理 */
        u8 TagType:5; /* 8:audio, 9:video, 18:script */

        u8 aDateSize[3];  /* 从StreamID后面开始，到tag结束的长度，即tag总长度-11 */
        u8 aTimestamp[3]; /* 时间戳低3字节，毫秒 */
        u8 TimeStampEx;   /* 时间戳高1字节，总共可以扩展成4字节 */
        u8 aStreamID[3];  /* 0 */

    }T_TagHeader;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
    typedef struct
    {
        u8 TagType:5; /* 8:audio, 9:video, 18:script */
        u8 Filter:1;  /* 是否需要解密等预处理 */
        u8 rsv:2; /* 0 */

        u8 aDateSize[3];  /* 从StreamID后面开始，到tag结束的长度，即tag总长度-11 */
        u8 aTimestamp[3]; /* 时间戳低3字节，毫秒 */
        u8 TimeStampEx;   /* 时间戳高1字节，总共可以扩展成4字节 */
        u8 aStreamID[3];  /* 0 */

    }T_TagHeader;
#else
    #error  need define __BYTE_ORDER__
#endif


/*
FrameType. The following values are defined:
1 = key frame (for AVC, a seekable frame)
2 = inter frame (for AVC, a non-seekable frame)
3 = disposable inter frame (H.263 only)
4 = generated key frame (reserved for server use only)
5 = video info/command frame

CodecID. The following values are defined:
2 = Sorenson H.263
3 = Screen video
4 = On2 VP6
5 = On2 VP6 with alpha channel
6 = Screen video version 2
7 = AVC
*/
#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
    typedef struct
    {
        u8 FrameType:4;
        u8 CodecID:4;

    }T_VideoHeader;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
    typedef struct
    {
        u8 CodecID:4;
        u8 FrameType:4;

    }T_VideoHeader;
#else
    #error  need define __BYTE_ORDER__
#endif
/*
0 = AVC sequence header
1 = AVC NALU
2 = AVC end of sequence (lower level NALU sequence ender is not required or supported)
*/
typedef struct
{
    u8 PacketType;
    u8 aCompostionTime[3];

}T_AVCPacketHeader;


#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
    typedef struct
    {
        u8 Version; /* 1 */
        u8 ProfileIndication;
        u8 ProfileCompatibility; /* sps中Profile和Level字段中间的那个字节 */
        u8 AVCLevelIndication;

        u8 rsv:6; /* 111111 */
        u8 LengthSizeMinusOne:2;

        /*
        u8 rsv2:3; // 111
        u8 SPSNum:5;
        u16 SPSLen;
        SPSData;

        u8 PPSNum;
        u16 PPSLen;
        PPSData;
        ...
        */

    }T_AVCDecoderConfig;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
    typedef struct
    {
        u8 Version; /* 1 */
        u8 ProfileIndication;
        u8 ProfileCompatibility; /* sps中Profile和Level字段中间的那个字节 */
        u8 AVCLevelIndication;

        u8 LengthSizeMinusOne:2;
        u8 rsv:6; /* 111111 */

    }T_AVCDecoderConfig;
#else
    #error  need define __BYTE_ORDER__
#endif

#pragma pack ()




/*
Format of SoundData.
0 = Linear PCM, platform endian
1 = ADPCM
2 = MP3
3 = Linear PCM, little endian
4 = Nellymoser 16 kHz mono
5 = Nellymoser 8 kHz mono
6 = Nellymoser
7 = G.711 A-law logarithmic PCM
8 = G.711 mu-law logarithmic PCM
9 = reserved
10 = AAC
11 = Speex
14 = MP3 8 kHz
15 = Device-specific sound
Formats 7, 8, 14, and 15 are reserved.

Sampling rate
0:5.5 kHz  1:11 kHz  2:22 kHz  3:44 kHz

SoundSize. This parameter only pertains to uncompressed formats. Compressed formats always decode to 16 bits internally.
0:8-bit samples  1:16-bit samples

SoundType
0:Mono sound  1:Stereo sound
*/
#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
    typedef struct
    {
        u8 SoundFormat:4;
        u8 SoundRate:2;
        u8 SoundSize:1;
        u8 SoundType:1;

    }T_AudioHeader;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
    typedef struct
    {
        u8 SoundType:1;
        u8 SoundSize:1;
        u8 SoundRate:2;
        u8 SoundFormat:4;

    }T_AudioHeader;
#else
    #error  need define __BYTE_ORDER__
#endif



/*
AACPacketType
0:AAC sequence header  1:AAC raw
*/
typedef struct
{
    u8 AACPacketType;

}T_AACPacketHeader;

















static s8 GetSampleRateID(u32 SamplRate)
{
    switch (SamplRate)
    {
         case 96000: return  0;
         case 88200: return  1;
         case 64000: return  2;
         case 48000: return  3;
         case 44100: return  4;
         case 32000: return  5;
         case 24000: return  6;
         case 22050: return  7;
         case 16000: return  8;
         case 12000: return  9;
         case 11025: return 10;
         case 8000 : return 11;
         case 7350 : return 12;
         default:    return -1;
    }
}


static s32 GetAudioSpecificConfig(u8 *pBuf, u32 Size, u8 AudioType, u32 SampleRate, u8 Channel)
{
    u16 Config;
    s8  SampleRateID;

    SampleRateID = GetSampleRateID(SampleRate);
    if (0 > SampleRateID)
        return -1;

    Config = (AudioType & 0x1f);
    Config <<= 4;
    Config |= SampleRateID & 0x0f;
    Config <<= 4;
    Config |= Channel & 0x0f;
    Config <<= 3;

    pBuf[0] = (Config >> 8) & 0xff;
    pBuf[1] =  Config & 0xff;

    return 2;
}


/* 返回的数据包括起始的4个字节0x00000001 */
static u8* Find264Nalu(u8 *pData, u32 Size, u8 *pNaluType, u32 *pNaluSize)
{
    u8 *pEnd;
    u8 *pCur;
    u8 *pOut;
    u8  NaluType;


    if (4 >= Size)
        return NULL;

    /* 找第一个0x00000001 */
    pCur = pData;
    pEnd = pData + Size - 4;
    while (pCur < pEnd)
    {
        if ( (0 == pCur[0]) && (0 == pCur[1]) && (0 == pCur[2]) && (1 == pCur[3]) )
            break;
        pCur++;
    }
    if (pCur >= pEnd)
        return NULL;

    NaluType = pCur[4] & 0x1f;
    *pNaluType = NaluType;

    if (1 == NaluType || 5 == NaluType) /* P帧、I帧, 假设每一包里P帧I帧都是最后一个 */
    {
        *pNaluSize  = Size - (pCur - pData);
        return pCur;
    }

    pOut = pCur;

    /* 找第二个0x00000001 */
    pCur += 5;
    while (pCur <= pEnd)
    {
        if ( (0 == pCur[0]) && (0 == pCur[1]) && (0 == pCur[2]) && (1 == pCur[3]) )
            break;
        pCur++;
    }
    if (pCur <= pEnd)
    {
        *pNaluSize  = pCur - pOut;
        return pOut;
    }

    *pNaluSize  = Size - (pOut - pData);
    return pOut;
}

static s32 GetFileHeader(T_FLVConfig *pConf, u8 *pBuf, u32 Size)
{
    T_FileHeader *pHeader = (T_FileHeader *)pBuf;


    if (sizeof(T_FileHeader) > Size)
        return -1;

    pHeader->aSignature[0] = 'F';
    pHeader->aSignature[1] = 'L';
    pHeader->aSignature[2] = 'V';
    pHeader->Version = 1;
    pHeader->rsv1 = 0;
    pHeader->rsv2 = 0;
    pHeader->TypeFlagAudio = !!(pConf->AudioFlag);
    pHeader->TypeFlagVideo = !!(pConf->VideoFlag);
    pHeader->aDataOffset[0] = 0;
    pHeader->aDataOffset[1] = 0;
    pHeader->aDataOffset[2] = 0;
    pHeader->aDataOffset[3] = 9;

    pBuf += sizeof(T_FileHeader);
    Size -= sizeof(T_FileHeader);
    WRITE_U32(pBuf, Size, 0);

    return sizeof(T_FileHeader) + 4;
}

static s32 GetTagHeader(u8 *pBuf, u32 BufSize, u8 Type, u32 TimeStamp)
{
    T_TagHeader *pHeader = (T_TagHeader *)pBuf;


    if (sizeof(T_TagHeader) > BufSize)
        return -1;

    pHeader->rsv     = 0;
    pHeader->Filter  = 0;
    pHeader->TagType = 0x1f & Type;
    pHeader->aDateSize[0]  = 0;
    pHeader->aDateSize[1]  = 0;
    pHeader->aDateSize[2]  = 0;
    pHeader->aTimestamp[0] = 0xff & (TimeStamp >> 16);
    pHeader->aTimestamp[1] = 0xff & (TimeStamp >>  8);
    pHeader->aTimestamp[2] = 0xff &  TimeStamp;
    pHeader->TimeStampEx   = 0xff & (TimeStamp >> 24);
    pHeader->aStreamID[0]  = 0;
    pHeader->aStreamID[1]  = 0;
    pHeader->aStreamID[2]  = 0;

    return sizeof(T_TagHeader);
}
static void SetTagHeaderSize(u8 *pBuf, u32 TagSize)
{
    T_TagHeader *pHeader = (T_TagHeader *)pBuf;

    pHeader->aDateSize[0]  = 0xff & (TagSize  >> 16);
    pHeader->aDateSize[1]  = 0xff & (TagSize  >>  8);
    pHeader->aDateSize[2]  = 0xff &  TagSize;

    return;
}


/*
FrameType
1:key frame  2:inter frame

CodecID
7:AVC

PacketType
0:AVC sequence header  1:AVC NALU
*/
static s32 GetVideoHeader(u8 *pBuf, u32 Size, u8 FrameType, u8 CodecID, u8 PacketType)
{
    T_VideoHeader *pVideoHeader = NULL;
    T_AVCPacketHeader *pAVCHeader = NULL;


    if (sizeof(*pVideoHeader) > Size)
        return -1;
    pVideoHeader = (T_VideoHeader *)pBuf;
    pVideoHeader->FrameType = 0xf & FrameType;
    pVideoHeader->CodecID   = 0xf & CodecID;

    if (FLV_V_CODECID_AVC != CodecID)
    {
        return sizeof(*pVideoHeader);
    }
    pBuf += sizeof(*pVideoHeader);
    Size -= sizeof(*pVideoHeader);

    if (sizeof(*pAVCHeader) > Size)
        return -1;
    pAVCHeader = (T_AVCPacketHeader *)pBuf;
    pAVCHeader->PacketType = PacketType;
    pAVCHeader->aCompostionTime[0] = 0;
    pAVCHeader->aCompostionTime[1] = 0;
    pAVCHeader->aCompostionTime[2] = 0;

    return sizeof(*pVideoHeader) + sizeof(*pAVCHeader);
}

static s32 GetAVCDecoderConfig(u8 *pBuf, u32 BufSize, u8 *pH264Data, u32 DataSize)
{
    u8 *pCur = pBuf;
    u8 *pNalu;
    u8  NaluType;
    u32 NaluSize;
    T_AVCDecoderConfig *pDecoderConf = NULL;


    /* 找到sps */
    pNalu = Find264Nalu(pH264Data, DataSize, &NaluType, &NaluSize);
    if ((NULL == pNalu) || (7 != NaluType) || (NaluSize < 8))
        return -1;
    pNalu += 4;
    NaluSize -= 4;

    /* 写AVCDecoderConf */
    if (sizeof(*pDecoderConf) > BufSize)
        return -1;
    pDecoderConf = (T_AVCDecoderConfig *)pCur;
    pDecoderConf->Version = 1;
    pDecoderConf->ProfileIndication    = pNalu[1];
    pDecoderConf->ProfileCompatibility = pNalu[2];
    pDecoderConf->AVCLevelIndication   = pNalu[3];
    pDecoderConf->rsv = 0x3f;
    pDecoderConf->LengthSizeMinusOne = 3;
    pCur += sizeof(*pDecoderConf);
    BufSize -= sizeof(*pDecoderConf);

    WRITE_U8 (pCur, BufSize, 0xe1);
    WRITE_U16(pCur, BufSize, (u16)NaluSize);
    WRITE_BUF(pCur, BufSize, pNalu, NaluSize);

    /* 找到pps */
    pNalu = Find264Nalu(pNalu + NaluSize, DataSize - (pNalu-pH264Data) - NaluSize, &NaluType, &NaluSize);
    if (NULL == pNalu || 8 != NaluType)
        return -1;
    pNalu += 4;
    NaluSize -= 4;

    WRITE_U8 (pCur, BufSize, 1);
    WRITE_U16(pCur, BufSize, (u16)NaluSize);
    WRITE_BUF(pCur, BufSize, pNalu, NaluSize);

    return pCur - pBuf;
}

/*
ScriptTagHeader
ScriptDataBody
    Name
        StringType

        StringLen
        String
    Value
        ObjectType

        PropName
            StringLen
            String
        PropData
            Type
            ...

        ObjectEnd
*/
static s32 GetScriptTag(T_FLVConfig *pConf, u8 *pBuf, u32 Size)
{
    u8 *pCur = pBuf;
    s32 HeaderSize;
    u32 TagSize;
    double Value;
    u32 rst;


    HeaderSize = GetTagHeader(pCur, Size, FLV_TAG_TYPE_SCRIPT, 0);
    if (0 > HeaderSize)
        return -1;
    pCur += HeaderSize;
    Size -= HeaderSize;


    /* 写ScriptTagBody */
    WRITE_U8 (pCur, Size, FLV_SCRIPT_VALUE_TYPE_STRING); /* body name */
    WRITE_STRING(pCur, Size, FLV_STRING_METADATA, FLV_STRING_METADATA_LEN);


    WRITE_U8(pCur, Size, FLV_SCRIPT_VALUE_TYPE_OBJECT); /* body value */

    /* video */
    if (pConf->VideoFlag)
    {
        WRITE_STRING(pCur, Size, FLV_STRING_WIDTH, FLV_STRING_WIDTH_LEN); /* width */
        WRITE_U8(pCur, Size, FLV_SCRIPT_VALUE_TYPE_NUM);
        Value = (double)(pConf->Width);
        WRITE_DOUBLE(pCur, Size, Value);

        WRITE_STRING(pCur, Size, FLV_STRING_HEIGHT, FLV_STRING_HEIGHT_LEN); /* height */
        WRITE_U8(pCur, Size, FLV_SCRIPT_VALUE_TYPE_NUM);
        Value = (double)(pConf->Height);
        WRITE_DOUBLE(pCur, Size, Value);

        WRITE_STRING(pCur, Size, FLV_STRING_FRAMERATE, FLV_STRING_FRAMERATE_LEN); /* framerate */
        WRITE_U8(pCur, Size, FLV_SCRIPT_VALUE_TYPE_NUM);
        Value = (double)(pConf->Framerate);
        WRITE_DOUBLE(pCur, Size, Value);

        WRITE_STRING(pCur, Size, FLV_STRING_VCODECID, FLV_STRING_VCODECID_LEN); /* videocodecid */
        WRITE_U8(pCur, Size, FLV_SCRIPT_VALUE_TYPE_NUM);
        Value = (double)FLV_V_CODECID_AVC;
        WRITE_DOUBLE(pCur, Size, Value);
    }


    /* audio */
    if (pConf->AudioFlag)
    {
        WRITE_STRING(pCur, Size, FLV_STRING_ASAMPLERATE, FLV_STRING_ASAMPLERATE_LEN); /* audiosamplerate */
        WRITE_U8(pCur, Size, FLV_SCRIPT_VALUE_TYPE_NUM);
        Value = (double)(pConf->SampleRate);
        WRITE_DOUBLE(pCur, Size, Value);

        WRITE_STRING(pCur, Size, FLV_STRING_ASAMPLESIZE, FLV_STRING_ASAMPLESIZE_LEN); /* audiosamplesize */
        WRITE_U8(pCur, Size, FLV_SCRIPT_VALUE_TYPE_NUM);
        Value = (double)(16);//pConf->SampleSize);
        WRITE_DOUBLE(pCur, Size, Value);

        WRITE_STRING(pCur, Size, FLV_STRING_STEREO, FLV_STRING_STEREO_LEN); /* stereo */
        WRITE_U8(pCur, Size, FLV_SCRIPT_VALUE_TYPE_BOOL);
        WRITE_U8(pCur, Size, !!(pConf->Stereo));

        WRITE_STRING(pCur, Size, FLV_STRING_ACODECID, FLV_STRING_ACODECID_LEN); /* audiocodecid */
        WRITE_U8(pCur, Size, FLV_SCRIPT_VALUE_TYPE_NUM);
        Value = (double)FLV_A_FORMATE_AAC;
        WRITE_DOUBLE(pCur, Size, Value);
    }


    /* body end */
    WRITE_U24(pCur, Size, 0x000009);

    TagSize = (u32)(pCur - pBuf);
    SetTagHeaderSize(pBuf, TagSize - HeaderSize);
    WRITE_U32(pCur, Size, TagSize);

    return TagSize + 4;
}

s32 FLV_Get264Tag_SPS(u8 *pBuf, u32 BufSize, u8 *pH264Data, u32 DataSize)
{
    u8 *pCur = pBuf;
    u32 TagSize;
    s32 HeaderSize;
    s32 rst;


    if (NULL == pBuf || NULL == pH264Data)
        return -1;

    /* 写TagHeader */
    HeaderSize = GetTagHeader(pCur, BufSize, FLV_TAG_TYPE_VIDEO, 0);
    if (0 > HeaderSize)
        return -1;
    pCur += HeaderSize;
    BufSize -= HeaderSize;


    /* 写VideoHeader */
    rst = GetVideoHeader(pCur, BufSize, FLV_V_FRAME_TYPE_I, FLV_V_CODECID_AVC, FLV_AVCPACKET_TYPE_SPS);
    if (0 > rst)
        return -1;
    pCur += rst;
    BufSize -= rst;


    /* 写AVCDecoderConfigurationRecord */
    rst = GetAVCDecoderConfig(pCur, BufSize, pH264Data, DataSize);
    if (0 > rst)
        return -1;
    pCur += rst;
    BufSize -= rst;


    TagSize = (u32)(pCur - pBuf);
    SetTagHeaderSize(pBuf, TagSize - HeaderSize);
    WRITE_U32(pCur, BufSize, TagSize);

    return TagSize + 4;
}


s32 FLV_Get264Tag(u8 *pBuf, u32 BufSize, u8 *pH264Data, u32 DataSize, u32 TimeStamp)
{
    u8 *pCur = pBuf;
    u8 *pNalu;
    u8  NaluType;
    u32 NaluSize = 0;
    u32 TagSize;
    s32 HeaderSize;
    s32 rst;


    if (NULL == pBuf || NULL == pH264Data)
        return -1;

    /* 写TagHeader */
    HeaderSize = GetTagHeader(pCur, BufSize, FLV_TAG_TYPE_VIDEO, TimeStamp);
    if (0 > HeaderSize)
        return -1;
    pCur += HeaderSize;
    BufSize -= HeaderSize;


    /* 找到I帧或P帧 找到一个就退出 */
    pNalu = pH264Data;
    while (1)
    {
        pNalu = Find264Nalu(pNalu + NaluSize, DataSize - (pNalu-pH264Data) - NaluSize, &NaluType, &NaluSize);
        if (NULL == pNalu)
            return -1;
        if ((1 == NaluType) || (5 == NaluType))
            break;
    }
    pNalu += 4;
    NaluSize -= 4;
    NaluType = (1 == NaluType)? FLV_V_FRAME_TYPE_P: FLV_V_FRAME_TYPE_I;


    /* 写VideoHeader */
    rst = GetVideoHeader(pCur, BufSize, NaluType, FLV_V_CODECID_AVC, FLV_AVCPACKET_TYPE_NALU);
    if (0 > rst)
        return -1;
    pCur += rst;
    BufSize -= rst;


    /* 写264帧 */
    WRITE_U32(pCur, BufSize, NaluSize);
    WRITE_BUF(pCur, BufSize, pNalu, NaluSize);

    TagSize = pCur - pBuf;
    SetTagHeaderSize(pBuf, TagSize - HeaderSize);
    WRITE_U32(pCur, BufSize, TagSize);

    return TagSize + 4;
}



// 默认AAC
static s32 GetAudioHeader(u8 *pBuf, u32 Size, u8 AACPacketType)
{
    T_AudioHeader *pAudioHeader;
    T_AACPacketHeader *pAACHeader;


    if (sizeof(*pAudioHeader) > Size)
        return -1;
    pAudioHeader = (T_AudioHeader *)pBuf;
    pAudioHeader->SoundFormat = FLV_A_FORMATE_AAC;
    pAudioHeader->SoundRate   = FLV_A_SAMPLE_RATE_44K; /* AAC时固定为44K */
    pAudioHeader->SoundSize   = FLV_A_SAMPLE_SIZE_16b; /* AAC时固定为16b */
    pAudioHeader->SoundType   = FLV_A_SOUND_TYPE_STEREO; /* AAC时固定为stereo */
    pBuf += sizeof(*pAudioHeader);
    Size -= sizeof(*pAudioHeader);

    /* 写AACPacketHeader */
    if (sizeof(*pAACHeader) > Size)
        return -1;
    pAACHeader = (T_AACPacketHeader *)pBuf;
    pAACHeader->AACPacketType = AACPacketType;

    return sizeof(*pAudioHeader) + sizeof(*pAACHeader);
}

static s32 GetAACTag_First(T_FLVConfig *pConf, u8 *pBuf, u32 Size)
{
    u8 *pCur = pBuf;
    s32 HeaderSize;
    s32 rst;
    u32 TagSize;


    if (NULL == pConf || NULL == pBuf)
        return -1;

    /* 写TagHeader */
    HeaderSize = GetTagHeader(pCur, Size, FLV_TAG_TYPE_AUDIO, 0);
    if (0 > HeaderSize)
        return -1;
    pCur += HeaderSize;
    Size -= HeaderSize;

    /* 写AudioHeader */
    rst = GetAudioHeader(pCur, Size, FLV_AACPACKET_TYPE_HEADER);
    if (0 > rst)
        return -1;
    pCur += rst;
    Size -= rst;

    /* 写SpecificConfig */
    rst = GetAudioSpecificConfig(pCur, Size, pConf->AudioType, pConf->SampleRate, pConf->Channel);
    if (0 > rst)
        return -1;
    pCur += rst;
    Size -= rst;


    TagSize = (u32)(pCur - pBuf);
    SetTagHeaderSize(pBuf, TagSize - HeaderSize);
    WRITE_U32(pCur, Size, TagSize);

    return TagSize + 4;
}


s32 FLV_GetAACTag(u8 *pBuf, u32 BufSize, u8 *pAACData, u32 DataSize, u32 TimeStamp)
{
    u8 *pCur = pBuf;
    s32 HeaderSize;
    s32 rst;
    u32 TagSize;


    if (NULL == pBuf || NULL == pAACData)
        return -1;

    /* 写TagHeader */
    HeaderSize = GetTagHeader(pCur, BufSize, FLV_TAG_TYPE_AUDIO, TimeStamp);
    if (0 > HeaderSize)
        return -1;
    pCur += HeaderSize;
    BufSize -= HeaderSize;

    /* 写AudioHeader */
    rst = GetAudioHeader(pCur, BufSize, FLV_AACPACKET_TYPE_RAW);
    if (0 > rst)
        return -1;
    pCur += rst;
    BufSize -= rst;


    /* 写AAC data */
    WRITE_BUF(pCur, BufSize, pAACData, DataSize);


    TagSize = (u32)(pCur - pBuf);
    SetTagHeaderSize(pBuf, TagSize - HeaderSize);
    WRITE_U32(pCur, BufSize, TagSize);

    return TagSize + 4;
}




s32 FLV_GetStreamHeader(T_FLVConfig *pConf, u8 *pBuf, u32 Size)
{
    u8 *pCur = pBuf;
    s32 rst;


    if (NULL == pConf || NULL == pBuf)
        return -1;

    if ((0 == pConf->VideoFlag) && (0 == pConf->AudioFlag))
        return -1;


    /* 写FileHeader */
    rst = GetFileHeader(pConf, pCur, Size);
    if (0 > rst)
        return -1;
    pCur += rst;
    Size -= rst;


    /* 写ScriptTag */
    rst = GetScriptTag(pConf, pCur, Size);
    if (0 > rst)
        return -1;
    pCur += rst;
    Size -= rst;


    /* 写起始的AudioTag */
    if (pConf->AudioFlag)
    {
        rst = GetAACTag_First(pConf, pCur, Size);
        if (0 > rst)
            return -1;
        pCur += rst;
        Size -= rst;
    }

    return pCur - pBuf;
}


