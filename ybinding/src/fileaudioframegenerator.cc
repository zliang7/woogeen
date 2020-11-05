#include <fcntl.h>
#include <unistd.h>

#include "fileaudioframegenerator.h"

namespace internal {

FileAudioFrameGenerator::FileAudioFrameGenerator(int file) :
    file_(file), data_beginning_(0),
    sample_rate_(44100), sample_size_(16), channel_number_(2) {
    struct FormatHeader {
        uint32_t chunkID;
        uint32_t chunkSize;
        uint16_t audioFormat;
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
    } format;

    lseek(file_, 12, SEEK_SET);
    if (read(file_, &format, sizeof(format)) == sizeof(format) &&
        format.chunkID == 0x20746d66 && format.audioFormat == 1 &&
        format.chunkSize >= sizeof(format) - 8) {
        sample_rate_ = format.sampleRate;
        sample_size_ = format.bitsPerSample;
        channel_number_ = format.numChannels;
        data_beginning_ =
            lseek(file_, 8 + format.chunkSize - sizeof(format) + 8, SEEK_CUR);
    }
    minimal_buffer_size_ =
        sample_rate_ * sample_size_ * channel_number_ / 8 / 100;
}

FileAudioFrameGenerator::~FileAudioFrameGenerator() {
    close(file_);
}

FileAudioFrameGenerator* FileAudioFrameGenerator::Create(
    const std::string& filename) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (!fd) {
        //std::cout << "Failed to open audio input file." << std::endl;
        return nullptr;
    }

    struct RiffHeader {
        uint32_t chunkID;
        uint32_t chunkSize;
        uint32_t format;
    } riff;
    if (read(fd, &riff, sizeof(riff)) == sizeof(riff) &&
        riff.chunkID == 0x46464952 && riff.format == 0x45564157) {
        // TODO: check the file size
        auto generator = new FileAudioFrameGenerator(fd);
        if (generator)  return generator;
    }

    close(fd);
    return nullptr;
}

uint32_t FileAudioFrameGenerator::GenerateFramesForNext10Ms(uint8_t* frame_buffer, const uint32_t capacity) {
    if (capacity < minimal_buffer_size_) {
        //std::cout << "The capacity is too small" << std::endl;
        return 0;
    }

    size_t remain = capacity;
    uint8_t* buffer = frame_buffer;
    do {
        size_t len = read(file_, buffer, remain);
        if (len < remain) {
            if (len >= 0) {
                //std::cout << "Reach the end of input file." << std::endl;
                lseek(file_, data_beginning_, SEEK_SET);
            } else {
                //std::cout << "Error while reading file" << std::endl;
                return 0;
            }
        }
        remain -= len;
        buffer += len;
    } while(remain);
    return capacity;
}

}
