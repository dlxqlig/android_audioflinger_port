#include <stdio.h>
#include <unistd.h>
#include "aud-dec/OMXProcess.h"
#include "aud-dec/Logger.h"

OMXPlayerAudio::OMXPlayerAudio()
{
    m_decoder       = NULL;
    m_player_error  = true;
    m_max_data_size = 3* 1024 * 1024;
    m_out_max_data_size = 3* 1024 * 1024;
    m_fifo_size     = 2.0f;

    pthread_cond_init(&m_packet_cond, NULL);
    pthread_cond_init(&m_out_packet_cond, NULL);
    pthread_mutex_init(&m_lock, NULL);
    pthread_mutex_init(&m_lock_decoder, NULL);
    pthread_mutex_init(&m_lock_out_packet, NULL);
}

OMXPlayerAudio::~OMXPlayerAudio()
{
    Close();

    pthread_cond_destroy(&m_packet_cond);
    pthread_cond_destroy(&m_out_packet_cond);
    pthread_mutex_destroy(&m_lock);
    pthread_mutex_destroy(&m_lock_decoder);
    pthread_mutex_destroy(&m_lock_out_packet);
}

void OMXPlayerAudio::Lock()
{
    pthread_mutex_lock(&m_lock);
}

void OMXPlayerAudio::UnLock()
{
    pthread_mutex_unlock(&m_lock);
}

void OMXPlayerAudio::LockDecoder()
{
    pthread_mutex_lock(&m_lock_decoder);
}

void OMXPlayerAudio::UnLockDecoder()
{
    pthread_mutex_unlock(&m_lock_decoder);
}

void OMXPlayerAudio::LockOutPacket()
{
    pthread_mutex_lock(&m_lock_out_packet);
}

void OMXPlayerAudio::UnLockOutPacket()
{
    pthread_mutex_unlock(&m_lock_out_packet);
}

bool OMXPlayerAudio::Open(int convert_fmt, int convert_channel, int channel, int sample_rate, int sample_size, float queue_size, float fifo_size)
{
    m_g_OMX.Initialize();

    m_cached_size = 0;
    m_out_cached_size = 0;
    if (queue_size != 0.0){
        m_max_data_size = queue_size * 1024 * 1024;
        m_out_max_data_size = queue_size * 1024 * 1024;
    }
    if (fifo_size != 0.0)
        m_fifo_size = fifo_size;

    m_bAbort = false;
    m_channel = channel;
    m_sample_rate = sample_rate;
    m_sample_size = sample_size;
    m_convert_fmt = convert_fmt;
    m_convert_channel = convert_channel;

    m_player_error = OpenDecoder();
    if(!m_player_error)
    {
        Close();
        return false;
    }

    // create process thread for decode
    Create(THREAD_PRIORITY_NORMAL);
    return true;
}

bool OMXPlayerAudio::Close()
{
    m_bAbort = true;

    if (ThreadHandle()){
        Lock();
        pthread_cond_broadcast(&m_packet_cond);
        UnLock();

        pthread_cond_broadcast(&m_out_packet_cond);

        StopThread();
    }

    CloseDecoder();
    m_g_OMX.Deinitialize();

  return true;
}

bool OMXPlayerAudio::Decode(OMXPacket *pkt)
{
    if(!pkt)
        return false;

    m_decoder->Decode(pkt->data, pkt->size);

    return true;
}

void OMXPlayerAudio::Process()
{
    OMXPacket *omx_pkt = NULL;

    while(!m_bStop && !m_bAbort) {
        Lock();
        if(m_packets.empty())
            pthread_cond_wait(&m_packet_cond, &m_lock);
        UnLock();

        if(m_bAbort)
            break;

        Lock();
        if(!omx_pkt && !m_packets.empty()) {
            omx_pkt = m_packets.front();
            m_cached_size -= omx_pkt->size;
            m_packets.pop_front();
        }
        UnLock();

        LockDecoder();
        if(omx_pkt && Decode(omx_pkt)) {
            FreePacket(omx_pkt);
            omx_pkt = NULL;
        }
        UnLockDecoder();
        OMXPacket *decode_pkt = m_decoder->GetData();
        if (!decode_pkt) {
            continue;
        }
        LockOutPacket();

        if (decode_pkt) {
            printf("push output to list, output size %d pkt_data %x, pkt_size %d\n",
                    m_out_packets.size(), decode_pkt->data, decode_pkt->size);
            if ((m_out_cached_size + decode_pkt->size) < m_out_max_data_size){
                m_out_cached_size += decode_pkt->size;
                m_out_packets.push_back(decode_pkt);
            }
        }
        if (m_out_packets.size() >= 50) {
            pthread_cond_wait(&m_out_packet_cond, &m_lock_out_packet);
        }

        UnLockOutPacket();
    }

    if(omx_pkt)
        FreePacket(omx_pkt);
}

bool OMXPlayerAudio::AddPacket(OMXPacket *pkt)
{
    bool ret = false;

    if(!pkt)
        return ret;

    if(m_bStop || m_bAbort)
        return ret;

    if((m_cached_size + pkt->size) < m_max_data_size) {
        Lock();
        m_cached_size += pkt->size;
        m_packets.push_back(pkt);
        UnLock();
        ret = true;
        pthread_cond_broadcast(&m_packet_cond);
    }

    return ret;
}

bool OMXPlayerAudio::OpenDecoder()
{
    bool bDecoderOpen = false;

    m_decoder = new COMXAudio();

    bDecoderOpen = m_decoder->Initialize(m_convert_fmt, m_convert_channel, m_channel, m_sample_rate, 16, 0, m_fifo_size);

    if(!bDecoderOpen) {
        delete m_decoder;
        m_decoder = NULL;
        return false;
    }

    return true;
}

bool OMXPlayerAudio::CloseDecoder()
{
    if(m_decoder)
        delete m_decoder;
    m_decoder   = NULL;
    return true;
}

OMXPacket *OMXPlayerAudio::GetData()
{
    OMXPacket *pkt = NULL;

    LockOutPacket();
    if(!m_out_packets.empty()) {
        pkt = m_out_packets.front();
        m_out_cached_size -= pkt->size;
        m_out_packets.pop_front();
        Logger::LogOut(LOG_LEVEL_DEBUG, "got one package from output list!");
        if (m_out_packets.size() < 5) {
            pthread_cond_broadcast(&m_out_packet_cond);
        }

    }
    else {
        Logger::LogOut(LOG_LEVEL_DEBUG,"output list is empty!");
        pthread_cond_broadcast(&m_out_packet_cond);
    }
    UnLockOutPacket();

    return pkt;
}

void OMXPlayerAudio::SubmitEOS()
{
    if (m_decoder)
        m_decoder->SubmitEOS();
}

bool OMXPlayerAudio::IsEOS()
{
    return m_packets.empty() && !m_decoder;
}
