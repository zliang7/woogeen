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
 * yunosaudioplayer.cc
 *
 * A YunOS-specific implementation of AudioPlayerInterface.
 * Streams audio to YunOS through the AudioRender API.
 *
 * author: Nathaniel Chen
 */

#include "yunosaudioplayer.h"
#include <memory>
#include <log/Log.h>
#include <audio/AudioRender.h>
#include "stdint.h"
#include <util/Utils.h>

#ifndef LOG_TAG
#define LOG_TAG "YUNOS_AUDIO_PLAYER"
#endif

#define SAMPLE_RATE 48000
#define BUFFER_SIZE 2048
#define FRAME_COUNT 1024

YunOSAudioPlayer::YunOSAudioPlayer(std::unique_ptr<YunOS::AudioRender> render) {
  yAudioRender = std::move(render);
}

YunOSAudioPlayer::~YunOSAudioPlayer() {
  //Stop the audio stream
  yAudioRender->stop();
}

//Instantiate audio player and initialize, start stream
YunOSAudioPlayer* YunOSAudioPlayer::Create() {
  std::unique_ptr<YunOS::AudioRender> audio_render;
  //instantiate AudioRender
  audio_render.reset(YunOS::AudioRender::create(
    AS_TYPE_DEFAULT,          //stream type
    SAMPLE_RATE,              //sample rate
    SND_FORMAT_PCM_16_BIT,    //format
    ADEV_CHANNEL_OUT_STEREO,  //channel mask
    FRAME_COUNT
  ));
  if (audio_render) {
    int started = -1;
    started = audio_render->start();
    if (started == STATUS_OK) {
      //instantiate YunOSAudioPlayer
      return new YunOSAudioPlayer(std::move(audio_render));
    } else {
      LOG_E("YunOSAudioFrameGenerator: Failed to start AudioRender");
    }
  } else {
    LOG_E("YunOSAudioPlayer: Failed to instantiate AudioRender");
  }
  return nullptr;
}

//Play PCM audio stream
void YunOSAudioPlayer::PlayAudio(std::unique_ptr<woogeen::base::PCMRawBuffer> buffer) {
  //write to YunOS AudioRender buffer
  this->yAudioRender->write(buffer->data, BUFFER_SIZE, BLOCKING_MODE);
}
