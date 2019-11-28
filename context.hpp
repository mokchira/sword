#ifndef CONTEXT_H
#define CONTEXT_H

#define VK_USE_PLATFORM_XCB_KHR

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include "commander.hpp"
#include "mem.hpp"

class Context
{
public:
	Context ();

	virtual ~Context ();

	void deviceReport();

	vk::Device getDevice();

	uint32_t pickQueueFamilyIndex(vk::SurfaceKHR surface) const;

	vk::PhysicalDevice physicalDevice;

	vk::Instance instance;

	vk::Device device;

	bool enableValidation = true;

	uint32_t getGraphicsQueueFamilyIndex() const;
	
	void setQueue();

	BufferBlock* getVertexBlock(uint32_t size);

	BufferBlock* getIndexBlock(uint32_t size);

	void printInstanceExtensionProperties();

	void printDeviceMemoryHeapInfo();

	void printDeviceMemoryTypeInfo();

	void printDeviceInfo();

	void printDeviceFeatures();

	void printDeviceQueueFamilyInfo();

	void printDeviceExtensionProperties();
	
	void printAvailableDevices();

	void checkLayers(std::vector<const char*>);

	vk::Queue queue;
	std::unique_ptr<Commander> pCommander;
	std::unique_ptr<MemoryManager> pMemory;
	//physical device properties
	vk::PhysicalDeviceProperties physicalDeviceProperties;

private:
	vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vk::PhysicalDeviceFeatures physicalDeviceFeatures;
	std::vector<vk::ExtensionProperties> instanceExtensionProperties;
	std::vector<vk::QueueFamilyProperties> queueFamilies;
	std::vector<vk::ExtensionProperties> deviceExtensionProperties;
	VkDebugUtilsMessengerEXT debugMessenger;
	vk::DispatchLoaderDynamic dispatcher;

	void createContext();

	void setupDebugMessenger2();

	void destroyDebugMessenger();

	void destroyContext();

	void createCommander();

	void checkInstanceExtensionProperties();

	void createInstance();

	void createPhysicalDevice();

	void createDevice();

	void getSurfaceCapabilities();

	void setDeviceExtensions(vk::DeviceCreateInfo&);

	void setValidationLayers();

	void setupDebugMessenger();

};

#endif /* ifndef CONTEXT_H */
