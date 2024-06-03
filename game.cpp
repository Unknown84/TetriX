#include <stdio.h>
#include <locale.h>
#include <SDL.h>
#include <stdlib.h>
#include <time.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <string.h>
typedef struct {
	bool matrix[4][4];
	int row;
	int col;
	SDL_Color color;
} Block;
const int WIDTH = 1920;
const int HEIGHT = 1080;
const int SIZE = 50;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int control,music, sound;
int speed = 0;
int blocks = 0;
int timer = 0;
float koef = 1;
int wose = -1;
int score = 0;
int mode = 0;
bool init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL не удалось инициализировать! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	window = SDL_CreateWindow("TetriX", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Окно не может быть создано! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		printf("Renderer не может быть создан! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	if (TTF_Init() == -1) {
		printf("SDL_ttf не удалось инициализировать! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer не удалось инициализировать! SDL_mixer Error: %s\n", Mix_GetError());
		return false;
	}
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	return true;
}
void close() {
	Mix_CloseAudio();
	TTF_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
SDL_Rect RenderText(const char* text, SDL_Color color, int x, int y, TTF_Font* my_font)
{
	SDL_Surface* textSurface = TTF_RenderText_Solid(my_font, text, color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_Rect Rect = { x,y,textSurface->w,textSurface->h };
	SDL_RenderCopy(renderer, texture, NULL, &Rect);
	SDL_FreeSurface(textSurface);
	SDL_DestroyTexture(texture);
	return Rect;
}
SDL_Rect RenderTexture(const char* image, int x, int y)
{
	SDL_Surface* surface = IMG_Load(image);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	SDL_Rect Rect;
	Rect.x = x;
	Rect.y = y;
	SDL_QueryTexture(texture, NULL, NULL, &Rect.w, &Rect.h);
	SDL_RenderCopy(renderer, texture, NULL, &Rect);
	SDL_DestroyTexture(texture);
	return Rect;
}
int Generate(Block* block,SDL_Color colors[10]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			block->matrix[i][j] = false;
		}
	}
	int shape = rand() % 7;
	switch (shape) {
	case 0:
		block->matrix[0][0] = block->matrix[0][1] = block->matrix[1][0] = block->matrix[1][1] = true;
		break;
	case 1:
		block->matrix[0][0] = block->matrix[0][1] = block->matrix[0][2] = block->matrix[0][3] = true;
		break;
	case 2:
		block->matrix[0][1] = block->matrix[1][0] = block->matrix[1][1] = block->matrix[1][2] = true;
		break;
	case 3:
		block->matrix[0][0] = block->matrix[1][0] = block->matrix[2][0] = block->matrix[2][1] = true;
		break;
	case 4:
		block->matrix[0][1] = block->matrix[1][1] = block->matrix[2][1] = block->matrix[2][0] = true;
		break;
	case 5:
		block->matrix[0][1] = block->matrix[0][2] = block->matrix[1][0] = block->matrix[1][1] = true;
		break;
	case 6:
		block->matrix[0][0] = block->matrix[0][1] = block->matrix[1][1] = block->matrix[1][2] = true;
		break;
	}
	block->row = -1;
	block->col = 10;
	int kef = rand() % 10;
	block->color = colors[kef];
	return kef;
}
void Rotate(Block* block) {
	bool tempMatrix[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			tempMatrix[i][j] = block->matrix[i][j];
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			block->matrix[j][3 - i] = tempMatrix[i][j];
		}
	}
}
int Menu(Mix_Chunk* Sound, TTF_Font* my_font,FILE*f)
{
	char text[50];
	while (fgets(text, 50, f))
	{
		text[strcspn(text, "\n")] = '\0';
		if (strcmp(text, "game control:") == 0)
			control = atoi(fgets(text, 50, f));
		if (strcmp(text, "music:") == 0)
			music = atoi(fgets(text, 50, f));
		if (strcmp(text, "sound:") == 0)
			sound = atoi(fgets(text, 50, f));
	}
	Mix_VolumeChunk(Sound, sound / 100.0 * MIX_MAX_VOLUME);
	Mix_VolumeMusic(music / 100.0 * MIX_MAX_VOLUME);
	SDL_Rect Rect[3];
	SDL_Event event;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_Color textColor = { 255,0,0,0 };
	RenderText("TetriX", textColor, 850, 200, my_font);
	textColor = { 255,255,255,0 };
	Rect[0] = RenderText("Game", textColor, 850, 500, my_font);
	Rect[1] = RenderText("Settings", textColor, 850, 650, my_font);
	Rect[2] = RenderText("Exit", textColor, 850, 800, my_font);
	int mouse_x;
	int mouse_y;
	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	if (mouse_x >= 850 && mouse_x <= 850 + Rect[0].w && mouse_y >= 500 && mouse_y <= 500 + Rect[0].h)
	{
		textColor = { 128,128,128,0 };
		Rect[0] = RenderText("Game", textColor, 850, 500, my_font);
	}
	if (mouse_x >= 850 && mouse_x <= 850 + Rect[1].w && mouse_y >= 650 && mouse_y <= 650 + Rect[1].h)
	{
		textColor = { 128,128,128,0 };
		Rect[1] = RenderText("Settings", textColor, 850, 650, my_font);
	}
	if (mouse_x >= 850 && mouse_x <= 850 + Rect[2].w && mouse_y >= 800 && mouse_y <= 800 + Rect[2].h)
	{
		textColor = { 128,128,128,0 };
		Rect[2] = RenderText("Exit", textColor, 850, 800, my_font);
	}
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				int x = event.button.x;
				int y = event.button.y;
				if (x >= 850 && x <= 850+Rect[0].w && y >= 500 && y <= 500+ Rect[0].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					return 3;
				}
				if (x >= 850 && x <= 850 + Rect[1].w && y >= 650 && y <= 650 + Rect[1].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					return 2;
				}
				if (x >= 850 && x <= 850+ Rect[2].w && y >= 800 && y <= 800+ Rect[2].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					return 0;
				}
			}
		}
	}
	SDL_RenderPresent(renderer);
	return 1;
}
int Settings(Mix_Chunk* Sound, TTF_Font* my_font,FILE*f)
{
	SDL_Rect rect[13];
	char text[50];
	while (fgets(text, 50, f))
	{
		text[strcspn(text, "\n")] = '\0';
		if (strcmp(text, "game control:") == 0)
			control = atoi(fgets(text, 50, f));
		if (strcmp(text, "music:") == 0)
			music= atoi(fgets(text, 50, f));
		if (strcmp(text, "sound:") == 0)
			sound = atoi(fgets(text, 50, f));
	}
	SDL_Event event;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_Color textColor = { 255,255,255,0 };
	rect[0] = RenderText("Back", textColor, 1550, 880, my_font);
	rect[1]=RenderText("Game control:", textColor, 400, 300, my_font);
	rect[2]=RenderText("Sound:", textColor, 400, 550, my_font);
	rect[3]=RenderText("Music:", textColor, 400, 700, my_font);
	rect[4] = RenderTexture("left.bmp", 450 + rect[1].w, 300);
	rect[5]= RenderTexture("left.bmp", 450 + rect[2].w, 550);
	rect[6] = RenderTexture("left.bmp", 450 + rect[3].w, 700);
	if (control == 1)
		rect[7] = RenderText("Mouse", textColor, 480 + rect[1].w + rect[4].w, 300,my_font);
	if (control==2)
		rect[7] = RenderText("Keyboard", textColor, 480 + rect[1].w + rect[4].w, 300,my_font);
	_itoa_s(sound, text, 50, 10);
	rect[8] = RenderText(text, textColor, 480 + rect[2].w + rect[5].w, 550, my_font);
	_itoa_s(music, text, 50, 10);
	rect[9] = RenderText(text, textColor, 480 + rect[3].w + rect[6].w, 700, my_font);
	rect[10] = RenderTexture("right.bmp", 530 + rect[1].w + rect[4].w + rect[7].w, 300);
	rect[11] = RenderTexture("right.bmp", 530 + rect[2].w + rect[5].w + rect[8].w, 550);
	rect[12] = RenderTexture("right.bmp", 530 + rect[3].w + rect[6].w + rect[9].w, 700);
	int mouse_x;
	int mouse_y;
	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	if (mouse_x >= 1550 && mouse_x <= 1550 + rect[0].w && mouse_y >= 880 && mouse_y <= 880 + rect[0].h)
	{
		textColor = { 128,128,128,0 };
		rect[0] = RenderText("Back", textColor, 1550, 880, my_font);
	}
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				int x = event.button.x;
				int y = event.button.y;
				if (x >= 1550 && x <= 1550 + rect[0].w && y >= 880 && y <= 880 + rect[0].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					return 1;
				}
				if ((x >= 450 + rect[1].w && x <= 450 + rect[1].w + rect[4].w && y >= 300 && y <= 300 + rect[4].h) || (x >= 530 + rect[1].w + rect[4].w + rect[7].w && x <= 530 + rect[1].w + rect[4].w + rect[7].w + rect[10].w && y >= 300 && y <= 300 + rect[10].h))
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (control == 2)
						control = 1;
					else
						control = 2;
				}
				if (x >= 450 + rect[2].w && x <= 450 + rect[2].w + rect[5].w && y >= 550 && y <= 550 + rect[5].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (sound-5>=0)
						sound-=5;
					Mix_VolumeChunk(Sound, sound / 100.0 * MIX_MAX_VOLUME);
				}
				if (x >= 530 + rect[2].w + rect[5].w + rect[8].w && x <= 530 + rect[2].w + rect[5].w + rect[8].w + rect[11].w && y >= 550 && y <= 550 + rect[11].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (sound + 5 <= 100)
						sound += 5;
					Mix_VolumeChunk(Sound, sound / 100.0 * MIX_MAX_VOLUME);
				}
				if (x >= 450 + rect[3].w && x <= 450 + rect[3].w + rect[6].w && y >= 700 && y <= 700 + rect[6].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (music - 5 >= 0)
						music -= 5;
					Mix_VolumeMusic(music / 100.0 * MIX_MAX_VOLUME);
				}
				if (x >= 530 + rect[3].w + rect[6].w + rect[9].w && x <= 530 + rect[3].w + rect[6].w + rect[9].w + rect[12].w && y >= 700 && y <= 700 + rect[12].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (music + 5 <= 100)
						music += 5;
					Mix_VolumeMusic(music / 100.0 * MIX_MAX_VOLUME);
				}
			}
		}
	}
	fseek(f, 0, SEEK_SET);
	fprintf(f, "game control:\n%d\n", control);
	fprintf(f, "music:\n%d\n", music);
	fprintf(f, "sound:\n%d\n", sound);
	SDL_RenderPresent(renderer);
	return 2;
}
int GameMode(Mix_Chunk* Sound, TTF_Font* my_font)
{
	SDL_Rect Rect[4];
	SDL_Event event;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_Color textColor = { 255,255,255,0 };
	RenderText("Choose game mode", textColor, 700, 200, my_font);
	Rect[0]= RenderText("Classic", textColor, 800, 400, my_font);
	Rect[1]= RenderText("With timer", textColor, 800, 550, my_font);
	Rect[2]= RenderText("Accelerated", textColor, 800, 700, my_font);
	Rect[3] = RenderText("Back", textColor, 1550, 880, my_font);
	int mouse_x;
	int mouse_y;
	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	if (mouse_x >= 1550 && mouse_x <= 1550 + Rect[3].w && mouse_y >= 880 && mouse_y <= 880 + Rect[3].h)
	{
		textColor = { 128,128,128,0 };
		Rect[3] = RenderText("Back", textColor, 1550, 880, my_font);
	}
	if (mouse_x >= 800 && mouse_x <= 800 + Rect[0].w && mouse_y >= 400 && mouse_y <= 400 + Rect[0].h)
	{
		textColor = { 128,128,128,0 };
		Rect[0] = RenderText("Classic", textColor, 800, 400, my_font);
	}
	if (mouse_x >= 800 && mouse_x <= 800 + Rect[1].w && mouse_y >= 550 && mouse_y <= 550 + Rect[1].h)
	{
		textColor = { 128,128,128,0 };
		Rect[1] = RenderText("With timer", textColor, 800, 550, my_font);
	}
	if (mouse_x >= 800 && mouse_x <= 800 + Rect[2].w && mouse_y >= 700 && mouse_y <= 700 + Rect[2].h)
	{
		textColor = { 128,128,128,0 };
		Rect[2] = RenderText("Accelerated", textColor, 800, 700, my_font);
	}
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				int x = event.button.x;
				int y = event.button.y;
				if (x >= 1550 && x <= 1550 + Rect[3].w && y >= 880 && y <= 880 + Rect[3].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					return 1;
				}
				if (x >= 800 && x <= 800 + Rect[0].w && y >= 400 && y <= 400 + Rect[0].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					return 5;
				}
				if (x >= 800 && x <= 800 + Rect[1].w && y >= 550 && y <= 550 + Rect[1].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					return 4;
				}
				if (x >= 800 && x <= 800 + Rect[2].w && y >= 700 && y <= 700 + Rect[2].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					return 6;
				}
			}
		}
	}
	SDL_RenderPresent(renderer);
	return 3;
}
int Time(Mix_Chunk* Sound, TTF_Font* my_font)
{
	char text[50];
	SDL_Rect rect[5];
	SDL_Event event;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_Color textColor = { 255,255,255,0 };
	RenderText("Choose time", textColor, 800, 200, my_font);
	rect[0] = RenderTexture("left.bmp", 800, 500);
	_itoa_s(timer, text, 50, 10);
	rect[1] = RenderText(text, textColor, 850 + rect[0].w, 500, my_font);
	rect[2] = RenderTexture("right.bmp", 900 + rect[0].w + rect[1].w, 500);
	rect[3] = RenderText("Next", textColor, 900, 800, my_font);
	rect[4] = RenderText("Back", textColor, 1550, 880, my_font);
	int mouse_x;
	int mouse_y;
	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	if (mouse_x >= 900 && mouse_x <= 900 + rect[3].w && mouse_y >= 800 && mouse_y <= 800 + rect[3].h)
	{
		textColor = { 128,128,128,0 };
		rect[3] = RenderText("Next", textColor, 900, 800, my_font);
	}
	if (mouse_x >= 1550 && mouse_x <= 1550 + rect[4].w && mouse_y >= 880 && mouse_y <= 880 + rect[4].h)
	{
		textColor = { 128,128,128,0 };
		rect[4] = RenderText("Back", textColor, 1550, 880, my_font);
	}
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				int x = event.button.x;
				int y = event.button.y;
				if (x >= 900 && x <= 900 + rect[3].w && y >= 800 && y <= 800 + rect[3].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (timer != 0)
					{
						switch (timer)
						{
						case 2:koef += 0.35; break;
						case 3:koef += 0.3; break;
						case 4:koef += 0.25; break;
						case 5:koef += 0.2; break;
						case 6:koef += 0.15; break;
						case 7:koef += 0.1; break;
						case 8:koef += 0.05; break;
						}
						return 5;
					}
				}
				if (x >= 1550 && x <= 1550 + rect[4].w && y >= 880 && y <= 880 + rect[4].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					timer = 0;
					return 3;
				}
				if (x >= 800 && x <= 800 + rect[0].w && y >= 500 && y <= 500 + rect[0].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (timer - 1 >= 2)
						timer--;
				}
				if (x >= 900 + rect[0].w + rect[1].w && x <= 900 + rect[0].w + rect[1].w + rect[2].w && y >= 500 && y <= 500 + rect[2].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (timer == 0)
						timer += 2;
					else if (timer + 1 <= 9)
						timer++;
				}
			}
		}
	}
	SDL_RenderPresent(renderer);
	return 4;
}
int Speed(Mix_Chunk* Sound, TTF_Font* my_font)
{
	char text[50];
	SDL_Rect rect[5];
	SDL_Event event;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_Color textColor = { 255,255,255,0 };
	RenderText("Choose speed", textColor, 800, 200, my_font);
	rect[0] = RenderTexture("left.bmp", 750, 500);
	switch (speed)
	{
	case 0: strcpy_s(text, "None"); break;
	case 1:strcpy_s(text, "Slow"); break;
	case 2:strcpy_s(text, "Basic"); break;
	case 3:strcpy_s(text, "Fast"); break;
	case 4:strcpy_s(text, "Very fast"); break;
	}
	rect[1] = RenderText(text, textColor, 800 + rect[0].w, 500, my_font);
	rect[2] = RenderTexture("right.bmp", 850 + rect[0].w + rect[1].w, 500);
	rect[3] = RenderText("Next", textColor, 900, 800, my_font);
	rect[4] = RenderText("Back", textColor, 1550, 880, my_font);
	int mouse_x;
	int mouse_y;
	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	if (mouse_x >= 900 && mouse_x <= 900 + rect[3].w && mouse_y >= 800 && mouse_y <= 800 + rect[3].h)
	{
		textColor = { 128,128,128,0 };
		rect[3] = RenderText("Next", textColor, 900, 800, my_font);
	}
	if (mouse_x >= 1550 && mouse_x <= 1550 + rect[4].w && mouse_y >= 880 && mouse_y <= 880 + rect[4].h)
	{
		textColor = { 128,128,128,0 };
		rect[4] = RenderText("Back", textColor, 1550, 880, my_font);
	}
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				int x = event.button.x;
				int y = event.button.y;
				if (x >= 750 && x <= 750 + rect[0].w && y >= 500 && y <= 500 + rect[0].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (speed - 1 >= 1)
						speed--;
				}
				if (x >= 850 + rect[0].w + rect[1].w && x <= 850 + rect[0].w + rect[1].w + rect[2].w && y >= 500 && y <= 500 + rect[2].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (speed + 1 <= 4)
						speed++;
				}
				if (x >= 900 && x <= 900 + rect[3].w && y >= 800 && y <= 800 + rect[3].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (speed != 0)
					{
						switch (speed)
						{
						case 1:koef -= 0.2; break;
						case 3:koef += 0.1; break;
						case 4:koef += 0.3; break;
						}
						return 6;
					}
				}
				if (x >= 1550 && x <= 1550 + rect[4].w && y >= 880 && y <= 880 + rect[4].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					speed = 0;
					if (timer == 0)
						return 3;
					else
					{
						switch (timer)
						{
						case 2:koef -= 0.35; break;
						case 3:koef -= 0.3; break;
						case 4:koef -= 0.25; break;
						case 5:koef -= 0.2; break;
						case 6:koef -= 0.15; break;
						case 7:koef -= 0.1; break;
						case 8:koef -= 0.05; break;
						}
						return 4;
					}
				}
			}
		}
	}
	SDL_RenderPresent(renderer);
	return 5;
}
int Blocks(Mix_Chunk* Sound, TTF_Font* my_font)
{
	char text[50];
	SDL_Rect rect[5];
	SDL_Event event;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_Color textColor = { 255,255,255,0 };
	RenderText("Choose number of rows", textColor, 600, 200, my_font);
	rect[0]= RenderTexture("left.bmp", 800, 500);
	_itoa_s(blocks, text, 50, 10);
	rect[1] = RenderText(text, textColor, 850 + rect[0].w, 500, my_font);
	rect[2] = RenderTexture("right.bmp", 900 + rect[0].w + rect[1].w, 500);
	rect[3] = RenderText("Play", textColor, 900, 800, my_font);
	rect[4] = RenderText("Back", textColor, 1550, 880, my_font);
	int mouse_x;
	int mouse_y;
	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	if (mouse_x >= 900 && mouse_x <= 900 + rect[3].w && mouse_y >= 800 && mouse_y <= 800 + rect[3].h)
	{
		textColor = { 128,128,128,0 };
		rect[3] = RenderText("Play", textColor, 900, 800, my_font);
	}
	if (mouse_x >= 1550 && mouse_x <= 1550 + rect[4].w && mouse_y >= 880 && mouse_y <= 880 + rect[4].h)
	{
		textColor = { 128,128,128,0 };
		rect[4] = RenderText("Back", textColor, 1550, 880, my_font);
	}
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				int x = event.button.x;
				int y = event.button.y;
				if (x >= 800 && x <= 800 + rect[0].w && y >= 500 && y <= 500 + rect[0].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (blocks - 1 >= 1)
						blocks--;
				}
				if (x >= 900 + rect[0].w + rect[1].w && x <= 900 + rect[0].w + rect[1].w + rect[2].w && y >= 500 && y <= 500 + rect[2].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (blocks + 1 <= 6)
						blocks++;
				}
				if (x >= 900 && x <= 900 + rect[3].w && y >= 800 && y <= 800 + rect[3].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					if (blocks != 0)
					{
						switch (blocks)
						{
						case 1:koef -= 0.25; break;
						case 2:koef -= 0.2; break;
						case 3:koef -= 0.1; break;
						case 4:koef -= 0.05; break;
						case 6:koef += 0.05; break;
						}
						return 7;
					}
				}
				if (x >= 1550 && x <= 1550 + rect[4].w && y >= 880 && y <= 880 + rect[4].h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					blocks = 0;
					if (speed == 0)
						return 3;
					else
					{
						switch (speed)
						{
						case 1:koef += 0.2; break;
						case 3:koef -= 0.1; break;
						case 4:koef -= 0.3; break;
						}
						return 5;
					}
				}
			}
		}
	}
	SDL_RenderPresent(renderer);
	return 6;
}
int Game(TTF_Font* my_font)
{
	Uint32 startTime;
	Uint32 currentTime;
	int gameTime;
	if (timer != 0)
	{
		gameTime = 60 * timer;
		startTime = SDL_GetTicks();
	}
	SDL_Color colors[10];
	colors[0] = { 128,128,128,0 };
	colors[1] = { 255,0,0,0 };
	colors[2] = { 0,255,0,0 };
	colors[3] = { 0,0,255,0 };
	colors[4] = { 255,255,0,0 };
	colors[5] = { 255,0,255,0 };
	colors[6] = { 0,255,255,0 };
	colors[7] = { 139,0,255,0 };
	colors[8] = { 155,45,48,0 };
	colors[9] = { 255,165,0,0 };
	SDL_Rect block = { 365,650,SIZE,SIZE };
	char text[50];
	char txt[50] = "Score:";
	SDL_Color textColor = { 0,0,0,0 };
	Mix_Chunk* Sound1 = Mix_LoadWAV("video-game-vintage-jump-ascend_zkbs6f4_.wav");
	Mix_Chunk* Sound2 = Mix_LoadWAV("leftfoot8bit.wav");
	Mix_Chunk* Sound3 = Mix_LoadWAV("boom3.wav");
	Mix_VolumeChunk(Sound1, sound / 100.0 * MIX_MAX_VOLUME);
	Mix_VolumeChunk(Sound2, sound / 100.0 * MIX_MAX_VOLUME);
	Mix_VolumeChunk(Sound3, sound / 100.0 * MIX_MAX_VOLUME);
	SDL_Event event;
	int tetrix[10][20];
	SDL_Rect location[10][20];
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
		{
			tetrix[i][j] = -1;
			location[i][j] = block;
			if (j == 19)
			{
				block.x = 365;
				block.y -= 60;
			}
			else
				block.x += 60;
		}
	for (int i = 0; i < blocks; i++)
		for (int j = 0; j < 20; j++)
			tetrix[i][j] = rand() % 9 + 1;
	int col;
	Block glock;
	int cola;
	cola = Generate(&glock, colors);
	while (1)
	{
		bool voed = true;
		if (timer != 0)
		{
			currentTime = SDL_GetTicks();
			Uint32 elapsedTime = (currentTime - startTime) / 1000;
			if (elapsedTime >= gameTime)
			{
				koef -= 0.1;
				wose = 2;
				return 8;
			}
		}
		if (tetrix[9][10] != -1)
		{
			koef -= 0.1;
			wose = 0;
			return 8;
		}
		for (int i = 0; i < 10; i++)
			for (int j = 0; j < 20; j++)
				if (tetrix[i][j] != -1)
					voed = false;
		if (voed)
		{
			koef += 0.1;
			wose = 1;
			return 8;
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
		SDL_Rect Rect = { 0,0,355,HEIGHT };
		SDL_RenderFillRect(renderer, &Rect);
		Rect.x = 1565;
		SDL_RenderFillRect(renderer, &Rect);
		Rect.y = 700;
		Rect.x = 355;
		Rect.w = 1210;
		Rect.h = HEIGHT - 700;
		SDL_RenderFillRect(renderer, &Rect);
		_itoa_s(score, text, 50, 10);
		strcat_s(txt, text);
		RenderText(txt, textColor, 850, 800, my_font);
		strcpy_s(txt, "Score:");
		Rect.y = 0;
		Rect.x = 330;
		Rect.w = 25;
		Rect.h = 725;
		SDL_SetRenderDrawColor(renderer, 255, 192, 203, 0);
		SDL_RenderFillRect(renderer, &Rect);
		Rect.x = 1565;
		SDL_RenderFillRect(renderer, &Rect);
		Rect.y = 700;
		Rect.x = 355;
		Rect.w = 1210;
		Rect.h = 25;
		SDL_RenderFillRect(renderer, &Rect);
		for (int i = 0; i < 10; i++)
			for (int j = 0; j < 20; j++)
			{
				if (tetrix[i][j] != -1)
				{
					SDL_SetRenderDrawColor(renderer, colors[tetrix[i][j]].r, colors[tetrix[i][j]].g, colors[tetrix[i][j]].b, colors[tetrix[i][j]].a);
					SDL_RenderFillRect(renderer, &location[i][j]);
				}
			}
		while (SDL_PollEvent(&event))
		{
			if (control == 1)
				if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					if (event.button.button == SDL_BUTTON_LEFT) {
						Mix_PlayChannel(-1, Sound1, 0);
						glock.col--;
						int flag = 0;
						for (int i = 0; i < 4; i++) {
							for (int j = 0; j < 4; j++) {
								if (glock.matrix[i][j])
								{
									int fl = 0;
									int x1 = 365 + (glock.col * (SIZE + 10)) + j * (SIZE + 10);
									int y1 = (glock.row * (SIZE + 10)) + i * (SIZE + 10) - 10;
									for (int k = 9; k > -1; k--)
										for (int m = 19; m > -1; m--)
											if (x1 == location[k][m].x && y1 == location[k][m].y && tetrix[k][m] == -1)
												fl = 1;
									if (fl == 0)
										flag = 1;
								}
							}
						}
						if (flag == 1)
							glock.col++;
					}
					if (event.button.button == SDL_BUTTON_RIGHT) {
						Mix_PlayChannel(-1, Sound1, 0);
						glock.col++;
						int flag = 0;
						for (int i = 0; i < 4; i++) {
							for (int j = 0; j < 4; j++) {
								if (glock.matrix[i][j])
								{
									int fl = 0;
									int x1 = 365 + (glock.col * (SIZE + 10)) + j * (SIZE + 10);
									int y1 = (glock.row * (SIZE + 10)) + i * (SIZE + 10) - 10;
									for (int k = 9; k > -1; k--)
										for (int m = 19; m > -1; m--)
											if (x1 == location[k][m].x && y1 == location[k][m].y && tetrix[k][m] == -1)
												fl = 1;
									if (fl == 0)
										flag = 1;
								}
							}
						}
						if (flag == 1)
							glock.col--;
					}
					if (event.button.button == SDL_BUTTON_MIDDLE) {
						Mix_PlayChannel(-1, Sound1, 0);
						Block rotated = glock;
						Rotate(&rotated);
						int flag = 0;
						for (int i = 0; i < 4; i++) {
							for (int j = 0; j < 4; j++) {
								if (rotated.matrix[i][j])
								{
									int fl = 0;
									int x1 = 365 + (rotated.col * (SIZE + 10)) + j * (SIZE + 10);
									int y1 = (rotated.row * (SIZE + 10)) + i * (SIZE + 10) - 10;
									for (int k = 9; k > -1; k--)
										for (int m = 19; m > -1; m--)
											if (x1 == location[k][m].x && y1 == location[k][m].y && tetrix[k][m] == -1)
												fl = 1;
									if (fl == 0)
										flag = 1;
								}
							}
						}
						if (flag == 0)
							Rotate(&glock);
					}
				}
			if (control == 2)
				if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_LEFT)
					{
						Mix_PlayChannel(-1, Sound1, 0);
						glock.col--;
						int flag = 0;
						for (int i = 0; i < 4; i++) {
							for (int j = 0; j < 4; j++) {
								if (glock.matrix[i][j])
								{
									int fl = 0;
									int x1 = 365 + (glock.col * (SIZE + 10)) + j * (SIZE + 10);
									int y1 = (glock.row * (SIZE + 10)) + i * (SIZE + 10) - 10;
									for (int k = 9; k > -1; k--)
										for (int m = 19; m > -1; m--)
											if (x1 == location[k][m].x && y1 == location[k][m].y && tetrix[k][m] == -1)
												fl = 1;
									if (fl == 0)
										flag = 1;
								}
							}
						}
						if (flag == 1)
							glock.col++;
					}
					if (event.key.keysym.sym == SDLK_RIGHT)
					{
						Mix_PlayChannel(-1, Sound1, 0);
						glock.col++;
						int flag = 0;
						for (int i = 0; i < 4; i++) {
							for (int j = 0; j < 4; j++) {
								if (glock.matrix[i][j])
								{
									int fl = 0;
									int x1 = 365 + (glock.col * (SIZE + 10)) + j * (SIZE + 10);
									int y1 = (glock.row * (SIZE + 10)) + i * (SIZE + 10) - 10;
									for (int k = 9; k > -1; k--)
										for (int m = 19; m > -1; m--)
											if (x1 == location[k][m].x && y1 == location[k][m].y && tetrix[k][m] == -1)
												fl = 1;
									if (fl == 0)
										flag = 1;
								}
							}
						}
						if (flag == 1)
							glock.col--;
					}
					if (event.key.keysym.sym == SDLK_SPACE)
					{
						Mix_PlayChannel(-1, Sound1, 0);
						Block rotated = glock;
						Rotate(&rotated);
						int flag = 0;
						for (int i = 0; i < 4; i++) {
							for (int j = 0; j < 4; j++) {
								if (rotated.matrix[i][j])
								{
									int fl = 0;
									int x1 = 365 + (rotated.col * (SIZE + 10)) + j * (SIZE + 10);
									int y1 = (rotated.row * (SIZE + 10)) + i * (SIZE + 10) - 10;
									for (int k = 9; k > -1; k--)
										for (int m = 19; m > -1; m--)
											if (x1 == location[k][m].x && y1 == location[k][m].y && tetrix[k][m] == -1)
												fl = 1;
									if (fl == 0)
										flag = 1;
								}
							}
						}
						if (flag == 0)
							Rotate(&glock);
					}
				}
		}
		glock.row++;
		int flag = 0;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (glock.matrix[i][j])
				{
					int fl = 0;
					int x1 = 365 + (glock.col * (SIZE + 10)) + j * (SIZE + 10);
					int y1 = (glock.row * (SIZE + 10)) + i * (SIZE + 10) - 10;
					for (int k = 9; k > -1; k--)
						for (int m = 19; m > -1; m--)
							if (x1 == location[k][m].x && y1 == location[k][m].y && tetrix[k][m] == -1)
								fl = 1;
					if (fl == 0)
						flag = 1;
				}
			}
		}
		if (flag == 1 && glock.row>1)
		{
			if (mode == 1 && speed<=5)
				speed += 0.1;
			Mix_PlayChannel(-1, Sound2, 0);
			glock.row--;
			SDL_SetRenderDrawColor(renderer, glock.color.r, glock.color.g, glock.color.b, glock.color.a);
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (glock.matrix[i][j]) {
						int x = 365 + (glock.col * (SIZE + 10)) + j * (SIZE + 10);
						int y = (glock.row * (SIZE + 10)) + i * (SIZE + 10) - 10;
						SDL_Rect rect = { x, y, SIZE, SIZE };
						SDL_RenderFillRect(renderer, &rect);
					}
				}
			}
			int x1 = -1, x2 = -1, x3 = -1,x4=-1, y1 = -1, y2 = -1, y3 = -1,y4=-1;
			int flagg = 0;
			int killed = 0;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (glock.matrix[i][j]) {
						int x = 365 + (glock.col * (SIZE + 10)) + j * (SIZE + 10);
						int y = (glock.row * (SIZE + 10)) + i * (SIZE + 10) - 10;
						SDL_Rect rect = { x, y, SIZE, SIZE };
						for (int k = 9; k > -1; k--)
							for (int m = 19; m > -1; m--)
								if (location[k][m].x == rect.x && location[k][m].y == rect.y)
								{
									if (x1 == -1)
									{
										x1 = k;
										y1 = m;
									}
									else if (x2 == -1)
									{
										x2 = k;
										y2 = m;
									}
									else if (x3 == -1)
									{
										x3 = k;
										y3 = m;
									}
									else
									{
										x4 = k;
										y4 = m;
									}
									if (cola == 0)
									{
										flagg = 1;
										tetrix[k - 1][m] = -1;
										tetrix[k + 1][m] = -1;
										tetrix[k][m - 1] = -1;
										tetrix[k][m + 1] = -1;
									}
									else
									{
										if (tetrix[k - 1][m] == cola && k-1!=x1 && k - 1 != x2 && k - 1 != x3 && k - 1 != x4)
										{
											tetrix[k - 1][m] = -1;
											flagg = 1;
											killed++;
										}
										if (tetrix[k + 1][m] == cola && k + 1 != x1 && k + 1 != x2 && k + 1 != x3 && k + 1 != x4)
										{
											tetrix[k + 1][m] = -1;
											flagg = 1;
											killed++;
										}
										if (tetrix[k][m - 1] == cola && m - 1 != y1 && m - 1 != y2 && m - 1 != y3 && m - 1 != y4)
										{
											tetrix[k][m - 1] = -1;
											flagg = 1;
											killed++;
										}
										if (tetrix[k][m + 1] == cola && m + 1 != y1 && m + 1 != y2 && m + 1 != y3 && m + 1 != y4)
										{
											tetrix[k][m + 1] = -1;
											flagg = 1;
											killed++;
										}
									}
									if (flagg == 0)
										tetrix[k][m] = cola;
									else
									{
										tetrix[x1][y1] = -1;
										tetrix[x2][y2] = -1;
										tetrix[x3][y3] = -1;
										killed += 4;
										Mix_PlayChannel(-1, Sound3, 0);
									}
								}

					}
				}
			}
			if (killed > 8)
				score =score+ 50 + 10 * killed;
			else
				score =score+ 10 * killed;
			cola = Generate(&glock, colors);
		}
		SDL_SetRenderDrawColor(renderer, glock.color.r, glock.color.g, glock.color.b, glock.color.a);
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (glock.matrix[i][j]) {
					int x = 365 + (glock.col * (SIZE + 10)) + j * (SIZE + 10);
					int y = (glock.row * (SIZE + 10)) + i * (SIZE + 10) - 10;
					SDL_Rect rect = { x, y, SIZE, SIZE };
					SDL_RenderFillRect(renderer, &rect);
				}
			}
		}
		if (speed == 0)
		{
			speed = 1;
			mode = 1;
		}
		SDL_RenderPresent(renderer);
		SDL_Delay(500/speed);
	}
	return 7;
}
int End(Mix_Chunk* Sound, TTF_Font* my_font)
{
	char text[50];
	char bext[50];
	SDL_Rect rect;
	SDL_Event event;
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_Color textColor = { 255,255,255,0 };
	if (wose == 0)
		strcpy_s(text, "You lose!Score:");
	if (wose==1)
		strcpy_s(text, "You win!Score:");
	if (wose==2)
		strcpy_s(text, "Time out!Score:");
	_itoa_s(score*koef, bext, 50, 10);
	strcat_s(text, bext);
	speed = 0;
	blocks = 0;
	timer = 0;
	RenderText(text, textColor, 800, 500, my_font);
	rect = RenderText("Back", textColor, 1550, 880, my_font);
	int mouse_x;
	int mouse_y;
	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	if (mouse_x >= 1550 && mouse_x <= 1550 + rect.w && mouse_y >= 880 && mouse_y <= 880 + rect.h)
	{
		textColor = { 128,128,128,0 };
		rect = RenderText("Back", textColor, 1550, 880, my_font);
	}
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				int x = event.button.x;
				int y = event.button.y;
				if (x >= 1550 && x <= 1550 + rect.w && y >= 880 && y <= 880 + rect.h)
				{
					Mix_PlayChannel(-1, Sound, 0);
					return 1;
				}
			}
		}
	}
	SDL_RenderPresent(renderer);
	return 8;
}
int main(int argc, char** argv) 
{
	srand(time(NULL));
	setlocale(LC_ALL, "ru");
	if (!init()) {
		printf("Ошибка при инициализации!\n");
		return -1;
	}
	int menu = 1;
	Mix_Chunk* Sound = Mix_LoadWAV("sound.wav");
	Mix_Music* Music = Mix_LoadMUS("486695_Chaoz-Fantasy-8-Bit.mp3");
	TTF_Font* my_font = TTF_OpenFont("Gabriola.ttf", 100);
	bool quit = false;
	Mix_PlayMusic(Music, -1);
	FILE* f;
	fopen_s(&f, "safe.txt", "r+");
	while (!quit) {
			switch (menu)
			{
			case 1:menu=Menu(Sound,my_font,f); break;
			case 2:menu=Settings(Sound,my_font,f); break;
			case 3:menu=GameMode(Sound,my_font); break;
			case 4:menu = Time(Sound, my_font);break;
			case 5:menu = Speed(Sound, my_font); break;
			case 6:menu = Blocks(Sound, my_font); break;
			case 7:menu=Game(my_font); break;
			case 8:menu = End(Sound, my_font); break;
			case 0:quit = true; break;
			}
	}
	fclose(f);
	SDL_Delay(300);
	Mix_FreeMusic(Music);
	Mix_FreeChunk(Sound);
	TTF_CloseFont(my_font);
	close();
	return 0;
}
