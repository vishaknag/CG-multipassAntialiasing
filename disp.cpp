 /*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include	"disp.h" 
#include	"limits.h"
#define RESOLUTIONX 256
#define RESOLUTIONY 256

int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
 -- pass back pointer 
*/ 

	long int fb_size = 0;
	if(width != RESOLUTIONX || height != RESOLUTIONY){
		AfxMessageBox( "the resolution is low (or) out of the bounds\n" );
		return GZ_FAILURE;
	}
	/* Allocating 3 bytes for each pixel */
	fb_size = 3 * width * height;
	*framebuffer = new char[fb_size];
	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, GzDisplayClass dispClass, int xRes, int yRes)
{
/* create a display:
  -- allocate memory for indicated class and resolution
  -- pass back pointer to GzDisplay object in display
*/

	long int disp_size = 0;
	if(xRes != RESOLUTIONX || yRes != RESOLUTIONY){
		AfxMessageBox("the resolution is low (or) out of the bounds\n");
		return GZ_FAILURE;
	}
	disp_size = xRes * yRes;
	*display = new GzDisplay;
	(*display)->fbuf = new GzPixel[disp_size];
	/* Make display open since memory is allocated to it */ 
	(*display)->open = 1;
	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */
	
    delete display;
	display = 0;
	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes, GzDisplayClass	*dispClass)
{
/* pass back values for an open display */

	/* Check for open status and non-NULL value of display */
	if(display == NULL || display->open != 1)
		return GZ_FAILURE;
	xRes = (int*)display->xres;
	yRes = (int*)display->yres;
	dispClass = (GzDisplayClass*)display->dispClass;
	return GZ_SUCCESS;
}

int GzInitDisplay(GzDisplay	*display)
{
/* set everything to some default values - start a new frame */

	int x = 0, y = 0;
	long int disp_i = 0;
	display->xres = 256;
	display->yres = 256;
	display->dispClass = GZ_RGBAZ_DISPLAY;
	display->open = 1;
	
	/* Set the background color to RosyBrown-4 */
	for(y = 0; y <= RESOLUTIONY; y++){
		for(x = 0; x <= RESOLUTIONX; x++){
			disp_i = ARRAY(x,y);
			display->fbuf[disp_i].red = 2032;
			display->fbuf[disp_i].green = 1776;
			display->fbuf[disp_i].blue = 1520;
			display->fbuf[disp_i].z = INT_MAX;
			display->fbuf[disp_i].alpha = 0;
		}
	} 
	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */
	long disp_i = 0;
	
	if(i > RESOLUTIONX || j > RESOLUTIONY || i < 0 || j < 0)
		return GZ_SUCCESS;
	if(i < RESOLUTIONX && j >= RESOLUTIONY)
		return GZ_SUCCESS;
	
	disp_i = ARRAY(i, j);
	if((z) <= (display->fbuf[disp_i].z)){
		display->fbuf[disp_i].red = r;
		display->fbuf[disp_i].green = g;
		display->fbuf[disp_i].blue = b;
		display->fbuf[disp_i].alpha = a;
		display->fbuf[disp_i].z = z;
	}
	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
/* pass back pixel value in the display */
/* check display class to see what vars are valid */

	if(i > RESOLUTIONX || j > RESOLUTIONY || i < 0 || j < 0)
		return GZ_SUCCESS;
	long disp_i = 0;
	disp_i = ARRAY(i,j);
    r = &(display->fbuf[disp_i].red);
    g = &(display->fbuf[disp_i].green);
	b = &(display->fbuf[disp_i].blue);
	a = &(display->fbuf[disp_i].alpha);
	z = &(display->fbuf[disp_i].z);
	return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{
/* write pixels to ppm file based on display class -- "P6 %d %d 255\r" */
	
	int x = 0, y = 0;
	long int disp_i = 0;
	char buf_i;

	/* Write header into ppm file in the following format
			- MagicValue	- P6 for binary
			- ImageWidth	- Width of image in pixels (ASCII decimal value)
			- ImageHeight   - Height of image in pixels (ASCII decimal value)
			- MaxGrey	    - Maximum color value (ASCII decimal value) 
	*/
	fprintf(outfile, "P6 %d %d 255\n", RESOLUTIONX, RESOLUTIONY);
	for(y = 1; y<= RESOLUTIONY; y++){
		for(x = 1; x <= RESOLUTIONX; x++){
		/* clamping and truncating the display values for r g and b */
		disp_i = ARRAY(x,y);
			/* for color red */
			if(display->fbuf[disp_i].red > 4095)
				display->fbuf[disp_i].red = 4095;
	        if(display->fbuf[disp_i].red < 0)
				display->fbuf[disp_i].red = 0;
			display->fbuf[disp_i].red = display->fbuf[disp_i].red >> 4;
			buf_i = ((char)(display->fbuf[disp_i].red));
			fwrite(&buf_i, 1, 1, outfile);

			/* for color green  */
			if(display->fbuf[disp_i].green > 4095)
				display->fbuf[disp_i].green = 4095;
	        if(display->fbuf[disp_i].green < 0)
				display->fbuf[disp_i].green = 0;
			display->fbuf[disp_i].green = display->fbuf[disp_i].green >> 4;
			buf_i = ((char)(display->fbuf[disp_i].green));
			fwrite(&buf_i, 1, 1, outfile);

			/* for color blue  */
			if(display->fbuf[disp_i].blue > 4095)
				display->fbuf[disp_i].blue = 4095;
	        if(display->fbuf[disp_i].blue < 0)
				display->fbuf[disp_i].blue = 0;
			display->fbuf[disp_i].blue = display->fbuf[disp_i].blue >> 4;
			buf_i = ((char)(display->fbuf[disp_i].blue));
			fwrite(&buf_i, 1, 1, outfile);			
		}
	}
	return GZ_SUCCESS;
}


int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{
/* write pixels to framebuffer: 
	- Put the pixels into the frame buffer
	- Caution: store the pixel to the frame buffer as the order of blue, green, and red 
	- Not red, green, and blue !!!
*/

	int x = 0, y = 0, fb_i = 0, disp_i = 0;
	for(y = 1; y<= RESOLUTIONY; y++){
		for(x = 1; x <= RESOLUTIONX; x++){
		/* - truncating the RGB values to 8 bits
		   - storing the pixels to the frame buffer in BGR order */
		    disp_i = ARRAY(x,y);
			/* for color blue */
			framebuffer[fb_i] = (char)display->fbuf[disp_i].blue;
	        fb_i++;
			/* for color green */
			framebuffer[fb_i] = (char)display->fbuf[disp_i].green;
			fb_i++;
			/* for color red */
			framebuffer[fb_i] = (char)display->fbuf[disp_i].red;
			fb_i++;
		}
	}
	return GZ_SUCCESS;
}