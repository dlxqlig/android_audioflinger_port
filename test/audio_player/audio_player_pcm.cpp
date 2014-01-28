#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <string.h>
#include <utility>
#include <sys/stat.h>
#include <string>
#include <deque>

#include <media/stagefright/AudioPlayer.h>

#define MAX_BUFFER_LEN 4 * 1024

using namespace android;

typedef struct Packet{
    int size;
    unsigned char *data;
}Packet;

Packet *m_pkt = NULL;
FILE *fp = NULL;
pthread_t m_thread;
char* m_filename = NULL;
AudioPlayer  *m_audio_player = NULL;
unsigned int m_max_data_size = 3 * 1024 * 1024;
unsigned int m_cached_size = 0;
pthread_mutex_t m_lock;
std::deque<Packet *> m_packets;
pthread_cond_t m_packet_cond;
bool m_eof = false;

void Sleep(int dwMilliSeconds)
{
    struct timespec req;
    req.tv_sec = dwMilliSeconds / 1000;
    req.tv_nsec = (dwMilliSeconds % 1000) * 1000000;

    while ( nanosleep(&req, &req) == -1 && errno == EINTR && (req.tv_nsec > 0 || req.tv_sec > 0));
}

Packet *AllocPacket(int size)
{
    Packet *pkt = (Packet *)malloc(sizeof(Packet));
    if (pkt) {
        memset(pkt, 0, sizeof(Packet));

        pkt->data = (unsigned char*) malloc(size + 16);
        if(!pkt->data) {
            free(pkt);
            pkt = NULL;
        }
        else {
            memset(pkt->data + size, 0, 16);
            pkt->size = size;
        }
    }
    return pkt;
}

void FreePacket(Packet *pkt)
{
    if (pkt) {
        if(pkt->data) {
            free(pkt->data);
            pkt->data = NULL;
        }
        free(pkt);
        pkt = NULL;
    }
}

void Lock()
{
    pthread_mutex_lock(&m_lock);
}

void UnLock()
{
    pthread_mutex_unlock(&m_lock);
}

static void* write_audio_frames_render(void* pcontext)
{
    Packet *pkt = NULL;
    int added = 0;

    m_audio_player->start();

    while (1) {
        if (m_eof && 0 == m_cached_size)
            break;

        Lock();
        if (m_packets.empty())
            pthread_cond_wait(&m_packet_cond, &m_lock);
        UnLock();

        Lock();
        if ( !pkt && !m_packets.empty()){
            pkt = m_packets.front();
            m_cached_size -= pkt->size;
            m_packets.pop_front();
        }
        UnLock();

        int pkt_size = pkt->size;

        while (pkt_size > 0) {
            added = m_audio_player->write(pkt->data, pkt->size);
            pkt_size -= added;
        }
        if (pkt){
            FreePacket(pkt);
            pkt = NULL;
        }
    }

    return (void*)0;
}

void sig_handler(int s)
{
    if (s==SIGINT) {
        signal(SIGINT, SIG_DFL);
        return;
    }
    signal(SIGABRT, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    abort();
}

bool file_exist(const std::string& path)
{
    struct stat buf;
    int error = stat(path.c_str(), &buf);
    return !error || errno != ENOENT;
}

int main(int argc, char *argv[])
{

    signal(SIGSEGV, sig_handler);
    signal(SIGABRT, sig_handler);
    signal(SIGFPE, sig_handler);
    signal(SIGINT, sig_handler);

    int read_len = 0;
    int file_size = 0;
    int pos = 0;
    int added = 0;

    pthread_cond_init(&m_packet_cond, NULL);
    pthread_mutex_init(&m_lock, NULL);

    if (optind >= argc) {
        return 0;
    }

    m_filename = argv[optind];
    if(!file_exist(m_filename)) {
        printf("file is not found[%s]", m_filename);
        return 0;
    }

    if (NULL == (fp = fopen(m_filename, "r"))){
        printf("failed to open the file %s", m_filename);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, 0);

    m_audio_player = new AudioPlayer();
    if (!m_audio_player) {
        printf("failed to create audio player audio\n");
        goto do_exit;
    }

    pthread_create(&m_thread, NULL, write_audio_frames_render, NULL);

    do{
        m_pkt = AllocPacket(MAX_BUFFER_LEN);
        read_len = fread(m_pkt->data, sizeof(char), MAX_BUFFER_LEN, fp);

        if(read_len > 0)
            pos += read_len;

        m_pkt->size = read_len;
        if (m_cached_size + read_len < m_max_data_size){
            Lock();
            m_cached_size += m_pkt->size;
            m_packets.push_back(m_pkt);
            UnLock();
            pthread_cond_broadcast(&m_packet_cond);
        }else{
            Sleep(100);
        }
    }while(pos < file_size);

    m_eof = true;

do_exit:
    pthread_join(m_thread, NULL);

    pthread_cond_destroy(&m_packet_cond);
    pthread_mutex_destroy(&m_lock);

    if (m_audio_player)
        delete m_audio_player;
    m_audio_player = NULL;

    fclose(fp);
    return 0;
}
