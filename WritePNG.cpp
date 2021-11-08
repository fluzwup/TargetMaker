#include <cstdlib>
#include <cstdio>
#include <png.h>

void WritePNG(char *filename, int width, int height, int bpl, 
	int dpi, int depth, unsigned char *data) 
{
	int y;
	int png_type;

	switch(depth)
	{
		case 8:
			png_type = PNG_COLOR_TYPE_GRAY;
			break;
		case 24:
			png_type = PNG_COLOR_TYPE_RGB;
			break;
		case 32:
			png_type = PNG_COLOR_TYPE_RGBA;
			break;
		default:
			throw "Unsupported bit depth";
	}

	FILE *fp = fopen(filename, "wb");
	if(!fp) throw "Could not open output file for writing";

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
					NULL, NULL, NULL);
	if(!png) throw "Could not create PNG write structure";

	png_infop info = png_create_info_struct(png);
	if(!info) throw "Could not create PNG info structure";

	png_set_pHYs(png, info, dpi * 39.37, dpi * 39.37, PNG_RESOLUTION_METER);
	
	if(setjmp(png_jmpbuf(png))) throw "Could not set PNG jmpbuf";

	png_init_io(png, fp);

	// Output is 8bit depth, RGBA format.
	png_set_IHDR(png, info, width, height, 8,
		png_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png, info);

	// set up array of png_bytep pointing to the first byte of each scanline
	png_bytep *row_pointers = new png_bytep[height];
	for(int y = 0; y < height; ++y)
		row_pointers[y] = (png_bytep)&data[y * bpl];

	png_write_image(png, row_pointers);

	png_write_end(png, NULL);

	delete[] row_pointers;

	fclose(fp);

	png_destroy_write_struct(&png, &info);
}

/*
int main(int, char **)
{
	int w = 500;
	int h = 500;
	int bpl = (w + 3) / 4 * 4;
	unsigned char *pixels = new unsigned char[bpl * h];

	for(int y = 0; y < h; y += 2)
	{
		memset(&pixels[y * bpl], 0xFF, w);
		memset(&pixels[(y + 1) * bpl], 0x00, w);
	}

	WritePNG("test.png", w, h, bpl, 100, 8, pixels);

	delete[] pixels;

	return 0;
}
*/
