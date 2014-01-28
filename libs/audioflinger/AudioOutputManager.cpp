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

#define LOG_TAG "AudioOutputManager"
//#define LOG_NDEBUG 0

//#define VERY_VERBOSE_LOGGING
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#include <utils/Log.h>

#include "AudioOutputManager.h"
#include "AudioFlinger.h"


namespace android {

// ----------------------------------------------------------------------------
// AudioOutputManager
// ----------------------------------------------------------------------------

void AudioOutputManager::defaultAudioPolicyConfig(void)
{
    EntryLogger entry(__FUNCTION__);
	HwModule *module;
    IOProfile *profile;

    mDefaultOutputDevice = AUDIO_DEVICE_OUT_SPEAKER;
    mAttachedOutputDevices = AUDIO_DEVICE_OUT_SPEAKER;

    module = new HwModule("usb");

    profile = new IOProfile(module);
    profile->mSamplingRates.add(44100);
    profile->mFormats.add(AUDIO_FORMAT_PCM_16_BIT);
    profile->mChannelMasks.add(AUDIO_CHANNEL_OUT_STEREO);
    profile->mSupportedDevices = AUDIO_DEVICE_OUT_SPEAKER;
    profile->mFlags = AUDIO_OUTPUT_FLAG_PRIMARY;
    module->mOutputProfiles.add(profile);

    mHwModules.add(module);
}

void AudioOutputManager::addOutput(audio_io_handle_t id, AudioOutputDescriptor *outputDesc)
{
    outputDesc->mId = id;
    mOutputs.add(id, outputDesc);
}

AudioOutputManager::AudioOutputManager(android::AudioFlinger* flinger)
{
	mAudioFlinger = flinger;
}

status_t AudioOutputManager::initCheck()
{
    return (mPrimaryOutput == 0) ? NO_INIT : NO_ERROR;
}

status_t AudioOutputManager::init()
{

    EntryLogger entry(__FUNCTION__);

	// load default output config.
	defaultAudioPolicyConfig();

    // open all output streams needed to access attached devices
    for (size_t i = 0; i < mHwModules.size(); i++) {
        mHwModules[i]->mHandle = mAudioFlinger->loadHwModule(mHwModules[i]->mName);
        if (mHwModules[i]->mHandle == 0) {
            ALOGW("could not open HW module %s", mHwModules[i]->mName);
            continue;
        }
        // open all output streams needed to access attached devices
        for (size_t j = 0; j < mHwModules[i]->mOutputProfiles.size(); j++)
        {
            const IOProfile *outProfile = mHwModules[i]->mOutputProfiles[j];

            if (outProfile->mSupportedDevices & mAttachedOutputDevices) {
                AudioOutputDescriptor *outputDesc = new AudioOutputDescriptor(outProfile);
                outputDesc->mDevice = (audio_devices_t)(mDefaultOutputDevice &
                                                            outProfile->mSupportedDevices);
                ALOGV("AudioOutputManager::init h:%d dev:%d sr:%d f:%d cm:%d lt:%d",
                		outProfile->mModule->mHandle,
                		outputDesc->mDevice,
                		outputDesc->mSamplingRate,
                		outputDesc->mFormat,
                		outputDesc->mChannelMask,
                		outputDesc->mLatency);
                audio_io_handle_t output = mAudioFlinger->openOutput(
                                                outProfile->mModule->mHandle,
                                                &outputDesc->mDevice,
                                                &outputDesc->mSamplingRate,
                                                &outputDesc->mFormat,
                                                &outputDesc->mChannelMask,
                                                &outputDesc->mLatency,
                                                outputDesc->mFlags);
                                                //AUDIO_OUTPUT_FLAG_DIRECT);
                if (output == 0) {
                    delete outputDesc;
                } else {
                    mAvailableOutputDevices = (audio_devices_t)(mAvailableOutputDevices |
                                            (outProfile->mSupportedDevices & mAttachedOutputDevices));
                    if (mPrimaryOutput == 0 &&
                            outProfile->mFlags & AUDIO_OUTPUT_FLAG_PRIMARY) {
                        mPrimaryOutput = output;
                    }
                    addOutput(output, outputDesc);
                }
            }
        }
    }

    ALOGE_IF((mPrimaryOutput == 0), "Failed to open primary output");
}

AudioOutputManager::~AudioOutputManager()
{
   for (size_t i = 0; i < mOutputs.size(); i++) {
        mAudioFlinger->closeOutput(mOutputs.keyAt(i));
        delete mOutputs.valueAt(i);
   }
   for (size_t i = 0; i < mInputs.size(); i++) {
        mAudioFlinger->closeInput(mInputs.keyAt(i));
        delete mInputs.valueAt(i);
   }
   for (size_t i = 0; i < mHwModules.size(); i++) {
        delete mHwModules[i];
   }
}

#define AUDIO_HARDWARE_MODULE_ID_MAX_LEN 32
AudioOutputManager::HwModule::HwModule(const char *name)
    : mName(strndup(name, AUDIO_HARDWARE_MODULE_ID_MAX_LEN)), mHandle(0)
{
}

AudioOutputManager::HwModule::~HwModule()
{
    for (size_t i = 0; i < mOutputProfiles.size(); i++) {
         delete mOutputProfiles[i];
    }
    for (size_t i = 0; i < mInputProfiles.size(); i++) {
         delete mInputProfiles[i];
    }
    free((void *)mName);
}

void AudioOutputManager::HwModule::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, "  - name: %s\n", mName);
    result.append(buffer);
    snprintf(buffer, SIZE, "  - handle: %d\n", mHandle);
    result.append(buffer);
    write(fd, result.string(), result.size());
    if (mOutputProfiles.size()) {
        write(fd, "  - outputs:\n", sizeof("  - outputs:\n"));
        for (size_t i = 0; i < mOutputProfiles.size(); i++) {
            snprintf(buffer, SIZE, "    output %d:\n", i);
            write(fd, buffer, strlen(buffer));
            mOutputProfiles[i]->dump(fd);
        }
    }
    if (mInputProfiles.size()) {
        write(fd, "  - inputs:\n", sizeof("  - inputs:\n"));
        for (size_t i = 0; i < mInputProfiles.size(); i++) {
            snprintf(buffer, SIZE, "    input %d:\n", i);
            write(fd, buffer, strlen(buffer));
            mInputProfiles[i]->dump(fd);
        }
    }
}

AudioOutputManager::IOProfile::IOProfile(HwModule *module)
    : mFlags((audio_output_flags_t)0), mModule(module)
{
}

AudioOutputManager::IOProfile::~IOProfile()
{
}

// checks if the IO profile is compatible with specified parameters. By convention a value of 0
// means a parameter is don't care
bool AudioOutputManager::IOProfile::isCompatibleProfile(audio_devices_t device,
                                                            uint32_t samplingRate,
                                                            uint32_t format,
                                                            uint32_t channelMask,
                                                            audio_output_flags_t flags) const
{
    if ((mSupportedDevices & device) != device) {
        return false;
    }
    if ((mFlags & flags) != flags) {
        return false;
    }
    if (samplingRate != 0) {
        size_t i;
        for (i = 0; i < mSamplingRates.size(); i++)
        {
            if (mSamplingRates[i] == samplingRate) {
                break;
            }
        }
        if (i == mSamplingRates.size()) {
            return false;
        }
    }
    if (format != 0) {
        size_t i;
        for (i = 0; i < mFormats.size(); i++)
        {
            if (mFormats[i] == format) {
                break;
            }
        }
        if (i == mFormats.size()) {
            return false;
        }
    }
    if (channelMask != 0) {
        size_t i;
        for (i = 0; i < mChannelMasks.size(); i++)
        {
            if (mChannelMasks[i] == channelMask) {
                break;
            }
        }
        if (i == mChannelMasks.size()) {
            return false;
        }
    }
    return true;
}

void AudioOutputManager::IOProfile::dump(int fd)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    snprintf(buffer, SIZE, "    - sampling rates: ");
    result.append(buffer);
    for (size_t i = 0; i < mSamplingRates.size(); i++) {
        snprintf(buffer, SIZE, "%d", mSamplingRates[i]);
        result.append(buffer);
        result.append(i == (mSamplingRates.size() - 1) ? "\n" : ", ");
    }

    snprintf(buffer, SIZE, "    - channel masks: ");
    result.append(buffer);
    for (size_t i = 0; i < mChannelMasks.size(); i++) {
        snprintf(buffer, SIZE, "%04x", mChannelMasks[i]);
        result.append(buffer);
        result.append(i == (mChannelMasks.size() - 1) ? "\n" : ", ");
    }

    snprintf(buffer, SIZE, "    - formats: ");
    result.append(buffer);
    for (size_t i = 0; i < mFormats.size(); i++) {
        snprintf(buffer, SIZE, "%d", mFormats[i]);
        result.append(buffer);
        result.append(i == (mFormats.size() - 1) ? "\n" : ", ");
    }

    snprintf(buffer, SIZE, "    - devices: %04x\n", mSupportedDevices);
    result.append(buffer);
    snprintf(buffer, SIZE, "    - flags: %04x\n", mFlags);
    result.append(buffer);

    write(fd, result.string(), result.size());
}

AudioOutputManager::AudioOutputDescriptor::AudioOutputDescriptor(
        const IOProfile *profile)
    : mId(0), mSamplingRate(0), mFormat((audio_format_t)0),
      mChannelMask((audio_channel_mask_t)0), mLatency(0),
    mFlags((audio_output_flags_t)0), mDevice(AUDIO_DEVICE_NONE),
    mOutput1(0), mOutput2(0), mProfile(profile), mDirectOpenCount(0)
{

}

audio_devices_t AudioOutputManager::AudioOutputDescriptor::device() const
{
}

uint32_t AudioOutputManager::AudioOutputDescriptor::latency()
{
	return 0;
}

bool AudioOutputManager::AudioOutputDescriptor::sharesHwModuleWith(
        const AudioOutputDescriptor *outputDesc)
{
	return false;
}

audio_devices_t AudioOutputManager::AudioOutputDescriptor::supportedDevices()
{
    if (isDuplicated()) {
        return (audio_devices_t)(mOutput1->supportedDevices() | mOutput2->supportedDevices());
    } else {
        return mProfile->mSupportedDevices ;
    }
}

bool AudioOutputManager::AudioOutputDescriptor::isActive(uint32_t inPastMs) const
{
	return false;
}

status_t AudioOutputManager::AudioOutputDescriptor::dump(int fd)
{

    return NO_ERROR;
}

}; // namespace android
