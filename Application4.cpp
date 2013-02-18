// Application5.cpp: implementation of the Application5 class.
//
//////////////////////////////////////////////////////////////////////

/*
 * application test code for homework assignment #6
*/

#include "stdafx.h"
#include "CS580HW.h"
#include "Application4.h"
#include "Gz.h"
#include "disp.h"
#include "rend.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define INFILE  "pot4.asc"
#define OUTFILE "output.ppm"


extern int tex_fun(float u, float v, GzColor color); /* image texture function */
extern int ptex_fun(float u, float v, GzColor color); /* procedural texture function */

void shade(GzCoord norm, GzCoord color);

float AAFilter[AAKERNEL_SIZE][3] =  			// X, Y, coef 
	{
		-0.52, 0.38, 0.128,
		0.41, 0.56, 0.119,
		0.27, 0.08, 0.294,
		-0.17, -0.29, 0.249,
		0.58, -0.55, 0.104,
		-0.31, -0.71, 0.106
	};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Application4::Application4()
{

}

Application4::~Application4()
{
	
}

int Application4::Initialize()
{
	GzCamera	camera;  
	int		    xRes = 0, yRes = 0, dispClass;	/* display parameters */ 

	GzToken		nameListShader[9]; 	    /* shader attribute names */
	GzPointer   valueListShader[9];		/* shader attribute pointers */
	GzToken     nameListLights[10];		/* light info */
	GzPointer   valueListLights[10];
	GzToken     nameListShifts[10];		/* Shift info */
	GzPointer   valueListShifts[10];

	int			shaderType, interpStyle;
	float		specpower;
	int			status; 
	int			index = 0;
 
	status = 0; 

	/* 
	 * Allocate memory for user input
	 */
	m_pUserInput = new GzInput;

	/* 
	 * initialize the display and the renderer 
	 */ 
 	m_nWidth = 256;		// frame buffer and display width
	m_nHeight = 256;    // frame buffer and display height
	
	/* Translation matrix */
	GzMatrix	scale = 
	{ 
		3.25,	0.0,	0.0,	0.0, 
		0.0,	3.25,	0.0,	-3.25, 
		0.0,	0.0,	3.25,	3.5, 
		0.0,	0.0,	0.0,	1.0 
	}; 
	 
	GzMatrix	rotateX = 
	{ 
		1.0,	0.0,	0.0,	0.0, 
		0.0,	.7071,	.7071,	0.0, 
		0.0,	-.7071,	.7071,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	}; 
	 
	GzMatrix	rotateY = 
	{ 
		.866,	0.0,	-0.5,	0.0, 
		0.0,	1.0,	0.0,	0.0, 
		0.5,	0.0,	.866,	0.0, 
		0.0,	0.0,	0.0,	1.0 
	}; 

		/* Light */
	GzLight	light1 = { {-0.7071, 0.7071, 0}, {0.5, 0.5, 0.9} };
	GzLight	light2 = { {0, -0.7071, -0.7071}, {0.9, 0.2, 0.3} };
	GzLight	light3 = { {0.7071, 0.0, -0.7071}, {0.2, 0.7, 0.3} };
	GzLight	ambientlight = { {0, 0, 0}, {0.3, 0.3, 0.3} };

	/* Material property */
	GzColor specularCoefficient = { 0.3, 0.3, 0.3 };
	GzColor ambientCoefficient = { 0.1, 0.1, 0.1 };
	GzColor diffuseCoefficient = {0.7, 0.7, 0.7};

#if 0 	/* set up app-defined camera if desired, else use camera defaults */
    camera.position[X] = -3;
    camera.position[Y] = -25;
    camera.position[Z] = -4;

    camera.lookat[X] = 7.8;
    camera.lookat[Y] = 0.7;
    camera.lookat[Z] = 6.5;

    camera.worldup[X] = -0.2;
    camera.worldup[Y] = 1.0;
    camera.worldup[Z] = 0.0;

    camera.FOV = 63.7;              /* degrees *              /* degrees */

	status |= GzPutCamera(m_pRender, &camera); 
#endif 

	status |= GzNewFrameBuffer(&m_pFrameBuffer, m_nWidth, m_nHeight);

// ****************************************************************************************
//								START	HW6 CHANGE
// ****************************************************************************************
	// Create multiple displays and framebuffers(display framebuffers)
	for(index = 0; index < AAKERNEL_SIZE; index++){
		status |= GzNewDisplay(&m_pDisplay[index], GZ_RGBAZ_DISPLAY, m_nWidth, m_nHeight);
		status |= GzGetDisplayParams(m_pDisplay[index], &xRes, &yRes, &dispClass);  
		status |= GzInitDisplay(m_pDisplay[index]); 
		status |= GzNewRender(&m_pRender[index], GZ_Z_BUFFER_RENDER, m_pDisplay[index]); 

		/* Start Renderer */
		status |= GzBeginRender(m_pRender[index]);

		/*
		 * Tokens associated with light parameters
		 */
		nameListLights[0] = GZ_DIRECTIONAL_LIGHT;
		valueListLights[0] = (GzPointer)&light1;
		nameListLights[1] = GZ_DIRECTIONAL_LIGHT;
		valueListLights[1] = (GzPointer)&light2;
		nameListLights[2] = GZ_DIRECTIONAL_LIGHT;
		valueListLights[2] = (GzPointer)&light3;

		// store the Directional Light values in the render structures
		status |= GzPutAttribute(m_pRender[index], 3, nameListLights, valueListLights);

		nameListLights[0] = GZ_AMBIENT_LIGHT;
		valueListLights[0] = (GzPointer)&ambientlight;
		
		// store the Ambient Light values in the render structures
		status |= GzPutAttribute(m_pRender[index], 1, nameListLights, valueListLights);

		/*
		 * Tokens associated with shading 
		 */
		nameListShader[0]  = GZ_DIFFUSE_COEFFICIENT;
		valueListShader[0] = (GzPointer)diffuseCoefficient;

		/* 
		* Select either GZ_COLOR or GZ_NORMALS as interpolation mode  
		*/
			nameListShader[1]  = GZ_INTERPOLATE;
	#if 0
			interpStyle = GZ_COLOR;         /* Gourand shading */
	#else
			interpStyle = GZ_NORMALS;         /* Phong shading */
	#endif

		valueListShader[1] = (GzPointer)&interpStyle;

		nameListShader[2]  = GZ_AMBIENT_COEFFICIENT;
		valueListShader[2] = (GzPointer)ambientCoefficient;
		nameListShader[3]  = GZ_SPECULAR_COEFFICIENT;
		valueListShader[3] = (GzPointer)specularCoefficient;
		nameListShader[4]  = GZ_DISTRIBUTION_COEFFICIENT;
		specpower = 32;
		valueListShader[4] = (GzPointer)&specpower;

		nameListShader[5]  = GZ_TEXTURE_MAP;
	#if 1   /* set up null texture function or valid pointer */
		valueListShader[5] = (GzPointer)0;
	#else
		valueListShader[5] = (GzPointer)(tex_fun);	/* or use ptex_fun */
	#endif

		// store the Ambient shading values in the render structures
		status |= GzPutAttribute(m_pRender[index], 6, nameListShader, valueListShader);
		

		// Pass the Sample offset X for the renderer defined for handling that jittered sample
		nameListShifts[0]  = GZ_AASHIFTX;
		valueListShifts[0] = (GzPointer)&AAFilter[index][X];
		status |= GzPutAttribute(m_pRender[index], 1, nameListShifts, valueListShifts);
		
		// Pass the Sample offset Y for the renderer defined for handling that jittered sample
		nameListShifts[0]  = GZ_AASHIFTY;
		valueListShifts[0] = (GzPointer)&AAFilter[index][Y];
		status |= GzPutAttribute(m_pRender[index], 1, nameListShifts, valueListShifts);
		
		// Push the transformation matrices into all the renderer stacks
		status |= GzPushMatrix(m_pRender[index], scale);  
		status |= GzPushMatrix(m_pRender[index], rotateY); 
		status |= GzPushMatrix(m_pRender[index], rotateX); 
	}
// ****************************************************************************************
//								END		HW6 CHANGE
// ****************************************************************************************
	if (status) exit(GZ_FAILURE); 

	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS); 
}

int Application4::Render() 
{
	GzToken		nameListTriangle[3]; 	/* vertex attribute names */
	GzPointer	valueListTriangle[3]; 	/* vertex attribute pointers */
	GzCoord		vertexList[3], tempVertexList[3];	/* vertex position coordinates */ 
	GzCoord		normalList[3], tempNormalList[3];	/* vertex normals */ 
	GzTextureIndex  	uvList[3], tempUVList[3];		/* vertex texture map indices */ 
	char		dummy[256]; 
	int			status; 
	int			index = 0, x = 0, y = 0, disp_i = 0;
	int		    xRes, yRes, dispClass;
	int i = 0, j = 0;

// ****************************************************************************************
//								START	HW6 CHANGE
// ****************************************************************************************
	/* Initialize Display */
	for(index = 0; index < AAKERNEL_SIZE; index++){
		status |= GzInitDisplay(m_pDisplay[index]); 
	}
// ****************************************************************************************
//								END		HW6 CHANGE
// ****************************************************************************************	
	/* 
	* Tokens associated with triangle vertex values 
	*/ 
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  

	// I/O File open
	FILE *infile;
	if( (infile  = fopen( INFILE , "r" )) == NULL )
	{
         AfxMessageBox( "The input file was not opened\n" );
		 return GZ_FAILURE;
	}

	FILE *outfile;
	if( (outfile  = fopen( OUTFILE , "wb" )) == NULL )
	{
         AfxMessageBox( "The output file was not opened\n" );
		 return GZ_FAILURE;
	}

	/* 
	* Walk through the list of triangles, set color 
	* and render each triangle 
	*/ 
	while( fscanf(infile, "%s", dummy) == 1) { 	/* read in tri word */
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[0][0]), &(vertexList[0][1]),  
		&(vertexList[0][2]), 
		&(normalList[0][0]), &(normalList[0][1]), 	
		&(normalList[0][2]), 
		&(uvList[0][0]), &(uvList[0][1]) ); 
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[1][0]), &(vertexList[1][1]), 	
		&(vertexList[1][2]), 
		&(normalList[1][0]), &(normalList[1][1]), 	
		&(normalList[1][2]), 
		&(uvList[1][0]), &(uvList[1][1]) ); 
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[2][0]), &(vertexList[2][1]), 	
		&(vertexList[2][2]), 
		&(normalList[2][0]), &(normalList[2][1]), 	
		&(normalList[2][2]), 
		&(uvList[2][0]), &(uvList[2][1]) ); 

		for(i = 0; i < 3; i++)
			for(j = 0; j < 3; j++)
				tempVertexList[i][j] = vertexList[i][j];

		for(i = 0; i < 3; i++)
			for(j = 0; j < 3; j++)
				tempNormalList[i][j] = normalList[i][j];

		for(i = 0; i < 3; i++)
			for(j = 0; j < 2; j++)
				tempUVList[i][j] = uvList[i][j];


	    /* 
	     * Set the value pointers to the first vertex of the 	
	     * triangle, then feed it to the renderer 
	     * NOTE: this sequence matches the nameList token sequence
	     */ 
// ****************************************************************************************
//								START		HW6 CHANGE
// ****************************************************************************************
		for(index = 0; index < AAKERNEL_SIZE; index++)
		{				 
			for(i = 0; i < 3; i++)
				for(j = 0; j < 3; j++)
					vertexList[i][j] = tempVertexList[i][j];

			for(i = 0; i < 3; i++)
				for(j = 0; j < 3; j++)
					 normalList[i][j] = tempNormalList[i][j];

			for(i = 0; i < 3; i++)
				for(j = 0; j < 2; j++)
					 uvList[i][j] = tempUVList[i][j];

			valueListTriangle[0] = (GzPointer)vertexList; 
			valueListTriangle[1] = (GzPointer)normalList; 
			valueListTriangle[2] = (GzPointer)uvList; 

			GzPutTriangle(m_pRender[index], 3, nameListTriangle, valueListTriangle); 
		}
// ****************************************************************************************
//								END			HW6 CHANGE
// ****************************************************************************************
	} 

// ****************************************************************************************
//								START	HW6 CHANGE
// ****************************************************************************************
	
	// Create The final display for rendering it into the framebuffer
	status |= GzNewDisplay(&m_pFinalDisplay, GZ_RGBAZ_DISPLAY, m_nWidth, m_nHeight);
	status |= GzGetDisplayParams(m_pFinalDisplay, &xRes, &yRes, &dispClass);  
	status |= GzInitDisplay(m_pFinalDisplay); 
	
	GzColor finalColor;
	for(y = 0; y <= RESOLUTIONY; y++){
		for(x = 0; x <= RESOLUTIONX; x++){
			disp_i = (x+(y*256));
			finalColor[RED]=0;
			finalColor[GREEN]=0;
			finalColor[BLUE]=0;
			for(index = 0; index < AAKERNEL_SIZE; index++){
			 
				finalColor[RED] += ((float)m_pRender[index]->display->fbuf[disp_i].red * AAFilter[index][2]);
				finalColor[GREEN] += ((float)m_pRender[index]->display->fbuf[disp_i].green * AAFilter[index][2]);
				finalColor[BLUE] += ((float)m_pRender[index]->display->fbuf[disp_i].blue * AAFilter[index][2]);
			
			}
			m_pFinalDisplay->fbuf[disp_i].red = (short)finalColor[RED];
			m_pFinalDisplay->fbuf[disp_i].green = (short)finalColor[GREEN];
			m_pFinalDisplay->fbuf[disp_i].blue = (short)finalColor[BLUE];
		}
	}
/*	
	// update the renderers with the sample weights
	for(index = 0; index < 1; index++){
		for(y = 0; y <= RESOLUTIONY; y++){
			for(x = 0; x <= RESOLUTIONX; x++){
				disp_i = (x+(y*256));
				m_pRender[index]->display->fbuf[disp_i].red *= AAFilter[index][2];
				m_pRender[index]->display->fbuf[disp_i].green *= AAFilter[index][2];
				m_pRender[index]->display->fbuf[disp_i].blue *= AAFilter[index][2];
			}
		}
	}

	// Compute the final pixel colors to render in the framebuffer
	for(index = 0; index < 1; index++){
		for(y = 0; y <= RESOLUTIONY; y++){
			for(x = 0; x <= RESOLUTIONX; x++){
				disp_i = (x+(y*256));
				m_pFinalDisplay->fbuf[disp_i].red += m_pRender[index]->display->fbuf[disp_i].red;
				m_pFinalDisplay->fbuf[disp_i].green += m_pRender[index]->display->fbuf[disp_i].green;
				m_pFinalDisplay->fbuf[disp_i].blue += m_pRender[index]->display->fbuf[disp_i].blue;
			}
		}
	}
*/
// ****************************************************************************************
//								END		HW6 CHANGE
// ****************************************************************************************

	
	GzFlushDisplay2File(outfile, m_pFinalDisplay); 	/* write out or update display to file*/
	GzFlushDisplay2FrameBuffer(m_pFrameBuffer, m_pFinalDisplay);	// write out or update display to frame buffer

	/* 
	 * Close file
	 */ 

	if( fclose( infile ) )
      AfxMessageBox( "The input file was not closed\n" );

	if( fclose( outfile ) )
      AfxMessageBox( "The output file was not closed\n" );
 
	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS); 
}

int Application4::Clean()
{
	/* 
	 * Clean up and exit 
	 */ 
	int	status = 0; 
	int index = 0;
// ****************************************************************************************
//								START		HW6 CHANGE
// ****************************************************************************************
	for(index = 0; index < AAKERNEL_SIZE; index++){
		status |= GzFreeRender(m_pRender[index]); 
		status |= GzFreeDisplay(m_pDisplay[index]);
	}
// ****************************************************************************************
//								END			HW6 CHANGE
// ****************************************************************************************	
	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS);
}



