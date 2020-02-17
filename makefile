
VK_VERSION= 1.2.131.2
VK_INCLUDE= -IC:\VulkanSDK\$(VK_VERSION)\Include
VK_LIB= -LC:\VulkanSDK\$(VK_VERSION)\Lib

GLM_INCLUDE= -IG:\libraries\glm
TINY_OBJ_INCLUDE= -IG:\libraries\tinyobjloader
STB_IMAGE_INCLUDE= -IG:\libraries\stb

VK_LINK_FLAG= -lvulkan-1
CPP_LINK_FLAG= -lstdc++ -g

SHADER_COMP= C:\libraries\VulkanSDK\$(VK_VERSION)\Bin\glslangValidator.exe

ALL: cclean
	gcc -c $(VK_INCLUDE) -std=c++17 ./src/rnd.cpp -o ./obj/rnd.o
	gcc -c $(VK_INCLUDE) $(GLM_INCLUDE) $(TINY_OBJ_INCLUDE) \
		-std=c++17 ./src/utils.cpp -o ./obj/utils.o
	gcc -c $(VK_INCLUDE) $(STB_IMAGE_INCLUDE) \
		-std=c++17 ./src/pipeline.cpp -o ./obj/pipeline.o
	gcc -c $(VK_INCLUDE) -std=c++17 ./src/rnd.cpp -o ./obj/rnd.o

	gcc -c $(VK_INCLUDE) -std=c++17 ./src/desc_sets.cpp -o ./obj/desc_sets.o

	glslangValidator -V -o ./shaders/vert_shader.spv ./src/shader.vert
	glslangValidator -V -o ./shaders/frag_shader.spv ./src/shader.frag

	gcc $(VK_LIB) ./obj/rnd.o ./obj/test.o ./obj/pipeline.o ./obj/utils.o \
		./obj/desc_sets.o -o test.exe $(VK_LINK_FLAG) $(CPP_LINK_FLAG)

print_dump:
	setx VK_INSTANCE_LAYERS VK_LAYER_LUNARG_api_dump VK_LAYER_LUNARG_core_validation

p1:
	setx VK_INSTANCE_LAYERS VK_LAYER_LUNARG_core_validation

#	VK_LAYER_LUNARG_swapchain,
#	VK_LAYER_LUNARG_object_tracker, VK_LAYER_GOOGLE_threading, or
#	VK_LAYER_LUNARG_api_dump,

cclean:
	rm -rf  *.cache

clean: cclean
	rm -rf  obj/*.o
	rm -rf  *.dll
	rm -rf  shaders/*.spv
	rm -rf  *.exe
