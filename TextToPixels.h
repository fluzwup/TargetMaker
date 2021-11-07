#include <X11/Xlib.h>
#include <X11/Xutil.h>

class TextToPixels
{
protected:
	Display *disp;
	GC normGC;
	GC boldGC;
	GC fillGC;
	GC hfNormGC;
	GC hfBoldGC;
	XFontStruct *fontinfo;
	XFontStruct *fontinfoBold;
	XFontStruct *fontinfoHF;
	XFontStruct *fontinfoHFBold;
	Pixmap drawto;
	int fontSize;
	int pixmapWidth;
	int pixmapHeight;
	int correctedResolution;
	bool initialized;
	bool msbFirst;
	long renderResolution;
	bool isBold;

	void Destroy();
	void Setup(int w, int h);
	void GetTextExtents(const char *str, int &w, int &h);
public:
	TextToPixels(int w, int h);
	~TextToPixels();

	// use XGetPixel(XImage *img, int x, int y);
	// 0 is black, 1 is white
	XImage *RenderTextToImage(const char *str);
	void DumpImageToConsole(XImage *img);

};
