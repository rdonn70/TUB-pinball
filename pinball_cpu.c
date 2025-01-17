/*
Pinball Central Module v0.5
by Ryan Donnellan

"I have the blueprint to the catacombs." - Dracula, 2024

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <termios.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "led-matrix-c.h"

#define NUM_CHANNELS 4 // Channel 1: Music | Channel 2: Voice Over | Channel 3: Sound Effect 1 | Channel 4: Sound Effect 2.

bool debug_mode = 0;
bool start_button_pressed = 0;
float deposited_money = 0;
float price = 0.75; //set to desired price
int highscore = 0;
int score = 0;
int volume = 128; //Volume is a value defined to be between 0 and 128 based on MIX_MAX_VOLUME in SDL3_mixer.

char coin_door_port[128] = "";
char led_driver_port[128] = "";
char score_driver_port[128] = "";

// debug_mode, start_button_pressed, deposited_money, score, and volume are mutexed 
pthread_mutex_t debug_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t deposited_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t score_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t volume_mutex = PTHREAD_MUTEX_INITIALIZER;

struct termios tty;
struct RGBLedMatrixOptions options;
struct RGBLedMatrix *matrix;
struct LedFont *font;
struct LedCanvas *canvas;
struct LedCanvas *offscreen;

static Mix_Music *music = NULL;
Mix_Chunk *music_channel;
Mix_Chunk *voice_channel;
Mix_Chunk *sound1_channel;
Mix_Chunk *sound2_channel;

// matrix_init(): initializes the RGB matrix display.
void matrix_init() {
	memset(&options, 0, sizeof(options));
	options.rows = 32;
	options.cols = 64;
	options.chain_length = 1;
	options.parallel = 1;
	options.gpio_slowdown = 3;
	options.daemon = 1;
	options.drop_privileges = 1;
	options.hardware_mapping = 'adafruit-hat';

	matrix = led_matrix_create_from_options(&options);
	canvas = led_matrix_get_canvas(matrix);
	offscreen = led_matrix_create_offscreen_canvas(matrix);
}

// play_sound(): Takes a file path, the desired channel (0 = music, 1 = voice, 2/3 = sound effects), and desired loop count (-1 = infinite). Plays the sound.
void play_sound(const char sound_dir, int desired_channel, int loop_count) {
	if (desired_channel == 0) {
		Mix_FreeChunk(music_channel);
		music_channel = Mix_LoadWAV(sound_dir);
		Mix_PlayChannel(0, chunk, loop_count);
	} else if (desired_channel == 1) {
		Mix_FreeChunk(voice_channel);
		voice_channel = Mix_LoadWAV(sound_dir);
		Mix_PlayChannel(1, chunk, loop_count); 
	} else if (desired_channel == 2) {
		Mix_FreeChunk(sound1_channel);
		sound1_channel = Mix_LoadWAV(sound_dir);
		Mix_PlayChannel(2, chunk, loop_count);
	} else {
		Mix_FreeChunk(sound2_channel);
		sound2_channel = Mix_LoadWAV(sound_dir);
		Mix_PlayChannel(3, chunk, loop_count);
	}
}

// stop_sound(): Takes desired channel (0 = music, 1 = voice, 2/3 = sound effects), and the desired fade out time. Fades out the channel.
void stop_sound(int desired_channel, int fade_out_ms) {
    Mix_FadeOutChannel(desired_channel, fade_out_ms);
}

// stop_all_sound(): Stops all sound.
void stop_all_sound() {
	Mix_HaltChannel(-1);
}

// set_volume(): Takes desired channel (0 = music, 1 = voice, 2/3 = sound effects, -1 = ALL CHANNELS), and the desired volume (an integer from 0 to 128) where 0 is muted and 128 is loudest.
void set_volume(int desired_channel, int volume) {
	pthread_mutex_lock(&volume_mutex);
	Mix_Volume(desired_channel, volume);
	pthread_mutex_unlock(&volume_mutex);
}

// load_highscore(): Reads the highscore.txt file to get the current highscore on startup. The highscore is updated on the file when a game ends with a new highscore.
void load_highscore() {
	FILE *highscore_file = fopen("highscore.txt", "r");
	if (highscore_file != NULL) {
		fscanf(highscore_file, "%d", &highscore);
		fclose(highscore_file);
	} else {
		highscore = 0;
	}
}

// check_usb_port(): Takes a port to read, and sees if that port is one of the desired peripheral boards (coin door, led driver, or score driver).
void check_usb_port(char *port) {
	
	tcgetattr(port, &tty);

	// Configuring USBs - if this doesnt work im going to shoot myself
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag |= CS8;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag |= CREAD | CLOCAL;
	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO;
	tty.c_lflag &= ~ECHOE;
	tty.c_lflag &= ~ECHONL;
	tty.c_lflag &= ~ISIG;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
	tty.c_oflag &= ~OPOST;
	tty.c_oflag &= ~ONLCR;
	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);

	tcsetattr(port, TCSANOW, &tty);
	
	char read_buffer [9];
	int num_bytes = read(port, &read_buffer, sizeof(read_buffer));

    if (strcmp(read_buffer, "COIN_DOOR") == 0) {
        strcpy(coin_door_port, port);
    }else if (strcmp(read_buffer, "LED_DRIVE") == 0) {
        strcpy(led_driver_port, port);
    }else if (strcmp(read_buffer, "SCOR_DRIV") == 0) {
        strcpy(score_driver_port,port);
	}
    usleep(100000);
	memset(&read_buf, '\0', sizeof(read_buf));
	close(port);
}

// gif_display(): Standard version of display a gif. Takes in the gif_directory (folder of frames), the frame_count (number of frames in folder), and the loop_count.
void gif_display(const char* gif_directory, int frame_count, int loop_count) {
	char file_name[256];
	uint8_t image_buffer[6144]; // 3(RGB) * image width(64) * image height(32) = 6144
	int playback_count = 0;
	int width = 64;
	int height = 32;
	int frame_index;

    led_canvas_clear(canvas);
    
    while(playback_count < loop_count) {
        for (frame_index = 0; frame_index < frame_count; frame_index++) {
			snprintf(file_name, sizeof(file_name), "%s/%d.png", gif_directory, frame_index);
            SDL_Surface *surface = IMG_Load(file_name);
			
			int buffer_index = 0;
            for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					uint32_t pixel = ((uint32_t*)surface->pixels)[y * width + x];
					uint8_t r, g, b;
					SDL_GetRGB(pixel, surface->format, &r, &g, &b);
					image_buffer[buffer_index++] = r;
					image_buffer[buffer_index++] = g;
					image_buffer[buffer_index++] = b;
				}
			}
            set_image(offscreen, 0, 0, image_buffer, 6144, 64, 32, 0); //canvas, x offset, y offset, image_buffer, bytes in image buffer, image width, image height, BGR=0 (makes it interpreted as RGB).
            offscreen = led_matrix_swap_on_vsync(matrix, offscreen);
        playback_count++;
		}
	}
	led_canvas_clear(canvas);
	led_canvas_clear(offscreen);
	SDL_FreeSurface(surface);
}

// gif_display_music(): gif_display() but with music. Takes in the gif_directory (folder of frames), the frame_count (number of frames in folder), the loop_count and the music_dir.
void gif_display_music(const char gif_directory, int frame_count, int loop_count, const char music_dir) {
    char file_name[256];
	uint8_t image_buffer[6144]; // 3(RGB) * image width(64) * image height(32) = 6144
	int playback_count = 0;
	int width = 64;
	int height = 32;
	int frame_index;
	
	Mix_HaltChannel(0);
	Mix_FreeChunk(music_channel);
	music_channel = Mix_LoadWAV(sound_dir);
	Mix_PlayChannel(0, chunk, loop_count);
	
    led_canvas_clear(canvas);
    
    while(playback_count < loop_count) {
        for (frame_index = 0; frame_index < frame_count; frame_index++) {
			snprintf(file_name, sizeof(file_name), "%s/%d.png", gif_directory, frame_index);
            SDL_Surface *surface = IMG_Load(file_name);
			
			int buffer_index = 0;
            for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					uint32_t pixel = ((uint32_t*)surface->pixels)[y * width + x];
					uint8_t r, g, b;
					SDL_GetRGB(pixel, surface->format, &r, &g, &b);
					image_buffer[buffer_index++] = r;
					image_buffer[buffer_index++] = g;
					image_buffer[buffer_index++] = b;
				}
			}
            set_image(offscreen, 0, 0, image_buffer, 6144, 64, 32, 0); //canvas, x offset, y offset, image_buffer, bytes in image buffer, image width, image height, BGR=0 (makes it interpreted as RGB).
            offscreen = led_matrix_swap_on_vsync(matrix, offscreen);
        playback_count++;
		}
	}
	Mix_HaltChannel(0);
	led_canvas_clear(canvas);
	led_canvas_clear(offscreen);
	SDL_FreeSurface(surface);
}

//highscore_display(): Displays the current machine highscore. Takes the time the screen will display for (display_time).
void highscore_display(double display_time) {
	clock_t start, end;
	double time_used;

	char highscore_string;

	uint8_t color_r = 255;
	uint8_t color_g = 0;
	uint8_t color b = 0;
    
	sprintf(highscore_string, "%09d", highscore);
	
	led_canvas_clear(canvas);
	led_canvas_clear(offscreen);
	
	start = clock();
    while(1) {
		// to determine x coordinate: 64 = (2*x) + (6 * number of characters)... (where x = start of text).
		// to determine y coordinate (for 2 line): 32 = (3*x) + 14 ... (where first line start = x, second line start = (2*x) + 7).
		// example syntax for coordinates: x = 2, y = 6.
        draw_text(offscreen, font, 2, 6, color_r, color_g, color_b, "HIGHSCORE:", 0);
        draw_text(offscreen, font, 5, 19, color_r, color_g, color_b, highscore_string, 0);
        
		offscreen = led_matrix_swap_on_vsync(matrix, offscreen);
        end = clock();
		time_used = ((double) (end - start)) / CLOCKS_PER_SEC; // in seconds
        if(time_used >= display_time) {
            break
        } else if (time_used >= 1) {
            if(color_g <= 200) {
                color_g += 4;
            } else {
                color_g = 0;
			}
		}
	}
	led_canvas_clear(canvas);
	led_canvas_clear(offscreen);
}

// score_display(): Displays score. Takes the length of time to display, and the rgb of the text.
void score_display(double display_time, uint8_t color_r, uint8_t, color_g, uint8_t color_b) {
    clock_t start, end;
	double time_used;
	
	char highscore_string;
	
	led_canvas_clear(canvas);
	led_canvas_clear(offscreen);
	
    start = time.time()
    while(1) {
		sprintf(score_string, "%09d", score);
		// to determine x coordinate: 64 = (2*x) + (6 * number of characters)... (where x = start of text).
		// to determine y coordinate (for 2 line): 32 = (3*x) + 14 ... (where first line start = x, second line start = (2*x) + 7).
        draw_text(offscreen, font, 14, 7, color_r, color_g, color_b, "SCORE:", 0);
        draw_text(offscreen, font, 22, 22, color_r, color_g, color_b, score_string, 0);
		offscreen = led_matrix_swap_on_vsync(matrix, offscreen);
        end = clock();
		
		time_used = ((double) (end - start)) / CLOCKS_PER_SEC; // in seconds
		
        if (time_used >= display_time) {
            break
		}
	}
}

// two_row_text(): Displays two rows of centered text. Takes in two strings, the time to display it for and the RGB value of the texts.
void two_row_text(const char* text1, const char* text2, double display_time, uint8_t color_r, uint8_t color_g, uint8_t color_b) {
    clock_t start, end;
	double time_used;
	
	led_canvas_clear(canvas);
	led_canvas_clear(offscreen);
	
	int length_text1 = strlen(text1);
	int length_text2 = strlen(text2);
	
	int x_text1 = ((64 - (6 * length_text1)) / 2);
	int x_text2 = ((64 - (6 * length_text2)) / 2);
	
	start = clock();
    while(1) {
		// to determine x coordinate: 64 = (2*x) + (6 * number of characters)... (where x = start of text).
		// to determine y coordinate (for 2 line): 32 = (3*x) + 14 ... (where first line start = x, second line start = (2*x) + 7).
        draw_text(offscreen, font, x_text1, 7, color_r, color_g, color_b, text1, 0);
        draw_text(offscreen, font, x_text2, 22, color_r, color_g, color_b, text2, 0);
		offscreen = led_matrix_swap_on_vsync(matrix, offscreen);
        end = clock();
		
		time_used = ((double) (end - start)) / CLOCKS_PER_SEC; // in seconds
		
        if (time_used >= display_time) {
            break
		}
	}
}

// waiting_for_money_display(): Displays the current amount of coins inserted into the machine. Takes Display time and RGB.
void waiting_for_money_display(double display_time, uint8_t color_r, uint8_t color_g, uint8_t color_b) {
    clock_t start, end;
	double time_used;
	char[128] text1;
	char[128] text2;
	
	led_canvas_clear(canvas);
	led_canvas_clear(offscreen);
	
	pthread_mutex_lock(&deposited_mutex);
    if (price == 0) {
        text1 = "FREE PLAY!";
    } else if (deposited_money == 0) {
        text1 = "INSERT COINS";
    } else {
		snprintf(text1, sizeof(text1), "CREDIT: %.2f", deposited_money);
    }
	if (deposited_money >= price) {
        text2 = "PRESS START!";
    } else {
        snprintf(text2, sizeof(text1), "PRICE: %.2f", price);
    }
	pthread_mutex_unlock(&deposited_mutex);
	
	int length_text1 = strlen(text1);
	int length_text2 = strlen(text2);
	
	int x_text1 = ((64 - (6 * length_text1)) / 2);
	int x_text2 = ((64 - (6 * length_text2)) / 2);
	
	start = clock();
    while(1) {
		// to determine x coordinate: 64 = (2*x) + (6 * number of characters)... (where x = start of text).
		// to determine y coordinate (for 2 line): 32 = (3*x) + 14 ... (where first line start = x, second line start = (2*x) + 7).
        draw_text(offscreen, font, x_text1, 7, color_r, color_g, color_b, text1, 0);
        draw_text(offscreen, font, x_text2, 22, color_r, color_g, color_b, text2, 0);
		offscreen = led_matrix_swap_on_vsync(matrix, offscreen);
        end = clock();
		
		time_used = ((double) (end - start)) / CLOCKS_PER_SEC; // in seconds
		
        if (time_used >= display_time) {
            break
		}
	}
}

// dvd_text_effect(): Classic DVD bounce effect. Used during attract (idle) mode. Takes in a string, the time to display for and RGB values for initial text.
void dvd_text_effect(const char* text, double display_time, uint8_t color_r, uint8_t color_g, uint8_t color_b) {
    clock_t start, end;
	double time_used;
	int x = 0;
	int y = 5;
	int dirX = 1;
	int dirY = 1;
	int speed = 1;
	int text_len = strlen(text);
    int text_height = 5;
    int text_width = ((6 * text_len) - 2); // character width of 4 + (2 for between character spacing of 1 pixel) - 2 to get rid of start and end spaces
    int screen_width = 64;
	int screen_height = 32;
	
	led_canvas_clear(canvas);
	led_canvas_clear(offscreen);
	
	start = clock();
    while(1) {
        draw_text(offscreen, font, 14, 7, color_r, color_g, color_b, "SCORE:", 0);
        offscreen = led_matrix_swap_on_vsync(matrix, offscreen);

        if (((y + text_height) >= screen_height) or (y <= 0) {
            dirY = dirY * (-1);
			srand(time(NULL));
            color_r = ((rand() + 50) % 255);
			color_g = ((rand() + 50) % 255);
			color_b = ((rand() + 50) % 255);
		}
        if (((x + text_width) >= screen_width) or (x <= 0)) {
            dirX = dirX * (-1);
			srand(time(NULL));
            color_r = ((rand() + 50) % 255);
			color_g = ((rand() + 50) % 255);
			color_b = ((rand() + 50) % 255);
		}

        x += (dirX * speed);
        y += (dirY * speed);
        
        end = clock();
		time_used = ((double) (end - start)) / CLOCKS_PER_SEC; // in seconds
        if (time_used >= display_time) {
            break
		}
	}
}

// thanks_text(): Takes in Colors RGB, writes scrolling "thank you" text on the screen that scrolls from bottom to top.
void thanks_text(uint8_t color_r, uint8_t color_g, uint8_t color_b) {
	// there is, without a doubt, an incredibly more efficient way to do that, but i got wayyy too many things to work on right now.
	const char* text1_1 = "CREATED";
	const char* text1_2 = "BY";
	const char* text2_1 = "RYAN";
	const char* text2_2 = "DONNELLAN";
	const char* text3_1 = "SPECIAL";
	const char* text3_2 = "THANKS";
	const char* text4_1 = "FELICIA";
	const char* text4_2 = "LAI";
	const char* text5_1 = "PAUL";
	const char* text5_2 = "NIEVES";
	const char* text6_1 = "DEON";
	const char* text6_2 = "RIVERS";
	const char* text7_1 = "ALEXA";
	const char* text7_2 = "SCHUSTER"; 
	const char* text8_1 = "RHEA";
	const char* text8_2 = "VURGHESE";
	const char* text9_1 = "KYLE";
	const char* text9_2 = "WILT";

	int i;
	int ii;
	
	char text1[64];
	char text2[64];
	
    for (i = 0; i < 8; i++) { //9 entries for the text, so i < 8
        int pos = 37; // height + 5 pixels?
		if (i == 0) {
			text1 = text1_1;
			text2 = text1_2;
		} else if (i == 1) {
			text1 = text2_1;
			text2 = text2_2;
		} else if (i == 2) {
			text1 = text3_1;
			text2 = text3_2;
		} else if (i == 3) {
			text1 = text4_1;
			text2 = text4_2;
		} else if (i == 4) {
			text1 = text5_1;
			text2 = text5_2;
		} else if (i == 5) {
			text1 = text6_1;
			text2 = text6_2;
		} else if (i == 6) {
			text1 = text7_1;
			text2 = text7_2;
		} else if (i == 7) {
			text1 = text8_1;
			text2 = text8_2;
		} else {
			text1 = text9_1;
			text2 = text9_2;
		}

        for (ii = 0; ii < 37; ii++) { // for x in range screen_height + 5
			int x_text1 = ((64 - (6 * strlen(text1))) / 2);
			int x_text2 = ((64 - (6 * strlen(text2))) / 2);
			draw_text(offscreen, font, x_text1, pos, color_r, color_g, color_b, text1, 0);
			draw_text(offscreen, font, x_text2, (pos + 6), color_r, color_g, color_b, text2, 0);
            pos -= 1;
            if((pos + 6) < -5) { // if the last text block is offscreen (y = -5 pixels)? 
                pos = 37;
			}
			offscreen = led_matrix_swap_on_vsync(matrix, offscreen);
		}
	}
}

//WIP!!
// main_game(): Gameplay loop that handles displaying animations, playing sounds, and reading and manipulating sensor data while someone is playing the game.
void main_game() {
    int lives = 3;
    
    //gameplay logic
    
    if(lives == 0) {
        end_game();
	}
}

// end_game(): Loop to display animations and other graphics, and play sounds once the game is finished.
void end_game() {
    stop_all_sound();
    play_sound("/home/pinball/Desktop/Pinball/pinball_sounds/Air.wav", 0, -1);
    score_display(5, 255, 0, 0);

    if (score > highscore) {
		// const char* text1, const char* text2, double display_time, uint8_t color_r, uint8_t color_g, uint8_t color_b
        two_row_text("NEW", "HIGHSCORE!", 5, 255, 255, 0);
        highscore = score;
        FILE *highscore_file = fopen("highscore.txt", "w");
        fprintf(highscore_file, "%d", highscore);
        fclose(highscore_file);
	} else {
		two_row_text("GAME", "OVER!", 5, 255, 0, 0);
    }
	highscore_display(10);
    thanks_text(255, 0, 0);
    idle_loop();
}

// idle_loop(): Attract mode. Cycles through sounds, coins inserted, highscore, animations, etc.
void idle_loop() {
    stop_all_sound();
    
    while(1) {
        play_music("/home/pinball/Desktop/Pinball/pinball_sounds/Air.ogg", -1)
        pthread_mutex_lock(&deposited_mutex);
		if (deposited_money == 0 and price != 0) {
			pthread_mutex_unlock(&deposited_mutex);
            dvd_text_effect("INSERT COINS", 15, 0, 0, 255);
			play_sound("/home/pinball/Desktop/Pinball/pinball_sounds/Anthem.wav", 0, -1);
            gif_display("/home/pinball/Desktop/Pinball/startup/", 18, 1);
            sleep(1);
            stop_sound(0, 2000);
            sleep(2);
            play_sound("/home/pinball/Desktop/Pinball/pinball_sounds/Fixer.wav", 0, -1);
            waiting_for_money_display(20, 255, 0, 0);
            sleep(1);
            //gif_display(random.choice(game_gifs_list), random.randint(2, 5), 12) <- need to figure out a way to do this in C...
            stop_sound(0, 1000);
            sleep(1);
        } else if (deposited_money > 0 and price != 0) {
			pthread_mutex_unlock(&deposited_mutex);
            while(deposited_money > 0) {
                waiting_for_money_display(20, 255, 0, 0);
                sleep(1);
                gif_display("/home/pinball/Desktop/Pinball/startup/", 18, 1);
                sleep(1);
			}
            stop_sound(0, 1000);
            sleep(1);
		}
		pthread_mutex_lock(&deposited_mutex);
        if (deposited_money >= price or price == 0) {
            if (start_button_pressed == 1) {
                deposited_money = deposited_money - price;
				pthread_mutex_unlock(&deposited_mutex);
                start_button_pressed = 0;
                main_game();
			}
		}
	}
}

void* coindoor_thread(const char* port) {
    char read_buffer[9];
	int cd_text_read = 0;
	
	tcgetattr(port, &tty);

	// Configuring USB port - if this doesnt work im going to shoot myself
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag |= CS8;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag |= CREAD | CLOCAL;
	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO;
	tty.c_lflag &= ~ECHOE;
	tty.c_lflag &= ~ECHONL;
	tty.c_lflag &= ~ISIG;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
	tty.c_oflag &= ~OPOST;
	tty.c_oflag &= ~ONLCR;
	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);

	tcsetattr(port, TCSANOW, &tty);
	while (cd_text_read < 9) {
		cd_text_read += read(port, &read_buffer, sizeof(read_buffer)); //reads initial "COIN_DOOR" text.
	}
    while(1) {
		char read_data[4];
		int credit = 0;
		int volume_up = 0;
		int volume_down = 0;
		int debug = 0;
		int read_bytes = 0;
		while (read_bytes < 4) {
			read_bytes += read(port, &read_data, sizeof(read_data)); //reads actual data coming in.
		}
		credit = atoi(read_data[0]);
		volume_up = atoi(read_data[1]);
		volume_down = atoi(read_data[2]);
		debug = atoi(read_data[3]);
		
		if (credit == 1) {
			pthread_mutex_lock(&deposited_mutex);
            deposited_money += 0.25;
			pthread_mutex_unlock(&deposited_mutex);
		}
		if(volume_down == 1) {
			pthread_mutex_lock(&volume_mutex);
			if (volume > 0) {
				volume -= 2;
				Mix_Volume(-1, volume);
			}
			pthread_mutex_unlock(&volume_mutex);
		}
		if(volume_up == 1) {
			pthread_mutex_lock(&volume_mutex);
			if (volume < 128) {
				volume += 2;
				Mix_Volume(-1, volume);
			}
			pthread_mutex_unlock(&volume_mutex);
		}
		if(debug == 1) {
			pthread_mutex_lock(&debug_mutex);
			debug_mode = 1;
			pthread_mutex_unlock(&debug_mutex);
		}
	}
}

// MAIN CODE START.
int main(int argc, char **argv) {
    SDL_AudioSpec spec;
	pthread_t thread1;
	
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
	
	*font = load_font("/home/pinball/Desktop/Pinball/donnell2.bdf");
	
	matrix_init();
	load_highscore();
	
    check_usb_port("/dev/ttyUSB0");
	check_usb_port("/dev/ttyUSB1");
    check_usb_port("/dev/ttyUSB2");
	check_usb_port("/dev/ttyUSB3");

    sleep(1);
    pthread_create(&thread1, NULL, coindoor_thread, NULL);
    sleep(1);
	
	gif_display_music("/home/pinball/Desktop/Pinball/startup/", 18, 0, "startup.wav"); // need to replace 18 with the correct frame count
    gif_display_music("/home/pinball/Desktop/Pinball/forge_splash/", 12, 0, "forge_splash.wav"); //need to replace 12 with correct frame count
    play_sound("/home/pinball/Desktop/Pinball/pinball_sounds/Fixer.wav", 0, -1);
    thanks_text(255, 0, 0);
    stop_sound(0, 2000);
    
	pthread_mutex_lock(&debug_mutex);
    if (debug_mode == 1) {
		// TO DO: ADD DEBUG STUFF
        debug_mode = 0;
		pthread_mutex_unlock(&debug_mutex);
        end_game();
	} else {
		pthread_mutex_unlock(&debug_mutex);
        idle_loop();
	}
}