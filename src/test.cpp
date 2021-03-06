#include "rnd.hpp"

#include <stdexcept>
#include <iostream>

#include <chrono>

void count_fps(float time){
	static uint32_t fps_counter = 0;
	static float fps_dur = 0;

	fps_counter++;
	fps_dur += time;

	if(fps_dur >= 1){
		std::cerr << fps_counter << std::endl;
		fps_counter = 0;
		fps_dur = 0;
	}
}

void main_loop(renderer &rnd){
	using namespace std::chrono;

	rnd.show_window();

	MSG msg;
	memset(&msg, 0, sizeof(MSG));

	high_resolution_clock::time_point new_time = high_resolution_clock::now();
	high_resolution_clock::time_point old_time;

	duration<double> time_span;
	while (true){
		old_time = new_time;
		new_time = high_resolution_clock::now();

		time_span = duration_cast<duration<double>>(new_time - old_time);
	//	count_fps(time_span.count());

		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		//	UpdateWindow(this->hWnd);

			if(msg.message == WM_QUIT){
				break;
			}
		}
		rnd.draw();
	}
}

int main() try {
//	indeced_mash mash = load_obj("./data/box.obj");
	indeced_mash mash = load_obj("./data/challenger/", "CHALLENGER71.obj");

	renderer r2{};
	r2.set_scene(&mash);
	main_loop(r2);

	return 0;

} catch (const std::exception& e){
	 std::cerr << "main() failed with: " << e.what();
} catch (...){
	 std::cerr << "main() failed with: unknown exception." <<std::endl;
}
