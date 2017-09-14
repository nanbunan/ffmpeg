#include "com_example_ffmpeg_codec_VideoCodec.h"
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"guan",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"guan",FORMAT,##__VA_ARGS__);
//封装格式
#include "include/libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"

//缩放
#include "libswscale/swscale.h"
JNIEXPORT void JNICALL Java_com_example_ffmpeg_1codec_VideoCodec_decode
  (JNIEnv *env, jobject jobject, jstring input_jstr, jstring output_jstr){
	//获取输入输出路径
	const char* input=(*env)-> GetStringUTFChars(env,input_jstr,NULL);
	const char* output=(*env)-> GetStringUTFChars(env,output_jstr,NULL);

	//注册组件
	av_register_all();
	avformat_network_init();
	LOGE("%s","1");
	//封装格式上下文nb number
	AVFormatContext *formatContext=avformat_alloc_context();
	//打开封装格式  0代表成功
	if(avformat_open_input(&formatContext,input,NULL,NULL)!=0){
		LOGE("%s",input);
		LOGE("%s","打开输入视频失败1");
		return;
	}
	//获取视频信息大于等于0代表成功
	if(avformat_find_stream_info(formatContext,NULL)<0){
		LOGE("%s","获取信息失败2");
		return;
	}

	//s视频解码，需要找到视频对应的AVStream所在的位置.
	int video_stream_index=-1;
	int i=0;
	for(; i<formatContext->nb_streams;i++){
		//根据类型判断，是否是视频流
		if(formatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			video_stream_index=i;
			LOGE("%d",formatContext->nb_streams);
		}
	}
	LOGE("%s","12");
	//获取解码器

	AVCodecContext* codecContext=formatContext->streams[video_stream_index]->codec;
	LOGE("%s","123");
	AVCodec *decoder=avcodec_find_decoder(codecContext->codec_id);
	LOGE("%s","1234");
	if(decoder==NULL){
		LOGE("无法解码3");
		return;
	}

	//打开解码器 此时并没有解码
	if(avcodec_open2(codecContext,decoder,NULL)){
		LOGE("%s","解码器无法打开4");
		return;
	}
	//一帧一帧读取压缩(编码)的视频数据AVPacket
	//编码数据
		AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//像素数据（解码数据）
	AVFrame *frame=av_frame_alloc();
	AVFrame *yuvFrame = av_frame_alloc();

	//只有指定了AVFrame的像素格式、画面大小才能真正分配内存
		//缓冲区分配内存
		uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height));
		//初始化缓冲区
		avpicture_fill((AVPicture *)yuvFrame, out_buffer, AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height);

LOGE("%s","123456");
		//输出文件
		FILE* fp_yuv = fopen(output,"wb+");
		LOGE("%s","123456789");

		//用于像素格式转换或者缩放
		/**
		 * Allocate and return an SwsContext. You need it to perform
		 * scaling/conversion operations using sws_scale().
		 *申请空间并返回swsContext，执行缩放/变换时用sws_scale()方法时需要用到SwsContext类型的参数
		 * @param srcW the width of the source image原视频的宽
		 * @param srcH the height of the source image原视频的高
		 * @param srcFormat the source image format原视频的格式
		 * @param dstW the width of the destination image输出视频的宽
		 * @param dstH the height of the destination image输出视频的高
		 * @param dstFormat the destination image format输出视频的格式
		 * @param flags specify which algorithm and options to use for rescaling再次缩放采用那个算法的标志位
		 * @return a pointer to an allocated context, or NULL in case of error
		 * @note this function is to be removed after a saner alternative is
		 *       written
		 */
		struct SwsContext *sws_ctx = sws_getContext(
				codecContext->width, codecContext->height, codecContext->pix_fmt,
				codecContext->width, codecContext->height, AV_PIX_FMT_YUV420P,
				SWS_BILINEAR, NULL, NULL, NULL);

	int len,framecount=0,got_frame=0;
	while(av_read_frame(formatContext,packet)>=0){
		//AVPacket解码
		//返回解析的字节大小
		len=avcodec_decode_video2(codecContext,frame,&got_frame,packet);
		//非零正在解码
		if(got_frame){
			//转为指定的YUV420p像素帧每四个Y共用一个UV
			/**
			 * int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
             * const int srcStride[], int srcSliceY, int srcSliceH,
             *uint8_t *const dst[], const int dstStride[]);
			 * */
			sws_scale(sws_ctx,frame->data,frame->linesize,0,frame->height,yuvFrame->data,yuvFrame->linesize);
			//向YUV文件保存解码之后的帧数据
					//AVFrame->YUV
					//一个像素包含一个Y
					int y_size = codecContext->width * codecContext->height;
					fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);//Y
					fwrite(yuvFrame->data[1], 1, y_size/4, fp_yuv);//U
					fwrite(yuvFrame->data[2], 1, y_size/4, fp_yuv);//V

					LOGI("解码%d帧",framecount++);
		}
		av_free_packet(packet);
	}

	fclose(fp_yuv);

		av_frame_free(&frame);
		avcodec_close(codecContext);
		avformat_free_context(formatContext);

	//释放数据
	(*env)->ReleaseStringUTFChars(env,input_jstr,input);
	(*env)->ReleaseStringUTFChars(env,output_jstr,output);
}
