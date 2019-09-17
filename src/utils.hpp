#ifndef UTILS_HPP
#define UTILS_HPP

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.hpp>
#include "windows.h"

#include <fstream>

struct swapchain_buffers_type {
	vk::Image image;
	vk::ImageView view;
};

template<typename T>
std::vector<T> load_file(std::string path);

template<typename T>
void save_file(std::string path, const std::vector<T> &bytecode);

vk::Image create_depth_image(const vk::Device &device, vk::Format format,
	vk::Extent2D window_size, vk::FormatProperties format_prop,
	vk::SampleCountFlagBits num_samples);

uint32_t memory_type_from_properties(
	const vk::PhysicalDeviceMemoryProperties &mem_prop,
	const vk::MemoryRequirements &mem_req, vk::MemoryPropertyFlags req_mask);

vk::DeviceMemory allocate_image_memory(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, const vk::Image &depth_image,
	vk::MemoryPropertyFlags requirements_mask);

vk::ImageView create_2d_image_view(const vk::Device &device,
	const vk::Image &image, const vk::Format &format,
	vk::ImageAspectFlags aspect_mask);

//===================================================================

template<typename T>
std::vector<T> load_file(std::string path){
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if(file){
		file.seekg(0, std::ios::end);
		const size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<T> bytecode(fileSize);
		bytecode.assign((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());

		return bytecode;
	} else
		return std::vector<T>();
}

template<typename T>
void save_file(std::string path, const std::vector<T> &bytecode){
	std::ofstream file(path, std::ios::out | std::ios::binary);
	file.write(reinterpret_cast<const char*>(bytecode.data()), bytecode.size());
}

#endif
