#if 0
tcc -lX11 -lasound -DSTBI_NO_SIMD=1 main.c -o chihuahuaspin.tcc
./chihuahuaspin.tcc
exit 0
#endif

#include <stdio.h>

#include "fenster.h"
#include "font.h"
#include "spin.gif.h"

#define STBI_NO_STDIO
#define STBI_ONLY_GIF
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define W 600
#define H 500
static uint32_t buf[W * H];

static int spin_gif_width;
static int spin_gif_height;
static int spin_gif_frames;
static uint32_t *spin_gif_buffer = NULL;

void draw_spin(uint16_t x, uint16_t y, uint8_t frame) {
    // For each row
    for (uint32_t t_x = 0; t_x < spin_gif_width; t_x++) {

        // For each column
        for (uint32_t t_y = 0; t_y < spin_gif_height; t_y++) {

            // We fetch the pixel...
            uint32_t p = spin_gif_buffer[
                (frame * spin_gif_width * spin_gif_height) + // Of frame <frame>
                (t_y * spin_gif_width + t_x) // At t_x, t_y
            ];

            // We push the pixel!
            buf[(y+t_y) * W + (x+t_x)] = p;
        }
    }
}

uint32_t text_size(char *text) {
    uint16_t text_length = strlen(text);

    uint32_t x = 0;
    for (uint16_t c = 0; c < text_length; c++) {
        char code = text[c];

        // If it's a space, skip!
        if (code == ' ') {
            x += 16;
            continue;
        }

        // Try finding the character we want
        Character character;
        for (int i = 0; i < font_character_amount; i++) {
            character = font_characters[i];

            if (character.codePoint == code)
                break;
        }

        // Go on!
        x += character.width;
    }

    return x;
}

void draw_text(char *text, uint16_t x, uint16_t y) {
    uint16_t text_length = strlen(text);

    // For each character in the string...
    for (uint16_t c = 0; c < text_length; c++) {
        char code = text[c];

        // If it's a space, skip!
        if (code == ' ') {
            x += 16;
            continue;
        }

        // Try finding the character we want
        Character character;
        for (int i = 0; i < font_character_amount; i++) {
            character = font_characters[i];

            if (character.codePoint == code)
                break;
        }

        // For each row
        for (uint32_t t_x = 0; t_x < character.width; t_x++) {
            int32_t f_x = (x+t_x)-character.originX;

            // Check if out of bounds
            if (f_x < 0) continue;
            if (f_x > W) break;

            // For each column
            for (uint32_t t_y = 0; t_y < character.height; t_y++) {
                int32_t f_y = (y+t_y)-character.originY;

                // Check if out of bounds
                if (f_y < 0) continue;
                if (f_y > H) break;

                // Fetch the pixel
                uint8_t p = font_pixels[(character.y+t_y) * FONT_WIDTH + (character.x+t_x)];

                // Turns brightness (0-255) into an ARGB value (or RGBA, for that matter)
                buf[f_y * W + f_x] = (p << 24) | (p << 16) | (p << 8) | p;
            }
        }

        // We go on!
        x += character.width;
    }
}

int main() {
    struct fenster f = {
        .title = "chihuahuaspin.com",
        .width = W,
        .height = H,
        .buf = buf,
    };

    // If it's windows, then change the name.
    #ifdef __WIN32
    f.title = "chihuahuaspin.exe";
    #endif

    // Loads a GIF into a buffer
    spin_gif_buffer = (void *)stbi_load_gif_from_memory(
        chihuahuaspin_gif, sizeof(chihuahuaspin_gif), NULL,
        &spin_gif_width, &spin_gif_height, &spin_gif_frames, NULL, 4
    );

    // Translates all the pixels in all of the frames in that GIF from RGBA to ARGB (the format we use)
    for (int i = 0; i < spin_gif_width * spin_gif_height * spin_gif_frames; i++) {
        uint32_t p = spin_gif_buffer[i];
        uint8_t a = (p >> 24) & 0xFF;
        uint8_t b = (p >> 16) & 0xFF;
        uint8_t g = (p >> 8) & 0xFF;
        uint8_t r = p & 0xFF;
        spin_gif_buffer[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    // Opens up a Fenster context thing
    fenster_open(&f);

    uint64_t origin_time = fenster_time();

    // We make this an obviously non-zero value so the program will clear
    // the screen right after loading
    uint64_t previous_spin = 100;

    // Run for as long as the user doesn't press the close button in the window
    while (fenster_loop(&f) == 0) {
        uint64_t t = fenster_time() - origin_time;

        // We calculate which frame to display, since the GIF runs at 17 fps
        uint64_t frame = (float)(t/1000.0) * 17.0;

        // We calculate how many times has this GIF played
        uint64_t spin_counter = frame / spin_gif_frames;

        // If GIF has finished once or several times
        if (spin_counter != previous_spin) {
            // Clears the buffer so it's all white
            for (int i = 0; i < W * H; i++)
                buf[i] = 0xFFFFFFFF;

            // Here's where we'll store our text
            static char buffer[50];

            // We generate it
            if (spin_counter == 1) {
                memcpy(buffer, "You have sat through 1 spin!", 29);
            } else {
                snprintf(buffer, 50, "You have sat through %lu spins!", spin_counter);
            }

            // We render it in the middle
            uint32_t text_w = text_size(buffer);
            draw_text(buffer, (W/2)-(text_w/2), 50+spin_gif_height);

            // We do all of this so it won't need to clear the screen every frame
            // Yes, I am aware it's a bit of an exaggerated optimization, but, eh, it's fun lol

            previous_spin = spin_counter;
        }

        // Render the GIF in the top middle of the screen
        draw_spin((W/2) - (spin_gif_width/2), 20, frame % spin_gif_frames);

        // So we do not starve the compositor thread
        fenster_sleep(1);
    }

    // GOODBYE!
    fenster_close(&f);

    return 0;
}
