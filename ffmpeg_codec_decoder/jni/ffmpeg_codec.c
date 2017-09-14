#include "com_example_ffmpeg_codec_VideoCodec.h"
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"guan",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"guan",FORMAT,##__VA_ARGS__);
//��װ��ʽ
#include "include/libavformat/avformat.h"
//����
#include "libavcodec/avcodec.h"

//����
#include "libswscale/swscale.h"
JNIEXPORT void JNICALL Java_com_example_ffmpeg_1codec_VideoCodec_decode
  (JNIEnv *env, jobject jobject, jstring input_jstr, jstring output_jstr){
	//��ȡ�������·��
	const char* input=(*env)-> GetStringUTFChars(env,input_jstr,NULL);
	const char* output=(*env)-> GetStringUTFChars(env,output_jstr,NULL);

	//ע�����
	av_register_all();
	avformat_network_init();
	LOGE("%s","1");
	//��װ��ʽ������nb number
	AVFormatContext *formatContext=avformat_alloc_context();
	//�򿪷�װ��ʽ  0����ɹ�
	if(avformat_open_input(&formatContext,input,NULL,NULL)!=0){
		LOGE("%s",input);
		LOGE("%s","��������Ƶʧ��1");
		return;
	}
	//��ȡ��Ƶ��Ϣ���ڵ���0����ɹ�
	if(avformat_find_stream_info(formatContext,NULL)<0){
		LOGE("%s","��ȡ��Ϣʧ��2");
		return;
	}

	//s��Ƶ���룬��Ҫ�ҵ���Ƶ��Ӧ��AVStream���ڵ�λ��.
	int video_stream_index=-1;
	int i=0;
	for(; i<formatContext->nb_streams;i++){
		//���������жϣ��Ƿ�����Ƶ��
		if(formatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			video_stream_index=i;
			LOGE("%d",formatContext->nb_streams);
		}
	}
	LOGE("%s","12");
	//��ȡ������

	AVCodecContext* codecContext=formatContext->streams[video_stream_index]->codec;
	LOGE("%s","123");
	AVCodec *decoder=avcodec_find_decoder(codecContext->codec_id);
	LOGE("%s","1234");
	if(decoder==NULL){
		LOGE("�޷�����3");
		return;
	}

	//�򿪽����� ��ʱ��û�н���
	if(avcodec_open2(codecContext,decoder,NULL)){
		LOGE("%s","�������޷���4");
		return;
	}
	//һ֡һ֡��ȡѹ��(����)����Ƶ����AVPacket
	//��������
		AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//�������ݣ��������ݣ�
	AVFrame *frame=av_frame_alloc();
	AVFrame *yuvFrame = av_frame_alloc();

	//ֻ��ָ����AVFrame�����ظ�ʽ�������С�������������ڴ�
		//�����������ڴ�
		uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height));
		//��ʼ��������
		avpicture_fill((AVPicture *)yuvFrame, out_buffer, AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height);

LOGE("%s","123456");
		//����ļ�
		FILE* fp_yuv = fopen(output,"wb+");
		LOGE("%s","123456789");

		//�������ظ�ʽת����������
		/**
		 * Allocate and return an SwsContext. You need it to perform
		 * scaling/conversion operations using sws_scale().
		 *����ռ䲢����swsContext��ִ������/�任ʱ��sws_scale()����ʱ��Ҫ�õ�SwsContext���͵Ĳ���
		 * @param srcW the width of the source imageԭ��Ƶ�Ŀ�
		 * @param srcH the height of the source imageԭ��Ƶ�ĸ�
		 * @param srcFormat the source image formatԭ��Ƶ�ĸ�ʽ
		 * @param dstW the width of the destination image�����Ƶ�Ŀ�
		 * @param dstH the height of the destination image�����Ƶ�ĸ�
		 * @param dstFormat the destination image format�����Ƶ�ĸ�ʽ
		 * @param flags specify which algorithm and options to use for rescaling�ٴ����Ų����Ǹ��㷨�ı�־λ
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
		//AVPacket����
		//���ؽ������ֽڴ�С
		len=avcodec_decode_video2(codecContext,frame,&got_frame,packet);
		//�������ڽ���
		if(got_frame){
			//תΪָ����YUV420p����֡ÿ�ĸ�Y����һ��UV
			/**
			 * int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
             * const int srcStride[], int srcSliceY, int srcSliceH,
             *uint8_t *const dst[], const int dstStride[]);
			 * */
			sws_scale(sws_ctx,frame->data,frame->linesize,0,frame->height,yuvFrame->data,yuvFrame->linesize);
			//��YUV�ļ��������֮���֡����
					//AVFrame->YUV
					//һ�����ذ���һ��Y
					int y_size = codecContext->width * codecContext->height;
					fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);//Y
					fwrite(yuvFrame->data[1], 1, y_size/4, fp_yuv);//U
					fwrite(yuvFrame->data[2], 1, y_size/4, fp_yuv);//V

					LOGI("����%d֡",framecount++);
		}
		av_free_packet(packet);
	}

	fclose(fp_yuv);

		av_frame_free(&frame);
		avcodec_close(codecContext);
		avformat_free_context(formatContext);

	//�ͷ�����
	(*env)->ReleaseStringUTFChars(env,input_jstr,input);
	(*env)->ReleaseStringUTFChars(env,output_jstr,output);
}
