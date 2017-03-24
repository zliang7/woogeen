#include <iostream>
#include "filevideoframegenerator.h"

namespace internal {

FileVideoFrameGenerator::FileVideoFrameGenerator(int width, int height, int fps, FILE* fd)
  : width_(width), height_(height), fps_(fps),
    type_(woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec::I420), fd_(fd) {
  int size = width_ * height_;
  int qsize = size / 4;
  frame_data_size_ = size + 2 * qsize;
}

FileVideoFrameGenerator::~FileVideoFrameGenerator() {
  fclose(fd_);
}

FileVideoFrameGenerator* FileVideoFrameGenerator::Create(
    const std::string& input_filename) {
  FILE* fd = fopen(input_filename.c_str(), "r");
  if(!fd) {
    std::cout << "failed to open the source.yuv." << std::endl;
    return nullptr;
  } else {
    std::cout << "sucessfully open the source.yuv." << std::endl;
  }

  auto p1 = input_filename.rfind('.');
  if (p1 == std::string::npos || input_filename.substr(p1 + 1) != "yuv")
      return nullptr;

  int fps;
  auto p2 = input_filename.rfind('.', p1 - 1);
  if (p2 == std::string::npos)  return nullptr;
  std::string str = input_filename.substr(p2 + 1, p1 - p2);
  try {
    fps = std::stoi(str, &p1);
  } catch(...) {
      return nullptr;
  }

  int width, height;
  if (str[p1] != '.') {
    fps = 24;
  } else {
    p1 = input_filename.rfind('.', p2 - 1);
    if (p1 == std::string::npos)  return nullptr;
    str = input_filename.substr(p1 + 1, p2 - p1);
  }
  try {
    width = std::stoi(str, &p1);
    if (str[p1] != 'X' && str[p1] != 'x')  return nullptr;
    str = str.substr(p1 + 1);
    height = std::stoi(str, &p2);
  } catch(...) {
    return nullptr;
  }
  if (str[p2] != '.')  return nullptr;

  return new FileVideoFrameGenerator(width, height, fps, fd);
}

uint32_t FileVideoFrameGenerator::GetNextFrameSize() {
  return frame_data_size_;
}

int FileVideoFrameGenerator::GetHeight() { return height_; }
int FileVideoFrameGenerator::GetWidth() { return width_; }
int FileVideoFrameGenerator::GetFps() { return fps_; }

woogeen::base::VideoFrameGeneratorInterface::VideoFrameCodec FileVideoFrameGenerator::GetType() { return type_; }

uint32_t FileVideoFrameGenerator::GenerateNextFrame(uint8_t* frame_buffer, const uint32_t capacity) {
  if (capacity < frame_data_size_) {
    return 0;
  }
  if (fread(frame_buffer, 1, frame_data_size_, fd_) != (size_t)frame_data_size_) {
    fseek(fd_, 0, SEEK_SET);
    fread(frame_buffer, 1, frame_data_size_, fd_);
  }
  return frame_data_size_;
}

}
