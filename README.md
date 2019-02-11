# flv-writer

FLV_GetStreamHeader() 返回 FLV 文件的 FileTag、ScriptTag 和 AacConfigTag

FLV_Get264Tag_SPS() 返回 264SpsTag，

FLV_Get264Tag() 和 FLV_GetAACTag() 返回一般的 264Tag 和 AACTag

 
**注意：**

 1. 在推 HTTP-FLV 流的开始，需要先把 FLV 文件的 FileTag、ScriptTag、AacConfigTag 和 264SpsTag 发出去（存 FLV 文件时也是需要先把这些写进去），其中前三个在 FLV_GetStreamHeader() 函数中一并返回了

 2. HTTP 的 chunk 应该需要和 Tag 对齐，一个 chunk 发送半个 Tag 时好像解析不出来（web用的flv.js）

 3. FLV_Get264Tag_SPS() 的入参是 SPS 和 PPS 帧，并且需要第一个是 SPS，第二个是 PPS，否则要自己修改一下代码

 4. FLV_Get264Tag() 在找到入参中第一个 I 帧或 P 帧后就结束，如果多个 I 帧 或 P 帧一起传入，需要自己修改一下代码

 5. 每个264帧的开始需要有四个字节 0x00000001 的 start code，否则要自己改一下分割函数 Find264Nalu()

 6. 一般编码器生成的 SPS、PPS 和 I 帧都是在一个包里的，所以在开始时需要以这个包作为入参调用 FLV_Get264Tag_SPS() 和 FLV_Get264Tag() 两个函数

 7. 传入的 AAC 数据需要去掉前7个或9个字节的 adts 头

 8. 把生成的数据存到文件后就是 FLV 视频文件，不过主要针对实时视频，所以ScriptTag中未写文件大小和持续时间

 9. 时间戳要从0开始
