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
 * yunosaudioplayer_unittest.cc
 *
 * Test that YunOSAudioPlayer can accept PCM audio
 * author: Nathaniel Chen
 */

#include <gtest/gtest.h>
#include <memory>
#include <fstream>
#include <woogeen/base/audioplayerinterface.h>
#include "yunosaudioplayer.h"

//Requires a 16-bit stereo PCM file
#define SAMPLE_BITS 16
#define SAMPLE_RATE 48000
#define AUDIO_CHANNELS 1
#define NUM_FRAMES 1024
//#define BUFFER_SIZE ((SAMPLE_RATE)/1000 * 2 /*16bit*/ * 2 /*channels*/ * 10 /*ms*/)

//Play audio from a PCM file
TEST(YunOSAudioPlayerTest, PlayFromFile) {
  //Create YunOSAudioPlayer
  std::unique_ptr<woogeen::base::AudioPlayerInterface> player(new YunOSAudioPlayer);
  ASSERT_TRUE(player) << "Failed to instantiate YunOSAudioPlayer";
  //Create input file stream from file
  std::ifstream yStream("test.wav", std::ifstream::binary);
  ASSERT_TRUE(yStream.is_open()) << "Failed to open audio file for reading";
  //Go to beginning
  yStream.seekg(0);
  //Create PCMRawBuffer from stream
  while (!yStream.eof()) {
    int16_t* buffer = new int16_t[NUM_FRAMES];
    yStream.read((char*)buffer, sizeof(int16_t)*NUM_FRAMES);
    std::unique_ptr<woogeen::base::PCMRawBuffer> pcm_buffer;
    pcm_buffer.reset(new woogeen::base::PCMRawBuffer {
      buffer,
      SAMPLE_BITS,
      SAMPLE_RATE,
      AUDIO_CHANNELS,
      NUM_FRAMES
    });
    //Play audio stream from buffer
    player->PlayAudio(std::move(pcm_buffer));
  }
  yStream.close();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
