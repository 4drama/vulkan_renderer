#include "rnd.hpp"

#include <stdexcept>
#include <iostream>


int main() try {
	renderer r2{};
	r2.main_loop();
	
	return 0;

} catch (const std::exception& e){
	 std::cerr << "main() failed with: " << e.what();
} catch (...){
	 std::cerr << "main() failed with: unknown exception." <<std::endl;
}
