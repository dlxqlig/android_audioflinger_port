#ifndef _OMX_PLAYERAUDIO_H_
#define _OMX_PLAYERAUDIO_H_

#include <deque>
#include <string>
#include <sys/types.h>

#include "OMXAudio.h"
#include "Thread.h"
#include "AudBuffer.h"

using namespace std;

class OMXPlayerAudio : public AThread
{
protected:
    pthread_cond_t            m_packet_cond;
    pthread_cond_t            m_out_packet_cond;
    pthread_mutex_t           m_lock;
    pthread_mutex_t           m_lock_decoder;
    pthread_mutex_t           m_lock_out_packet;
    COMXAudio                 *m_decoder;
    unsigned int              m_cached_size;
    unsigned int              m_max_data_size;
    unsigned int              m_out_cached_size;
    unsigned int              m_out_max_data_size;
    int                       m_channel;
    unsigned int              m_sample_rate;
    unsigned int              m_sample_size;
    int                       m_convert_fmt;
    int                       m_convert_channel;
    float                     m_fifo_size;
    COMXCore                  m_g_OMX;
    std::deque<OMXPacket *>   m_packets;
    std::deque<OMXPacket *>   m_out_packets;
    bool m_player_error;
    bool m_bAbort;
    void Lock();
    void UnLock();
    void LockDecoder();
    void UnLockDecoder();
    void LockOutPacket();
    void UnLockOutPacket();

public:
    OMXPlayerAudio();
    ~OMXPlayerAudio();
    bool Open(int convert_fmt, int convert_channel, int channel, int sample_rate, int sample_size, float queue_size, float fifo_size);
    bool Close();
    bool Decode(OMXPacket *pkt);
    void Process();
    bool AddPacket(OMXPacket *pkt);
    bool OpenDecoder();
    bool CloseDecoder();
    unsigned int GetCached() {return m_cached_size;};
    unsigned int GetOutCached() {return m_out_cached_size;};
    OMXPacket *GetData();
    void SubmitEOS();
    bool IsEOS();
};
#endif
