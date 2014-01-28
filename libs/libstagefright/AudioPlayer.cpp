/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "AudioPlayer"
#include <media/AudioTrack.h>
#include <media/stagefright/AudioPlayer.h>

namespace android {

AudioPlayer::AudioPlayer()
    : mAudioTrack(NULL) {
}

AudioPlayer::~AudioPlayer() {
}

status_t AudioPlayer::write(unsigned char* data, int size) {
    return mAudioTrack->write((void*)data, size);
}

status_t AudioPlayer::start() {

    int32_t numChannels, channelMask;
    audio_channel_mask_t audioMask = 0;

    mAudioTrack = new AudioTrack(
            AUDIO_STREAM_MUSIC, 48000, AUDIO_FORMAT_PCM_16_BIT, audioMask,
            0, AUDIO_OUTPUT_FLAG_NONE, &AudioCallback, this, 0);

    mAudioTrack->start();

    return OK;
}

// static
void AudioPlayer::AudioCallback(int event, void *user, void *info) {
    static_cast<AudioPlayer *>(user)->AudioCallback(event, info);
}

void AudioPlayer::AudioCallback(int event, void *info) {
    if (event != AudioTrack::EVENT_MORE_DATA) {
        return;
    }

    AudioTrack::Buffer *buffer = (AudioTrack::Buffer *)info;
    buffer->size = 0;
}

size_t AudioPlayer::fillBuffer(void *data, size_t size) {

    size_t size_done = 0;
    size_t size_remaining = size;
    while (size_remaining > 0) {
        size_t copy = size_remaining;
        size_done += copy;
        size_remaining -= copy;
    }

    return size_done;
}
}
