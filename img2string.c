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
#define HEIGHT 'h'
#define GAMMA 'g'
#define ALGORITHM 'a'
#define PREVIEW 'p'

struct
{
    int width;
    int height;
    float gamma;
    const char* image;
    const char* algorithm;
    int preview;
} options = { 80, 24, 0.5f, NULL, "none", 0 };

int parse_args(int argc, char** argv)
{
    int indexptr = 0;
    char* endptr = NULL;

    struct option longopts[] = {
        { "help", no_argument, NULL, HELP },
        { "width", required_argument, NULL, WIDTH },
        { "height", required_argument, NULL, HEIGHT },
        { "gamma", required_argument, NULL, GAMMA },
        { "algorithm", required_argument, NULL, ALGORITHM },
        { "preview", no_argument, &options.preview, 'p' },
    };

    int val = 0;
    while((val = getopt_long(argc, argv, "w:h:g:a:p", longopts, &indexptr)) != -1)
    {
        switch(val)
        {
        case WIDTH:
            options.width = strtoul(optarg, &endptr, 10);
            if(*endptr != 0) goto help_print;
            break;
        case HEIGHT:
            options.height = strtoul(optarg, &endptr, 10);
            if(*endptr != 0) goto help_print;
            break;
        case GAMMA:
            options.gamma = strtod(optarg, &endptr);
            if(*endptr != 0) goto help_print;
            break;
        case ALGORITHM:
            if(
                strcmp(optarg, "none") != 0 &&
                strcmp(optarg, "ordered2") != 0 &&
                strcmp(optarg, "ordered4") != 0 &&
                strcmp(optarg, "ordered8") != 0 &&
                strcmp(optarg, "random") != 0 &&
                strcmp(optarg, "fstein") != 0
            ) goto help_print;
            options.algorithm = optarg;
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
        "Usage: %s [-w width] [-h height] [-g gamma] [-a algorithm] [-p] image\n"
        "algorithm can be one of the following:\n"
        "\tnone (default)\n"
        "\tordered2\n"
        "\tordered4\n"
        "\tordered8\n"
        "\trandom\n"
        "\tfstein\n",
        argv[0]
    );
return 1;
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

    /*printf(
        "(%s mode) %dx%d, (%f): %s\n",
        options.preview ? "preview" : "string",
        options.width,
        options.height,
        options.gamma,
        options.image
    );*/

    caca_canvas_t* canvas = caca_create_canvas(options.width, options.height);
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
    caca_set_dither_algorithm(dither, options.algorithm);

    caca_dither_bitmap(
        canvas,
        0,
        0,
        options.width,
        options.height,
        dither,
        input_data
    );

    size_t bytes = 0;
    char* art = caca_export_canvas_to_memory(canvas, "ansi", &bytes);

    printf("%.*s\n", (int)bytes, art);
    free(art);

    caca_free_dither(dither);
    caca_free_canvas(canvas);

    stbi_image_free(input_data);

    return 0;
}
