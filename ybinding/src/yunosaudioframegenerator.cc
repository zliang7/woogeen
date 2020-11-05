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
 * yunosaudioframegenerator.cc
 *
 * A YunOS-specfic implementation of the AudioFrameGeneratorInterface.
 * Capture audio using the YunOS AudioCapture API.
 *
 * author: Nathaniel Chen
 */

#include "yunosaudioframegenerator.h"
#include <memory>
#include "stdint.h"
#include <log/Log.h>
#include <audio/AudioCapture.h>
#include <iostream>
#include <util/Utils.h>

#ifndef LOG_TAG
#define LOG_TAG "YUNOS_AUDIO_FRAME_GENERATOR"
#endif

#define SAMPLE_RATE 48000
#define BUFFER_SIZE 4096  //((SAMPLE_RATE)/1000 * 2 /*16bit*/ * 2 /*channels*/ * 10 /*ms*/)

YunOSAudioFrameGenerator::YunOSAudioFrameGenerator(std::unique_ptr<YunOS::AudioCapture> ac) {
  audio_capture = std::move(ac);
}

YunOSAudioFrameGenerator::~YunOSAudioFrameGenerator() {
  //stop audio stream
  audio_capture->stop();
}

//Instantiate AudioCapture and initialize
//returns null if failed to instantiate AudioCapture or failed to start stream
std::unique_ptr<YunOSAudioFrameGenerator> YunOSAudioFrameGenerator::Create() {
  std::unique_ptr<YunOSAudioFrameGenerator> frame_generator;
  std::unique_ptr<YunOS::AudioCapture> audio_capture;
  audio_capture.reset(YunOS::AudioCapture::create(
    ADEV_SOURCE_DEFAULT,                     //input source
    SAMPLE_RATE,
    SND_FORMAT_PCM_16_BIT,                   //sound format
    ADEV_CHANNEL_IN_STEREO,                  //channel mask
    BUFFER_SIZE
  ));
  if (audio_capture) {
    //Start capturing audio
    int started = -1;
    started = audio_capture->start(SYNC_EVENT_NONE);
    if (started == STATUS_OK) {
      frame_generator.reset(new YunOSAudioFrameGenerator(move(audio_capture)));
    } else {
      LOG_E("YunOSAudioFrameGenerator: Failed to start AudioCapture");
      frame_generator.reset(nullptr);
    }
  } else {
    LOG_E("YunOSAudioFrameGenerator: Failed to instantiate AudioCapture");
    frame_generator.reset(nullptr);
  }
  return frame_generator;
}

//Get audio frame from the buffer
//returns 0 if there is an error in AudioCapture
//Caller should check if returned value is equal to buffer size
uint32_t YunOSAudioFrameGenerator::GenerateFramesForNext10Ms(uint8_t* buffer, const uint32_t capacity) {
  uint32_t retval;
  //read from AudioCapture buffer
  size_t cap = static_cast<size_t>(capacity);
  ssize_t bytes_read = this->audio_capture->read(buffer, cap);
  uint32_t yBytesRead = static_cast<uint32_t>(bytes_read);
  retval = yBytesRead;
#if defined(_DEBUG)
  if (bytes_read == 0) {
    LOG_D("YunOSAudioFrameGenerator: No bytes read from AudioCapture");
  } else if (yBytesRead < capacity) {
    LOG_D("YunOSAudioFrameGenerator: Bytes read did not fill buffer capacity: %d / %d", bytes_read, cap);
  }
#endif
  if (bytes_read == STATUS_INVALID_OPERATION) {
    LOG_E("YunOSAudioFrameGenerator: Invalid operation in AudioCapture");
    retval = 0;
  } else if (bytes_read == STATUS_INVALID_PARAM) {
    LOG_E("YunOSAudioFrameGenerator: Invalid parameter in AudioCapture");
    retval = 0;
  }
  return retval;
}

//Gets the audio sample rate in Hz
int YunOSAudioFrameGenerator::GetSampleRate() {
  int sample_rate = static_cast<int>(this->audio_capture->getSampleRate());
  return sample_rate;
}

//Gets the number of audio channels
int YunOSAudioFrameGenerator::GetChannelNumber() {
  int channel_number = static_cast<int>(this->audio_capture->getChannelCount());
  return channel_number;
}
