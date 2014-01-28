#include <algorithm>
#include "aud-dec/OMXAudio.h"
#include "aud-dec/Logger.h"

#define CLASSNAME "COMXAudio"
#define OMX_MAX_CHANNELS 9
#ifndef VOLUME_MINIMUM
#define VOLUME_MINIMUM -6000  // -60dB
#endif

using namespace std;

COMXAudio::COMXAudio() :
    m_Initialized(false),
    m_CurrentVolume(0),
    m_BytesPerSec(0),
    m_BufferLen(0),
    m_ChunkLen(0),
    m_InputChannels(0),
    m_eEncoding(OMX_AUDIO_CodingPCM),
    m_extradata(NULL),
    m_extrasize(0),
    m_fifo_size(0.0),
    m_settings_changed(false)
{
}

COMXAudio::~COMXAudio()
{
    if(m_Initialized)
        Deinitialize();
}


bool COMXAudio::PortSettingsChanged()
{
    if (m_settings_changed) {
        m_omx_decoder.DisablePort(m_omx_decoder.GetOutputPort(), true);
        m_omx_decoder.EnablePort(m_omx_decoder.GetOutputPort(), true);
        return true;
    }

    m_settings_changed = true;

    return true;
}

bool COMXAudio::Initialize(int iConvertFmt, int iConvertChannel, int iChannels, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, long initialVolume, float fifo_size)
{

    m_fifo_size = fifo_size;
    m_CurrentVolume = initialVolume;
    m_InputChannels = iChannels;

    // the buffer is 16-bit integer and channels are rounded up
    m_BytesPerSec = uiSamplesPerSec * (16 >> 3) * (m_InputChannels > 4 ? 8 : m_InputChannels);
    m_BufferLen = m_BytesPerSec*m_fifo_size;
    m_ChunkLen = 32*1024;

    OMX_ERRORTYPE omx_err = OMX_ErrorNone;
    if (!m_omx_decoder.Initialize("OMX.st.audio_decoder.ac3", OMX_IndexParamAudioInit)) {
        return false;
    }

    // set up the number/size of buffers for decoder input
    OMX_PARAM_PORTDEFINITIONTYPE port_param;
    OMX_AUDIO_PARAM_CONVERTTYPE convert;

    OMX_INIT_STRUCTURE(port_param);
    port_param.nPortIndex = m_omx_decoder.GetInputPort();
    omx_err = m_omx_decoder.GetParameter(OMX_IndexParamPortDefinition, &port_param);
    if(omx_err != OMX_ErrorNone) {
        Logger::LogOut(LOG_LEVEL_ERROR, "COMXAudio::Initialize error get OMX_IndexParamPortDefinition (input) omx_err(0x%08x)", omx_err);
        return false;
    }

    port_param.format.audio.eEncoding = m_eEncoding;
    port_param.nBufferSize = m_ChunkLen;
    port_param.nBufferCountActual = port_param.nBufferCountMin;

    omx_err = m_omx_decoder.SetParameter(OMX_IndexParamPortDefinition, &port_param);
    if(omx_err != OMX_ErrorNone) {
        Logger::LogOut(LOG_LEVEL_ERROR, "COMXAudio::Initialize error set OMX_IndexParamPortDefinition (intput) omx_err(0x%08x)", omx_err);
    return false;
    }

    OMX_INIT_STRUCTURE(convert);
    convert.nPortIndex = m_omx_decoder.GetInputPort();
    convert.sample_fmt = iConvertFmt;
    convert.channel = iConvertChannel;
    omx_err = m_omx_decoder.SetParameter(OMX_IndexParamAudioConvert, &convert);
    if(omx_err != OMX_ErrorNone) {
        Logger::LogOut(LOG_LEVEL_ERROR, "COMXAudio::Initialize error set OMX_IndexParamAudioConvert (output) omx_err(0x%08x)\n", omx_err);
        return false;
    }

    // set up the number/size of buffers for decoder output
    OMX_INIT_STRUCTURE(port_param);
    port_param.nPortIndex = m_omx_decoder.GetOutputPort();
    omx_err = m_omx_decoder.GetParameter(OMX_IndexParamPortDefinition, &port_param);
    if(omx_err != OMX_ErrorNone) {
        Logger::LogOut(LOG_LEVEL_ERROR, "COMXAudio::Initialize error get OMX_IndexParamPortDefinition (output) omx_err(0x%08x)", omx_err);
        return false;
    }
    port_param.nBufferCountActual = std::max(1, (int)(m_BufferLen / port_param.nBufferSize));
    omx_err = m_omx_decoder.SetParameter(OMX_IndexParamPortDefinition, &port_param);
    if(omx_err != OMX_ErrorNone) {
        Logger::LogOut(LOG_LEVEL_ERROR, "COMXAudio::Initialize error set OMX_IndexParamPortDefinition (output) omx_err(0x%08x)", omx_err);
        return false;
    }

    omx_err = m_omx_decoder.AllocInputBuffers();
    if(omx_err != OMX_ErrorNone) {
        Logger::LogOut(LOG_LEVEL_ERROR, "COMXAudio::Initialize - Error alloc buffers 0x%08x", omx_err);
        return false;
    }

    omx_err = m_omx_decoder.AllocOutputBuffers();
    if(omx_err != OMX_ErrorNone) {
        Logger::LogOut(LOG_LEVEL_ERROR, "COMXAudio::Initialize - Error alloc buffers 0x%08x", omx_err);
        return false;
    }

    omx_err = m_omx_decoder.SetStateForComponent(OMX_StateExecuting);
    if(omx_err != OMX_ErrorNone) {
        Logger::LogOut(LOG_LEVEL_ERROR, "COMXAudio::Initialize - Error setting OMX_StateExecuting 0x%08x", omx_err);
        return false;
    }

    OMX_BUFFERHEADERTYPE *omx_input_buffer = m_omx_decoder.GetInputBuffer();
    if(omx_input_buffer == NULL) {
        Logger::LogOut(LOG_LEVEL_ERROR, "%s::%s - buffer error 0x%08x", CLASSNAME, __func__, omx_err);
        return false;
    }

    omx_input_buffer->nOffset = 0;
    omx_input_buffer->nFilledLen = m_extrasize;
    if(omx_input_buffer->nFilledLen > omx_input_buffer->nAllocLen) {
        Logger::LogOut(LOG_LEVEL_ERROR, "%s::%s - omx_buffer->nFilledLen > omx_buffer->nAllocLen", CLASSNAME, __func__);
        return false;
    }

    memset((unsigned char *)omx_input_buffer->pBuffer, 0x0, omx_input_buffer->nAllocLen);
    memcpy((unsigned char *)omx_input_buffer->pBuffer, m_extradata, omx_input_buffer->nFilledLen);
    omx_input_buffer->nFlags = OMX_BUFFERFLAG_CODECCONFIG | OMX_BUFFERFLAG_ENDOFFRAME;

    omx_err = m_omx_decoder.EmptyThisBuffer(omx_input_buffer);
    if (omx_err != OMX_ErrorNone) {
        Logger::LogOut(LOG_LEVEL_ERROR, "%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)", CLASSNAME, __func__, omx_err);
        return false;
    }

    OMX_BUFFERHEADERTYPE *omx_output_buffer = m_omx_decoder.GetOutputBuffer();
    if(omx_output_buffer == NULL) {
        Logger::LogOut(LOG_LEVEL_ERROR, "%s::%s - buffer1 error 0x%08x", CLASSNAME, __func__, omx_err);
        return false;
    }


    omx_err = m_omx_decoder.FillThisBuffer(omx_output_buffer);
    if (omx_err != OMX_ErrorNone) {
        Logger::LogOut(LOG_LEVEL_ERROR, "%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)", CLASSNAME, __func__, omx_err);
        return false;
    }

    m_Initialized = true;
    m_settings_changed = false;

    return true;
}

bool COMXAudio::Deinitialize()
{
    if(!m_Initialized)
        return true;
    m_omx_decoder.FlushInput();
    m_omx_decoder.Deinitialize(true);

    m_Initialized = false;
    m_BytesPerSec = 0;
    m_BufferLen = 0;

    m_Initialized = false;

    if(m_extradata)
        free(m_extradata);
    m_extradata = NULL;
    m_extrasize = 0;

    return true;
}

void COMXAudio::Flush()
{
    if(!m_Initialized)
        return;

    m_omx_decoder.FlushInput();
}

long COMXAudio::GetCurrentVolume() const
{
    return m_CurrentVolume;
}

void COMXAudio::Mute(bool bMute)
{
    if(!m_Initialized)
        return;

    if (bMute)
        SetCurrentVolume(VOLUME_MINIMUM);
    else
        SetCurrentVolume(m_CurrentVolume);
}

bool COMXAudio::SetCurrentVolume(long nVolume)
{
    return true;
}

unsigned int COMXAudio::GetSpace()
{
    int free = m_omx_decoder.GetInputBufferSpace();
    return free;
}

unsigned int COMXAudio::AddPackets(const void* data, unsigned int len)
{
    return AddPackets(data, len, 0, 0);
}

unsigned int COMXAudio::AddPackets(const void* data, unsigned int len, double dts, double pts)
{
    return 0;
}

float COMXAudio::GetDelay()
{
    unsigned int free = m_omx_decoder.GetInputBufferSize() - m_omx_decoder.GetInputBufferSpace();
    return (float)free / (float)m_BytesPerSec;
}

float COMXAudio::GetCacheTime()
{
    float fBufferLenFull = (float)m_BufferLen - (float)GetSpace();
    if(fBufferLenFull < 0)
        fBufferLenFull = 0;
    float ret = fBufferLenFull / (float)m_BytesPerSec;
    return ret;
}

float COMXAudio::GetCacheTotal()
{
    return (float)m_BufferLen / (float)m_BytesPerSec;
}

unsigned int COMXAudio::GetChunkLen()
{
    return m_ChunkLen;
}

unsigned int COMXAudio::GetAudioRenderingLatency()
{
    return 0;
}

bool COMXAudio::IsEOS()
{
    if(!m_Initialized)
        return false;
    unsigned int latency = GetAudioRenderingLatency();
    bool ret = m_omx_render.IsEOS() && latency <= 0;
    Logger::LogOut(LOG_LEVEL_DEBUG, "%s::%s = %d (%d,%d)", CLASSNAME, __func__, ret, m_omx_decoder.IsEOS(), latency);
    return ret;
}

int COMXAudio::Decode(uint8_t *pData, int iSize)
{
    OMX_ERRORTYPE omx_err;

    if (pData || iSize > 0)	{
        unsigned int demuxer_bytes = (unsigned int)iSize;
        uint8_t *demuxer_content = pData;

        while(demuxer_bytes) {
            // 500ms timeout
            OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer(500);
            if(omx_buffer == NULL) {
                Logger::LogOut(LOG_LEVEL_ERROR, "OMXAudio::Decode timeout");
                return false;
            }

            omx_buffer->nFlags = 0;
            omx_buffer->nOffset = 0;

            omx_buffer->nFilledLen = (demuxer_bytes > omx_buffer->nAllocLen) ? omx_buffer->nAllocLen : demuxer_bytes;
            memcpy(omx_buffer->pBuffer, demuxer_content, omx_buffer->nFilledLen);

            demuxer_bytes -= omx_buffer->nFilledLen;
            demuxer_content += omx_buffer->nFilledLen;

            if(demuxer_bytes == 0) {
                omx_buffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
            }

            int nRetry = 0;
            while(true) {
                omx_err = m_omx_decoder.EmptyThisBuffer(omx_buffer);
                if (omx_err == OMX_ErrorNone) {
                    break;
                }
                else {
                    Logger::LogOut(LOG_LEVEL_ERROR, "%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)", CLASSNAME, __func__, omx_err);
                    nRetry++;
                }
                if(nRetry == 5) {
                    Logger::LogOut(LOG_LEVEL_ERROR, "%s::%s - OMX_EmptyThisBuffer() finally failed", CLASSNAME, __func__);
                    return false;
                }
            }
        }
        return true;
    }

    return false;
}

OMXPacket *COMXAudio::GetData()
{
    OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetOutputBuffer();
    OMXPacket *pkt = NULL;
    if(omx_buffer) {
        if(omx_buffer->nFilledLen) {
            pkt = AllocPacket(omx_buffer->nFilledLen);
            if(pkt) {
                pkt->size = omx_buffer->nFilledLen;
                memcpy(pkt->data, omx_buffer->pBuffer, omx_buffer->nFilledLen);
            }
        }
        m_omx_decoder.FillThisBuffer(omx_buffer);
    }
    return pkt;
}

void COMXAudio::SubmitEOS()
{
    if (!m_Initialized)
        return;
    OMX_ERRORTYPE omx_err = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *omx_buffer = m_omx_decoder.GetInputBuffer();

    if(omx_buffer == NULL)
    {
        Logger::LogOut(LOG_LEVEL_ERROR, "%s::%s - buffer error 0x%08x", CLASSNAME, __func__, omx_err);
        return;
    }

    omx_buffer->nOffset = 0;
    omx_buffer->nFilledLen = 0;

    omx_buffer->nFlags = OMX_BUFFERFLAG_ENDOFFRAME | OMX_BUFFERFLAG_EOS;

    omx_err = m_omx_decoder.EmptyThisBuffer(omx_buffer);
    if (omx_err != OMX_ErrorNone)
    {
        Logger::LogOut(LOG_LEVEL_ERROR, "%s::%s - OMX_EmptyThisBuffer() failed with result(0x%x)\n", CLASSNAME, __func__, omx_err);
        return;
    }
    Logger::LogOut(LOG_LEVEL_DEBUG, "%s::%s\n", CLASSNAME, __func__);
}
