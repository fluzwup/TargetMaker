#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <math.h>
#include <vector>
#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "TextToPixels.h"
#include "WritePNG.h"

// handle rounding errors that drop the value below zero by clipping to zero
inline float safeSqrt(float f)
{
	if(f <= 0.0) return 0.0;
	return sqrt(f);
}

// values in inches
void FillCircle(float centerX, float centerY, float radius, unsigned char color,
		int dpi, int w, int h, int bpl, unsigned char *pixels)
{
	// convert inches to pixels
	centerX *= (float)dpi;
	centerY *= (float)dpi;
	radius *= (float)dpi;

	int top = round(centerY - radius);
	int bottom = round(centerY + radius);

	if(top < 0) top = 0;
	if(bottom >= h) bottom = h - 1;

	for(int y = top; y <= bottom; ++y)
	{
		float span = safeSqrt(radius * radius - (y  - centerY) * (y - centerY));
		int left = round(centerX - span);
		int count = round(span * 2.0);
		if(left < 0) left = 0;
		if(left + span >= w) span = w - 1 - left;
		if(span <= 0) continue;
		memset(pixels + y * bpl + left, color, count);
	}
}

void DrawTarget(float cx, float cy, float thick, std::vector<float> diameters, int bullseye,
		int dpi, int width, int height, int bpl, unsigned char *output)
{
	for(int index = diameters.size() - 1; index >= 0; --index)
	{
		float d = diameters[index];
		if(index > bullseye)
		{
			// black rings on white
			FillCircle(cx, cy, d / 2.0, 0x00, dpi, width, height, bpl, output);
			FillCircle(cx, cy, d / 2.0 - thick, 0xFF, dpi, width, height, bpl, output);
		}
		else if(index == bullseye)
		{
			// solid black, for outer ring of bullseye
			FillCircle(cx, cy, d / 2.0, 0x00, dpi, width, height, bpl, output);
		}
		else
		{
			// white rings on black
			FillCircle(cx, cy, d / 2.0, 0xFF, dpi, width, height, bpl, output);
			FillCircle(cx, cy, d / 2.0 - thick, 0x00, dpi, width, height, bpl, output);
		}
	}
}

// file format:
// title
// dpi, width, height
// bullindex, label, label, label...
// diameter, diameter, diameter...
// centerx, centery
// centerx, centery
// ...
int main(int argc, char **argv)
{
	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL) return -1;

	char buffer[4096];
	fgets(buffer, 4096, fp);	// title
	printf("%s", buffer);

	std::string title = buffer;
	// remove newline
	title = title.substr(0, title.length() - 1);

	fgets(buffer, 4096, fp);	// dpi, w, h, ring thickness
	printf("%s", buffer);
	int dpi = atoi(strtok(buffer, ","));
	int width = round(atof(strtok(NULL, ",")) * (float)dpi);
	int height = round(atof(strtok(NULL, ",")) * (float)dpi);
	float thickness = atof(strtok(NULL, "\n"));

	// round up to multiple of 4 bytes per line
	int bpl = (width + 3) / 4 * 4;
	int bytes = height * bpl;
	unsigned char *output = new unsigned char[bytes];
	memset(output, 0xFF, bytes);

	std::vector<std::string> labels;
	fgets(buffer, 4096, fp);
	printf("%s", buffer);
	char *token = strtok(buffer, ",");
	int bullIndex = atoi(token);
	token = strtok(NULL, ",\n");
	while(token != NULL)
	{
		while(token[0] == ' ') ++token;
		labels.push_back(token);
		token = strtok(NULL, ",\n");
	}

	std::vector<float> diameters;
	fgets(buffer, 4096, fp);
	printf("%s", buffer);
	token = strtok(buffer, ",");
	while(token != NULL)
	{
		diameters.push_back(atof(token));
		token = strtok(NULL, ",\n");
	}

	float cx, cy;
	TextToPixels ttp(10, 10);
	while(!feof(fp))
	{
		if(NULL == fgets(buffer, 4096, fp)) break;
		printf("%s", buffer);
		token = strtok(buffer, ",");
		cx = atof(token);
		token = strtok(NULL, "\n");
		cy = atof(token);

		DrawTarget(cx, cy, thickness, diameters, bullIndex, dpi, width, height, bpl, output);
		// overlay labels on target; use XOR so it works on black or white
		for(int i = 0; i < labels.size(); ++i)
		{
			XImage *img = ttp.RenderTextToImage(labels[i].c_str());
	
			// center label horizontally, put middle value at center, others midway between rings
			int labelY;
			int labelX = round(dpi * cx);
			if(i == 0) 
					labelY = round(dpi * cy);
			else
					// divide by 4 to average diameters, and conver to a radius
					labelY = round(dpi * (cy - (diameters[i] + diameters[i - 1]) / 4));
	
			// don't run off the edge
			if(labelY - img->height < 0) break;
	
			// adjust for size of label
			labelY -= img->height / 2;
			labelX -= img->width / 2;
			for(int y = 0; y < img->height; ++y)
			{
				unsigned char *outline = 
						&output[(labelY  + y) * bpl];
	
				for(int x = 0; x < img->width; ++x)
				{
					// 0 is black, so if it's zero, flip the color
					long l = XGetPixel(img, x, y); 
					if(l == 0)
							outline[labelX + x] ^= 0xFF;
				}
			}
			XFree(img);
		}
	}

	
	// title the image
	XImage *img = ttp.RenderTextToImage(title.c_str());
	int titleX = round(0.1 * dpi);
	int titleY = img->height;
	for(int y = 0; y < img->height; ++y)
	{
		unsigned char *outline = 
				&output[(titleY  + y) * bpl];

		for(int x = 0; x < img->width; ++x)
		{
			// 0 is black, so if it's zero, flip the color
			long l = XGetPixel(img, x, y); 
			if(l == 0)
					outline[titleX + x] ^= 0xFF;
		}
	}
	XFree(img);
	
	// overwrite extension of data file with "png" for output filename
	sprintf(buffer, "%s", argv[1]);
	int i;
	for(i = strlen(buffer); i >= 0; --i) if(buffer[i] == '.') break;

	if(i <= 0) 
		sprintf(buffer, "%s", "target.png");
	else
		memcpy(buffer + i, ".png", 4);

	WritePNG(buffer, width, height, bpl, dpi, 8, output); 
	delete[] output;

	return 0;
}


