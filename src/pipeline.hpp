#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.hpp>
#include "windows.h"

#include "utils.hpp"

class pipeline_t{
public:

	void init_depth_buffer(const vk::Device &device,
		const vk::PhysicalDevice &physical_device, vk::Extent2D window_size);

	void init_graphic_pipeline(const vk::Device &device);

	// not comlited. need randerpass
	std::vector<vk::Framebuffer> create_framebuffers(
		const vk::Device &device,
		const std::vector<swapchain_buffers_type> &buffers,
		vk::Extent2D window_size) const;
private:
	static constexpr vk::SampleCountFlagBits num_samples
		= vk::SampleCountFlagBits::e1;

	struct {
		vk::Format format;

		vk::Image image;
		vk::DeviceMemory mem;
		vk::ImageView view;
	} depth;

	std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages;

	std::vector<vk::DescriptorSetLayout> desc_set_layout;
	std::vector<vk::PushConstantRange> const_range;

	vk::PipelineLayout pipeline_layout;

	vk::DescriptorPool desc_pool;
//	std::vector<vk::DescriptorSet> desc_set;

	vk::PipelineCache pipeline_cache;

	void init_descriptor_set_layouts(const vk::Device &device,
		const std::vector<vk::DescriptorSetLayoutBinding> &layout_bindings);
	void init_const_range();

	void init_pipeline_layouts(const vk::Device &device);
	void init_descriptor_pool(const vk::Device &device,
		const std::vector<vk::DescriptorSetLayoutBinding> &layout_bindings);
};

#endif
