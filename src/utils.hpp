#ifndef UTILS_HPP
#define UTILS_HPP

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.hpp>
#include "windows.h"

#include <fstream>

struct buffer_t{
	vk::Buffer buf;
	vk::DeviceMemory mem;
	vk::DescriptorBufferInfo info;
};

struct texture_buffer_t{

};

buffer_t create_buffer(const vk::Device &device,
	const vk::PhysicalDeviceMemoryProperties &mem_prop,
	vk::BufferUsageFlags usage_flag, vk::MemoryPropertyFlags prop_flag,
	vk::DeviceSize size);

void destroy(const vk::Device &device, buffer_t &bufs);

struct swapchain_buffers_type {
	vk::Image image;
	vk::ImageView view;
};

struct vertex{
	float pos_x, pos_y, pos_z;
	float norm_x = 0, norm_y = 0, norm_z = 0;
	float u = 0, v = 0;
	float r = 1, g = 1, b = 1;
};

struct polygon{
	std::array<vertex, 3> data;
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
	file.write(reinterpret_cast<const char*>(bytecode.data()),
		bytecode.size() * sizeof(T));
}

struct camera{
//	float pos_x, pos_y, pos_z;
};

std::size_t get_mvp_buffer_size();
void update_mvp_buffer(const camera &cam,
	const vk::Device &device, const buffer_t &buf);

struct material_t{
	std::string diffuse_texname;
};

struct material_range_t{
	uint32_t id;
//	uint32_t offset;
	uint32_t range;
};

struct indeced_mash{
	std::vector<vertex> verteces;
	std::vector<uint32_t> indeces;

	std::vector<material_t> materials;
	std::vector<material_range_t> materials_ranges;
};

indeced_mash load_obj(std::string path);

#endif
