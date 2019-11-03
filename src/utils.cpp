#include "utils.hpp"

#include <cmath>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include<iostream>

buffer_t create_buffer(const vk::Device &device,
	const vk::PhysicalDeviceMemoryProperties &mem_prop,
	vk::BufferUsageFlags usage_flag, vk::MemoryPropertyFlags prop_flag,
	vk::DeviceSize size){

	buffer_t buffer{};
	buffer.buf = device.createBuffer(vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(usage_flag));

	vk::MemoryRequirements mem_req = device.getBufferMemoryRequirements(buffer.buf);;

	buffer.mem = device.allocateMemory(vk::MemoryAllocateInfo()
		.setAllocationSize(mem_req.size)
		.setMemoryTypeIndex(memory_type_from_properties(mem_prop, mem_req, prop_flag)));

	buffer.info = vk::DescriptorBufferInfo()
		.setBuffer(buffer.buf)
		.setOffset(0)
		.setRange(mem_req.size);

	device.bindBufferMemory(buffer.buf, buffer.mem, buffer.info.offset);
	return buffer;
}

image_t create_sampled_image(const vk::Device &device, vk::Format format,
	vk::FormatProperties format_properties, bool linear_filtering, vk::Extent3D extent,
	uint32_t num_mipmaps, uint32_t num_layers, vk::SampleCountFlagBits samples,
	vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect, bool is_cubemap,
	const vk::PhysicalDeviceMemoryProperties &mem_prop, vk::MemoryPropertyFlags prop_flag){

	if(!(format_properties.optimalTilingFeatures
		& vk::FormatFeatureFlagBits::eSampledImage))
		throw std::runtime_error
			("Provided format is not supported for a sampled image.");

	if(linear_filtering && !(format_properties.optimalTilingFeatures
		& vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
		throw std::runtime_error
			("Provided format is not supported for a linear image filtering.");

	image_t image{};
	image.img = device.createImage(vk::ImageCreateInfo()
		.setFlags(is_cubemap ?
			vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlags())
		.setImageType(vk::ImageType::e2D)
		.setFormat(format)
		.setExtent(extent)
		.setMipLevels(num_mipmaps)
		.setArrayLayers(num_layers)
		.setSamples(samples)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(usage | vk::ImageUsageFlagBits::eSampled)
	/*	.setSharingMode()
		.setQueueFamilyIndexCount()
		.setPQueueFamilyIndices()		*/
		.setInitialLayout(vk::ImageLayout::eUndefined)
	);

	vk::MemoryRequirements memory_requirements =
		device.getImageMemoryRequirements(image.img);

	image.mem = device.allocateMemory(vk::MemoryAllocateInfo()
		.setAllocationSize(memory_requirements.size)
		.setMemoryTypeIndex(
			memory_type_from_properties(mem_prop, memory_requirements, prop_flag))
	);

	image.info = vk::DescriptorImageInfo()
		.setSampler(device.createSampler(vk::SamplerCreateInfo()
	/*		.setFlags()
			.setMagFilter()
			.setMinFilter()
			.setMipmapMode()
			.setAddressModeU()
			.setAddressModeV()
			.setAddressModeW()
			.setMipLodBias()
			.setAnisotropyEnable()
			.setMaxAnisotropy()
			.setCompareEnable()
			.setCompareOp()
			.setMinLod()
			.setMaxLod()
			.setBorderColor()
			.setUnnormalizedCoordinates()*/))
		.setImageView(device.createImageView(vk::ImageViewCreateInfo()
			.setFlags(vk::ImageViewCreateFlags())
			.setImage(image.img)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(format)
			.setComponents(vk::ComponentMapping())
			.setSubresourceRange(vk::ImageSubresourceRange()
				.setAspectMask(aspect)
				.setBaseMipLevel(0)
				.setLevelCount(VK_REMAINING_MIP_LEVELS)
				.setBaseArrayLayer(0)
				.setLayerCount(VK_REMAINING_ARRAY_LAYERS))))
		.setImageLayout(vk::ImageLayout::eUndefined);

	device.bindImageMemory(image.img, image.mem, 0);
	return image;
}

void destroy(const vk::Device &device, buffer_t &buf){
	device.destroy(buf.buf);
	device.free(buf.mem);
	buf.info = vk::DescriptorBufferInfo();
};

vk::Image create_depth_image(const vk::Device &device, vk::Format format,
	vk::Extent2D window_size, vk::FormatProperties format_prop,
	vk::SampleCountFlagBits num_samples){

	vk::ImageTiling tiling;
	if(format_prop.linearTilingFeatures
		& vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		tiling = vk::ImageTiling::eLinear;
	else if(format_prop.optimalTilingFeatures
		& vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		tiling = vk::ImageTiling::eOptimal;
	else
		throw std::runtime_error("init_depth_buffer: "
			+ vk::to_string(format) + " unsupported.");

	const vk::ImageCreateInfo image_info = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(format)
		.setExtent(vk::Extent3D(window_size, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(num_samples)
		.setTiling(tiling)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
	/*	.setSharingMode()
		.setQueueFamilyIndexCount()
		.setPQueueFamilyIndices();*/
		.setInitialLayout(vk::ImageLayout::eUndefined);

	return device.createImage(image_info);
}

uint32_t memory_type_from_properties(const vk::PhysicalDeviceMemoryProperties &mem_prop,
	const vk::MemoryRequirements &mem_req, vk::MemoryPropertyFlags req_mask){

	uint32_t typeBits = mem_req.memoryTypeBits;

	for(uint32_t i = 0; i < mem_prop.memoryTypeCount; i++){
		if ((typeBits & 1) == 1) {
			if ((mem_prop.memoryTypes[i].propertyFlags & req_mask) == req_mask) {
				return i;
			}
		}
		typeBits >>= 1;
	}
	throw std::runtime_error("ERROR: memory_type_from_properties");
}

vk::DeviceMemory allocate_image_memory(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, const vk::Image &depth_image,
	vk::MemoryPropertyFlags requirements_mask){
	const vk::MemoryRequirements mem_req = device.getImageMemoryRequirements(depth_image);
	const vk::PhysicalDeviceMemoryProperties mem_prop
		= physical_device.getMemoryProperties();

	const uint32_t typeIndex
		= memory_type_from_properties(mem_prop, mem_req, requirements_mask);

	const vk::MemoryAllocateInfo allocate_info = vk::MemoryAllocateInfo()
		.setAllocationSize(mem_req.size)
		.setMemoryTypeIndex(typeIndex);
	return device.allocateMemory(allocate_info);
}

vk::ImageView create_2d_image_view(const vk::Device &device,
	const vk::Image &image, const vk::Format &format, vk::ImageAspectFlags aspect_mask){

	const vk::ImageViewCreateInfo image_view_info = vk::ImageViewCreateInfo()
		.setImage(image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format)
		.setSubresourceRange(vk::ImageSubresourceRange(
			aspect_mask, 0, 1, 0, 1));
	return device.createImageView(image_view_info);
}

std::size_t get_mvp_buffer_size(){
	return sizeof(glm::mat4) * 2;
}

void update_mvp_buffer(const camera &cam,
	const vk::Device &device, const buffer_t &buf){
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 2.5, -7), // Camera is at (-5,3,-10), in World Space
		glm::vec3(0, 0, 0),    // and looks at the origin
		glm::vec3(0, 1, 0)    // Head is up (set to 0,-1,0 to look upside-down)
	);
	static float angle = 0.0f;
	glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(angle),
		glm::vec3(0.0f, 1.0f, 0.0f));
	angle += 1.0f;
	// Vulkan clip space has inverted Y and half Z.
	glm::mat4 Clip = glm::mat4(
		1.0f,  0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f,  0.0f, 0.5f, 0.0f,
		0.0f,  0.0f, 0.5f, 1.0f);

	glm::mat4 mv = glm::transpose(glm::inverse(Model * View));
	glm::mat4 mvp = Clip * Projection * View * Model;

//	vk::DeviceSize size = device.getMemoryCommitment(mem);

	void *data_ptr = device.mapMemory(buf.mem, 0, buf.info.range, vk::MemoryMapFlags());
	memcpy(data_ptr, &mvp, sizeof(glm::mat4));
	memcpy(data_ptr + sizeof(glm::mat4), &mv, sizeof(glm::mat4));
    device.unmapMemory(buf.mem);
}

struct normal{
	float x = 0, y = 0, z = 0;
};

struct tex_coord{
	float u = 0, v = 0;
};

void add_normal(vertex &v, normal n){
	v.norm_x += n.x;
	v.norm_y += n.y;
	v.norm_z += n.z;
}

void set_uv(vertex &vert, tex_coord t){
	vert.u = t.u;
	vert.v = t.v;
}

void normalize(vertex &v){
	float length = sqrt(pow(v.norm_x, 2) + pow(v.norm_y, 2) + pow(v.norm_z, 2));
	v.norm_x /= length;
	v.norm_y /= length;
	v.norm_z /= length;
}

glm::vec3 vertex_pos_vec(vertex &v){
	return glm::vec3(v.pos_x, v.pos_y, v.pos_z);
}

indeced_mash load_obj(std::string path){
	tinyobj::ObjReader obj{};
	if(!obj.ParseFromFile(path)){
		std::string msg("Could not open: " + path);
		if( 0 < obj.Error().size() ) {
			msg += '\n' + obj.Error();
		}
		throw std::runtime_error(msg);
	}

	const tinyobj::attrib_t &attribs = obj.GetAttrib();

	indeced_mash mash;
	for(uint32_t i = 0; i < attribs.vertices.size(); i += 3){
		mash.verteces.emplace_back(vertex{attribs.vertices[i], attribs.vertices[i+2],
			attribs.vertices[i+1]});
	}

	std::vector<normal> normals{};
	for(uint32_t i = 0; i < attribs.normals.size(); i += 3){
		normals.emplace_back(normal{attribs.normals[i], attribs.normals[i+2],
			attribs.normals[i+1]});
	}

	std::vector<tex_coord> uvs{};
	for(uint32_t i = 0; i < attribs.texcoords.size(); i += 2){
		uvs.emplace_back(tex_coord{attribs.texcoords[i], attribs.texcoords[i+1]});
	}

	mash.materials_ranges.emplace_back(material_range_t{0, 0});
	for(auto &shape : obj.GetShapes()){
		for(auto &index : shape.mesh.indices){
			mash.indeces.emplace_back(index.vertex_index);

			if(normals.size() != 0)
				add_normal(mash.verteces[index.vertex_index], normals[index.normal_index]);

			if(uvs.size() != 0)
				set_uv(mash.verteces[index.vertex_index], uvs[index.texcoord_index]);
		}

		for(uint32_t i = 0; i < shape.mesh.material_ids.size(); ++i){
			material_range_t& last_range = mash.materials_ranges.back();

			const uint32_t &id = shape.mesh.material_ids[i];
			const uint8_t &num_face = shape.mesh.num_face_vertices[i];

			if(last_range.id == id)
				last_range.range += num_face;
			else
				mash.materials_ranges.emplace_back(material_range_t{id, num_face});
		}
	}

	for(auto &mat : obj.GetMaterials()){
		mash.materials.emplace_back(material_t{mat.diffuse_texname});
	}

	if(normals.size() == 0){
		for(uint32_t i = 0; i < mash.indeces.size(); i += 3){
			glm::vec3 vec1 = vertex_pos_vec(mash.verteces[mash.indeces[i + 1]]) -
				vertex_pos_vec(mash.verteces[mash.indeces[i]]);
			glm::vec3 vec2 = vertex_pos_vec(mash.verteces[mash.indeces[i + 2]]) -
				vertex_pos_vec(mash.verteces[mash.indeces[i]]);
			glm::vec3 norm = cross(vec2, vec1);

			add_normal(mash.verteces[mash.indeces[i]], normal{norm.x, norm.y, norm.z});
			add_normal(mash.verteces[mash.indeces[i + 1]], normal{norm.x, norm.y, norm.z});
			add_normal(mash.verteces[mash.indeces[i + 2]], normal{norm.x, norm.y, norm.z});
		}
	}

	for(auto &vertex : mash.verteces){
		normalize(vertex);
	}

	return mash;
}
