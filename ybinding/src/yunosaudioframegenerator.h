/*
 * Copyright © 2017 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * yunosaudioframegenerator.h
 *
 * This is a YunOS-specfic implementation of the AudioFrameGeneratorInterface
 * Capture audio using the YunOS AudioCapture API
 * author: Nathaniel Chen
 */

#ifndef YUNOS_AUDIO_FRAME_GENERATOR_H_
#define YUNOS_AUDIO_FRAME_GENERATOR_H_

#include <woogeen/base/framegeneratorinterface.h>
#include "stdint.h"
#include <memory>
#include <audio/AudioCapture.h>

class YunOSAudioFrameGenerator : public woogeen::base::AudioFrameGeneratorInterface {
  public:
    // @brief Constructor
    YunOSAudioFrameGenerator(std::unique_ptr<YunOS::AudioCapture>);

    // @brief Destructor closes audio capture stream
    ~YunOSAudioFrameGenerator();

    // @brief Instantiate and start an audio capture stream
    __attribute__ ((visibility("default")))
    static std::unique_ptr<YunOSAudioFrameGenerator> Create();

    // @brief Capture audio 10 milliseconds at a time
    uint32_t GenerateFramesForNext10Ms(uint8_t* buffer, const uint32_t capacity) override;

    // @brief Get the audio sampling rate
    int GetSampleRate() override;

    // @brief Get the number of audio channels
    int GetChannelNumber() override;

  private:
    std::unique_ptr<YunOS::AudioCapture> audio_capture;
};

#endif //YUNOS_AUDIO_FRAME_GENERATOR_H_
