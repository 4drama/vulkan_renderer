#include "rnd.hpp"

#include <stdexcept>
#include <iostream>


int main() try {
	scene_t scene_one{};
	scene_one.add_object(
		mesh{
			std::vector<polygon>{
				{
					vertex{-1, -1, 0, 1},
					vertex{1, -1, 0, 1},
					vertex{-1, 1, 0, 1}
				}
			}
		}
	);

	renderer r2{};
	r2.set_scene(&scene_one);
	r2.main_loop();

	return 0;

} catch (const std::exception& e){
	 std::cerr << "main() failed with: " << e.what();
} catch (...){
	 std::cerr << "main() failed with: unknown exception." <<std::endl;
}
