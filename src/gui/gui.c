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

void _print_text(char *text) {
	SDL_Color color = {255, 255, 255};
	SDL_Rect dest = {0, 0, 0, 0};
	SDL_Surface* message = TTF_RenderText_Solid(s.font, text, color);
	SDL_BlitSurface(message, NULL, s.renderSurface, &dest);
	SDL_FreeSurface(message);
}

void _update_dt() {
	unsigned long int currentTime = SDL_GetPerformanceCounter();
	s.dt = ((float)currentTime - (float)s.prevFrameTime) / (float)SDL_GetPerformanceFrequency();
	s.prevFrameTime = currentTime;
}

void _procces_events() {
	float movementSpeed = 1;
	float turnSpeed = M_PI / 16;
	SDL_Event e;

	while(SDL_PollEvent(&e) != 0) {
		if(e.type == SDL_QUIT) {
			s.quit = 1;
		
		} else if(e.type == SDL_KEYDOWN) {
			switch(e.key.keysym.sym) {
				case SDLK_ESCAPE:
					s.quit = 1;
					break;
				
				case SDLK_w:
					r.camera.pos.z += movementSpeed * s.dt;
					r.restartRender = 1;
					break;
				
				case SDLK_s:
					r.camera.pos.z -= movementSpeed * s.dt;
					r.restartRender = 1;
					break;
				
				case SDLK_a:
					r.camera.pos.x -= movementSpeed * s.dt;
					r.restartRender = 1;
					break;
				
				case SDLK_d:
					r.camera.pos.x += movementSpeed * s.dt;
					r.restartRender = 1;
					break;
				
				case SDLK_q:
					r.camera.pos.y += movementSpeed * s.dt;
					r.restartRender = 1;
					break;
				
				case SDLK_e:
					r.camera.pos.y -= movementSpeed * s.dt;
					r.restartRender = 1;
					break;
				
				case SDLK_UP:
					r.camera.rot.x += turnSpeed * s.dt;
					r.restartRender = 1;
					break;
				
				case SDLK_DOWN:
					r.camera.rot.x -= turnSpeed * s.dt;
					r.restartRender = 1;
					break;
				
				case SDLK_LEFT:
					r.camera.rot.y -= turnSpeed * s.dt;
					r.restartRender = 1;
					break;
				
				case SDLK_RIGHT:
					r.camera.rot.y += turnSpeed * s.dt;
					r.restartRender = 1;
					break;

			}
		}
	}
}

//---- public ----------------------------------------------------------------//

GUIStatus create_window(int width, int height) {
	s.window = NULL;
	s.windowSurface = NULL;
	s.renderSurface = NULL;
	s.prevFrameTime = SDL_GetPerformanceCounter();
	s.font = TTF_OpenFont("opensans.ttf", 24);

	if(SDL_Init( SDL_INIT_VIDEO ) < 0) return GUI_FAILURE;
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

		if(r.restartRender) {
			r.restartRender = 0;
			frame = 0;
			begin_image_rendering();
		}

		render_sample(frame);
		read_image();

		s.renderSurface = _get_render_surface();

		SDL_BlitSurface(s.renderSurface, NULL, s.windowSurface, NULL);

		char fps[100];
		sprintf(fps, "%f", 1 / s.dt);
		_print_text(fps);

		SDL_UpdateWindowSurface(s.window);

		frame++;
	}

	end_image_rendering();

	return GUI_SUCCESS;
}