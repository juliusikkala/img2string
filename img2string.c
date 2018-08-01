/*
    Copyright 2018 Julius Ikkala

    This file is part of img2string.

    img2string is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    img2string is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with img2string.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <caca.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "extern/stb_image.h"
#define HELP 1
#define WIDTH 'w'
#define RATIO 'r'
#define GAMMA 'g'
#define DITHER 'd'
#define ANTIALIAS 'a'
#define PREVIEW 'p'

struct
{
    int width;
    float ratio;
    float gamma;
    const char* image;
    const char* dither;
    int antialias;
    int preview;
} options = { 80, 0.5f, 0.5f, NULL, "none", 0, 0 };

int parse_args(int argc, char** argv)
{
    int indexptr = 0;
    char* endptr = NULL;

    struct option longopts[] = {
        { "help", no_argument, NULL, HELP },
        { "width", required_argument, NULL, WIDTH },
        { "ratio", required_argument, NULL, RATIO },
        { "gamma", required_argument, NULL, GAMMA },
        { "dither", required_argument, NULL, DITHER },
        { "preview", no_argument, &options.preview, PREVIEW }
    };

    int val = 0;
    while((val = getopt_long(argc, argv, "w:r:g:d:ap", longopts, &indexptr)) != -1)
    {
        switch(val)
        {
        case WIDTH:
            options.width = strtoul(optarg, &endptr, 10);
            if(*endptr != 0) goto help_print;
            break;
        case RATIO:
            options.ratio = strtod(optarg, &endptr);
            if(*endptr != 0) goto help_print;
            break;
        case GAMMA:
            options.gamma = strtod(optarg, &endptr);
            if(*endptr != 0) goto help_print;
            break;
        case DITHER:
            if(
                strcmp(optarg, "none") != 0 &&
                strcmp(optarg, "ordered2") != 0 &&
                strcmp(optarg, "ordered4") != 0 &&
                strcmp(optarg, "ordered8") != 0 &&
                strcmp(optarg, "random") != 0 &&
                strcmp(optarg, "fstein") != 0
            ) goto help_print;
            options.dither = optarg;
            break;
        case ANTIALIAS:
            options.antialias = 1;
            break;
        case PREVIEW:
            options.preview = 1;
            break;
        case HELP:
            goto help_print;
        default: break;
        }
    }

    if(optind + 1 != argc)
    {
        goto help_print;
    }
    options.image = argv[optind];

    return 0;

help_print:
    printf(
        "Usage: %s [-w width] [-g gamma] [-d dither] [-r pixelratio] [-a] [-p] image\n"
        "dither can be one of the following:\n"
        "\tnone (default)\n"
        "\tordered2\n"
        "\tordered4\n"
        "\tordered8\n"
        "\trandom\n"
        "\tfstein\n"
        "pixelratio is the aspect ratio of a pixel.\n"
        "-a enables antialiasing.\n",
        argv[0]
    );
return 1;
}

int caca_to_ansi(enum caca_color color)
{
    switch(color)
    {
    case CACA_BLACK: return 30;
    case CACA_BLUE: return 34;
    case CACA_GREEN: return 32;
    case CACA_CYAN: return 36;
    case CACA_RED: return 31;
    case CACA_MAGENTA: return 35;
    case CACA_BROWN: return 33;
    case CACA_LIGHTGRAY: return 37;
    case CACA_DARKGRAY: return 90;
    case CACA_LIGHTBLUE: return 94;
    case CACA_LIGHTGREEN: return 92;
    case CACA_LIGHTCYAN: return 96;
    case CACA_LIGHTRED: return 91;
    case CACA_LIGHTMAGENTA: return 95;
    case CACA_YELLOW: return 93;
    case CACA_WHITE: return 97;
    case CACA_DEFAULT: return 0;
    default:
    case CACA_TRANSPARENT: return 0;
    };
}

void print_canvas(caca_canvas_t* cv)
{
    int w = caca_get_canvas_width(cv);
    int h = caca_get_canvas_height(cv);

    printf("\x1b[0m");
    for(int y= 0; y < h; ++y)
    {
        uint32_t prev_a = 0;
        for(int x = 0; x < w; ++x)
        {
            char c = caca_get_char(cv, x, y);
            uint32_t a = caca_get_attr(cv, x, y);
            if(a != prev_a)
            {
                uint8_t fg = caca_attr_to_ansi_fg(a);
                uint8_t bg = caca_attr_to_ansi_bg(a);

                if(bg == CACA_TRANSPARENT)
                {
                    printf("\x1b[0m");
                    if(fg != CACA_TRANSPARENT)
                        printf("\x1b[%dm", caca_to_ansi(fg));
                }
                else printf(
                    "\x1b[%d;%dm",
                    caca_to_ansi(fg),
                    caca_to_ansi(bg)+10
                );
            }
            putchar(c);
            prev_a = a;
        }
        printf("\x1b[0m\n");
    }
}

void stringify_canvas(caca_canvas_t* cv)
{
    int w = caca_get_canvas_width(cv);
    int h = caca_get_canvas_height(cv);

    printf("\"\\x1b[0m");
    for(int y= 0; y < h; ++y)
    {
        if(y != 0) putchar('\"');

        uint32_t prev_a = 0;
        for(int x = 0; x < w; ++x)
        {
            char c = caca_get_char(cv, x, y);
            uint32_t a = caca_get_attr(cv, x, y);
            if(a != prev_a)
            {
                uint8_t fg = caca_attr_to_ansi_fg(a);
                uint8_t bg = caca_attr_to_ansi_bg(a);

                if(bg == CACA_TRANSPARENT)
                {
                    printf("\\x1b[0m");
                    if(fg != CACA_TRANSPARENT)
                        printf("\\x1b[%dm", caca_to_ansi(fg));
                }
                else printf(
                    "\\x1b[%d;%dm",
                    caca_to_ansi(fg),
                    caca_to_ansi(bg)+10
                );
            }
            putchar(c);
            prev_a = a;
        }
        printf("\\x1b[0m\\n\"\n");
    }
}

int main(int argc, char** argv)
{
    int ret = 0;
    if((ret = parse_args(argc, argv))) return ret;

    int in_w, in_h;
    int n;

    unsigned char* input_data = stbi_load(options.image, &in_w, &in_h, &n, 4);

    if(!input_data)
    {
        printf("Failed to load image %s\n", options.image);
        return 1;
    }

    int height = (int)(options.width*options.ratio*in_h/in_w);

    caca_canvas_t* canvas = caca_create_canvas(options.width, height);
    caca_set_color_ansi(canvas, CACA_TRANSPARENT, CACA_TRANSPARENT);
    caca_clear_canvas(canvas);
    caca_dither_t* dither = caca_create_dither(
        32,
        in_w,
        in_h,
        4 * in_w,
        0x000000FF,
        0x0000FF00,
        0x00FF0000,
        0xFF000000
    );
    caca_set_dither_gamma(dither, options.gamma);
    caca_set_dither_algorithm(dither, options.dither);
    caca_set_dither_antialias(dither, options.antialias ? "prefilter" : "none");

    caca_dither_bitmap(
        canvas,
        0,
        0,
        options.width,
        height,
        dither,
        input_data
    );

    if(options.preview) print_canvas(canvas);
    else stringify_canvas(canvas);

    caca_free_dither(dither);
    caca_free_canvas(canvas);

    stbi_image_free(input_data);

    return 0;
}
