# img2string

img2string lets you generate ASCII ANSI art directly in the format you need it!
It can generate ready-to-print strings in several programming languages. If
there is a programming language that is not yet supported and you'd like it to
be, create an issue.

## Dependencies

img2string depends on [libcaca](http://caca.zoy.org/wiki/libcaca) and the [GNU
C library](https://www.gnu.org/software/libc/). It is therefore advised that
you compile this program using GCC.

In addition to those, img2string uses the header-only library
[stb\_image](https://github.com/nothings/stb/blob/master/stb_image.h) for
loading the image files. You don't have to install it manually, since it is
entirely contained in the single header file in `extern/`.

## Build

Note that this program has not been tested on macOS and Windows isn't supported
at all.

To build using the [Meson build system](https://mesonbuild.com/), run the
following commands:

```sh
meson build
ninja -C build
```

The resulting executable is then `build/img2string`

If you want to build it without Meson, that's easy too. Just write

```sh
gcc img2string.c -lcaca -lm -o img2string
```

and you'll get your executable without Meson.

## Usage

```sh
img2string [-w width] [-g gamma] [-d dither] [-r pixelratio] [-o mode] [-a] image
```

`width` specifies the number of columns the ANSI art can use. The height of the
generated art is determined automatically based on the original image
dimensions. Defaults to 80.

`gamma` determines the gamma correction applied on the image. This has a huge
impact on the image. Higher values cause the image to be brighter, lower values
dimmer. Defaults to 0.5.

`dither` selects the dithering algorithm to use. Available options:

* none (default)
* ordered2
* ordered4
* ordered8
* random
* fstein

`pixelratio` determines the width/height ratio of a single "pixel" (terminal
character in this case). Defaults to 0.5.

`mode` specifies the output mode used. Available options:

| Option | Description                                |
|:-------|:-------------------------------------------|
| `s`    | The resulting ANSI art directly. (default) |
| `c`    | A C string.                                |
| `py`   | A Python string.                           |
| `js`   | A Javascript (node.js compatible) string.  |
| `sh`   | A Bash string.                             |

`-a` enables antialiasing. May cause small arts to look better, but easily
loses detail. Disabled by default.

`image` is the image file to convert. img2string can load the following image
formats thanks to stb\_image:

* JPEG
* PNG
* TGA
* BMP
* GIF
* HDR
* PIC
* PNM
