#include "validation_layers.hpp"
#include "logs.hpp"
#include "utils.hpp"
#include "def.hpp"

auto vk::required_extensions() -> std::vector<const char*>
{
  u32 glfw_extension_count{};
  const char** glfw_extensions = nullptr;

  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
  
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  
  return extensions;
}

bool vk::check_validation_layer_support(std::span<const char *> layers)
{
  u32 layer_count{};
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  // NOTE: const char* -> std::string_view conversion can be costly, 
  // but in this case it doesn't matter coz layers.size() is small
  for(const std::string_view name : layers)
  {
    bool found = false;
    for(const auto& props : available_layers) 
    {
      if(name == props.layerName)
      {
        found = true;
        break;
      }
    }
    
    if(!found)
    {
      log_error("Requested layer {} is unavailable", name);
      return false;
    }
  }
  
  return true;
}

VKAPI_ATTR auto VKAPI_CALL vk::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, 
  const VkDebugUtilsMessengerCallbackDataEXT *data, void *user_data) -> VkBool32
{
  switch (severity) {
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
    log_warn("Vulkan: {}", data->pMessage);
    break;
  }
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
    log_error("Vulkan: {}", data->pMessage);
    break;
  }
  default: 
    log_info("Vulkan: {}", data->pMessage);
  }
  
  return VK_FALSE;

}

auto vk::create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT& pCreateInfo, 
  const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) -> VkResult 
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, STR(vkCreateDebugUtilsMessengerEXT));
  
  if (func != nullptr)
  {
    return func(instance, &pCreateInfo, pAllocator, pDebugMessenger);
  } 
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void vk::destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, STR(vkDestroyDebugUtilsMessengerEXT));
  
  if (func)
  {
    return func(instance, debug_messenger, allocator);
  } 
  else
  {
    log_fatal("Vulkan Extension not present");
  }
}
