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

#ifndef LIB_AUDIO_OUTPUT_MANAGER
#define LIB_AUDIO_OUTPUT_MANAGER

#include <stdint.h>
#include <sys/types.h>
#include <cutils/config_utils.h>
#include <cutils/misc.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/SortedVector.h>

#include <hardware/audio_policy.h>
#include <hardware_legacy/AudioSystemLegacy.h>


using namespace android_audio_legacy;

namespace android {
class AudioFlinger;

class AudioOutputManager {


public:
                AudioOutputManager(android::AudioFlinger* flinger );
        virtual ~AudioOutputManager();

        virtual status_t init();
        virtual status_t initCheck();

protected:

        class IOProfile;

        class HwModule {
        public:
                    HwModule(const char *name);
                    ~HwModule();

            void dump(int fd);

            const char *const mName; // base name of the audio HW module (primary, a2dp ...)
            audio_module_handle_t mHandle;
            Vector <IOProfile *> mOutputProfiles; // output profiles exposed by this module
            Vector <IOProfile *> mInputProfiles;  // input profiles exposed by this module
        };

        // the IOProfile class describes the capabilities of an output or input stream.
        // It is currently assumed that all combination of listed parameters are supported.
        // It is used by the policy manager to determine if an output or input is suitable for
        // a given use case,  open/close it accordingly and connect/disconnect audio tracks
        // to/from it.
        class IOProfile
        {
        public:
            IOProfile(HwModule *module);
            ~IOProfile();

            bool isCompatibleProfile(audio_devices_t device,
                                     uint32_t samplingRate,
                                     uint32_t format,
                                     uint32_t channelMask,
                                     audio_output_flags_t flags) const;

            void dump(int fd);

            // by convention, "0' in the first entry in mSamplingRates, mChannelMasks or mFormats
            // indicates the supported parameters should be read from the output stream
            // after it is opened for the first time
            Vector <uint32_t> mSamplingRates; // supported sampling rates
            Vector <audio_channel_mask_t> mChannelMasks; // supported channel masks
            Vector <audio_format_t> mFormats; // supported audio formats
            audio_devices_t mSupportedDevices; // supported devices (devices this output can be
                                               // routed to)
            audio_output_flags_t mFlags; // attribute flags (e.g primary output,
                                                // direct output...). For outputs only.
            HwModule *mModule;                     // audio HW module exposing this I/O stream
        };


        class AudioOutputDescriptor
        {
        public:
            AudioOutputDescriptor(const IOProfile *profile);

            status_t    dump(int fd);

            audio_devices_t device() const;
            bool isDuplicated() const { return (mOutput1 != NULL && mOutput2 != NULL); }
            audio_devices_t supportedDevices();
            uint32_t latency();
            bool sharesHwModuleWith(const AudioOutputDescriptor *outputDesc);
            bool isActive(uint32_t inPastMs = 0) const;


            audio_io_handle_t mId;              // output handle
            uint32_t mSamplingRate;             //
            audio_format_t mFormat;             //
            audio_channel_mask_t mChannelMask;     // output configuration
            uint32_t mLatency;                  //
            audio_output_flags_t mFlags;   //
            audio_devices_t mDevice;                   // current device this output is routed to
            AudioOutputDescriptor *mOutput1;    // used by duplicated outputs: first output
            AudioOutputDescriptor *mOutput2;    // used by duplicated outputs: second output
            const IOProfile *mProfile;          // I/O profile this output derives from
                                                // device selection. See checkDeviceMuteStrategies()
            uint32_t mDirectOpenCount; // number of clients using this output (direct outputs only)
        };

        // descriptor for audio inputs. Used to maintain current configuration of each opened audio input
        // and keep track of the usage of this input.
        class AudioInputDescriptor
        {
        public:
            AudioInputDescriptor(const IOProfile *profile);

            status_t    dump(int fd);

            uint32_t mSamplingRate;                     //
            audio_format_t mFormat;                     // input configuration
            audio_channel_mask_t mChannelMask;             //
            audio_devices_t mDevice;                    // current device this input is routed to
            uint32_t mRefCount;                         // number of AudioRecord clients using this output
            int      mInputSource;                      // input source selected by application (mediarecorder.h)
            const IOProfile *mProfile;                  // I/O profile this output derives from
        };

        void addOutput(audio_io_handle_t id, AudioOutputDescriptor *outputDesc);
        void defaultAudioPolicyConfig(void);

         audio_io_handle_t mPrimaryOutput;              // primary output handle
        // list of descriptors for outputs currently opened
        DefaultKeyedVector<audio_io_handle_t, AudioOutputDescriptor *> mOutputs;
        DefaultKeyedVector<audio_io_handle_t, AudioInputDescriptor *> mInputs;     // list of input descriptors
        audio_devices_t mAvailableOutputDevices; // bit field of all available output devices
        audio_devices_t mAvailableInputDevices; // bit field of all available input devices
                                                // without AUDIO_DEVICE_BIT_IN to allow direct bit
                                                // field comparisons

        audio_devices_t mAttachedOutputDevices; // output devices always available on the platform
        audio_devices_t mDefaultOutputDevice; // output device selected by default at boot time
                                              // (must be in mAttachedOutputDevices)

        Vector <HwModule *> mHwModules;

        android::AudioFlinger* mAudioFlinger;
};

};

#endif

