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

#include <memory>

#include <log/Log.h>
#include <audio/AudioRender.h>
#include <util/Utils.h>

#include "yunosaudioplayer.h"

#ifndef LOG_TAG
#define LOG_TAG "YUNOS_AUDIO_PLAYER"
#endif

static snd_format_t to_pcm_format(int bits_per_sample) {
    static snd_format_t format_map[] = {
        SND_FORMAT_INVALID,
        SND_FORMAT_PCM_8_BIT,
        SND_FORMAT_PCM_16_BIT,
        SND_FORMAT_PCM_8_24_BIT,
        SND_FORMAT_PCM_32_BIT
    };
    assert(size_t(bits_per_sample) >> 3 <
           sizeof(format_map) / sizeof(snd_format_t));
    return format_map[bits_per_sample >> 3];
}
static adev_channel_mask_t to_channel_mask(size_t number_of_channels) {
    static adev_channel_mask_t channel_map[] = {
        ADEV_CHANNEL_NONE,
        ADEV_CHANNEL_OUT_MONO,
        ADEV_CHANNEL_OUT_STEREO,
        ADEV_CHANNEL_INVALID,
        ADEV_CHANNEL_OUT_QUAD
    };
    assert(number_of_channels <
           sizeof(channel_map) / sizeof(adev_channel_mask_t));
    return channel_map[number_of_channels];
}

//Play PCM audio stream
void YunOSAudioPlayer::PlayAudio(std::unique_ptr<woogeen::base::PCMRawBuffer> buffer) {
    auto pcm_format = to_pcm_format(buffer->bits_per_sample);
    auto channel_mask = to_channel_mask(buffer->number_of_channels);
    uint32_t sample_rate = buffer->sample_rate;

    if (render_ == nullptr ||
        render_->getFormat() != pcm_format ||
        render_->getSampleRate() != sample_rate ||
        render_->getChannelMask() != channel_mask) {
        // audio format is changed, recreated the render
        if (render_)  render_->stop();
        render_.reset(YunOS::AudioRender::create(
                AS_TYPE_VOICE_CALL, sample_rate, pcm_format, channel_mask));
        if (!render_ || render_->start() != STATUS_OK) {
            LOG_E("Failed to %s AudioRender", render_ ? "start" : "create");
            render_.reset();
            return;
        }
        LOG_I("AudioRender (re)created: sample_rate=%d, number_of_channel=%zu",
              buffer->sample_rate, buffer->number_of_channels);
    }

    //write to YunOS AudioRender buffer
    size_t size = sizeof(int16_t) *
                  buffer->number_of_frames * buffer->number_of_channels;
    render_->write(buffer->data, size, BLOCKING_MODE);
}
