#ifndef __OPENMAXAUDIORENDER_H__
#define __OPENMAXAUDIORENDER_H__

#include "OMXCore.h"
#include "AudBuffer.h"

class COMXAudio
{
public:
    int Decode(uint8_t *pData, int iSize);
    OMXPacket *GetData();
    unsigned int GetChunkLen();
    float GetDelay();
    float GetCacheTime();
    float GetCacheTotal();
    unsigned int GetAudioRenderingLatency();
    COMXAudio();
    bool Initialize(int iConvertFmt, int iConvertChannel, int iChannels, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, long initialVolume = 0, float fifo_size = 0);
    ~COMXAudio();
    bool PortSettingsChanged();

    unsigned int AddPackets(const void* data, unsigned int len);
    unsigned int AddPackets(const void* data, unsigned int len, double dts, double pts);
    unsigned int GetSpace();
    bool Deinitialize();

    long GetCurrentVolume() const;
    void Mute(bool bMute);
    bool SetCurrentVolume(long nVolume);
    void SubmitEOS();
    bool IsEOS();

    void Flush();

    void Process();

private:
    bool          m_Initialized;
    long          m_CurrentVolume;
    unsigned int  m_BytesPerSec;
    unsigned int  m_BufferLen;
    unsigned int  m_ChunkLen;
    unsigned int  m_InputChannels;
    OMX_AUDIO_CODINGTYPE m_eEncoding;
    uint8_t       *m_extradata;
    int           m_extrasize;
    float         m_fifo_size;

protected:
    COMXCoreComponent m_omx_render;
    COMXCoreComponent m_omx_decoder;
    bool              m_settings_changed;
};
#endif

