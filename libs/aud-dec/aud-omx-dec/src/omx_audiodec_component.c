#include <bellagio/omxcore.h>
#include <bellagio/omx_base_audio_port.h>
#include <stdbool.h>
#include <OMX_Audio.h>

#include <config.h>

#include "omx_audiodec_component.h"

#define MAX_COMPONENT_AUDIODEC 4

// output length argument passed along decoding function
#define OUTPUT_LEN_STANDARD_FFMPEG 288000

// Number of Audio Component Instance
static OMX_U32 noAudioDecInstance=0;

#undef ALIGN
#define ALIGN(value, alignment) (((value)+(alignment-1))&~(alignment-1))
unsigned char* buffer = NULL;

void *_aligned_malloc(size_t s, size_t alignTo) {

	char *pFull = (char*)malloc(s + alignTo + sizeof(char *));
	char *pAlligned = (char *)ALIGN(((unsigned long)pFull + sizeof(char *)), alignTo);

	*(char **)(pAlligned - sizeof(char*)) = pFull;

	return(pAlligned);
}

void _aligned_free(void *p) {
	if (!p)
		return;

	char *pFull = *(char **)(((char *)p) - sizeof(char *));
	free(pFull);
}

OMX_ERRORTYPE omx_audiodec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) {

	OMX_ERRORTYPE err = OMX_ErrorNone;
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private;
	omx_base_audio_PortType *pPort;
	OMX_U32 i;

	OMX_U32 target_codecID;
	DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);

	if (!openmaxStandComp->pComponentPrivate) {
		DEBUG(DEB_LEV_FUNCTION_NAME,"In %s, allocating component\n",__func__);
		openmaxStandComp->pComponentPrivate = calloc(1, sizeof(omx_audiodec_component_PrivateType));
		if(openmaxStandComp->pComponentPrivate==NULL)
			return OMX_ErrorInsufficientResources;
	}
	else
		DEBUG(DEB_LEV_FUNCTION_NAME,"In %s, Error Component %x Already Allocated\n",__func__, (int)openmaxStandComp->pComponentPrivate);

	omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	omx_audiodec_component_Private->ports = NULL;

	// Calling base filter constructor
	err = omx_base_filter_Constructor(openmaxStandComp,cComponentName);

	omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nStartPortNumber = 0;
	omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts = 2;

	// Allocate Ports and call port constructor.
	if (omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts && !omx_audiodec_component_Private->ports) {
		omx_audiodec_component_Private->ports = calloc(omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts, sizeof(omx_base_PortType *));
		if (!omx_audiodec_component_Private->ports) {
			return OMX_ErrorInsufficientResources;
		}
		for (i=0; i < omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) {
			omx_audiodec_component_Private->ports[i] = calloc(1, sizeof(omx_base_audio_PortType));
			if (!omx_audiodec_component_Private->ports[i]) {
				return OMX_ErrorInsufficientResources;
			}
		}
	}

	base_audio_port_Constructor(openmaxStandComp, &omx_audiodec_component_Private->ports[0], 0, OMX_TRUE);
	base_audio_port_Constructor(openmaxStandComp, &omx_audiodec_component_Private->ports[1], 1, OMX_FALSE);

	//common parameters related to input port
	omx_audiodec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.nBufferSize = DEFAULT_IN_BUFFER_SIZE;
	omx_audiodec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	omx_audiodec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.nVersion.s.nVersionMajor  = SPECVERSIONMAJOR;
	omx_audiodec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.nVersion.s.nVersionMinor = SPECVERSIONMINOR;
	omx_audiodec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.nVersion.s.nRevision = SPECREVISION;
	omx_audiodec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.nVersion.s.nStep = SPECSTEP;

	//common parameters related to output port
	pPort = (omx_base_audio_PortType *) omx_audiodec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	pPort->sAudioParam.nIndex = OMX_IndexParamAudioPcm;
	pPort->sAudioParam.eEncoding = OMX_AUDIO_CodingPCM;
	pPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
	pPort->sPortParam.nBufferSize = DEFAULT_OUT_BUFFER_SIZE * 2;
	pPort->sPortParam.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	pPort->sPortParam.nVersion.s.nVersionMajor = SPECVERSIONMAJOR;
	pPort->sPortParam.nVersion.s.nVersionMinor = SPECVERSIONMINOR;
	pPort->sPortParam.nVersion.s.nRevision = SPECREVISION;
	pPort->sPortParam.nVersion.s.nStep = SPECSTEP;

	// output is pcm mode for all decoders
	setHeader(&omx_audiodec_component_Private->pAudioPcmMode,sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
	omx_audiodec_component_Private->pAudioPcmMode.nPortIndex = 1;
	omx_audiodec_component_Private->pAudioPcmMode.nChannels = 2;
	omx_audiodec_component_Private->pAudioPcmMode.eNumData = OMX_NumericalDataSigned;
	omx_audiodec_component_Private->pAudioPcmMode.eEndian = OMX_EndianLittle;
	omx_audiodec_component_Private->pAudioPcmMode.bInterleaved = OMX_TRUE;
	omx_audiodec_component_Private->pAudioPcmMode.nBitPerSample = 16;
	omx_audiodec_component_Private->pAudioPcmMode.nSamplingRate = 44100;
	omx_audiodec_component_Private->pAudioPcmMode.ePCMMode = OMX_AUDIO_PCMModeLinear;
	omx_audiodec_component_Private->pAudioPcmMode.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
	omx_audiodec_component_Private->pAudioPcmMode.eChannelMapping[1] = OMX_AUDIO_ChannelRF;

	// now it's time to know the audio coding type of the component
	if(strcmp(cComponentName, AUDIO_DEC_AC3_NAME) == 0) {
		omx_audiodec_component_Private->audio_coding_type = OMX_AUDIO_CodingAC3;
	}
	else {
		// IL client specified an invalid component name
		return OMX_ErrorInvalidComponentName;
	}

	// set internal port parameters
	omx_audiodec_component_SetInternalParameters(openmaxStandComp);

	//setting other parameters of omx_audiodec_component_private
	omx_audiodec_component_Private->avCodec = NULL;
	omx_audiodec_component_Private->avCodecContext= NULL;
	omx_audiodec_component_Private->avcodecReady = OMX_FALSE;
	omx_audiodec_component_Private->extradata = NULL;
	omx_audiodec_component_Private->extradata_size = 0;
	omx_audiodec_component_Private->isFirstBuffer = OMX_TRUE;

	omx_audiodec_component_Private->BufferMgmtCallback = omx_audiodec_component_BufferMgmtCallback;

	switch(omx_audiodec_component_Private->audio_coding_type) {
		case OMX_AUDIO_CodingAC3 :
			target_codecID = AV_CODEC_ID_AC3;
			break;
		default :
			DEBUG(DEB_LEV_ERR, "Audio format other than not supported\nCodec not found\n");
			return OMX_ErrorComponentNotFound;
	}

	av_register_all();
	omx_audiodec_component_Private->avCodec = avcodec_find_decoder(target_codecID);
		if (!omx_audiodec_component_Private->avCodec) {
			return OMX_ErrorInsufficientResources;
	}

	omx_audiodec_component_Private->avCodecContext = avcodec_alloc_context3(omx_audiodec_component_Private->avCodec);
	omx_audiodec_component_Private->messageHandler = omx_audiodec_component_MessageHandler;
	omx_audiodec_component_Private->destructor = omx_audiodec_component_Destructor;
	openmaxStandComp->SetParameter = omx_audiodec_component_SetParameter;
	openmaxStandComp->GetParameter = omx_audiodec_component_GetParameter;
	openmaxStandComp->ComponentRoleEnum = omx_audiodec_component_ComponentRoleEnum;

	noAudioDecInstance++;

	// allocate output buffer for decoding
	buffer = (unsigned char*)_aligned_malloc(OUTPUT_LEN_STANDARD_FFMPEG * 2 + FF_INPUT_BUFFER_PADDING_SIZE, 16);
	if (!buffer) {
		return OMX_ErrorInsufficientResources;
	}
	memset(buffer, 0, OUTPUT_LEN_STANDARD_FFMPEG * 2 + FF_INPUT_BUFFER_PADDING_SIZE);

	if(noAudioDecInstance>MAX_COMPONENT_AUDIODEC)
		return OMX_ErrorInsufficientResources;

	return err;
}

OMX_ERRORTYPE omx_audiodec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp)
{
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_U32 i;

	if(omx_audiodec_component_Private->extradata) {
		free(omx_audiodec_component_Private->extradata);
	}

	av_free (omx_audiodec_component_Private->avCodecContext);

	// frees ports
	if (omx_audiodec_component_Private->ports) {
		for (i=0; i < omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio].nPorts; i++) {
			if(omx_audiodec_component_Private->ports[i])
				omx_audiodec_component_Private->ports[i]->PortDestructor(omx_audiodec_component_Private->ports[i]);
		}
		free(omx_audiodec_component_Private->ports);
		omx_audiodec_component_Private->ports=NULL;
	}

	DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);

	omx_base_filter_Destructor(openmaxStandComp);

	noAudioDecInstance--;
	if(buffer)
		_aligned_free(buffer);

	return OMX_ErrorNone;
}

// It init the FFmpeg framework, and opens an FFmpeg audiodecoder of type specified by IL client - currently only used for AC3 decoding
OMX_ERRORTYPE omx_audiodec_component_ffmpegLibInit(omx_audiodec_component_PrivateType* omx_audiodec_component_Private) {
	OMX_U32 target_codecID;

	DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);

	switch(omx_audiodec_component_Private->audio_coding_type){
	case OMX_AUDIO_CodingAC3 :
		target_codecID = AV_CODEC_ID_AC3;
		break;
	default :
		DEBUG(DEB_LEV_ERR, "Audio format other than not supported\nCodec not found\n");
		return OMX_ErrorComponentNotFound;
	}

	// Find the  decoder corresponding to the audio type specified by IL client
	omx_audiodec_component_Private->avCodec = avcodec_find_decoder(target_codecID);
	if (omx_audiodec_component_Private->avCodec == NULL) {
		DEBUG(DEB_LEV_ERR, "Codec %x Not found\n", (int)target_codecID);
		return OMX_ErrorInsufficientResources;
	}

	omx_audiodec_component_Private->avCodecContext->extradata = omx_audiodec_component_Private->extradata;
	omx_audiodec_component_Private->avCodecContext->extradata_size = (int)omx_audiodec_component_Private->extradata_size;

	omx_audiodec_component_Private->avCodecContext->channels = 2;

	// open the avcodec if AC3 format selected
	if (avcodec_open2(omx_audiodec_component_Private->avCodecContext, omx_audiodec_component_Private->avCodec, NULL) < 0) {
		DEBUG(DEB_LEV_ERR, "Could not open codec\n");
		return OMX_ErrorInsufficientResources;
	}

	omx_audiodec_component_Private->avCodecContext->flags |= CODEC_FLAG_EMU_EDGE;
	omx_audiodec_component_Private->avCodecContext->workaround_bugs |= FF_BUG_AUTODETECT;

	DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s\n", __func__);
	return OMX_ErrorNone;
}

// It Deinitializates the FFmpeg framework, and close the FFmpeg AC3 decoder
void omx_audiodec_component_ffmpegLibDeInit(omx_audiodec_component_PrivateType* omx_audiodec_component_Private) {

	avcodec_close(omx_audiodec_component_Private->avCodecContext);
	omx_audiodec_component_Private->extradata_size = 0;

}

void omx_audiodec_component_SetInternalParameters(OMX_COMPONENTTYPE *openmaxStandComp) {
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType *pPort = (omx_base_audio_PortType *) omx_audiodec_component_Private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];

	if(omx_audiodec_component_Private->audio_coding_type == OMX_AUDIO_CodingAC3) {
		strcpy(pPort->sPortParam.format.audio.cMIMEType, "audio/ac3");
		pPort->sPortParam.format.audio.eEncoding = OMX_AUDIO_CodingAC3;

		pPort->sAudioParam.nIndex = OMX_IndexParamAudioAc3;
		pPort->sAudioParam.eEncoding = OMX_AUDIO_CodingAC3;

		setHeader(&omx_audiodec_component_Private->pAudioAc3,sizeof(OMX_AUDIO_PARAM_AC3TYPE));
		omx_audiodec_component_Private->pAudioAc3.nPortIndex = 0;
		omx_audiodec_component_Private->pAudioAc3.nChannels = 2;
		omx_audiodec_component_Private->pAudioAc3.nBitRate = 28000;
		omx_audiodec_component_Private->pAudioAc3.nSampleRate = 44100;
		omx_audiodec_component_Private->pAudioAc3.nAudioBandWidth = 0;
		omx_audiodec_component_Private->pAudioAc3.eChannelMode = OMX_AUDIO_ChannelModeStereo;
	} else {
		return;
	}
}

OMX_ERRORTYPE omx_audiodec_component_Init(OMX_COMPONENTTYPE *openmaxStandComp)
{
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_U32 nBufferSize;

	// Temporary First Output buffer size
	omx_audiodec_component_Private->inputCurrBuffer=NULL;
	omx_audiodec_component_Private->inputCurrLength=0;
	nBufferSize = omx_audiodec_component_Private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX]->sPortParam.nBufferSize * 2;
	omx_audiodec_component_Private->internalOutputBuffer = calloc(1,nBufferSize);
	omx_audiodec_component_Private->positionInOutBuf = 0;
	omx_audiodec_component_Private->isNewBuffer=1;

	return err;

}

OMX_ERRORTYPE omx_audiodec_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp) {
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE err = OMX_ErrorNone;

	if (omx_audiodec_component_Private->avcodecReady) {
		omx_audiodec_component_ffmpegLibDeInit(omx_audiodec_component_Private);
		omx_audiodec_component_Private->avcodecReady = OMX_FALSE;
	}

	free(omx_audiodec_component_Private->internalOutputBuffer);
	omx_audiodec_component_Private->internalOutputBuffer = NULL;

	return err;
}

// buffer management callback
void omx_audiodec_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_BUFFERHEADERTYPE* pOutputBuffer)
{
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	int output_length, len;
	OMX_ERRORTYPE err;
	AVFrame *decoded_frame = NULL;
	struct SwrContext *pConvert = NULL;
	AVPacket avpkt;
	int got_frame = 0;
	int data_size1 = 0, data_size2 = 0, linesize1 = 0, linesize2 = 0;
	bool is_planar = false;
	int unpadded_linesize = 0;

	av_init_packet(&avpkt);

	DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n",__func__);
	if (!pInputBuffer || !pOutputBuffer || !pOutputBuffer->pBuffer){
		DEBUG(DEB_LEV_ERR, "In %s pInputBuffer or pOutputBuffer is null\n",__func__);
	}

	if (omx_audiodec_component_Private->isFirstBuffer == OMX_TRUE) {
		omx_audiodec_component_Private->isFirstBuffer = OMX_FALSE;

	if ((pInputBuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG) == OMX_BUFFERFLAG_CODECCONFIG) {
		omx_audiodec_component_Private->extradata_size = pInputBuffer->nFilledLen;
		if (omx_audiodec_component_Private->extradata_size > 0) {
			if(omx_audiodec_component_Private->extradata) {
				free(omx_audiodec_component_Private->extradata);
			}
			omx_audiodec_component_Private->extradata = malloc(pInputBuffer->nFilledLen);
			memcpy(omx_audiodec_component_Private->extradata, pInputBuffer->pBuffer,pInputBuffer->nFilledLen);

			omx_audiodec_component_Private->avCodecContext->extradata = omx_audiodec_component_Private->extradata;
			omx_audiodec_component_Private->avCodecContext->extradata_size = (int)omx_audiodec_component_Private->extradata_size;

		}

		DEBUG(DEB_ALL_MESS, "In %s Received First Buffer Extra Data Size=%d\n",__func__,(int)pInputBuffer->nFilledLen);
		pInputBuffer->nFlags = 0x0;
		pInputBuffer->nFilledLen = 0;
    }

	if (!omx_audiodec_component_Private->avcodecReady) {
		err = omx_audiodec_component_ffmpegLibInit(omx_audiodec_component_Private);
		if (err != OMX_ErrorNone) {
			DEBUG(DEB_LEV_ERR, "In %s omx_audiodec_component_ffmpegLibInit Failed\n",__func__);
			return;
		}
		omx_audiodec_component_Private->avcodecReady = OMX_TRUE;
	}

	if(pInputBuffer->nFilledLen == 0) {
		return;
		}
	}

	if(omx_audiodec_component_Private->avcodecReady == OMX_FALSE) {
		DEBUG(DEB_LEV_ERR, "In %s avcodec Not Ready \n",__func__);
		return;
	}


	if(omx_audiodec_component_Private->isNewBuffer) {
		omx_audiodec_component_Private->isNewBuffer = 0;
	}
	pOutputBuffer->nFilledLen = 0;
	pOutputBuffer->nOffset=0;

	// resetting output length to a predefined value
	output_length = OUTPUT_LEN_STANDARD_FFMPEG;

	if (!decoded_frame) {
		if (!(decoded_frame = avcodec_alloc_frame())) {
			DEBUG(DEB_LEV_ERR, "In %s could not allocate audio frame\n",__func__);
			return;
		}
	} else {
		avcodec_get_frame_defaults(decoded_frame);
	}
	avpkt.data = pInputBuffer->pBuffer;
	avpkt.size = pInputBuffer->nFilledLen;

	len  = avcodec_decode_audio4(omx_audiodec_component_Private->avCodecContext,
					decoded_frame,
					&got_frame,
					&avpkt);
	if (got_frame) {
		data_size1 = av_samples_get_buffer_size(&linesize1, omx_audiodec_component_Private->avCodecContext->channels, decoded_frame->nb_samples, omx_audiodec_component_Private->avCodecContext->sample_fmt, 1);
		data_size2 = av_samples_get_buffer_size(&linesize2, omx_audiodec_component_Private->pAudioConvert.channel, decoded_frame->nb_samples, omx_audiodec_component_Private->pAudioConvert.sample_fmt, 1);
	}
	if (len > pInputBuffer->nFilledLen) {
		len = pInputBuffer->nFilledLen;
	}
	is_planar = av_sample_fmt_is_planar(omx_audiodec_component_Private->avCodecContext->sample_fmt);
	unpadded_linesize = decoded_frame->nb_samples * (av_get_bytes_per_sample(decoded_frame->format)) * omx_audiodec_component_Private->avCodecContext->channels;

	if (is_planar) {
		if (data_size1 > 0){
			if (pConvert){
				swr_free(&pConvert);
			}
			if (!pConvert){
				pConvert = swr_alloc_set_opts(NULL,
						av_get_default_channel_layout(omx_audiodec_component_Private->pAudioConvert.channel),
						omx_audiodec_component_Private->pAudioConvert.sample_fmt,
						omx_audiodec_component_Private->avCodecContext->sample_rate,
						av_get_default_channel_layout(omx_audiodec_component_Private->avCodecContext->channels),
						omx_audiodec_component_Private->avCodecContext->sample_fmt,
						omx_audiodec_component_Private->avCodecContext->sample_rate,
						0, NULL);
			}
			if (!pConvert || swr_init(pConvert) < 0){
				DEBUG(DEB_LEV_ERR, "In %s swr_init failed!\n", __func__);
				return;
			}

			data_size1 = 0;
			unsigned char *out_planes[]={buffer + 0 * linesize2,buffer + 1 * linesize2, buffer + 2 * linesize2,
					buffer + 3 * linesize2, buffer + 4 * linesize2, buffer + 5 * linesize2, buffer + 6 * linesize2,
					buffer + 7 * linesize2,
			};
			if (swr_convert(pConvert, out_planes, decoded_frame->nb_samples, (const uint8_t**)decoded_frame->data, decoded_frame->nb_samples) < 0){
				DEBUG(DEB_LEV_ERR,"In %s swr_convert failed!\n", __func__);
				return;
			}
			memcpy(pOutputBuffer->pBuffer, buffer, data_size2);
			output_length = data_size2;
		}
	} else {
		memcpy(pOutputBuffer->pBuffer, decoded_frame->extended_data[0], unpadded_linesize);
		output_length = unpadded_linesize;
	}

	DEBUG(DEB_LEV_FULL_SEQ, "In %s chl=%d sRate=%d \n", __func__,
			(int)omx_audiodec_component_Private->pAudioPcmMode.nChannels,
			(int)omx_audiodec_component_Private->pAudioPcmMode.nSamplingRate);

	if ((omx_audiodec_component_Private->pAudioPcmMode.nSamplingRate != omx_audiodec_component_Private->avCodecContext->sample_rate) ||
			( omx_audiodec_component_Private->pAudioPcmMode.nChannels != omx_audiodec_component_Private->avCodecContext->channels)) {
		DEBUG(DEB_LEV_FULL_SEQ, "Sending Port Settings Change Event\n");

		switch (omx_audiodec_component_Private->audio_coding_type) {
		case OMX_AUDIO_CodingAC3 :
			// pAudioAc3 is for input port AC3 data
			omx_audiodec_component_Private->pAudioAc3.nChannels = omx_audiodec_component_Private->avCodecContext->channels;
			omx_audiodec_component_Private->pAudioAc3.nBitRate = omx_audiodec_component_Private->avCodecContext->bit_rate;
			omx_audiodec_component_Private->pAudioAc3.nSampleRate = omx_audiodec_component_Private->avCodecContext->sample_rate;
			break;
		default :
			DEBUG(DEB_LEV_ERR, "Audio format other than AC3, not supported\nCodec type %lu not found\n",omx_audiodec_component_Private->audio_coding_type);
			break;
		}

		// pAudioPcmMode is for output port PCM data
		omx_audiodec_component_Private->pAudioPcmMode.nChannels = omx_audiodec_component_Private->avCodecContext->channels;
		if (omx_audiodec_component_Private->avCodecContext->sample_fmt== AV_SAMPLE_FMT_S16)
			omx_audiodec_component_Private->pAudioPcmMode.nBitPerSample = 16;
		else if (omx_audiodec_component_Private->avCodecContext->sample_fmt== AV_SAMPLE_FMT_S32)
			omx_audiodec_component_Private->pAudioPcmMode.nBitPerSample = 32;
		omx_audiodec_component_Private->pAudioPcmMode.nSamplingRate = omx_audiodec_component_Private->avCodecContext->sample_rate;

		// Send Port Settings changed call back
		(*(omx_audiodec_component_Private->callbacks->EventHandler))
			  (openmaxStandComp,
					  omx_audiodec_component_Private->callbackData,
					  OMX_EventPortSettingsChanged, /* The command was completed */
					  0,
					  1, /* This is the output port index */
					  NULL);
		}

	if (len < 0) {
		DEBUG(DEB_LEV_ERR,"error in packet decoding in audio decoder \n");
	} else {
		// If output is max length it might be an error, so Don't send output buffer
		if((output_length != OUTPUT_LEN_STANDARD_FFMPEG) || (output_length <= pOutputBuffer->nAllocLen)) {
			pOutputBuffer->nFilledLen += output_length;
		}
		pInputBuffer->nFilledLen = 0;
		omx_audiodec_component_Private->isNewBuffer = 1;
	}

	avcodec_free_frame(&decoded_frame);
}

OMX_ERRORTYPE omx_audiodec_component_SetParameter(
		OMX_HANDLETYPE hComponent,
		OMX_INDEXTYPE nParamIndex,
		OMX_PTR ComponentParameterStructure)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_AUDIO_PARAM_CONVERTTYPE *pAudioConvert;
	OMX_PARAM_PORTDEFINITIONTYPE* pPortParam;
	OMX_U32 portIndex;

	// Check which structure we are being fed and make control its header
	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = openmaxStandComp->pComponentPrivate;
	omx_base_audio_PortType *port;
	if (ComponentParameterStructure == NULL) {
		return OMX_ErrorBadParameter;
	}

	DEBUG(DEB_LEV_SIMPLE_SEQ, "   Setting parameter %i\n", nParamIndex);
	switch(nParamIndex) {
	case OMX_IndexParamAudioConvert:
		pAudioConvert = (OMX_AUDIO_PARAM_CONVERTTYPE*) ComponentParameterStructure;
		portIndex = pAudioConvert->nPortIndex;
		err = omx_base_component_ParameterSanityCheck(hComponent,portIndex,pAudioConvert,sizeof(OMX_AUDIO_PARAM_CONVERTTYPE));
		if(err!=OMX_ErrorNone) {
			DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,err);
			break;
		}
		if (pAudioConvert->nPortIndex <= 1) {
			memcpy(&omx_audiodec_component_Private->pAudioConvert,pAudioConvert,sizeof(OMX_AUDIO_PARAM_CONVERTTYPE));
		} else {
			return OMX_ErrorBadPortIndex;
		}
		break;
	case OMX_IndexParamPortDefinition:
		pPortParam = (OMX_PARAM_PORTDEFINITIONTYPE*) ComponentParameterStructure;
		portIndex = pPortParam->nPortIndex;
		err = omx_base_component_ParameterSanityCheck(hComponent,portIndex,pPortParam,sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
		if(err!=OMX_ErrorNone) {
			DEBUG(DEB_LEV_ERR, "In %s Parameter Check Error=%x\n",__func__,err);
			break;
		}
		port = (omx_base_audio_PortType *) omx_audiodec_component_Private->ports[portIndex];
		memcpy(&port->sPortParam,pPortParam,sizeof(OMX_PARAM_PORTDEFINITIONTYPE));

		break;
	default: /*Call the base component function*/
		return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
	}
	return err;
}

OMX_ERRORTYPE omx_audiodec_component_GetParameter(
		OMX_HANDLETYPE hComponent,
		OMX_INDEXTYPE nParamIndex,
		OMX_PTR ComponentParameterStructure)
{
	OMX_AUDIO_PARAM_CONVERTTYPE *pAudioConvert;
	OMX_PARAM_PORTDEFINITIONTYPE* pPortParam;
	omx_base_audio_PortType *port;
	OMX_ERRORTYPE err = OMX_ErrorNone;

	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = (omx_audiodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;
	if (ComponentParameterStructure == NULL) {
		return OMX_ErrorBadParameter;
	}
	DEBUG(DEB_LEV_SIMPLE_SEQ, "   Getting parameter %i\n", nParamIndex);

	switch(nParamIndex) {
	case OMX_IndexParamAudioInit:
		if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone) {
			break;
		}
		memcpy(ComponentParameterStructure, &omx_audiodec_component_Private->sPortTypesParam[OMX_PortDomainAudio], sizeof(OMX_PORT_PARAM_TYPE));
		break;
	case OMX_IndexParamAudioConvert:
		pAudioConvert = (OMX_AUDIO_PARAM_CONVERTTYPE*) ComponentParameterStructure;
		if (pAudioConvert->nPortIndex > 1) {
			return OMX_ErrorBadPortIndex;
		}
		if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_AUDIO_PARAM_CONVERTTYPE))) != OMX_ErrorNone) {
			break;
		}
		memcpy(pAudioConvert, &omx_audiodec_component_Private->pAudioConvert,sizeof(OMX_AUDIO_PARAM_CONVERTTYPE));
		break;
	case OMX_IndexParamPortDefinition:
		pPortParam = (OMX_PARAM_PORTDEFINITIONTYPE*) ComponentParameterStructure;

		if ((err = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_PORTDEFINITIONTYPE))) != OMX_ErrorNone) {
			break;
		}
		port = (omx_base_audio_PortType *)omx_audiodec_component_Private->ports[pPortParam->nPortIndex];
		memcpy(pPortParam, &port->sPortParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
		break;
	default:
		return omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
	}
	return err;
}

OMX_ERRORTYPE omx_audiodec_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp,internalRequestMessageType *message)
{
	omx_audiodec_component_PrivateType* omx_audiodec_component_Private = (omx_audiodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE err;
	OMX_STATETYPE eCurrentState = omx_audiodec_component_Private->state;

	DEBUG(DEB_LEV_FUNCTION_NAME, "In %s\n", __func__);

	if (message->messageType == OMX_CommandStateSet){
		if ((message->messageParam == OMX_StateExecuting ) && (omx_audiodec_component_Private->state == OMX_StateIdle)) {
			omx_audiodec_component_Private->isFirstBuffer = OMX_TRUE;
		}
		else if ((message->messageParam == OMX_StateIdle ) && (omx_audiodec_component_Private->state == OMX_StateLoaded)) {
			err = omx_audiodec_component_Init(openmaxStandComp);
			if(err!=OMX_ErrorNone) {
				DEBUG(DEB_LEV_ERR, "In %s Audio Decoder Init Failed Error=%x\n",__func__,err);
				return err;
			}
		} else if ((message->messageParam == OMX_StateLoaded) && (omx_audiodec_component_Private->state == OMX_StateIdle)) {
			err = omx_audiodec_component_Deinit(openmaxStandComp);
			if(err!=OMX_ErrorNone) {
				DEBUG(DEB_LEV_ERR, "In %s Audio Decoder Deinit Failed Error=%x\n",__func__,err);
				return err;
			}
		}
	}

	// Execute the base message handling
	err =  omx_base_component_MessageHandler(openmaxStandComp,message);

	if (message->messageType == OMX_CommandStateSet){
		if ((message->messageParam == OMX_StateIdle  ) && (eCurrentState == OMX_StateExecuting)) {
			if (omx_audiodec_component_Private->avcodecReady) {
				omx_audiodec_component_ffmpegLibDeInit(omx_audiodec_component_Private);
				omx_audiodec_component_Private->avcodecReady = OMX_FALSE;
			}
		}
	}
	return err;
}

OMX_ERRORTYPE omx_audiodec_component_ComponentRoleEnum(
  OMX_HANDLETYPE hComponent,
  OMX_U8 *cRole,
  OMX_U32 nIndex)
{
	if (nIndex == 0) {
		strcpy((char*)cRole, AUDIO_DEC_AC3_ROLE);
	} else {
		return OMX_ErrorUnsupportedIndex;
	}
	return OMX_ErrorNone;
}
