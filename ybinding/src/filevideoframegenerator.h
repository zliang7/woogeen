#ifndef FILEVIDEOFRAMEGENERATOR_H
#define FILEVIDEOFRAMEGENERATOR_H

#include <stdio.h>
#include <woogeen/base/framegeneratorinterface.h>

namespace internal {

/// This class generate video frames from input file.
class FileVideoFrameGenerator
    : public woogeen::base::VideoFrameGeneratorInterface {
 public:
  static FileVideoFrameGenerator* Create(const std::string& input_filename);
  FileVideoFrameGenerator(int width, int height, int fps, FILE* fd);
  virtual ~FileVideoFrameGenerator();

  uint32_t GetNextFrameSize();
  uint32_t GenerateNextFrame(uint8_t* frame_buffer, const uint32_t capacity);
  int GetHeight();
  int GetWidth();
  int GetFps();
  woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec GetType();

 private:
  int width_;
  int height_;
  int fps_;
  uint32_t frame_data_size_;
  woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec type_;
  FILE * fd_;
};

}

#endif  // FILEVIDEOFRAMEGENERATOR_H
