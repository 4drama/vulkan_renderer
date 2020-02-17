#include "desc_sets.hpp"

#include <algorithm>

#include <iostream>

descriptor_t::descriptor_t(vk::Device device, std::vector<layout_f> bindings_)
	: bindings(bindings_),
	layout(create_descriptor_set_layout(device, this->bindings)){
}

descriptor_t::descriptor_t(std::vector<layout_f> bindings_, vk::DescriptorSetLayout layout_)
	: bindings(bindings_), layout(layout_){
}

pool_size_t descriptor_t::get_size() const{
	return pool_size_t(this->bindings);
}

void descriptor_t::allocate_set(vk::Device device, vk::DescriptorPool pool){
//	for(auto &binding : this->bindings){
	const vk::DescriptorSetAllocateInfo desc_sets_info =
		vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(pool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&this->layout);

	auto tmp_desc_sets = device.allocateDescriptorSets(desc_sets_info);

	for(auto &set : tmp_desc_sets){
		this->sets.emplace_back(set);	//only one
	}
//	}
}

void descriptor_t::update(vk::Device device){
//	const uint32_t size = this->sets.size();
	const uint32_t size = this->bindings.size();
//	std::cerr << this->sets.size() << std::endl;
	std::vector<vk::WriteDescriptorSet> writes(size);
	for(uint32_t i = 0 ; i < size; ++i){
		writes[i] = vk::WriteDescriptorSet()
			.setDstSet(this->sets[0])
			.setDstBinding(this->bindings[i].descriptor_set_binding.binding)
			.setDescriptorCount(this->bindings[i].descriptor_set_binding.descriptorCount)
			.setDescriptorType(this->bindings[i].descriptor_set_binding.descriptorType)
			.setPImageInfo(this->bindings[i].pImageInfo_)
			.setPBufferInfo(this->bindings[i].pBufferInfo_)
			.setPTexelBufferView(this->bindings[i].pTexelBufferView_);
	}
	device.updateDescriptorSets(writes, std::vector<vk::CopyDescriptorSet>{});
}

vk::DescriptorSet descriptor_t::get() const{
	return *this->sets.begin();
}

pool_size_t::pool_size_t(layout_f layout){
	const vk::DescriptorSetLayoutBinding &binding = layout.descriptor_set_binding;
	this->size[binding.descriptorType] = binding.descriptorCount;
}

pool_size_t::pool_size_t(std::vector<layout_f> layouts){
	for(auto &layout : layouts){
		const vk::DescriptorSetLayoutBinding &binding = layout.descriptor_set_binding;
		this->size[binding.descriptorType] += binding.descriptorCount;
	}
}

std::vector<vk::DescriptorPoolSize> pool_size_t::get_types() const{
	std::vector<vk::DescriptorPoolSize> pool_size{};

	std::for_each(this->size.begin(), this->size.end(), [&pool_size]
		(const std::pair<vk::DescriptorType, uint32_t> &el){

		pool_size.emplace_back(el.first, el.second);
	});
	return pool_size;
}

uint32_t pool_size_t::get_count() const{
	uint32_t count = 0;
	std::for_each(this->size.begin(), this->size.end(), [&count]
		(const std::pair<vk::DescriptorType, uint32_t> &el){

		count += el.second;
	});
	return count;
}

pool_size_t& pool_size_t::add(vk::DescriptorType descriptorType, uint32_t descriptorCount){
	this->size[descriptorType] += descriptorCount;
	return *this;
}

pool_size_t& pool_size_t::operator+=(const pool_size_t& rhs){
   *this = *this + rhs;
   return *this;
}

pool_size_t& pool_size_t::operator= (pool_size_t const& rhs) {
	if(this != &rhs){
		pool_size_t tmp(rhs);
		this->size = tmp.size;
	}
	return *this;
}

pool_size_t operator+(const pool_size_t &lhs, const pool_size_t &rhs){
	pool_size_t tmp(lhs);
	std::for_each(rhs.size.begin(), rhs.size.end(), [&tmp]
		(const std::pair<vk::DescriptorType, uint32_t> &el){

		tmp.size[el.first] += el.second;
	});
	return tmp;
}

vk::DescriptorPool create_descriptor_pool(vk::Device device, pool_size_t size){
	const std::vector<vk::DescriptorPoolSize> vec_size = size.get_types();
	const vk::DescriptorPoolCreateInfo descriptor_pool_info =
		vk::DescriptorPoolCreateInfo()
		.setMaxSets(size.get_count())
		.setPoolSizeCount(vec_size.size())
		.setPPoolSizes(vec_size.data());

/*	for(auto &size : vec_size){
		std::cerr << to_string(size.type) << ' ' << size.descriptorCount << std::endl;
	}*/

	return device.createDescriptorPool(descriptor_pool_info);
}

vk::DescriptorSetLayout create_descriptor_set_layout(vk::Device device,
	std::vector<layout_f> &layouts){

	if(layouts.size() != 0){
		std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
		for(auto &layout : layouts){
			layout_bindings.emplace_back(layout.descriptor_set_binding);
		}

		const vk::DescriptorSetLayoutCreateInfo desc_set_layout_info
			= vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(layout_bindings.size())
			.setPBindings(layout_bindings.data());

		return device.createDescriptorSetLayout(desc_set_layout_info);
	} else
		return vk::DescriptorSetLayout();
}
