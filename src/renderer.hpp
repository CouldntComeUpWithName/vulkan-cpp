#pragma once
#include <vulkan.h>

class Renderer {
public:
  auto instance() -> VkInstance {
    return instance_;
  }

private:
  VkInstance instance_;
};