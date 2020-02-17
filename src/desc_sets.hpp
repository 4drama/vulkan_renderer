#ifndef DESC_SETS
#define DESC_SETS

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.hpp>
#include "windows.h"

#include <vector>
#include <map>

struct layout_f{
	vk::DescriptorSetLayoutBinding descriptor_set_binding;

	const vk::DescriptorImageInfo* pImageInfo_ = nullptr;
	const vk::DescriptorBufferInfo* pBufferInfo_ = nullptr;
	const vk::BufferView* pTexelBufferView_ = nullptr;
};

class pool_size_t;

struct descriptor_t{	//need redo
	std::vector<layout_f> bindings;
	vk::DescriptorSetLayout layout;
	std::vector<vk::DescriptorSet> sets;	//vk::DescriptorSet set;

	descriptor_t() = default;
	descriptor_t(vk::Device device, std::vector<layout_f> bindings);
	descriptor_t(std::vector<layout_f> bindings, vk::DescriptorSetLayout layout);

	pool_size_t get_size() const;
	void allocate_set(vk::Device device, vk::DescriptorPool pool);
	void update(vk::Device device);
	vk::DescriptorSet get() const;
};

/*class descriptor2_t{
public:

private:

};*/

class pool_size_t{
public:
	pool_size_t() = default;
	pool_size_t(layout_f layout);
	pool_size_t(std::vector<layout_f> layouts);

	std::vector<vk::DescriptorPoolSize> get_types() const;
	uint32_t get_count() const;

	pool_size_t& add(vk::DescriptorType descriptorType, uint32_t descriptorCount);

	pool_size_t& operator+=(const pool_size_t& rhs);
	pool_size_t& operator=(pool_size_t const& rhs);
private:
	std::map<vk::DescriptorType, uint32_t> size;

	friend pool_size_t operator+(const pool_size_t &lhs, const pool_size_t &rhs);

};

pool_size_t operator+(const pool_size_t &lhs, const pool_size_t &rhs);

vk::DescriptorPool create_descriptor_pool(vk::Device device, pool_size_t size);

vk::DescriptorSetLayout create_descriptor_set_layout(vk::Device device,
	std::vector<layout_f> &layouts);

#endif
