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
 * yunosaudioplayer.h
 *
 * A YunOS-specific implementation of AudioPlayerInterface.
 * Streams audio to YunOS through the AudioRender API.
 *
 * author: Nathaniel Chen
 */

#ifndef YUNOS_AUDIO_PLAYER_H_
#define YUNOS_AUDIO_PLAYER_H_

#include <woogeen/base/audioplayerinterface.h>
#include <memory>
#include <audio/AudioRender.h>

class YunOSAudioPlayer : public woogeen::base::AudioPlayerInterface {

  public:
    // @brief Constructor
    YunOSAudioPlayer(std::unique_ptr<YunOS::AudioRender>);

    // @brief Destructor stops the stream
    ~YunOSAudioPlayer();

    // @brief Instantiate audio player and initialize, starts stream
    __attribute__ ((visibility("default")))
    static YunOSAudioPlayer* Create();

    // @brief Play raw PCM audio through device
    void PlayAudio(std::unique_ptr<woogeen::base::PCMRawBuffer> buffer) override;

  private:
    std::unique_ptr<YunOS::AudioRender> yAudioRender;
};

#endif //YUNOS_AUDIO_PLAYER_H_
