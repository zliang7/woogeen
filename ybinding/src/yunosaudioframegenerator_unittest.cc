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
 * yunosaudioframegenerator_unittest.cc
 *
 * Shows that YunOSAudioFrameGenerator can accept different buffer sizes,
 * get stream properties, and output valid PCM audio.
 * author: Nathaniel Chen
 */

#include <gtest/gtest.h>
#include <memory>
#include "yunosaudioframegenerator.h"
#include <fstream>
#include <time.h>

//0-byte buffer
TEST(YunOSAudioFrameGeneratorTest, GenerateFrames0) {
  std::unique_ptr<YunOSAudioFrameGenerator> audio_frame_generator(YunOSAudioFrameGenerator::Create());
  ASSERT_TRUE(audio_frame_generator) << "Failed to instantiate YunOSAudioFrameGenerator";
  uint32_t capacity = 0;
  uint8_t buffer [capacity];
  uint32_t bytes_read = audio_frame_generator->GenerateFramesForNext10Ms(buffer, capacity);
  ASSERT_EQ(bytes_read, capacity) << "YunOSAudioFrameGenerator: Bytes read did not equal capacity";
}

//1024-byte buffer
TEST(YunOSAudioFrameGeneratorTest, GenerateFrames1024) {
  std::unique_ptr<YunOSAudioFrameGenerator> audio_frame_generator(YunOSAudioFrameGenerator::Create());
  ASSERT_TRUE(audio_frame_generator) << "Failed to instantiate YunOSAudioFrameGenerator";
  uint32_t capacity = 1024;
  uint8_t buffer [capacity];
  uint32_t bytes_read = audio_frame_generator->GenerateFramesForNext10Ms(buffer, capacity);
  ASSERT_EQ(bytes_read, capacity) << "YunOSAudioFrameGenerator: Bytes read did not equal capacity";
}

//2048-byte buffer
TEST(YunOSAudioFrameGeneratorTest, GenerateFrames2048) {
  std::unique_ptr<YunOSAudioFrameGenerator> audio_frame_generator(YunOSAudioFrameGenerator::Create());
  ASSERT_TRUE(audio_frame_generator) << "Failed to instantiate YunOSAudioFrameGenerator";
  uint32_t capacity = 2048;
  uint8_t buffer [capacity];
  uint32_t bytes_read = audio_frame_generator->GenerateFramesForNext10Ms(buffer, capacity);
  ASSERT_EQ(bytes_read, capacity) << "YunOSAudioFrameGenerator: Bytes read did not equal capacity";
}

//4096-byte buffer
TEST(YunOSAudioFrameGeneratorTest, GenerateFrames4096) {
  std::unique_ptr<YunOSAudioFrameGenerator> audio_frame_generator(YunOSAudioFrameGenerator::Create());
  ASSERT_TRUE(audio_frame_generator) << "Failed to instantiate YunOSAudioFrameGenerator";
  uint32_t capacity = 4096;
  uint8_t buffer [capacity];
  uint32_t bytes_read = audio_frame_generator->GenerateFramesForNext10Ms(buffer, capacity);
  ASSERT_EQ(bytes_read, capacity) << "YunOSAudioFrameGenerator: Bytes read did not equal capacity";
}

//8192-byte buffer
TEST(YunOSAudioFrameGeneratorTest, GenerateFrames8192) {
  std::unique_ptr<YunOSAudioFrameGenerator> audio_frame_generator(YunOSAudioFrameGenerator::Create());
  ASSERT_TRUE(audio_frame_generator) << "Failed to instantiate YunOSAudioFrameGenerator";
  uint32_t capacity = 8192;
  uint8_t buffer [capacity];
  uint32_t bytes_read = audio_frame_generator->GenerateFramesForNext10Ms(buffer, capacity);
  ASSERT_EQ(bytes_read, capacity) << "YunOSAudioFrameGenerator: Bytes read did not equal capacity";
}

//Write buffer to file
TEST(YunOSAudioFrameGeneratorTest, WriteToFile) {
  std::unique_ptr<YunOSAudioFrameGenerator> audio_frame_generator(YunOSAudioFrameGenerator::Create());
  ASSERT_TRUE(audio_frame_generator) << "Failed to instantiate YunOSAudioFrameGenerator";
  uint32_t capacity = 4096;
  uint8_t buffer [capacity];
  //Open file for writing
  std::ofstream yStream("test.wav", std::ofstream::binary);
  ASSERT_TRUE(yStream.is_open()) << "Failed to open file for writing";
  //clock ticks per 10ms
  clock_t frame_time = CLOCKS_PER_SEC / 100;
  //record a short clip
  int i = 0;
  clock_t last_update = std::clock();
  //10 seconds of audio
  while (i<1000) {
    if ((std::clock() - last_update) > frame_time) {
      uint32_t bytes_read = audio_frame_generator->GenerateFramesForNext10Ms(buffer, capacity);
      ASSERT_EQ(bytes_read, capacity) << "YunOSAudioFrameGenerator: Bytes read did not equal capacity";
      //write buffer to file
      yStream.write((char*)buffer, capacity);
      last_update = std::clock();
      i++;
    }
  }
  yStream.close();
}

//Get the sample rate in Hz
TEST(YunOSAudioFrameGeneratorTest, GetSampleRate) {
  std::unique_ptr<YunOSAudioFrameGenerator> audio_frame_generator(YunOSAudioFrameGenerator::Create());
  ASSERT_TRUE(audio_frame_generator) << "Failed to instantiate YunOSAudioFrameGenerator";
  int rate = audio_frame_generator->GetSampleRate();
  ASSERT_GT(rate, 0) << "YunOSAudioFrameGenerator: No sample rate found";
}

//Get the number of audio channels
TEST(YunOSAudioFrameGeneratorTest, GetChannelNumber) {
  std::unique_ptr<YunOSAudioFrameGenerator> audio_frame_generator(YunOSAudioFrameGenerator::Create());
  ASSERT_TRUE(audio_frame_generator) << "Failed to instantiate YunOSAudioFrameGenerator";
  int channel_number = audio_frame_generator->GetChannelNumber();
  ASSERT_GT(channel_number, 0) << "YunOSAudioFrameGenerator: No audio channels found";
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
