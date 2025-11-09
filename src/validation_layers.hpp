#pragma once
#include "vulkan.hpp"

namespace vk {
  
  // this function probably doesn't belong here
  auto required_extensions() -> std::vector<const char*>;

  bool check_validation_layer_support(std::span<const char*> layers);
  
  auto create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT& pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) -> VkResult;
  
  void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator);
  
  auto VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, 
    const VkDebugUtilsMessengerCallbackDataEXT* data, void* user_data) -> VkBool32;
}