#include "gui.h"

//---- private ---------------------------------------------------------------//

static k_Image *_CLImage_to_k_Image(CLImage clImage) {
	k_Image *image = k_create_image(clImage.size.x, clImage.size.y);

	for(int i = 0; i < image->width * image->height; i++) {
		if(clImage.data[i].x < 0) clImage.data[i].x = 0;
		if(clImage.data[i].y < 0) clImage.data[i].y = 0;
		if(clImage.data[i].z < 0) clImage.data[i].z = 0;
		
		if(clImage.data[i].x > 1) clImage.data[i].x = 1;
		if(clImage.data[i].y > 1) clImage.data[i].y = 1;
		if(clImage.data[i].z > 1) clImage.data[i].z = 1;

		image->data[i * 3 + 0] = (int)(clImage.data[i].x * 255);
		image->data[i * 3 + 1] = (int)(clImage.data[i].y * 255);
		image->data[i * 3 + 2] = (int)(clImage.data[i].z * 255);
	}

	return image;
}

SDL_Surface *_get_render_surface() {
	if(s.renderSurface != NULL) {
		SDL_FreeSurface(s.renderSurface);
		s.renderSurface = NULL;
	}

	k_Image *kImage = _CLImage_to_k_Image(r.clImage);

	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
		(void *)kImage->data,
		kImage->width, kImage->height,
		24, 3 * kImage->width,
		0x0000ff, 0x00ff00, 0xff0000, 0x000000
	);

	if(surface == NULL){
		msg("%s\n", SDL_GetError());
		exit(-1);
	}

	k_destroy_image(kImage);

	return surface;
}

void _update_dt() {
	unsigned long int currentTime = SDL_GetPerformanceCounter();
	s.dt = ((float)currentTime - (float)s.prevFrameTime) / (float)SDL_GetPerformanceFrequency();
	s.prevFrameTime = currentTime;
}

void _procces_events() {
	SDL_Event e;

	while(SDL_PollEvent(&e) != 0) {
		if(e.type == SDL_QUIT) {
			s.quit = 1;
		
		}
	}
}

void _process_kb_input() {
	float movementSpeed = 1;
	float turnSpeed = M_PI / 32;

	SDL_PumpEvents();

	if(s.keyState[SDL_SCANCODE_Q]) {
		s.quit = 1;
	}
	
	if(s.keyState[SDL_SCANCODE_W]) {
		r.camera.pos.z += movementSpeed * s.dt;
		r.restartRender = 1;
	}
	
	if(s.keyState[SDL_SCANCODE_S]) {
		r.camera.pos.z -= movementSpeed * s.dt;
		r.restartRender = 1;
	}
	
	if(s.keyState[SDL_SCANCODE_A]) {
		r.camera.pos.x -= movementSpeed * s.dt;
		r.restartRender = 1;
	}
	
	if(s.keyState[SDL_SCANCODE_D]) {
		r.camera.pos.x += movementSpeed * s.dt;
		r.restartRender = 1;
	}
	
	if(s.keyState[SDL_SCANCODE_LCTRL]) {
		r.camera.pos.y += movementSpeed * s.dt;
		r.restartRender = 1;
	}
	
	if(s.keyState[SDL_SCANCODE_SPACE]) {
		r.camera.pos.y -= movementSpeed * s.dt;
		r.restartRender = 1;
	}
	
	if(s.keyState[SDL_SCANCODE_UP]) {
		r.camera.rot.x += turnSpeed * s.dt;
		r.restartRender = 1;
	}
	
	if(s.keyState[SDL_SCANCODE_DOWN]) {
		r.camera.rot.x -= turnSpeed * s.dt;
		r.restartRender = 1;
	}
	
	if(s.keyState[SDL_SCANCODE_LEFT]) {
		r.camera.rot.y -= turnSpeed * s.dt;
		r.restartRender = 1;
	}
	
	if(s.keyState[SDL_SCANCODE_RIGHT]) {
		r.camera.rot.y += turnSpeed * s.dt;
		r.restartRender = 1;
	}
}

//---- public ----------------------------------------------------------------//

GUIStatus create_window(int width, int height) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) return GUI_FAILURE;
	if(TTF_Init() < 0) return GUI_FAILURE;

	s.window = NULL;
	s.windowSurface = NULL;
	s.renderSurface = NULL;
	s.prevFrameTime = SDL_GetPerformanceCounter();
	// s.font = TTF_OpenFont("opensans.ttf", 24);
	s.keyState = SDL_GetKeyboardState(NULL);

	s.window = SDL_CreateWindow("pathtracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	if(s.window == NULL) return GUI_FAILURE;
	s.windowSurface = SDL_GetWindowSurface(s.window);

	return GUI_SUCCESS;
}

GUIStatus start_main_loop() {
	begin_image_rendering();
	
	int frame = 0;

	while(!s.quit) {
		_update_dt();
		_procces_events();
		_process_kb_input();

		if(r.restartRender) {
			r.restartRender = 0;
			frame = 0;
			begin_image_rendering();
		}

		render_sample(frame);
		read_image();

		if(s.renderSurface != NULL) {
			SDL_FreeSurface(s.renderSurface);
			s.renderSurface = NULL;
		}

		s.renderSurface = _get_render_surface();

		SDL_BlitSurface(s.renderSurface, NULL, s.windowSurface, NULL);

		SDL_UpdateWindowSurface(s.window);

		frame++;
	}

	end_image_rendering();

	return GUI_SUCCESS;
}

GUIStatus close_window() {
	if(s.renderSurface != NULL) {
		SDL_FreeSurface(s.renderSurface);
		s.renderSurface = NULL;
	}

	if(s.renderSurface != NULL) {
		SDL_FreeSurface(s.windowSurface);
		s.windowSurface = NULL;
	}

	SDL_DestroyWindow(s.window);
	TTF_Quit();
	SDL_Quit();

	return GUI_SUCCESS;
}