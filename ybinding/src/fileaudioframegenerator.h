#ifndef FILEAUDIOFRAMEGENERATOR_H
#define FILEAUDIOFRAMEGENERATOR_H

#include <string>
#include <woogeen/base/framegeneratorinterface.h>

namespace internal {

/// This class generate audio frames from input file.
class FileAudioFrameGenerator
    : public woogeen::base::AudioFrameGeneratorInterface {
 public:
  static FileAudioFrameGenerator* Create(const std::string& filename);

  virtual ~FileAudioFrameGenerator();
  uint32_t GenerateFramesForNext10Ms(uint8_t* frame_buffer, uint32_t capacity) override;
  int GetSampleRate() override {
      return sample_rate_;
  }
  int GetChannelNumber() override {
      return channel_number_;
  }

private:
    explicit FileAudioFrameGenerator(int file);
    int file_;
    off_t data_beginning_;
    int sample_rate_;
    int sample_size_;
    int channel_number_;
    uint32_t minimal_buffer_size_;
};

}

#endif  // FILEAUDIOFRAMEGENERATOR_H
