/*
 * Vulkan Windowed Program
 *
 * Copyright (C) 2016 Valve Corporation
 * Copyright (C) 2016 LunarG, Inc.
 * Modified By JackMyth
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
Vulkan C++ Empty Project Template
Modified By JackMyth
*/

#pragma comment(lib,"glfw3.lib")
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <vector>
#include <set>
#include <fstream>

struct QueueFamilyIndices 
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() 
	{
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
} SCSdetails;

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const int WIDTH = 800;
const int HEIGHT = 600;

void InitVulkan();
bool IsDeviceSuitable(VkPhysicalDevice& DeviceToTest);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
void CreateGraphicsPipeline();
void ClearUp();

GLFWwindow* window;
VkSurfaceKHR surface;
VkDevice device;
VkInstance MainVkInstance;
QueueFamilyIndices queueindex;
VkSwapchainKHR SwapChain;
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "TestForVulkan", nullptr, nullptr);
	InitVulkan();
	while (!glfwWindowShouldClose(window))
		glfwPollEvents();
	ClearUp();
    return 0;
}

void InitVulkan()
{
	VkApplicationInfo TmpVkApplicationInfo={};
	TmpVkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	TmpVkApplicationInfo.apiVersion = VK_API_VERSION_1_0;
	TmpVkApplicationInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
	TmpVkApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	TmpVkApplicationInfo.pEngineName = "Test Vulkan Engine";
	TmpVkApplicationInfo.pNext = nullptr;
	VkInstanceCreateInfo TmpVkCreateInfo={};
	TmpVkCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	TmpVkCreateInfo.pApplicationInfo = &TmpVkApplicationInfo;
	TmpVkCreateInfo.enabledLayerCount = 0;
	unsigned int ExtensionsCount = 0;
	TmpVkCreateInfo.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(&ExtensionsCount);
	TmpVkCreateInfo.enabledExtensionCount = ExtensionsCount;
	VkResult tmpVkResult = vkCreateInstance(&TmpVkCreateInfo, nullptr, &MainVkInstance);
	if (tmpVkResult!=VK_SUCCESS)
	{
		MessageBox(nullptr, TEXT("Create Vulkan Instance Failed!"), TEXT("Error"), MB_OK);
		exit(-1);
	}
	glfwCreateWindowSurface(MainVkInstance, window, nullptr, &surface);
	VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	unsigned int DeviceCount = 0;
	vkEnumeratePhysicalDevices(MainVkInstance, &DeviceCount, nullptr);
	if (!DeviceCount)
	{
		MessageBox(nullptr, TEXT("No Device Support Vulkan,Abort!"), TEXT("Error"), MB_OK);
		exit(-1);
	}
	std::vector<VkPhysicalDevice> PhysicalDeviceCollection(DeviceCount);
	vkEnumeratePhysicalDevices(MainVkInstance, &DeviceCount, PhysicalDeviceCollection.data());
	for(auto &K:PhysicalDeviceCollection)
	{
		if (IsDeviceSuitable(K))
		{
			PhysicalDevice = K;
			break;
		}
	}
	queueindex = findQueueFamilies(PhysicalDevice);
	if (PhysicalDevice == VK_NULL_HANDLE)
	{
		MessageBox(nullptr, TEXT("No Suitable Device,Abort!"), TEXT("Error"), MB_OK);
		exit(-1);
	}
	std::vector<VkDeviceQueueCreateInfo> DeviceQueueCreateInfoCollection;
	VkDeviceQueueCreateInfo DeviceQueueCreateInfo = {};
	DeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	DeviceQueueCreateInfo.pNext = nullptr;
	DeviceQueueCreateInfo.queueFamilyIndex = queueindex.graphicsFamily;
	DeviceQueueCreateInfo.queueCount = 1;
	DeviceQueueCreateInfo.queueFamilyIndex = queueindex.graphicsFamily;
	DeviceQueueCreateInfoCollection.push_back(DeviceQueueCreateInfo);
	DeviceQueueCreateInfo.queueFamilyIndex = queueindex.presentFamily;
	DeviceQueueCreateInfoCollection.push_back(DeviceQueueCreateInfo);
	VkPhysicalDeviceFeatures PhysicalDeviceFeatures = {};
	VkDeviceCreateInfo DeviceCreateInfo = {};
	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.queueCreateInfoCount = (unsigned int)DeviceQueueCreateInfoCollection.size();
	DeviceCreateInfo.pQueueCreateInfos = DeviceQueueCreateInfoCollection.data();
	DeviceCreateInfo.pEnabledFeatures = &PhysicalDeviceFeatures;
	DeviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	DeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	if (vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &device) != VK_SUCCESS)
	{
		MessageBox(nullptr, TEXT("Logic Device Create Failed!"), TEXT("Error"), MB_OK);
		exit(-1);
	}
	VkQueue GraphicQueue;
	vkGetDeviceQueue(device, queueindex.graphicsFamily, 0, &GraphicQueue);
	VkQueue presentQueue;
	vkGetDeviceQueue(device, queueindex.presentFamily, 0, &presentQueue);
	SCSdetails = querySwapChainSupport(PhysicalDevice);
	auto SurfaceFormat = chooseSwapSurfaceFormat(SCSdetails.formats);
	auto PresentMode = chooseSwapPresentMode(SCSdetails.presentModes);
	auto SwapExtent = chooseSwapExtent(SCSdetails.capabilities);
	int imageCount;
	if (!SCSdetails.capabilities.maxImageCount)
		imageCount = SCSdetails.capabilities.minImageCount + 5;
	else
		imageCount = std::min(SCSdetails.capabilities.minImageCount + 5, SCSdetails.capabilities.maxImageCount);
	VkSwapchainCreateInfoKHR SwapchainCreateInfo = {};
	SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainCreateInfo.imageArrayLayers = 1;
	SwapchainCreateInfo.imageExtent = SwapExtent;
	SwapchainCreateInfo.presentMode = PresentMode;
	SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
	SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
	SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	unsigned int QueueCollection[] = { queueindex.graphicsFamily,queueindex.presentFamily };
	if (queueindex.graphicsFamily != queueindex.presentFamily)
	{
		SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		SwapchainCreateInfo.queueFamilyIndexCount = 2;
		SwapchainCreateInfo.pQueueFamilyIndices = QueueCollection;
	}
	else
		SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainCreateInfo.clipped = VK_TRUE;
	SwapchainCreateInfo.oldSwapchain = nullptr;
	SwapchainCreateInfo.surface = surface;
	if (vkCreateSwapchainKHR(device, &SwapchainCreateInfo, nullptr, &SwapChain) != VK_SUCCESS)
	{
		MessageBox(nullptr, TEXT("SwapChain Create Failed!"), TEXT("Error"), MB_OK);
		exit(-1);
	}
	unsigned int SwapChainImagesCount;
	vkGetSwapchainImagesKHR(device, SwapChain, &SwapChainImagesCount, nullptr);
	std::vector<VkImage> SwapChainImages(SwapChainImagesCount);
	vkGetSwapchainImagesKHR(device, SwapChain, &SwapChainImagesCount, SwapChainImages.data());
	std::vector<VkImageView> SwapChainImageViews(SwapChainImagesCount);
	for (unsigned int i=0;i<SwapChainImagesCount;i++)
	{
		VkImageViewCreateInfo tmpImageViewCreateInfo = {};
		tmpImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		tmpImageViewCreateInfo.image = SwapChainImages[i];
		tmpImageViewCreateInfo.format = SurfaceFormat.format;
		tmpImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		tmpImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		tmpImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		tmpImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		tmpImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		tmpImageViewCreateInfo.subresourceRange.layerCount = 1;
		tmpImageViewCreateInfo.subresourceRange.levelCount = 1;
		vkCreateImageView(device, &tmpImageViewCreateInfo, nullptr, &SwapChainImageViews[i]);
	}

	return;
}

bool IsDeviceSuitable(VkPhysicalDevice& DeviceToTest)
{
	if (!checkDeviceExtensionSupport(DeviceToTest))
		return false;
	SwapChainSupportDetails details;
	details = querySwapChainSupport(DeviceToTest);
	if (details.formats.empty() || details.presentModes.empty())
		return false;
	return true;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) 
{
	uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}


SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) 
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) 
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats) 
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) 
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) 
		{
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		return capabilities.currentExtent;
	}
	else 
	{
		VkExtent2D actualExtent = { WIDTH, HEIGHT };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}
		VkBool32 isSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &isSupport);
		if (queueFamily.queueCount > 0 && isSupport)
		{
			indices.presentFamily = i;
		}
		if (indices.isComplete())
		{
			break;
		}
		i++;
	}
	return indices;
}

static std::vector<char> readFile(const std::string& filename) 
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) 
	{
		throw std::runtime_error("failed to open file!");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

void CreateGraphicsPipeline()
{
	auto VertexShaderCode = readFile("Shaders/VertexShader.spv");
	auto PixelShaderCode = readFile("Shaders/PixelShader.spv");
	VkShaderModule vertShaderModule,fragShaderModule;
	vertShaderModule = createShaderModule(VertexShaderCode);
	fragShaderModule = createShaderModule(PixelShaderCode);


	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

VkShaderModule createShaderModule(const std::vector<char>& code) 
{
	VkShaderModuleCreateInfo ShaderCreateInfo = {};
	ShaderCreateInfo.codeSize = code.size();
	ShaderCreateInfo.pCode = (unsigned int*)code.data();
	ShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	VkShaderModule ShaderModule;
	if (vkCreateShaderModule(device,&ShaderCreateInfo,nullptr,&ShaderModule)!=VK_SUCCESS)
		throw std::runtime_error("Create Vertex Shader Failed");
	return ShaderModule;
}

void ClearUp()
{
	vkDestroySwapchainKHR(device, SwapChain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(MainVkInstance, surface, nullptr);
	vkDestroyInstance(MainVkInstance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}
