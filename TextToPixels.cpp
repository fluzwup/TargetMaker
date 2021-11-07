#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include "TextToPixels.h"

TextToPixels::TextToPixels(int w, int h)
{
	initialized = false;
	isBold = true;
	Setup(w, h);
}

TextToPixels::~TextToPixels()
{
	Destroy();
}

void TextToPixels::Destroy()
{
	if(initialized)
	{
		if(normGC != NULL)
			XFreeGC(disp, normGC);
		if(fillGC != NULL)
			XFreeGC(disp, fillGC);
		if(drawto != 0)
			XFreePixmap(disp, drawto);
		if(fontinfo != NULL)
			XFreeFont(disp, fontinfo);
		if(fontinfoBold != NULL)
			XFreeFont(disp, fontinfoBold);
		if(disp != NULL)
			XCloseDisplay(disp);
		initialized = false;
	}
}

void TextToPixels::Setup(int width, int height)
{
	if(initialized)
		Destroy();

	if(!initialized)
	{
		XGCValues gcvals;

		pixmapWidth = width + 100;
		pixmapHeight = height + 100;

		disp = XOpenDisplay(NULL);
		if(disp == 0)
		{
			int i = 0;
			while(disp == 0)
			{
				if(i > 1)
					throw "Cannot open X Windows display for rendering";

				char hostname[4096];
				char display[4196];
				gethostname(hostname, 4095);
				sprintf(display, "%s:%i.0", hostname, i++);

				disp = XOpenDisplay(display);
			}
		}

		if(disp == NULL)
		{
			throw "Cannot open X Windows display for rendering";
		}

		if(BitmapBitOrder(disp) == MSBFirst)
			msbFirst = true;
		else
			msbFirst = false;

		char** fontlist;
		char patt[256];
		int num;
		int fontSize = 24;
		
		sprintf(patt, "*-courier-medium-r-normal--%i*", fontSize);

		// grab first matching font
		fontlist = XListFonts(disp, patt, 1, &num); 

		if(num < 1)
		{
			sprintf(patt, "*--%i*", fontSize);
			fontlist = XListFonts(disp, patt, 1, &num);
			if(num < 1)
				fontlist = XListFonts(disp, "*", 1, &num);
		}
		
		fontinfo = XLoadQueryFont(disp, fontlist[0]);
		XFreeFontNames(fontlist);
		
		sprintf(patt, "*-courier-bold-r-normal--%i*", fontSize);

		// grab first matching font
		fontlist = XListFonts(disp, patt, 1, &num); 

		if(num < 1)
		{
			sprintf(patt, "*--%i*", fontSize);
			fontlist = XListFonts(disp, patt, 1, &num);
			if(num < 1)
				fontlist = XListFonts(disp, "*", 1, &num);
		}
			
		fontinfoBold = XLoadQueryFont(disp, fontlist[0]);
		XFreeFontNames(fontlist);
		
		drawto = XCreatePixmap(disp, RootWindow(disp, DefaultScreen(disp)), pixmapWidth, pixmapHeight, 8);

		gcvals.foreground = 0;
		gcvals.background = 255;
		gcvals.font = fontinfo->fid;
		normGC = XCreateGC(disp, drawto, GCForeground | GCBackground | GCFont, &gcvals);
		
		gcvals.font = fontinfoBold->fid;
		boldGC = XCreateGC(disp, drawto, GCForeground | GCBackground | GCFont, &gcvals);
		
		gcvals.foreground = 255;
		gcvals.background = 0;
		fillGC = XCreateGC(disp, drawto, GCForeground | GCBackground, &gcvals);

		initialized = true;
	}
}

void TextToPixels::GetTextExtents(const char *pString, int &width, int &height)
{
	int direction, ascent, descent;
	XCharStruct overall;
	direction = FontLeftToRight;

	// empty string returns a 0 width
	if(pString[0] == 0)
	{
		// get the height of a couple of characters
		XTextExtents(fontinfo, "jP", 2, &direction, 
							&ascent, &descent, &overall);
		width = 0;
		height = ascent + descent;
		return;
	}
	
	if(!initialized) Setup(100, 100);
			
	if(isBold)
		XTextExtents(fontinfoBold, pString, strlen(pString), &direction, &ascent, &descent, &overall);
	else
		XTextExtents(fontinfo, pString, strlen(pString), &direction, &ascent, &descent, &overall);

	width = overall.width;
	height = ascent + descent;
}

XImage *TextToPixels::RenderTextToImage(const char *pString)
{
	int direction, ascent, descent;
	XCharStruct overall;
	direction = FontLeftToRight;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	
	GetTextExtents(pString, width, height);

	if(width > pixmapWidth || height > pixmapHeight)
	{
		Destroy();
		Setup(width, height);
	}

	// erase the area we're drawing into
	XFillRectangle(disp, drawto, fillGC, 0, 0, width, height);
	
	if(isBold)
	{
		XTextExtents(fontinfoBold, pString, strlen(pString), &direction, &ascent, &descent, &overall);
		XDrawString(disp, drawto, boldGC, x, y + ascent, pString, strlen(pString));
	}
	else
	{
		XTextExtents(fontinfo, pString, strlen(pString), &direction, &ascent, &descent, &overall);
		XDrawString(disp, drawto, normGC, x, y + ascent, pString, strlen(pString));
	}

	XImage *pixels = XGetImage(disp, drawto, 0, 0, width, height, 0xff, XYPixmap);

	return pixels;
}

void TextToPixels::DumpImageToConsole(XImage *img)
{
		for(int y = 0; y < img-> height; ++y)
		{
			for(int x = 0; x < img->width; ++x)
			{
					long p = XGetPixel(img, x, y);
					if(p == 0)
							printf("*");
					else
							printf(" ");
			}
			printf("\n");
		}
}
