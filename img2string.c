/*
 *  Copyright 2018 Julius Ikkala
 *
 *  This file is part of img2string.
 *
 *  img2string is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  img2string is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with img2string.  If not, see <http://www.gnu.org/licenses/>.
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
#define OUTPUT 'o'

enum output_mode
{
    OUTPUT_STDOUT = 0,
    OUTPUT_C,
    OUTPUT_PYTHON,
    /* Specifically, node.js compatible Javascript since ANSI escape codes
     * wouldn't make sense elsewhere.
     */
    OUTPUT_JAVASCRIPT,
    /* You'll have to print it like this:
     * echo -e "$IMAGE"
     */
    OUTPUT_BASH
};

struct
{
    int width;
    float ratio;
    float gamma;
    const char* image;
    const char* dither;
    int antialias;
    enum output_mode output;
} options = { 80, 0.5f, 0.5f, NULL, "none", 0, OUTPUT_STDOUT };

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
        { "antialias", no_argument, &options.antialias, ANTIALIAS },
        { "output", required_argument, NULL, OUTPUT }
    };

    int val = 0;
    while((val = getopt_long(argc, argv, "w:r:g:d:ao:", longopts, &indexptr)) != -1)
    {
        switch(val)
        {
        case WIDTH:
            options.width = strtoul(optarg, &endptr, 10);

            if(*endptr != 0)
            {
                printf("Width must be a positive integer\n");
                goto help_print;
            }
            break;
        case RATIO:
            options.ratio = strtod(optarg, &endptr);

            if(*endptr != 0)
            {
                printf("Ratio must be a number\n");
                goto help_print;
            }
            break;
        case GAMMA:
            options.gamma = strtod(optarg, &endptr);

            if(*endptr != 0)
            {
                printf("Gamma must be a number\n");
                goto help_print;
            }
            break;
        case DITHER:
            if(
                strcmp(optarg, "none") != 0 &&
                strcmp(optarg, "ordered2") != 0 &&
                strcmp(optarg, "ordered4") != 0 &&
                strcmp(optarg, "ordered8") != 0 &&
                strcmp(optarg, "random") != 0 &&
                strcmp(optarg, "fstein") != 0
            ) {
                printf("Unknown dither %s\n", optarg);
                goto help_print;
            }
            options.dither = optarg;
            break;
        case ANTIALIAS:
            options.antialias = 1;
            break;
        case OUTPUT:
            if(!strcmp(optarg, "s") || !strcmp(optarg, "stdout"))
                options.output = OUTPUT_STDOUT;
            else if(!strcmp(optarg, "c")) options.output = OUTPUT_C;
            else if(!strcmp(optarg, "py") || !strcmp(optarg, "python"))
                options.output = OUTPUT_PYTHON;
            else if(!strcmp(optarg, "js") || !strcmp(optarg, "javascript"))
                options.output = OUTPUT_JAVASCRIPT;
            else if(!strcmp(optarg, "sh") || !strcmp(optarg, "bash"))
                options.output = OUTPUT_BASH;
            else {
                printf("Unknown output mode %s\n", optarg);
                goto help_print;
            }
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
        "Usage: %s [-w width] [-g gamma] [-d dither] [-r pixelratio] "
        "[-o mode] [-a] image\n"
        "\ndither can be one of the following:\n"
        "\tnone (default)\n"
        "\tordered2\n"
        "\tordered4\n"
        "\tordered8\n"
        "\trandom\n"
        "\tfstein\n"
        "\npixelratio is the aspect ratio of a pixel.\n"
        "\nmode is the output mode. It can be one of the following:\n"
        "\ts \tShows the result directly.\n"
        "\tc \tOutputs a C string.\n"
        "\tpy\tOutputs a Python string.\n"
        "\tjs\tOutputs a Javascript (node.js compatible) string.\n"
        "\tsh\tOutputs a Bash string.\n"
        "\n-a enables antialiasing.\n",
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

#define nop

#define output_canvas_pixels(cv, pre, post, reset, line_start, line_end, output, set_fg, set_both) { \
    int w = caca_get_canvas_width(cv); \
    int h = caca_get_canvas_height(cv); \
    pre;\
    for(int y= 0; y < h; ++y) { \
        uint32_t prev_a = 0; \
        line_start; \
        for(int x = 0; x < w; ++x) { \
            char c = caca_get_char(cv, x, y); \
            uint32_t a = caca_get_attr(cv, x, y); \
            if(a != prev_a) { \
                uint8_t fg = caca_attr_to_ansi_fg(a); \
                uint8_t bg = caca_attr_to_ansi_bg(a); \
                if(bg == CACA_TRANSPARENT) { \
                    reset; \
                    if(fg != CACA_TRANSPARENT) set_fg(caca_to_ansi(fg)); \
                } else set_both(caca_to_ansi(fg), caca_to_ansi(bg)+10); \
            } \
            output(c); \
            prev_a = a; \
        } \
        reset; \
        line_end; \
    } \
    post; \
}

#define stdout_fg(fg) printf("\x1b[%dm", fg)
#define stdout_both(fg, bg) printf("\x1b[%d;%dm", fg, bg)

void print_canvas_stdout(caca_canvas_t* cv) output_canvas_pixels(
    cv,
    printf("\x1b[0m"),
    nop,
    printf("\x1b[0m"),
    nop,
    printf("\x1b[0m\n"),
    putchar,
    stdout_fg,
    stdout_both
);

void print_escaped_char(char c)
{
    switch(c)
    {
    case '\"':
    case '\'':
    case '\?':
    case '\\':
        printf("\\%c", c);
        break;
    default:
        putchar(c);
        break;
    }
}

#define stringout_fg(fg) printf("\\x1b[%dm", fg)
#define stringout_both(fg, bg) printf("\\x1b[%d;%dm", fg, bg)

void print_canvas_c(caca_canvas_t* cv) output_canvas_pixels(
    cv,
    printf("const char* image = \"\\x1b[0m"),
    printf("\";\n");,
    printf("\\x1b[0m"),
    nop,
    printf("\\x1b[0m\\n"),
    print_escaped_char,
    stringout_fg,
    stringout_both
);

void print_canvas_python(caca_canvas_t* cv) output_canvas_pixels(
    cv,
    printf("image = \"\"\"\\x1b[0m"),
    printf("\"\"\"\n");,
    printf("\\x1b[0m"),
    nop,
    printf("\\x1b[0m\n"),
    print_escaped_char,
    stringout_fg,
    stringout_both
);

void print_canvas_javascript(caca_canvas_t* cv) output_canvas_pixels(
    cv,
    printf("const image = \'\\x1b[0m"),
    printf("\';\n");,
    printf("\\x1b[0m"),
    nop,
    printf("\\x1b[0m\\n"),
    print_escaped_char,
    stringout_fg,
    stringout_both
);

void print_escaped_char_bash(char c)
{
    switch(c)
    {
    case '\"':
    case '\\':
    case '$':
    case '`':
    case '!':
        printf("\\%c", c);
        break;
    default:
        putchar(c);
        break;
    }
}

#define bash_fg(fg) printf("\\033[%dm", fg)
#define bash_both(fg, bg) printf("\\033[%d;%dm", fg, bg)

void print_canvas_bash(caca_canvas_t* cv) output_canvas_pixels(
    cv,
    printf("IMAGE=\"\\033[0m"),
    printf("\"\n");,
    printf("\\033[0m"),
    nop,
    printf("\\033[0m\\n"),
    print_escaped_char_bash,
    bash_fg,
    bash_both
);

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

    int height = (int)round(options.width*options.ratio*in_h/in_w);

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

    switch(options.output)
    {
    case OUTPUT_STDOUT:
        print_canvas_stdout(canvas);
        break;
    case OUTPUT_C:
        print_canvas_c(canvas);
        break;
    case OUTPUT_PYTHON:
        print_canvas_python(canvas);
        break;
    case OUTPUT_JAVASCRIPT:
        print_canvas_javascript(canvas);
        break;
    case OUTPUT_BASH:
        print_canvas_bash(canvas);
        break;
    }

    caca_free_dither(dither);
    caca_free_canvas(canvas);

    stbi_image_free(input_data);

    return 0;
}
