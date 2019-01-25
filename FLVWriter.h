#ifndef _FLVWRITER_H_
#define _FLVWRITER_H_


#define MP4_AUDIO_TYPE_INVALID      0
#define MP4_AUDIO_TYPE_AAC_MAIN     1
#define MP4_AUDIO_TYPE_AAC_LC       2
#define MP4_AUDIO_TYPE_AAC_SSR      3
#define MP4_AUDIO_TYPE_AAC_LD      23


typedef struct
{
    u8  VideoFlag; /* 是否有视频数据 */
    u8  AudioFlag; /* 是否有音频数据 */

    /* video */
    u16 Width;
    u16 Height;
    u8  Framerate;
    //u8  Videocodecid;


    /* audio */
    //u8  SampleSize; /* 16 or 8 */
    u16 SampleRate; /* Hz */
    u8  Stereo;     /* 0:mono  1:stereo */
    u8  AudioType;  /* aac needed, MPEG-4 Audio Object Types */
    u8  Channel;

}T_FLVConfig;



s32 FLV_GetStreamHeader(T_FLVConfig *pConf, u8 *pBuf, u32 Size);
s32 FLV_Get264Tag_SPS(u8 *pBuf, u32 BufSize, u8 *pH264Data, u32 DataSize);
s32 FLV_Get264Tag(u8 *pBuf, u32 BufSize, u8 *pH264Data, u32 DataSize, u32 TimeStamp);
s32 FLV_GetAACTag(u8 *pBuf, u32 BufSize, u8 *pAACData, u32 DataSize, u32 TimeStamp);

#endif