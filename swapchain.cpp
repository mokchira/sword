#include "swapchain.hpp"
#include "window.hpp"

Swapchain::Swapchain(const Context& context, const Window& window) :
	context(context),
	window(window)
{
	createSurface();
	//this will pass the surface to the context in order to select
	//an appropriate queue. currently the context will just print out
	//queue information, and we manually select the queue in another 
	//function. but this keeps validation layers happy.
	setQueueFamilyIndex(); 
	setSurfaceCapabilities();
	setSwapExtent();
	setFormat();
	setPresentMode();
	setImageCount();
	createSwapchain();
	setImages();
	createImageViews();
}

Swapchain::~Swapchain()
{	
	destroyImageViews();
	context.device.destroySwapchainKHR(swapchain);
	context.instance.destroySurfaceKHR(surface);
}

void Swapchain::createSurface()
{
	vk::XcbSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.connection  = window.connection;
	surfaceCreateInfo.window = window.window;
	surface = context.instance.createXcbSurfaceKHR(surfaceCreateInfo);
}

void Swapchain::setQueueFamilyIndex()
{
	context.pickQueueFamilyIndex(surface);
}

void Swapchain::setSurfaceCapabilities()
{
	surfCaps = context.physicalDevice.getSurfaceCapabilitiesKHR(surface);
}

void Swapchain::setFormat()
{
	//make this call simply to make validation layers happy for now
	context.physicalDevice.getSurfaceFormatsKHR(surface);
	colorFormat = vk::Format::eB8G8R8A8Unorm; //availabe
	colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear; //available
}

void Swapchain::setSwapExtent(int width, int height)
{
	if (surfCaps.currentExtent == -1) //forget what this means
	{
		swapchainExtent.width = window.size[0];
		swapchainExtent.height = window.size[1];
	}
	if (width!=0 && height!=0)
	{
	}
	else
	{
		swapchainExtent.width = surfCaps.currentExtent.width;
		swapchainExtent.height = surfCaps.currentExtent.height;
	}
}

void Swapchain::setPresentMode()
{
	//called just to make validation layers happy
	context.physicalDevice.getSurfacePresentModesKHR(surface);
	presentMode = vk::PresentModeKHR::eImmediate;
}

void Swapchain::setImageCount(int count)
{
	imageCount = count;
}

void Swapchain::createSwapchain()
{
	vk::SwapchainCreateInfoKHR createInfo;

	createInfo.setSurface(surface);
	createInfo.setImageExtent(swapchainExtent);
	createInfo.setImageFormat(colorFormat);
	createInfo.setImageColorSpace(colorSpace);
	createInfo.setPresentMode(presentMode);
	createInfo.setMinImageCount(imageCount);
	createInfo.setImageArrayLayers(1); //more than 1 for VR applications
	//we will be drawing directly to the image
	//as opposed to transfering data to it
	createInfo.setImageUsage(
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferDst);
	//this is so that each queue has exclusive 
	//ownership of an image at a time
	createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
	createInfo.setPreTransform(surfCaps.currentTransform);
	createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
	//means we dont care about the color of obscured pixels
	createInfo.setClipped(true); 
	swapchain = context.device.createSwapchainKHR(createInfo);
	std::cout << "Swapchain created!" << std::endl;
	swapchainCreated = true;
}

void Swapchain::setImages()
{
	if (!swapchainCreated) {
		std::cout << "Attempted to get images"
		       << " from nonexistant swapchain"
	       	       << std::endl;
	}
	images = context.device.getSwapchainImagesKHR(swapchain);
}

void Swapchain::createImageViews()
{
	vk::ImageViewCreateInfo createInfo;
	createInfo.setViewType(vk::ImageViewType::e2D);
	createInfo.setFormat(colorFormat);
	
	vk::ImageSubresourceRange subResRange;
	subResRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subResRange.setLayerCount(1);
	subResRange.setLevelCount(1);

	createInfo.setSubresourceRange(subResRange);

	for (const auto image : images) {
		createInfo.setImage(image);
		imageViews.push_back(context.device.createImageView(createInfo));
	}
}

uint32_t Swapchain::acquireNextImage(const vk::Semaphore& semaphore)
{
	auto result = context.device.acquireNextImageKHR(
			swapchain,
			UINT64_MAX, //so it will wait forever
			semaphore, //will be signalled when we can do something with this
			vk::Fence()); //empty fence
	if (result.result != vk::Result::eSuccess) 
	{
		std::cerr << "Invalid acquire result: " << vk::to_string(result.result);
		throw std::error_code(result.result);
	}

	currentIndex = result.value;

	return currentIndex;
}

void Swapchain::destroyImageViews()
{
	for (auto imageView : imageViews) 
	{
		context.device.destroyImageView(imageView);
	}
}
