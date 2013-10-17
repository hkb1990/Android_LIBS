#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "au_channel.h"
#include "aacplus.h"

#include <jni.h>
#include <android/log.h>

typedef struct
{
    aacplusEncHandle    hEncoder;
    aacplusEncConfiguration *cfg;
    WavInfo             *inputInfo;     //源文件的信息
    int                 nchannels;      //AAC通道数
    unsigned int        sampleRateAAC;  //采样率
    int                 bitrate;        //比特率
    int                 bandwidth;      //带宽
    unsigned long       inputSamples;
    unsigned long       maxOutputBytes;
} Encoder;

/*初始化*/
jlong Java_com_vvku_aacencoder_heaacEncInterface_init(JNIEnv* env, jobject thiz, jint samplerate, jint channels, jint bitrate, jint bandwidth, jlongArray param_out, jstring input_file)
{
    __android_log_print(ANDROID_LOG_INFO, "encoderInterface native", "begin init");
    jlong *info     = (jlong*)(*env)->GetLongArrayElements(env, param_out, 0);
    Encoder * en    = (Encoder *) malloc(sizeof(Encoder));
    en->inputInfo   = (WavInfo *) malloc(sizeof(WavInfo));
    en->bitrate     = 16000;
    en->sampleRateAAC   = 44100;

    unsigned char* wav_file = (char*)(*env)->GetStringUTFChars(env, input_file, 0);
    FILE *inputfile;
    inputfile = AuChannelOpen(wav_file, en->inputInfo);

    if(bitrate > 0){
        en->bitrate     = bitrate * 1000;
    }
    if(!inputfile){
        en->inputInfo->nChannels = 2;
        en->inputInfo->sampleRate = 32000;
    }
    if(samplerate > 0){
        en->inputInfo->sampleRate = samplerate;
    }

    if(channels > 0){
        en->inputInfo->nChannels = channels;
    }

    en->hEncoder = aacplusEncOpen(en->inputInfo->sampleRate,
            en->inputInfo->nChannels,
            &en->inputSamples,
            &en->maxOutputBytes);

    info[0] = en->inputSamples*2;
    info[1] = en->maxOutputBytes;

    en->cfg = aacplusEncGetCurrentConfiguration(en->hEncoder);
    en->cfg->bitRate = en->bitrate;
    en->cfg->bandWidth = 0;
    en->cfg->outputFormat = 0; // 设置为1的话，会加上adts头，直接保存成aac文件的时候需要
    en->cfg->nChannelsOut = en->inputInfo->nChannels;
    //en->cfg->inputFormat = AACPLUS_INPUT_FLOAT;

    int ret = 0;
    if((ret = aacplusEncSetConfiguration(en->hEncoder, en->cfg)) == 0) {
        __android_log_print(ANDROID_LOG_INFO, "encoderInterface native", "Init failed.");
        if(inputfile) AuChannelClose(inputfile);
        (*env)->ReleaseLongArrayElements(env, param_out, info, 0);
        (*env)->ReleaseStringUTFChars(env, input_file, wav_file);
        return -2;
    }
    if(inputfile) AuChannelClose(inputfile);
    (*env)->ReleaseLongArrayElements(env, param_out, info, 0);
    (*env)->ReleaseStringUTFChars(env, input_file, wav_file);
    __android_log_print(ANDROID_LOG_INFO, "encoderInterface native", "init success.");

    return (jlong) en;
}

/*编码一帧数据*/
jint Java_com_vvku_aacencoder_heaacEncInterface_encodeframe(JNIEnv* env, jobject thiz, jlong handle, jbyteArray in, jint samplesRead, jbyteArray out)
{
    Encoder * en = (Encoder *) handle;
    jbyte * pcmbuf = (jbyte*)(*env)->GetByteArrayElements(env, in, 0);
    jbyte * outputBuffer= (jbyte*)(*env)->GetByteArrayElements(env, out, 0);
    short *in_buf  = (short*) pcmbuf;
    char  out_buf[en->maxOutputBytes];
    memset(out_buf, 0, sizeof(out_buf));

    int numOutBytes=0;
    numOutBytes = aacplusEncEncode(en->hEncoder, (int32_t *)in_buf, en->inputSamples, out_buf, en->maxOutputBytes);
    memcpy(outputBuffer, out_buf, numOutBytes);
    (*env)->ReleaseByteArrayElements(env,out,outputBuffer,0); //释放内存，防止泄露
    (*env)->ReleaseByteArrayElements(env,in,pcmbuf,0);

    return numOutBytes;
}

/*销毁aac编码器*/
jint Java_com_vvku_aacencoder_heaacEncInterface_destory(JNIEnv* env, jobject thiz,jlong handle)
{
    Encoder * en = (Encoder *) handle;
    aacplusEncClose(en->hEncoder);
}
