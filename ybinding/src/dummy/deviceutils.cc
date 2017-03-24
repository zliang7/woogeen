/*
 * Intel License
 */

#include <woogeen/base/deviceutils.h>

namespace woogeen {
namespace base {
std::vector<std::string> DeviceUtils::VideoCapturerIds() {
  std::vector<std::string> device_ids = { "camera1", "camera2" };
  return device_ids;
}

std::vector<Resolution> DeviceUtils::VideoCapturerSupportedResolutions(
    const std::string& id) {
  std::vector<Resolution> resolutions;
  if (id == "camera1") {
      resolutions.push_back(Resolution(640, 480));
  } else if (id == "camera2") {
      resolutions.push_back(Resolution(640, 480));
      resolutions.push_back(Resolution(1280, 720));
  }
  return resolutions;
}

}
}
