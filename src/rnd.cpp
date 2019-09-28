#include "rnd.hpp"

#include <iostream>
//#include <fstream>

namespace {

	void print_extension_properties_f(){
		auto properties = vk::enumerateInstanceExtensionProperties();
		for(auto& prop : properties)
			std::cerr << prop.extensionName << std::endl;
	}

	bool instance_extensions_is_support_f(
		std::vector<const char *> &instance_extension_names){

		auto properties = vk::enumerateInstanceExtensionProperties();

		uint32_t hits = 0;
		for(auto& prop : properties){
			for(auto& extension : instance_extension_names){
				if(std::string(extension) == std::string(prop.extensionName)){
					++hits;
					break;
				}
			}
		}
		return hits == instance_extension_names.size();
	};
}

void renderer::set_scene(scene_t *scene){
//	this->scene_ptr = scene;
	this->pipeline.load_scene(this->device, this->physical_device, *scene);
}

void renderer::init_window(){
	uint32_t width = 800;
	uint32_t height = 600;

	this->hInstance = GetModuleHandleA(NULL);

	memset(&this->wcex, 0, sizeof(WNDCLASSEX));

	this->wcex.cbSize = sizeof(WNDCLASSEX);
	this->wcex.style = CS_HREDRAW | CS_VREDRAW;
	this->wcex.lpfnWndProc = WndProc;
	this->wcex.hInstance = this->hInstance;
	this->wcex.hIcon = LoadIcon(this->hInstance, IDI_APPLICATION);
	this->wcex.hIconSm = LoadIcon(this->wcex.hInstance, IDI_APPLICATION);
	this->wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	this->wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	this->wcex.lpszClassName = TEXT("MyWndClass");

	if(!RegisterClassEx(&this->wcex)){
		fprintf(stderr, "ERROR: Register class failed.\n");
		exit(-1);
	}

	this->hWnd = CreateWindow(
		TEXT("MyWndClass"),
		TEXT("My Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		NULL,
		NULL,
		this->hInstance,
		NULL);

	if(!this->hWnd){
		fprintf(stderr, "ERROR: Create window failed.\n");
		exit(-1);
	}
}

void renderer::init_instance(){
	auto app_info = vk::ApplicationInfo("Renderer")
		.setApiVersion(VK_API_VERSION_1_0);

	if(!instance_extensions_is_support_f(this->instance_extension_names))
		throw std::runtime_error("Instance extensions could not support.");

	const auto instance_info = vk::InstanceCreateInfo{}
		.setPApplicationInfo(&app_info)
		.setEnabledExtensionCount(instance_extension_names.size())
		.setPpEnabledExtensionNames(instance_extension_names.data());

	this->instance = vk::createInstance(instance_info);
}

vk::PhysicalDeviceFeatures operator&(vk::PhysicalDeviceFeatures const & lhs,
	vk::PhysicalDeviceFeatures const & rhs){

	return vk::PhysicalDeviceFeatures(
		  ( lhs.robustBufferAccess & rhs.robustBufferAccess )
		, ( lhs.fullDrawIndexUint32 & rhs.fullDrawIndexUint32 )
		, ( lhs.imageCubeArray & rhs.imageCubeArray )
		, ( lhs.independentBlend & rhs.independentBlend )
		, ( lhs.geometryShader & rhs.geometryShader )
		, ( lhs.tessellationShader & rhs.tessellationShader )
		, ( lhs.sampleRateShading & rhs.sampleRateShading )
		, ( lhs.dualSrcBlend & rhs.dualSrcBlend )
		, ( lhs.logicOp & rhs.logicOp )
		, ( lhs.multiDrawIndirect & rhs.multiDrawIndirect )
		, ( lhs.drawIndirectFirstInstance & rhs.drawIndirectFirstInstance )
		, ( lhs.depthClamp & rhs.depthClamp )
		, ( lhs.depthBiasClamp & rhs.depthBiasClamp )
		, ( lhs.fillModeNonSolid & rhs.fillModeNonSolid )
		, ( lhs.depthBounds & rhs.depthBounds )
		, ( lhs.wideLines & rhs.wideLines )
		, ( lhs.largePoints & rhs.largePoints )
		, ( lhs.alphaToOne & rhs.alphaToOne )
		, ( lhs.multiViewport & rhs.multiViewport )
		, ( lhs.samplerAnisotropy & rhs.samplerAnisotropy )
		, ( lhs.textureCompressionETC2 & rhs.textureCompressionETC2 )
		, ( lhs.textureCompressionASTC_LDR & rhs.textureCompressionASTC_LDR )
		, ( lhs.textureCompressionBC & rhs.textureCompressionBC )
		, ( lhs.occlusionQueryPrecise & rhs.occlusionQueryPrecise )
		, ( lhs.pipelineStatisticsQuery & rhs.pipelineStatisticsQuery )
		, ( lhs.vertexPipelineStoresAndAtomics & rhs.vertexPipelineStoresAndAtomics )
		, ( lhs.fragmentStoresAndAtomics & rhs.fragmentStoresAndAtomics )
		, ( lhs.shaderTessellationAndGeometryPointSize & rhs.shaderTessellationAndGeometryPointSize )
		, ( lhs.shaderImageGatherExtended & rhs.shaderImageGatherExtended )
		, ( lhs.shaderStorageImageExtendedFormats & rhs.shaderStorageImageExtendedFormats )
		, ( lhs.shaderStorageImageMultisample & rhs.shaderStorageImageMultisample )
		, ( lhs.shaderStorageImageReadWithoutFormat & rhs.shaderStorageImageReadWithoutFormat )
		, ( lhs.shaderStorageImageWriteWithoutFormat & rhs.shaderStorageImageWriteWithoutFormat )
		, ( lhs.shaderUniformBufferArrayDynamicIndexing & rhs.shaderUniformBufferArrayDynamicIndexing )
		, ( lhs.shaderSampledImageArrayDynamicIndexing & rhs.shaderSampledImageArrayDynamicIndexing )
		, ( lhs.shaderStorageBufferArrayDynamicIndexing & rhs.shaderStorageBufferArrayDynamicIndexing )
		, ( lhs.shaderStorageImageArrayDynamicIndexing & rhs.shaderStorageImageArrayDynamicIndexing )
		, ( lhs.shaderClipDistance & rhs.shaderClipDistance )
		, ( lhs.shaderCullDistance & rhs.shaderCullDistance )
		, ( lhs.shaderFloat64 & rhs.shaderFloat64 )
		, ( lhs.shaderInt64 & rhs.shaderInt64 )
		, ( lhs.shaderInt16 & rhs.shaderInt16 )
		, ( lhs.shaderResourceResidency & rhs.shaderResourceResidency )
		, ( lhs.shaderResourceMinLod & rhs.shaderResourceMinLod )
		, ( lhs.sparseBinding & rhs.sparseBinding )
		, ( lhs.sparseResidencyBuffer & rhs.sparseResidencyBuffer )
		, ( lhs.sparseResidencyImage2D & rhs.sparseResidencyImage2D )
		, ( lhs.sparseResidencyImage3D & rhs.sparseResidencyImage3D )
		, ( lhs.sparseResidency2Samples & rhs.sparseResidency2Samples )
		, ( lhs.sparseResidency4Samples & rhs.sparseResidency4Samples )
		, ( lhs.sparseResidency8Samples & rhs.sparseResidency8Samples )
		, ( lhs.sparseResidency16Samples & rhs.sparseResidency16Samples )
		, ( lhs.sparseResidencyAliased & rhs.sparseResidencyAliased )
		, ( lhs.variableMultisampleRate & rhs.variableMultisampleRate )
		, ( lhs.inheritedQueries & rhs.inheritedQueries )
	);
}

namespace{

	void print_physical_device_properties_f(vk::PhysicalDevice *physical_device){
		vk::PhysicalDeviceProperties prop = physical_device->getProperties();
		std::cerr << prop.deviceName << std::endl;

		vk::PhysicalDeviceMemoryProperties mem_prop = physical_device->getMemoryProperties();
		for(uint32_t i = 0; i < mem_prop.memoryTypeCount; ++i){
			std::cerr << "\tMemory Type " << i << " : "
				<< "heapIndex: " << mem_prop.memoryTypes[i].heapIndex << ' ';
	//		if(mem_prop.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)
	//			std::cerr << '\t' << vk::to_string(vk::MemoryPropertyFlagBits::eDeviceLocal) << std::endl;

			std::cerr << vk::to_string(mem_prop.memoryTypes[i].propertyFlags) << std::endl;
		}

		std::cerr << std::endl;

		for(uint32_t i = 0; i < mem_prop.memoryHeapCount; ++i){
			std::cerr << "\tHeap " << i << " : "
				<< mem_prop.memoryHeaps[i].size / 1024 / 1024 << ' '
 				<< to_string(mem_prop.memoryHeaps[i].flags) << std::endl;
		}

		std::cerr << std::endl;
	}

	void print_device_extension_names_f(vk::PhysicalDevice &physical_device){
		std::vector<vk::ExtensionProperties> extensions
			= physical_device.enumerateDeviceExtensionProperties();
		for(auto &ext : extensions){
			std::cerr << ext.extensionName << std::endl;
		}
	}

	[[nodiscard]] bool device_extensions_is_support_f(
		vk::PhysicalDevice &physical_device,
		std::vector<const char *> &device_extension_names){

/*		for(auto &extension_name : device_extension_names){
			physical_device.enumerateDeviceExtensionProperties(
				std::string(extension_name));
		}*/

		auto properties = physical_device.enumerateDeviceExtensionProperties();
		uint32_t hits = 0;
		for(auto& prop : properties){
			for(auto& extension : device_extension_names){
				if(std::string(extension) == std::string(prop.extensionName)){
					++hits;
					break;
				}
			}
		}
		return hits == device_extension_names.size();
	}

	bool physical_device_features_is_support_f(
		vk::PhysicalDevice &physical_device,
		vk::PhysicalDeviceFeatures &req_features){

		vk::PhysicalDeviceFeatures features = physical_device.getFeatures();
		return (req_features == (features & req_features)) ? true : false;
	}
}

void renderer::init_surface(){
#ifdef VK_USE_PLATFORM_WIN32_KHR
	vk::Win32SurfaceCreateInfoKHR surface_info(
		vk::Win32SurfaceCreateFlagsKHR(),
		this->hInstance,
		this->hWnd);

	this->surface = this->instance.createWin32SurfaceKHR(surface_info);
#endif /*VK_USE_PLATFORM_WIN32_KHR*/
}

void renderer::init_physical_device(){
	struct queue_family_indeces{
		uint32_t graphics = UINT32_MAX;
		uint32_t present = UINT32_MAX;
	};

	auto get_queue_indeces = [](vk::PhysicalDevice &physical_device,
		vk::SurfaceKHR &surface) -> queue_family_indeces {
		queue_family_indeces result{};
		std::vector<vk::QueueFamilyProperties> queue_family_props
			= physical_device.getQueueFamilyProperties();
		for(uint32_t index = 0; index < queue_family_props.size(); ++index){
			if(queue_family_props[index].queueFlags & vk::QueueFlagBits::eGraphics){
				if(result.graphics == UINT32_MAX)
					result.graphics = index;
			}

			if(physical_device.getSurfaceSupportKHR(index, surface)) {
				result.graphics = index;
				result.present = index;
				break;
			}
		}

		if(result.present == UINT32_MAX){
			for(uint32_t index = 0; index < queue_family_props.size(); ++index){
				if(physical_device.getSurfaceSupportKHR(index, surface)) {
					result.present = index;
					break;
				}
			}
		}
		return result;
	};

	std::vector<vk::PhysicalDevice> physical_devices
		= this->instance.enumeratePhysicalDevices();

	struct choosen_device_t{
		vk::PhysicalDevice physical_device;
		queue_family_indeces family_indeces;
		vk::DeviceSize available_vram = 0;
	} choosen_device{};
	for(auto &device : physical_devices){
		if(!(device_extensions_is_support_f(device, this->device_extension_names)
			&& physical_device_features_is_support_f(device, this->features)))
			continue;

		vk::PhysicalDeviceMemoryProperties mem_prop
			= device.getMemoryProperties();
		vk::DeviceSize available_vram = 0;
		for(uint32_t i = 0; i < mem_prop.memoryHeapCount; ++i){
			if(mem_prop.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal)
				available_vram += mem_prop.memoryHeaps[i].size;
		}
		queue_family_indeces family_indeces = get_queue_indeces(device, this->surface);

		if(choosen_device.available_vram < available_vram)
			choosen_device = choosen_device_t{device, family_indeces, available_vram};
	}

	this->physical_device = choosen_device.physical_device;
	this->graphics_queue_family_index = choosen_device.family_indeces.graphics;
	this->present_queue_family_index = choosen_device.family_indeces.present;

	if(!this->physical_device)
		throw std::runtime_error("physical_device is VK_NULL_HANDLE");
}

void renderer::init_device(){
	const float queue_priorities[1] = {0.0};
	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{
		{
			vk::DeviceQueueCreateFlags(),
			this->graphics_queue_family_index,
			1,
			queue_priorities
		}
	};

	if(this->graphics_queue_family_index != this->present_queue_family_index){
		queue_create_infos.emplace_back(
			vk::DeviceQueueCreateFlags(),
			this->present_queue_family_index,
			1,
			queue_priorities);
	}

	const vk::DeviceCreateInfo device_info(vk::DeviceCreateFlags(),
		queue_create_infos.size(), queue_create_infos.data(),
		0, nullptr,
		this->device_extension_names.size(), this->device_extension_names.data(),
		&this->features);
	this->device = this->physical_device.createDevice(device_info);
}

namespace {
	[[nodiscard]] vk::SurfaceFormatKHR get_surface_format_f(
		const vk::PhysicalDevice &physical_device,
		const vk::SurfaceKHR &surface,
		vk::SurfaceFormatKHR desired_surface_format){

		vk::SurfaceFormatKHR result_format;

		const std::vector<vk::SurfaceFormatKHR> supported_surface_formats
			= physical_device.getSurfaceFormatsKHR(surface);

		if( (supported_surface_formats.size() == 1) &&
			(supported_surface_formats[0].format == vk::Format::eUndefined) ) {

			result_format = supported_surface_formats[0];

		} else {
			bool hit = false;
			for(auto &surface_format : supported_surface_formats){
				if(	desired_surface_format == surface_format){

					result_format = surface_format;
					hit = true;
					break;
				}
			}

			if(hit == false){
				for(auto &surface_format : supported_surface_formats){
					if(desired_surface_format.format == surface_format.format){
						result_format = surface_format;

						std::cerr
							<< "Desired combination of format and colorspace is not supported."
							<< "Selecting other colorspace." << std::endl;
						hit = true;
						break;
					}
				}

				if(hit == false){
					result_format = supported_surface_formats[0];

					std::cerr
						<< "Desired format is not supported. Selecting available format"
						<< " - colorspace combination" << std::endl;
				}
			}
		}
		return result_format;
	}

	[[nodiscard]] vk::Extent2D get_swapchain_extent_f(
		const vk::SurfaceCapabilitiesKHR &surface_capabilities,
		vk::Extent2D window_size){

		vk::Extent2D swapchainExtent{};

		if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {

           swapchainExtent.width = window_size.width;
           swapchainExtent.height = window_size.height;

           if (swapchainExtent.width < surface_capabilities.minImageExtent.width) {
               swapchainExtent.width = surface_capabilities.minImageExtent.width;
           } else if (swapchainExtent.width > surface_capabilities.maxImageExtent.width) {
               swapchainExtent.width = surface_capabilities.maxImageExtent.width;
           }

           if (swapchainExtent.height < surface_capabilities.minImageExtent.height) {
               swapchainExtent.height = surface_capabilities.minImageExtent.height;
           } else if (swapchainExtent.height > surface_capabilities.maxImageExtent.height) {
               swapchainExtent.height = surface_capabilities.maxImageExtent.height;
           }
       } else {
           swapchainExtent = surface_capabilities.currentExtent;
       }
	   return swapchainExtent;
	}

	[[nodiscard]] uint32_t get_number_of_swapchain_images_f(
		const vk::SurfaceCapabilitiesKHR &surface_capabilities,
		uint32_t desired_number_images){

		uint32_t number_of_swapchain_images = desired_number_images;

		if(number_of_swapchain_images < surface_capabilities.minImageCount)
			number_of_swapchain_images = surface_capabilities.minImageCount;
		else if(number_of_swapchain_images > surface_capabilities.maxImageCount)
			number_of_swapchain_images = surface_capabilities.maxImageCount;

		return number_of_swapchain_images;
	}

	[[nodiscard]] vk::PresentModeKHR get_swapchain_present_mode_f(
		const vk::PhysicalDevice &physical_device,
		const vk::SurfaceKHR &surface,
		vk::PresentModeKHR desired_present_mode){

		vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;

		const std::vector<vk::PresentModeKHR> present_modes =
			physical_device.getSurfacePresentModesKHR(surface);

		for(auto &curr_mode : present_modes){
			if(curr_mode == desired_present_mode){
				present_mode = desired_present_mode;
				break;
			}
		}
		return present_mode;
	}

	[[nodiscard]] vk::SurfaceTransformFlagBitsKHR get_pre_transform_f(
		const vk::SurfaceCapabilitiesKHR &surface_capabilities,
		vk::SurfaceTransformFlagBitsKHR desired_pre_transform){

		vk::SurfaceTransformFlagBitsKHR pre_transform;
		if (surface_capabilities.supportedTransforms & desired_pre_transform) {
			pre_transform = desired_pre_transform;
		} else {
			pre_transform = surface_capabilities.currentTransform;
		}

		return pre_transform;
	}

	[[nodiscard]] vk::CompositeAlphaFlagBitsKHR get_composite_alpha_f(
		const vk::SurfaceCapabilitiesKHR &surface_capabilities,
		vk::CompositeAlphaFlagBitsKHR desired_composite_alpha){

		vk::CompositeAlphaFlagBitsKHR composite_alpha =
			vk::CompositeAlphaFlagBitsKHR::eOpaque;

		const std::array<vk::CompositeAlphaFlagBitsKHR, 4> composite_alpha_flags{
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
			vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
			vk::CompositeAlphaFlagBitsKHR::eInherit
		};

		for(auto & composite_alpha_bit : composite_alpha_flags){
			if(surface_capabilities.supportedCompositeAlpha & composite_alpha_bit){
				composite_alpha = composite_alpha_bit;
				if(composite_alpha == desired_composite_alpha)
					break;
			}
		}

		return composite_alpha;
	}

	[[nodiscard]] vk::ImageUsageFlags get_image_usage_flags_f(
		const vk::SurfaceCapabilitiesKHR &surface_capabilities,
		vk::ImageUsageFlags desired_usages){

		vk::ImageUsageFlags image_usage =
			desired_usages & surface_capabilities.supportedUsageFlags;

		if(desired_usages != image_usage)
			throw std::runtime_error("desired_usages not supported.");

		return image_usage;
	}

	[[nodiscard]] std::vector<swapchain_buffers_type> get_swapchain_buffers(
		const vk::Device &device,
		const vk::SwapchainKHR &swapchain,
		const vk::Format &format){

		const std::vector<vk::Image> images = device.getSwapchainImagesKHR(swapchain);

		std::vector<swapchain_buffers_type> swapchain_buffers{};
		for(auto image : images){
			const vk::ImageViewCreateInfo image_view_info = vk::ImageViewCreateInfo()
				.setImage(image)
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(format)
				.setSubresourceRange(vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

			swapchain_buffers.emplace_back(
				swapchain_buffers_type{image, device.createImageView(image_view_info)});
		}
		return swapchain_buffers;
	}
}

void renderer::init_swapchain(){
	vk::SurfaceFormatKHR formatKHR = get_surface_format_f(
		this->physical_device, this->surface,
		{vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear});

	const vk::SurfaceCapabilitiesKHR surface_capabilities =
		this->physical_device.getSurfaceCapabilitiesKHR(this->surface);

	const vk::Extent2D swapchain_extent =
		get_swapchain_extent_f(surface_capabilities, this->get_window_size());

	const uint32_t number_of_swapchain_images =	get_number_of_swapchain_images_f(
		surface_capabilities, surface_capabilities.minImageCount + 1);

	const vk::PresentModeKHR swapchain_present_mode =
		get_swapchain_present_mode_f(this->physical_device, this->surface,
		vk::PresentModeKHR::eFifo);

	const vk::SurfaceTransformFlagBitsKHR pre_transform = get_pre_transform_f(
		surface_capabilities, vk::SurfaceTransformFlagBitsKHR::eIdentity);

	const vk::CompositeAlphaFlagBitsKHR composite_alpha = get_composite_alpha_f(
		surface_capabilities, vk::CompositeAlphaFlagBitsKHR::eOpaque);

	const vk::ImageUsageFlags image_usage = get_image_usage_flags_f(
		surface_capabilities, vk::ImageUsageFlagBits::eColorAttachment);

	const vk::SwapchainCreateInfoKHR swapchain_info = vk::SwapchainCreateInfoKHR()
		.setSurface(this->surface)
		.setMinImageCount(number_of_swapchain_images)
		.setImageFormat(formatKHR.format)
		.setImageColorSpace(formatKHR.colorSpace)
		.setImageExtent(swapchain_extent)
		.setImageArrayLayers(1)
		.setImageUsage(image_usage)

//		.setImageSharingMode(SharingMode::eExclusive)
//		.setQueueFamilyIndexCount(0)
//		.setPQueueFamilyIndices(nullptr)

		.setPreTransform(pre_transform)
		.setCompositeAlpha(composite_alpha)
		.setPresentMode(swapchain_present_mode)
		.setClipped(true);

//		.setOldSwapchain();

	this->format = formatKHR.format;
	this->swapchain = this->device.createSwapchainKHR(swapchain_info);
	this->buffers = get_swapchain_buffers(this->device, this->swapchain, formatKHR.format);
}

void renderer::init_queues(){
	this->graphics_queue = this->device.getQueue(this->graphics_queue_family_index, 0);

	if(this->graphics_queue_family_index == this->present_queue_family_index)
		this->present_queue = this->graphics_queue;
	else
		this->present_queue = this->device.getQueue(this->present_queue_family_index, 0);
}

void renderer::init_command_pools(){
	const vk::CommandPoolCreateInfo command_pool_info = vk::CommandPoolCreateInfo()
		.setQueueFamilyIndex(this->graphics_queue_family_index);
	this->graphics_cmd_pool = this->device.createCommandPool(command_pool_info);
}

void renderer::init_command_buffers(){
	const vk::CommandBufferAllocateInfo cmd_buffer_info = vk::CommandBufferAllocateInfo()
		.setCommandPool(this->graphics_cmd_pool)
		.setCommandBufferCount(1);
	std::vector<vk::CommandBuffer> cmd_buffers
		= this->device.allocateCommandBuffers(cmd_buffer_info);
	this->cmd_buffer = cmd_buffers.front();
}

void renderer::init_framebuffers(){
	const vk::SurfaceCapabilitiesKHR surface_capabilities =
		this->physical_device.getSurfaceCapabilitiesKHR(this->surface);

	this->framebuffers = this->pipeline.create_framebuffers(this->device,
		this->physical_device, this->buffers,
		get_swapchain_extent_f(surface_capabilities, this->get_window_size()),
		this->format);
}

renderer::renderer(){
	this->init_window();

	this->instance_extension_names =
		{VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
	this->init_instance();

	this->device_extension_names = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	this->features.setGeometryShader(true).setTessellationShader(true);
	this->init_surface();
	this->init_physical_device();
	this->init_device();

	this->init_swapchain();
	this->init_queues();
	this->init_command_pools();
	this->init_command_buffers();

	this->init_framebuffers();

	this->pipeline.init_graphic_pipeline(this->device);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

void renderer::main_loop(){
	ShowWindow(this->hWnd, SW_SHOWNORMAL);
	UpdateWindow(this->hWnd);

	MSG msg;
	memset(&msg, 0, sizeof(MSG));

	while (1){
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			this->draw();
			UpdateWindow(this->hWnd);

			if(msg.message == WM_QUIT){
				break;
			}
		}
	}
}

void renderer::draw(){
	vk::Semaphore semaphore = this->device.createSemaphore(vk::SemaphoreCreateInfo());

	vk::ResultValue<uint32_t> frame_result = this->device.acquireNextImageKHR(
		this->swapchain, UINT64_MAX, semaphore, vk::Fence());
	if(frame_result.result == vk::Result::eSuccess)
		this->current_frame = frame_result.value;
	else
		throw std::runtime_error("Could not free frames.");

	cmd_buffer.begin(vk::CommandBufferBeginInfo());
	//cmd_begin_render_pass ... vkCmdEndRenderPass
	cmd_buffer.end();

	vk::Fence fance = this->device.createFence(vk::FenceCreateInfo());
	vk::PipelineStageFlags pipe_stage_flags
		= vk::PipelineStageFlagBits::eBottomOfPipe;
	const std::array<vk::SubmitInfo, 1> submit_info{
		vk::SubmitInfo()
			.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(&semaphore)
			.setPWaitDstStageMask(&pipe_stage_flags)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&cmd_buffer)
			.setSignalSemaphoreCount(0)
			.setPSignalSemaphores(nullptr)
	};
	this->graphics_queue.submit(submit_info, fance);

	this->device.waitForFences(fance, true, 100000);

	this->present_queue.presentKHR(vk::PresentInfoKHR()
//		.setWaitSemaphoreCount()
//		.setPWaitSemaphores()
		.setSwapchainCount(1)
		.setPSwapchains(&this->swapchain)
		.setPImageIndices(&this->current_frame)
//		.setPResults()
	);

	this->device.destroy(fance);
	this->device.destroy(semaphore);
}

vk::Extent2D renderer::get_window_size() const noexcept{
	RECT rect{};
	GetWindowRect(this->hWnd, &rect);

	return vk::Extent2D(static_cast<uint32_t>(rect.right - rect.left),
		static_cast<uint32_t>(rect.bottom - rect.top));
}
