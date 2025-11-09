#include "def.hpp"
#include "logs.hpp"
#include "validation_layers.hpp"

struct Window {
  GLFWwindow* handle;
  std::string title;
  u32 width;
  u32 height;
  bool vsync;
  bool windowed;
};

struct QueueFamilyIndices {
  std::optional<u32> graphics_family;
  std::optional<u32> present_family;

  bool is_complete() const noexcept {
    return graphics_family.has_value() && present_family.has_value();
  }
};

static auto find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) -> QueueFamilyIndices {
  QueueFamilyIndices indices;

  u32 queue_family_count{};
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
  
  for(u32 i = 0; const auto& queue_family : queue_families) 
  {
    VkBool32 present_supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_supported);
    
    if(present_supported)
    {
      indices.present_family = i;
    }

    if(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
      indices.graphics_family = i;
      break;
    }
    i++;
  }
  return indices;
}

class App {
public:
  App() = default;
  
  ~App() {
    cleanup();
  }
  
  void init() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // ::glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    u32 extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    
    std::vector<VkExtensionProperties> extensions;
    extensions.resize(extension_count);

    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    u32 counter{};
    for(const VkExtensionProperties& extension : extensions) 
    {
      log_info("Found an extension: {}", extension.extensionName);
    }
    log_info("{} available extensions", extensions.size());
    
    main_window_ = create_window("VulkanApp", 1280, 720);
    
    log_info("Window {} created", main_window_.title);
    
    init_vk();
    create_surface();
    pick_physical_device();
    create_logical_device();
  }

  void run() {
    while(!::glfwWindowShouldClose(main_window().handle)) 
    {
      ::glfwPollEvents();
      ::glfwSwapBuffers(main_window().handle);
    }
  }
  
  void cleanup() {
    log_info("Shutting down application...");
    
    vk::destroy_debug_utils_messenger_ext(vk_instance_, debug_messenger_, nullptr);
    vkDestroySurfaceKHR(vk_instance_, surface_, nullptr);
    vkDestroyDevice(device_, nullptr);
    vkDestroyInstance(vk_instance_, nullptr);
    
    glfwDestroyWindow(main_window_.handle);
    glfwTerminate();
  }

  auto main_window() -> Window& {
    return main_window_;
  }

private:
  auto create_window(std::string_view name, u32 width, u32 height) -> Window {
    return Window {
      .handle   = glfwCreateWindow(i32(width), i32(height), name.data(), nullptr, nullptr),
      .title    = name.data(),
      .width    = width,
      .height   = height,
      .vsync    = true,
      .windowed = true
    };
  }

  void init_vk() {
    log_info("Initializing Vulkan, Vulkan Version: 1.0.0");
    
    layers_ = std::vector<const char*>{
      "VK_LAYER_KHRONOS_validation"
    };
    
    if(!vk::check_validation_layer_support(layers_)) 
    {
      std::terminate();  
    }

    VkApplicationInfo app_info {
      .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName   = main_window().title.c_str(),
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName        = "No Engine",
      .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion         = VK_API_VERSION_1_0,
    };
    
    VkInstanceCreateInfo create_info {
      .sType             = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo  = &app_info,
      .enabledLayerCount = (u32)layers_.size(),
      .ppEnabledLayerNames = layers_.data()
    };

    std::vector extensions = vk::required_extensions();

    create_info.enabledExtensionCount   = (u32)extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT dbg_create_info {
      .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
                         | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
                         | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = &vk::debug_callback
    };
    
    create_info.pNext = &dbg_create_info;
    
    VkResult result = ::vkCreateInstance(&create_info, nullptr, &vk_instance_);
    
    if (::vkCreateInstance(&create_info, nullptr, &vk_instance_) != VK_SUCCESS)
    {
      log_fatal("Failed to create Vulkan Instance");
    }

    if (vk::create_debug_utils_messenger_ext(vk_instance_, dbg_create_info, nullptr, &debug_messenger_) != VK_SUCCESS) 
    {
      log_fatal("failed to set up debug messenger!");
    }
  }

  void create_surface() {
    VkWin32SurfaceCreateInfoKHR create_info {
      .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = GetModuleHandle(nullptr), 
      .hwnd      = glfwGetWin32Window(main_window_.handle),
    };

    if(vkCreateWin32SurfaceKHR(vk_instance_, &create_info, nullptr, &surface_) != VK_SUCCESS) 
    {
      log_fatal("Failed to create window surface!");
    }
    log_info("Window surface created for {}", main_window_.title);
  }

  void pick_physical_device() {
    u32 device_count{};
    vkEnumeratePhysicalDevices(vk_instance_, &device_count, nullptr);
    
    assert(device_count != 0);
    
    if(device_count == 0) 
    {
      log_fatal("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(vk_instance_, &device_count, devices.data());

    for(VkPhysicalDevice device : devices)
    {
      if(is_device_suitable(device)) 
      {
        phys_device_ = device;
        break;
      }
      else 
      {
        log_warn("Found device is not suitable");
      }
    }
    
    if(phys_device_ == VK_NULL_HANDLE) 
    {
      log_fatal("Failed to find a suitable GPU!");
    }
  }

  bool is_device_suitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;
    
    vkGetPhysicalDeviceProperties(device, &props);
    vkGetPhysicalDeviceFeatures(device, &features);
    
    log_info("Found Physical Device: {}", props.deviceName);
    
    QueueFamilyIndices indices = find_queue_families(device, surface_);

    return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 
      && features.geometryShader && indices.graphics_family;
  }

  void create_logical_device() {
    QueueFamilyIndices indices = find_queue_families(phys_device_, surface_);

    float queue_priority = 1.f;
    VkDeviceQueueCreateInfo queue_create_info {
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = indices.graphics_family.value(),
      .queueCount       = 1u,
      .pQueuePriorities = &queue_priority
    };
    
    VkPhysicalDeviceFeatures features {};
    
    VkDeviceCreateInfo create_info{
      .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount    = 1u,
      .pQueueCreateInfos       = &queue_create_info,
      .enabledLayerCount       = u32(layers_.size()),
      .ppEnabledLayerNames     = layers_.data(),
      .pEnabledFeatures        = &features,
    };
    
    if(vkCreateDevice(phys_device_, &create_info, nullptr, &device_) != VK_SUCCESS)
    {
      log_fatal("Failed to create logical device!");
    }
    log_info("Logical device created");

    vkGetDeviceQueue(device_, indices.graphics_family.value(), 0, &vk_queue_);
  }

private:
  VkInstance vk_instance_ = {};
  VkPhysicalDevice phys_device_ = {};
  VkDebugUtilsMessengerEXT debug_messenger_ = {};
  VkDevice device_ = {};
  VkQueue vk_queue_ = {};
  VkQueue vk_present_queue_ = {};
  VkSurfaceKHR surface_ = {};
  std::vector<const char*> layers_;
  Window main_window_;
};

auto main() -> int {
  log_info("Starting up...");

  App app;
  app.init();
  app.run();
}