/* CS580 Homework 6 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
#define OPEN 1
#define CLOSE 0
#define PI 3.14285714
#define FIRST_LIGHT 0 
#define SECOND_LIGHT 1 
#define THIRD_LIGHT 2

#ifndef GZDDA
#define GZDDA
typedef struct {			/* define a edge DDA */
	GzCoord start;			/* start vertex of the edge */
	GzCoord end;			/* end vertex of the edge */
	GzCoord current;		/* current vertex of the edge */
	float slopex;			/* slope of x along y */
	float slopez;			/* slope of z along y */
	float rPerScanLine;			/* slope of r along y */
	float gPerScanLine;			/* slope of g along y */
	float bPerScanLine;			/* slope of b along y */
	float xPerScanLine;			/* slope of x along y */
	float yPerScanLine;			/* slope of y along y */
	float zPerScanLine;			/* slope of z along y */
} DDA;
#endif

GzCoord E = {0,0,-1};
GzCoord resCross = {0,0,0};
GzCoord color = {0,0,0};
GzMatrix result = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

/* sort the vertices in the increasing order of "y" */
void sort_verts(GzCoord **verts, short* vert_index)
{
	if(((*verts)[0][1]) > ((*verts)[1][1])){ /* V1[y] > V2[y] */
		if(((*verts)[0][1]) > ((*verts)[2][1])){ /* V1[y] > V3[y] */
			vert_index[2] = 0; /* V1[y] is greatest */
			if(((*verts)[1][1]) > ((*verts)[2][1])){ /* V2[y] > V3[y] */
				vert_index[1] = 1; /* V2[y] is second */
				vert_index[0] = 2; /* V3[y] is smallest */
			}else{
				vert_index[0] = 1; /* V2[y] is smallest */
				vert_index[1] = 2; /* V3[y] is second */
			}
		}else{
			vert_index[0] = 1; /* V2[y] is smallest */
			vert_index[1] = 0; /* V3[y] is second */
			vert_index[2] = 2; /* V1[y] is greatest */
		}
	}else{
		if(((*verts)[1][1]) > ((*verts)[2][1])){
			vert_index[2] = 1; /* V2[y] is greatest */
			if(((*verts)[0][1]) > ((*verts)[2][1])){ /* V2[y] > V3[y] */
				vert_index[1] = 0; /* V1[y] is second */
				vert_index[0] = 2; /* V3[y] is smallest */
			}else{
				vert_index[0] = 0; /* V1[y] is smallest */
				vert_index[1] = 2; /* V3[y] is second */
			}
		}else{
			vert_index[0] = 0; /* V1[y] is smallest */
			vert_index[1] = 1; /* V2[y] is second */
			vert_index[2] = 2; /* V3[y] is greatest */
		}
	}
}
/* Function to calculate the Dot product of two vectors */
float DotProduct(GzCoord A, GzCoord B) {
	
	float resDot = 0;

	resDot = (A[0] * B[0]) + (A[1] * B[1]) + (A[2] * B[2]);
	if(resDot == -0)
		resDot = 0;

	return resDot;
}


/* Function to calculate the Cross product of two vectors */
GzCoord* CrossProduct(GzCoord A, GzCoord B) {

	resCross[0] = (A[1] * B[2]) - (A[2] * B[1]);
	resCross[1] = (A[0] * B[2]) - (A[2] * B[0]);
	resCross[2] = (A[0] * B[1]) - (A[1] * B[0]);

	return (&resCross);
}

GzCoord* FindColor(GzRender *render, GzCoord normal) {

	GzCoord diffuseFactor = {0,0,0}, specularFactor = {0,0,0}, ambientFactor = {0,0,0};
	GzCoord diffuseSummation = {0,0,0}, specularSummation = {0,0,0};
	GzCoord R1 = {0,0,0}, R2 = {0,0,0}, R3 = {0,0,0};
	GzCoord normalL1 = {0,0,0}, normalL2 = {0,0,0}, normalL3 = {0,0,0}; 
	float nDotL1 = 0 , nDotL2 = 0, nDotL3 = 0;
	float nDotE = 0;
	float r1DotE = 0, r2DotE = 0, r3DotE = 0; 
	int index = 0, skipL1 = 0, skipL2 = 0, skipL3 = 0;
	
	//---------------------------------------------------------------------------------------
	//					COMPUTE THE SPECULAR FACTOR IN THE COLOR FORMULA
	//---------------------------------------------------------------------------------------
	// Compute N . L for each light vector
	nDotL1 = DotProduct(normal, ((GzLight*) &(render->lights[FIRST_LIGHT]))->direction);
	nDotL2 = DotProduct(normal, ((GzLight*) &(render->lights[SECOND_LIGHT]))->direction);
	nDotL3 = DotProduct(normal, ((GzLight*) &(render->lights[THIRD_LIGHT]))->direction);
	
	// compute N.E to validate the contribution of lights
	nDotE = DotProduct(normal, E);

	// before flipping the normals store the vertex normal for all the lights
	for(index = 0; index <= 2; index++){
		normalL1[index] = normal[index];
		normalL2[index] = normal[index];
		normalL3[index] = normal[index];
	}
	//------------------------------------------------------------------------------------
	//								CASES THAT COME UP IN SHADING
	//------------------------------------------------------------------------------------
	// CASES OF LIGHT 1
	if((nDotL1 < 0)&&(nDotE < 0)){			// if N.E is negative and if N.L1 is also negative
		for(index = 0; index <= 2; index++)	{
			normalL1[index] = normal[index] * -1;		// then flip the normal
		}
			nDotL1 = DotProduct(normalL1, ((GzLight*) &(render->lights[FIRST_LIGHT]))->direction);	// recalculate the N.L1
		
	}else{
		if(((nDotE < 0)&&(nDotL1 > 0)) || ((nDotE > 0)&&(nDotL1 < 0)))	
			skipL1 = 1;		// if both are of different signs then skip the light since it will not contribute to the shading
	}
	
	// CASES OF LIGHT 2
	if((nDotL2 < 0)&&(nDotE < 0)){			// if N.E is negative and if N.L2 is also negative
		for(index = 0; index <= 2; index++)	{
			normalL2[index] = normal[index] * -1;		// then flip the normal
		}
			nDotL2 = DotProduct(normalL2, ((GzLight*) &(render->lights[SECOND_LIGHT]))->direction);	// recalculate the N.L2
	}else{
		if(((nDotE < 0)&&(nDotL2 > 0)) || ((nDotE > 0)&&(nDotL2 < 0)))	
			skipL2 = 1;		// if both are of different signs then skip the light since it will not contribute to the shading
	}

	// CASES OF LIGHT 3
	if((nDotL3 < 0)&&(nDotE < 0)){			// if N.E is negative and if N.L3 is also negative
		for(index = 0; index <= 2; index++)	{
			normalL3[index] = normal[index] * -1;		// then flip the normal
		}
			nDotL3 = DotProduct(normalL3, ((GzLight*) &(render->lights[THIRD_LIGHT]))->direction);	// recalculate the N.L3
	}else{
		if(((nDotE < 0)&&(nDotL3 > 0)) || ((nDotE > 0)&&(nDotL3 < 0)))	
			skipL3 = 1;		// if both are of different signs then skip the light since it will not contribute to the shading
	}

	
	// R is computed for each light which contributes to the shading
	if(skipL1 == 0){
		for(index = 0; index <= 2; index++){
			R1[index] = (2 * nDotL1 * normalL1[index]) - ((GzLight*)&(render->lights[FIRST_LIGHT]))->direction[index];
		}
		r1DotE = DotProduct(R1, E);	
	}

	if(skipL2 == 0){
		for(index = 0; index <= 2; index++){
			R2[index] = (2 * nDotL2 * normalL2[index]) - ((GzLight*)&(render->lights[SECOND_LIGHT]))->direction[index];
		}
		r2DotE = DotProduct(R2, E);	
	}

	if(skipL3 == 0){
		for(index = 0; index <= 2; index++){
			R3[index] = (2 * nDotL3 * normalL3[index]) - ((GzLight*)&(render->lights[THIRD_LIGHT]))->direction[index];
		}
		r3DotE = DotProduct(R3, E);	
	}
	
	
	
	
	// Compute the le * (R.E pow s) for every L
	// add all the computed values to get the summation for the specular lighting 

	if(skipL1 == 0){
		for(index = 0; index <= 2; index++)
			specularSummation[index] += ((GzLight*)&(render->lights[FIRST_LIGHT]))->color[index] * pow(r1DotE, render->spec); 
	}
	if(skipL2 == 0){
		for(index = 0; index <= 2; index++)
			specularSummation[index] += ((GzLight*)&(render->lights[SECOND_LIGHT]))->color[index] * pow(r2DotE, render->spec); 
	}
	if(skipL3 == 0){
		for(index = 0; index <= 2; index++)
			specularSummation[index] += ((GzLight*)&(render->lights[THIRD_LIGHT]))->color[index] * pow(r3DotE, render->spec); 
	}
	// Specular lighting is the product of the specular coefficient and the summation result obtained above
	// Ks * specularSummation
	for(index = 0; index <= 2; index++)
		specularFactor[index] = render->Ks[index] * specularSummation[index];
	

	//---------------------------------------------------------------------------------------
	//					COMPUTE THE DIFFUSE FACTOR IN THE COLOR FORMULA
	//---------------------------------------------------------------------------------------
	
	// Compute the le * (N . L) for every L
	// add all the computed values to get the summation for the diffuse lighting 
	if(skipL1 == 0){
		for(index = 0; index <= 2; index++)
			diffuseSummation[index] += ((GzLight*)&(render->lights[FIRST_LIGHT]))->color[index] * nDotL1;	
	}
	if(skipL2 == 0){
		for(index = 0; index <= 2; index++)
			diffuseSummation[index] += ((GzLight*)&(render->lights[SECOND_LIGHT]))->color[index] * nDotL2;	
	}
	if(skipL3 == 0){
		for(index = 0; index <= 2; index++)
			diffuseSummation[index] += ((GzLight*)&(render->lights[THIRD_LIGHT]))->color[index] * nDotL3;	
	}
	// Diffuse lighting is the product of the diffuse coefficient and the summation result obtained above
	// Kd * diffuseSummation
	for(index = 0; index <= 2; index++){
		diffuseFactor[index] = render->Kd[index] * diffuseSummation[index];
	}

	//---------------------------------------------------------------------------------------
	//					COMPUTE THE AMBIENT FACTOR IN THE COLOR FORMULA
	//---------------------------------------------------------------------------------------

	// Ambient lighting is the product of the ambient coefficient and the ambient light color vector
	// Ka * ambient Light color vector
	
	for(index = 0; index <= 2; index++)
		ambientFactor[index] = render->Ka[index] * ((GzLight*)&(render->ambientlight))->color[index];

	//-----------------------------------------------------------------------------------------------------
	// Finally, compute the resulting color vector by adding all the specular, diffuse and ambient factors
	// C = S factor + D factor + A factor

	for(index = 0; index <= 2; index++)
		color[index] = specularFactor[index] + diffuseFactor[index] + ambientFactor[index];
	
	return (&color);
}


/* 4X4 Matrix multiplier function
 - mat	: multiplier
 - xform: multiplicand
 returns a GzMatrix pointer to the resulting matrix
*/
GzMatrix* fourX4MatrixMult(GzMatrix xform, GzMatrix mat)
{	
	int i = 0, j = 0, row = 0;
	float temp = 0;
	
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			result[i][j] = 0;

	// for every row on the multiplier matrix
	for(row = 0; row < 4; row++)
	{	
		// for all 4X4 values in the mulplicand matrix 
		for(i = 0; i < 4; i++)
		{
			for(j = 0; j < 4; j++)
			{
				temp += mat[j][i] * xform[row][j];
			}
			result[row][i] = temp;
			temp = 0;
		}
	}
	return (&result);
}


int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	float cosVal = 0, sinVal = 0;
	float radians = 0;
	radians = (float)(PI/180)*degree;

	cosVal = (float)(cos(radians));
	sinVal = (float)(sin(radians));

	mat[0][0] = 1;
	mat[1][1] = cosVal;
	mat[1][2] = -(sinVal);
	mat[2][1] = sinVal;
	mat[2][2] = cosVal;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value

	float cosVal = 0, sinVal = 0;
	float radians = 0;
	radians = (float)(PI/180)*degree;

	cosVal = (float)(cos(radians));
	sinVal = (float)(sin(radians));

	mat[0][0] = cosVal;
	mat[0][2] = sinVal;
	mat[1][1] = 1;
	mat[2][0] = -(sinVal);
	mat[2][2] = cosVal;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value

	float cosVal = 0, sinVal = 0;
	float radians = 0;
	radians = (float)(PI/180)*degree;

	cosVal = (float)(cos(radians));
	sinVal = (float)(sin(radians));

	mat[0][0] = cosVal;
	mat[0][1] = -(sinVal);
	mat[1][0] = sinVal;
	mat[1][1] = cosVal;
	mat[2][2] = 1;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value

	mat[0][3] = translate[0];
	mat[1][3] = translate[1];
	mat[2][3] = translate[2];
	mat[0][0] = 1;
	mat[1][1] = 1;
	mat[2][2] = 1;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value

	mat[0][0] = scale[0];
	mat[1][1] = scale[1];
	mat[2][2] = scale[2];
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay	*display)
{
/*  
- malloc a renderer struct 
- keep closed until all inits are done 
- setup Xsp and anything only done once 
- span interpolator needs pointer to display 
- check for legal class GZ_Z_BUFFER_RENDER 
- init default camera 
*/ 

GzMatrix xformSP = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float xby2 = 0;
float yby2 = 0;
float d = 0, fov = 0;
	
if(renderClass != GZ_Z_BUFFER_RENDER)
	return GZ_FAILURE;
(*render) = (GzRender*)malloc(sizeof(GzRender));
(*render)->open = OPEN;
(*render)->renderClass = renderClass;
(*render)->display = display;
(*render)->matlevel = -1;
(*render)->camera.FOV = DEFAULT_FOV;

(*render)->camera.lookat[X] = 0;
(*render)->camera.lookat[Y] = 0;
(*render)->camera.lookat[Z] = 0;

(*render)->camera.position[X] = DEFAULT_IM_X;
(*render)->camera.position[Y] = DEFAULT_IM_Y;
(*render)->camera.position[Z] = DEFAULT_IM_Z;

(*render)->camera.worldup[X] = 0;
(*render)->camera.worldup[Y] = 1;
(*render)->camera.worldup[Z] = 0;

// ****************************************************************************************
//								START		HW6 CHANGE
// ****************************************************************************************
// Default values of shift is zero
(*render)->aashiftx = 0;
(*render)->aashifty = 0;
// ****************************************************************************************
//								END			HW6 CHANGE
// ****************************************************************************************
fov = (float)((PI/180)*DEFAULT_FOV);
d = (float)(1/tan(fov/2));
xby2 = (float)((*render)->display->xres)/2;
yby2 = (float)((*render)->display->yres)/2;		
xformSP[0][0] = xby2;
xformSP[0][3] = xby2;
xformSP[1][1] = -yby2;
xformSP[1][3] = yby2;
xformSP[2][2] = (INT_MAX/d);
xformSP[3][3] = 1;

GzPushMatrix(*render, xformSP);

	return GZ_SUCCESS;
}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/

	free(render);
	render = 0;

	return GZ_SUCCESS;
}


int GzBeginRender(GzRender *render)
{
/*  
- set up for start of each frame - clear frame buffer 
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms if it want to. 
*/ 

	GzMatrix xformSP = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	
	GzMatrix xformPI = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	GzMatrix xformIW = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	GzMatrix temp = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	float magnitude = 0, xdiff = 0, ydiff = 0, zdiff = 0, sum = 0;
	float dotProduct = 0, d = 0, fov = 0;
	float xby2 = (float)(render->display->xres)/2;
	float yby2 = (float)(render->display->yres)/2;
	int i = 0, j = 0;
	GzCoord tempCoord = {0,0,0}, upPrime = {0,0,0};
	
	// START OF TRANFORMS PUSH

	// pop the old Xsp matrix for default camera
	// and load the new matrix Xsp for the new camera position
	GzPopMatrix(render); 
	
	fov = (float)(PI/180)*render->camera.FOV;
	d = 1/tan(fov/2);
		
	xformSP[0][0] = xby2;
	xformSP[0][3] = xby2;
	xformSP[1][1] = -yby2;
	xformSP[1][3] = yby2;
	xformSP[2][2] = (INT_MAX/d);
	xformSP[3][3] = 1;
	
	// Push Xsp
	GzPushMatrix(render, xformSP);	 
	
	xformPI[0][0] = 1;
	xformPI[1][1] = 1;
	xformPI[2][2] = 1;
	xformPI[3][3] = 1;
	xformPI[3][2] = (1/d);
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			temp[i][j] = render->Ximage[render->matlevel][i][j];
	 
	// Push Xpi
	GzPushMatrix(render, xformPI);
	
	// Z = cl / ||cl||
	xdiff = render->camera.lookat[X] - render->camera.position[X];
	ydiff = render->camera.lookat[Y] - render->camera.position[Y];
	zdiff = render->camera.lookat[Z] - render->camera.position[Z];
	sum = pow(xdiff, 2) + pow(ydiff, 2) + pow(zdiff, 2);
	magnitude = sqrt(sum);

	xformIW[Z][X] = xdiff/magnitude;
	xformIW[Z][Y] = ydiff/magnitude;
	xformIW[Z][Z] = zdiff/magnitude;

	// up' = up - (up.Z)Z
	// Y = up' / ||up'||
	dotProduct = (render->camera.worldup[X] * xformIW[Z][X]) + (render->camera.worldup[Y] * xformIW[Z][Y]) + (render->camera.worldup[Z] * xformIW[Z][Z]);	
	
	tempCoord[X] =  dotProduct * xformIW[Z][X];
	tempCoord[Y] =  dotProduct * xformIW[Z][Y];
	tempCoord[Z] =  dotProduct * xformIW[Z][Z];

	upPrime[X] = render->camera.worldup[X] - tempCoord[X];
	upPrime[Y] = render->camera.worldup[Y] - tempCoord[Y];
	upPrime[Z] = render->camera.worldup[Z] - tempCoord[Z];

	sum = pow(upPrime[X], 2) + pow(upPrime[Y], 2) + pow(upPrime[Z], 2);
	magnitude = sqrt(sum);

	xformIW[Y][X] = upPrime[X]/magnitude;
	xformIW[Y][Y] = upPrime[Y]/magnitude;
	xformIW[Y][Z] = upPrime[Z]/magnitude;

	// Cross product of Y & Z
	xformIW[X][X] = (xformIW[Y][Y]*xformIW[Z][Z]) - (xformIW[Y][Z]*xformIW[Z][Y]);
	xformIW[X][Y] = (xformIW[Y][Z]*xformIW[Z][X]) - (xformIW[Y][X]*xformIW[Z][Z]);
	xformIW[X][Z] = (xformIW[Y][X]*xformIW[Z][Y]) - (xformIW[Y][Y]*xformIW[Z][X]);
	
	// add the camera origin in world co-ordinates to the world to image matrix
	xformIW[X][3] = render->camera.position[X];
	xformIW[Y][3] = render->camera.position[Y];
	xformIW[Z][3] = render->camera.position[Z];
	
	xformIW[3][3] = 1;

	// convert Xwc to Xcw
	// cx = -X.c
	dotProduct = -((render->camera.position[X] * xformIW[X][X]) + (render->camera.position[Y] * xformIW[X][Y]) + (render->camera.position[Z] * xformIW[X][Z]));		
	xformIW[X][3] = dotProduct;
	// cy = -Y.c
	dotProduct = -((render->camera.position[X] * xformIW[Y][X]) + (render->camera.position[Y] * xformIW[Y][Y]) + (render->camera.position[Z] * xformIW[Y][Z]));		
	xformIW[Y][3] = dotProduct;
	// cz = -Z.c
	dotProduct = -((render->camera.position[X] * xformIW[Z][X]) + (render->camera.position[Y] * xformIW[Z][Y]) + (render->camera.position[Z] * xformIW[Z][Z]));		
	xformIW[Z][3] = dotProduct;
	
	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++)
			temp[i][j] = render->Ximage[render->matlevel][i][j];

	//  Push Xcw
	GzPushMatrix(render, xformIW);

	// END OF TRANFORMS PUSH

	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/

	render->camera.FOV = camera->FOV;

	render->camera.lookat[X] = camera->lookat[X];
	render->camera.lookat[Y] = camera->lookat[Y];
	render->camera.lookat[Z] = camera->lookat[Z];

	render->camera.position[X] = camera->position[X];
	render->camera.position[Y] = camera->position[Y];
	render->camera.position[Z] = camera->position[Z];

	render->camera.worldup[X] = camera->worldup[X];
	render->camera.worldup[Y] = camera->worldup[Y];
	render->camera.worldup[Z] = camera->worldup[Z];

	return GZ_SUCCESS;	
}

int GzPushMatrix(GzRender *render, GzMatrix matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/
	int i = 0, j = 0;
	float uniRotation = 0, kFactor = 0;
	GzMatrix unitMat = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
	GzMatrix tempMatrix = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	if(render->matlevel == MATLEVELS){
		printf("Stack is FULL\n");
		return GZ_FAILURE;
	}
		
	// Move the stack "top" up by one position
	render->matlevel++;

	// if the matrix to be pushed is Xiw
	if(render->matlevel == 2){
		
		for(i = 0; i < 4;i++){
				for(j = 0; j < 4; j++){
					render->Ximage[render->matlevel][i][j] = matrix[i][j];
			}
		}
		
		for(i = 0; i < 4;i++){
				for(j = 0; j < 4; j++){
					tempMatrix[i][j] = matrix[i][j];
				}
		}
		// Make the translation column elements zero
		tempMatrix[X][3] = 0;
		tempMatrix[Y][3] = 0;
		tempMatrix[Z][3] = 0;
		
		for(i = 0; i < 4;i++){
			for(j = 0; j < 4; j++){
				render->Xnorm[render->matlevel][i][j] = tempMatrix[i][j];
			}
		}
	
	}// if the matix to be pushed is Xsp or Xpi
	else if((render->matlevel == 0) || (render->matlevel == 1)){
		for(i = 0; i < 4;i++){
				for(j = 0; j < 4; j++){
					render->Ximage[render->matlevel][i][j] = matrix[i][j];
			}
		}

		for(i = 0; i < 4;i++){
				for(j = 0; j < 4; j++){
					render->Xnorm[render->matlevel][i][j] = unitMat[i][j];
			}
		}
	}else if(render->matlevel > 2){

		// Check if the matrix is a unitary rotation matrix before pushing it onto the xnorm stack
		uniRotation = sqrt(pow(matrix[X][X], 2) + pow(matrix[X][Y], 2) + pow(matrix[X][Z], 2));
		
		if(uniRotation == 1){
			// If Unitary matrix then push it onto both the stacks
			for(i = 0; i < 4;i++){
				for(j = 0; j < 4; j++){
					render->Ximage[render->matlevel][i][j] = matrix[i][j];
				}
			}
			for(i = 0; i < 4;i++){
				for(j = 0; j < 4; j++){
					tempMatrix[i][j] = matrix[i][j];
				}
			}
			// Make the translation column elements zero
			tempMatrix[X][3] = 0;
			tempMatrix[Y][3] = 0;
			tempMatrix[Z][3] = 0;
			
			for(i = 0; i < 4;i++){
				for(j = 0; j < 4; j++){
					render->Xnorm[render->matlevel][i][j] = tempMatrix[i][j];
				}
			}
			
		}// if not unitary then convert it into unitary
		else{
				
			// If not unitary then push the matrix as it is onto the ximage stack
			for(i = 0; i < 4;i++){
				for(j = 0; j < 4; j++){
					render->Ximage[render->matlevel][i][j] = matrix[i][j];
				}
			}
			// make the matrix unitary rotation matrix by multiplying the matrix elements
			// with the "k" factor and then push it onto the xnorm stack
			kFactor = 1 / sqrt(pow(matrix[X][X], 2) + pow(matrix[X][Y], 2) + pow(matrix[X][Z], 2));
			
			for(i = 0; i < 4;i++){
				for(j = 0; j < 4; j++){
					tempMatrix[i][j] = 0;
				}
			}
			for(i = 0; i < 3;i++){
				for(j = 0; j < 3; j++){
					tempMatrix[i][j] = matrix[i][j] * kFactor;
				}
			}
			
			// Make the translation column elements zero
			tempMatrix[X][3] = 0;
			tempMatrix[Y][3] = 0;
			tempMatrix[Z][3] = 0;

			for(i = 0; i < 4;i++){
				for(j = 0; j < 4; j++){
					render->Xnorm[render->matlevel][i][j] = tempMatrix[i][j];
				}
			}

		}
	}

	return GZ_SUCCESS;
}


int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/

	if(render->matlevel == -1){
		printf("Stack UNDERFLOW\n");
		return GZ_FAILURE;
	}

	// pop the "top" of the stack
	render->matlevel--;

	return GZ_SUCCESS;
}


int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	
	int index = 0, indexi = 0, tokenType = 0;
	GzLight	*light = NULL;
	GzCoord *color = NULL;
	int* interpMode = NULL;
	float *specPower = NULL;

	for(index = 0; index < numAttributes; index++){
		
		tokenType = nameList[index];
			
		switch(tokenType){
		
			case GZ_RGB_COLOR:
						
						color = (GzCoord*)valueList[index];
						render->flatcolor[0] = (*color)[0];
						render->flatcolor[1] = (*color)[1];
						render->flatcolor[2] = (*color)[2];
						break;
		
			case GZ_DIRECTIONAL_LIGHT:
						
						light = (GzLight*)valueList[index];					
						for(indexi = 0; indexi < 3; indexi++){
							render->lights[index].direction[indexi] = (*light).direction[indexi];
							render->lights[index].color[indexi] = (*light).color[indexi];
						}
						break;

			case GZ_AMBIENT_LIGHT:
						
						light = (GzLight*)valueList[index];
						for(indexi = 0; indexi < 3; indexi++){
							render->ambientlight.direction[indexi] = (*light).direction[indexi];
							render->ambientlight.color[indexi] = (*light).color[indexi];
						}
						break;

			case GZ_DIFFUSE_COEFFICIENT:
						
						color = (GzCoord*)valueList[index];
						render->Kd[0] = (*color)[0];
						render->Kd[1] = (*color)[1];
						render->Kd[2] = (*color)[2];
						break;

			case GZ_INTERPOLATE:
						
						interpMode = (int*)valueList[index];
						render->interp_mode = (*interpMode);
						break;

			case GZ_AMBIENT_COEFFICIENT:
						
						color = (GzCoord*)valueList[index];
						render->Ka[0] = (*color)[0];
						render->Ka[1] = (*color)[1];
						render->Ka[2] = (*color)[2];
						break;

			case GZ_SPECULAR_COEFFICIENT:
						
						color = (GzCoord*)valueList[index];
						render->Ks[0] = (*color)[0];
						render->Ks[1] = (*color)[1];
						render->Ks[2] = (*color)[2];
						break;

			case GZ_DISTRIBUTION_COEFFICIENT:
						
						specPower = (float*)valueList[index];	
						render->spec = (*specPower);
						break;

			case GZ_AASHIFTX:

						render->aashiftx = *((float*)valueList[index]);
						break;

			case GZ_AASHIFTY:

						render->aashifty = *((float*)valueList[index]);
						break;
		}
	}

	return GZ_SUCCESS;
}

int GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList, GzPointer *valueList) 
/* numParts - how many names and values */
{

/*  
- pass in a triangle description with tokens and values corresponding to 
      GZ_POSITION:3 vert positions in model space 
- Xform positions of verts  
- Clip - just discard any triangle with verts behind view plane 
       - test for triangles with all three verts off-screen 
- invoke triangle rasterizer  
*/ 

	DDA edge1, edge2, edge3, SPAN;
	DDA left1, left2, right1, right2;
	
	GzMatrix vertMat = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	GzMatrix normMat = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	GzMatrix *resMult = NULL;
	
	GzColor colorV1 = {0,0,0}, colorV2 = {0,0,0}, colorV3 = {0,0,0};
	GzColor rightColor = {0,0,0}, leftColor = {0,0,0}, pixelColor = {0,0,0};
	GzCoord leftNormal = {0,0,0}, rightNormal = {0,0,0}, pixelNormal = {0,0,0};
	GzColor *resColor = NULL, *phongColor;
	GzCoord **verts;
	GzCoord thirdColor = {0,0,0}, thirdNormal = {0,0,0};
	GzCoord *norms, interpolatedNormal;

	long disp_i = 0;
	
	double h = 0, w = 0, height = 0, slope = 0;
	double rPerPixel = 0, gPerPixel = 0, bPerPixel = 0;
	double xPerPixel = 0, yPerPixel = 0, zPerPixel = 0;
	double x = 0, deltax = 0, deltay = 0, deltaz = 0, indexy = 0, indexx = 0;
	double r = 0, g = 0, b = 0;
	double Xval = 0, Yval = 0, Zval = 0;
	double val = 0;
	short finalR = 0, finalG = 0, finalB = 0;
	int a = 0, z = 0;
	int h_bottom = 0, h_top = 0;
	int multCount = 0, indexi = 0, tokenType = 0;

	short vert_index[3] = {0,0,0}, norm_index[3] = {0,0,0};
	short i = 0, j = 0, flag = 0, index = 0;/* flag to swap the edge only once */
	
	// Received the valuelist containing the vertices and the normals 
	// Sort both the vertices and the normals along the y-axis
	for(indexi = 0; indexi < numParts; indexi++){
		
		tokenType = nameList[indexi];
	
		switch(tokenType){

			case GZ_POSITION:

				verts = (GzCoord**)(valueList);
	
				break;	// GZ_POSITION CASE END

				
			case GZ_NORMAL:
				
				norms = (GzCoord*)(valueList[indexi]);

				break;	// GZ_NORMAL CASE END 

		}// SWITCH END

	}// FOR END
	
	
	// Transform all three vertices -> HW3
	for(index = 0; index < 3; index++)
	{
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
				vertMat[i][j] = 0;

		// Store the vertices into 4x4 matrix from 4X1 matrix
		vertMat[0][0] =  (*verts)[index][0];
		vertMat[1][0] =  (*verts)[index][1];
		vertMat[2][0] =  (*verts)[index][2];
		// w = 1
		vertMat[3][0] =  1;	

		for(multCount = render->matlevel; multCount >= 0; multCount--)
		{
			// multiply the vertex with the matrices on the stack from top 
			// till you reach Xsp
			resMult = fourX4MatrixMult(render->Ximage[multCount], vertMat);
			for(i = 0; i < 4; i++)
				for(j = 0; j < 4; j++)
					vertMat[i][j] = (*resMult)[i][j];
		}
					
		// Divide all the coordinates with the new "w"
		vertMat[0][0] /= vertMat[3][0];  
		vertMat[1][0] /= vertMat[3][0];  
		vertMat[2][0] /= vertMat[3][0];  
		vertMat[3][0] /= vertMat[3][0];  
				
		// Restore the vertices into 4X1 matrix from 4x4 matrix
		(*verts)[index][0] = vertMat[0][0];
		(*verts)[index][1] = vertMat[1][0];
		(*verts)[index][2] = vertMat[2][0];

		if((*verts)[index][2] < 0)
			return GZ_SUCCESS;
	}
	// END OF HW3 CHANGE
	
	sort_verts(verts, vert_index);
	
//--------------------------------------------------------------------------------	
//						START		HW6 CHANGE 
//--------------------------------------------------------------------------------	
	for(index = 0; index < 3; index++){
		(*verts)[index][X] += render->aashiftx;
		(*verts)[index][Y] += render->aashifty;
	}
//--------------------------------------------------------------------------------	
//						END			HW6 CHANGE
//--------------------------------------------------------------------------------	


	// HW4 CHANGE
	// START OF NORMAL VECTORS TRANSFORM	
	for(index = 0; index < 3; index++)
	{
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
				normMat[i][j] = 0;

		// Store the normal into 4x4 matrix from 4X1 matrix
		normMat[0][0] =  (norms)[index][0];
		normMat[1][0] =  (norms)[index][1];
		normMat[2][0] =  (norms)[index][2];
		
		for(multCount = render->matlevel; multCount >= 0; multCount--)
		{
			// multiply the normal with the matrices on the stack from top 
			// till you reach Xsp
			resMult = fourX4MatrixMult(render->Xnorm[multCount], normMat);
			for(i = 0; i < 4; i++)
				for(j = 0; j < 4; j++)
					normMat[i][j] = (*resMult)[i][j];
		}
		
		// Restore the normals into 4X1 matrix from 4x4 matrix
		(norms)[index][0] = normMat[0][0];
		(norms)[index][1] = normMat[1][0];
		(norms)[index][2] = normMat[2][0];

		// normalise the normal vector
		val = sqrt(pow(norms[index][X], 2) + pow(norms[index][Y], 2) + pow(norms[index][Z], 2));
		for(j = 0; j <= 2; j++)
			norms[index][j] /= val;
	}

	// END OF NORMAL VECTORS TRANSFORM 
	    
	if(render->interp_mode == GZ_COLOR){
					resColor = FindColor(render, (norms)[vert_index[0]]);
					for(index = 0; index <=2; index++){
						colorV1[index] = (*resColor)[index];
						if(colorV1[index] > 1)
							colorV1[index] = 1;

						colorV1[index] *= 4095;
					}

					resColor = FindColor(render, (norms)[vert_index[1]]);
					for(index = 0; index <=2; index++){
						colorV2[index] = (*resColor)[index];
						if(colorV2[index] > 1)
							colorV2[index] = 1;

						colorV2[index] *= 4095;
					}

					resColor = FindColor(render, (norms)[vert_index[2]]);
					for(index = 0; index <=2; index++){
						colorV3[index] = (*resColor)[index];
						if(colorV3[index] > 1)
							colorV3[index] = 1;

						colorV3[index] *= 4095;
					}
	}
	
	/* Triangle having bottom edge/top edge being horizontal 
	- Check by comparing the y value of the last two/first two vertices
	*/
	if((((*verts)[vert_index[1]][1])) == (((*verts)[vert_index[2]][1]))) // if both y values are same then
		h_bottom = 1;

	if((((*verts)[vert_index[0]][1])) == (((*verts)[vert_index[1]][1]))) // - top edge is horizontal and merges with one of the scan line
		h_top = 1;

	/* fill up DDA s for all three edges */
	
	/* If no horizontal top edge then need to setup DDA for the V1-V2 edge */
	if(h_top == 0){		
		//-----------------------------------------------------------------------------------------------------
		/* Setup DDA for edge 1 -> V1-V2 */
		//-----------------------------------------------------------------------------------------------------

		for(i = 0; i < 3; i++){
			edge1.start[i] = (*verts)[vert_index[0]][i];
			edge1.current[i] = edge1.start[i];
		}
		for(i = 0; i < 3; i++)
			edge1.end[i] = (*verts)[vert_index[1]][i];
		
		edge1.slopex = (((*verts)[vert_index[1]][0])-((*verts)[vert_index[0]][0])) / (((*verts)[vert_index[1]][1])-((*verts)[vert_index[0]][1]));
		edge1.slopez = (((*verts)[vert_index[1]][2])-((*verts)[vert_index[0]][2])) / (((*verts)[vert_index[1]][1])-((*verts)[vert_index[0]][1]));

		if(render->interp_mode == GZ_COLOR){
			// COLOR INTERPOLATION HW4
			h = (((*verts)[vert_index[1]][1])-((*verts)[vert_index[0]][1]));
			edge1.rPerScanLine = (colorV2[RED]-colorV1[RED]) / h;
			edge1.gPerScanLine = (colorV2[GREEN]-colorV1[GREEN]) / h;
			edge1.bPerScanLine = (colorV2[BLUE]-colorV1[BLUE]) / h;
		}
		else if(render->interp_mode == GZ_NORMALS){
			h = (((*verts)[vert_index[1]][1])-((*verts)[vert_index[0]][1]));
			edge1.xPerScanLine = (norms[vert_index[1]][0]-norms[vert_index[0]][0]) / h;
			edge1.yPerScanLine = (norms[vert_index[1]][1]-norms[vert_index[0]][1]) / h;
			edge1.zPerScanLine = (norms[vert_index[1]][2]-norms[vert_index[0]][2]) / h;
		}
	}
	
	//-----------------------------------------------------------------------------------------------------
	/* Setup DDA for edge 2 -> V1-V3 */
	//-----------------------------------------------------------------------------------------------------

	for(i = 0; i < 3; i++){
		edge2.start[i] = (*verts)[vert_index[0]][i];
		edge2.current[i] = edge2.start[i];
	}
	for(i = 0; i < 3; i++)
		edge2.end[i] = (*verts)[vert_index[2]][i];

	edge2.slopex = (((*verts)[vert_index[2]][0])-((*verts)[vert_index[0]][0])) / (((*verts)[vert_index[2]][1])-((*verts)[vert_index[0]][1]));
	edge2.slopez = (((*verts)[vert_index[2]][2])-((*verts)[vert_index[0]][2])) / (((*verts)[vert_index[2]][1])-((*verts)[vert_index[0]][1]));

	if(render->interp_mode == GZ_COLOR){
		// COLOR INTERPOLATION HW4
		h = (((*verts)[vert_index[2]][1])-((*verts)[vert_index[0]][1]));
		edge2.rPerScanLine = (colorV3[RED]-colorV1[RED]) / h;
		edge2.gPerScanLine = (colorV3[GREEN]-colorV1[GREEN]) / h;
		edge2.bPerScanLine = (colorV3[BLUE]-colorV1[BLUE]) / h;
	}
	else if(render->interp_mode == GZ_NORMALS){
		h = (((*verts)[vert_index[2]][1])-((*verts)[vert_index[0]][1]));
		edge2.xPerScanLine = (norms[vert_index[2]][0]-norms[vert_index[0]][0]) / h;
		edge2.yPerScanLine = (norms[vert_index[2]][1]-norms[vert_index[0]][1]) / h;
		edge2.zPerScanLine = (norms[vert_index[2]][2]-norms[vert_index[0]][2]) / h;
	}

	/* If no horizontal bottom edge then need to setup DDA for the V2-V3 edge */
	if(h_bottom == 0){
		//-----------------------------------------------------------------------------------------------------
		/* Setup DDA for edge 3 -> V2-V3 */
		//-----------------------------------------------------------------------------------------------------

		for(i = 0; i < 3; i++){
			edge3.start[i] = (*verts)[vert_index[1]][i];
			edge3.current[i] = edge3.start[i];
		}
		for(i = 0; i < 3; i++)
			edge3.end[i] = (*verts)[vert_index[2]][i];
		
		edge3.slopex = (((*verts)[vert_index[2]][0])-((*verts)[vert_index[1]][0])) / (((*verts)[vert_index[2]][1])-((*verts)[vert_index[1]][1]));
		edge3.slopez = (((*verts)[vert_index[2]][2])-((*verts)[vert_index[1]][2])) / (((*verts)[vert_index[2]][1])-((*verts)[vert_index[1]][1]));

		if(render->interp_mode == GZ_COLOR){
			// COLOR INTERPOLATION HW4
			h = (((*verts)[vert_index[2]][1])-((*verts)[vert_index[1]][1]));
			edge3.rPerScanLine = ((colorV3[RED]-colorV2[RED])) / h;
			edge3.gPerScanLine = ((colorV3[GREEN]-colorV2[GREEN])) / h;
			edge3.bPerScanLine = ((colorV3[BLUE]-colorV2[BLUE])) / h;
		}
		else if(render->interp_mode == GZ_NORMALS){
			h = (((*verts)[vert_index[2]][1])-((*verts)[vert_index[1]][1]));
			edge3.xPerScanLine = ((norms[vert_index[2]][0]-norms[vert_index[1]][0])) / h;
			edge3.yPerScanLine = ((norms[vert_index[2]][1]-norms[vert_index[1]][1])) / h;
			edge3.zPerScanLine = ((norms[vert_index[2]][2]-norms[vert_index[1]][2])) / h;
		}
	}

	/* if there is no bottom/top horizontal edge then
	- find "x" value at the point where the horizontal edge(which you draw)intersects 
	- x at intersection = V1[x] + (V2[y] * dx/dy)
	*/
	if((h_bottom == 0) && (h_top == 0)){ 
		height = (((*verts)[vert_index[1]][1]) - ((*verts)[vert_index[0]][1]));
		slope = (((*verts)[vert_index[2]][0] - (*verts)[vert_index[0]][0]) / ((*verts)[vert_index[2]][1] - (*verts)[vert_index[0]][1]));
		x = ((*verts)[vert_index[0]][0]) + (height * slope);
	}

	/* check for the type of triangle */
	if((h_top == 0) && (h_bottom == 0)){
		if(x > ((*verts)[vert_index[1]][0])){ /* If x > V2[x] */
			/* TYPE1 - tris with V3[x] > V2[x] */
			left1 = edge1;
			left2 = edge3;				
			right1 = edge2;
			if(render->interp_mode == GZ_COLOR){

				leftColor[RED] = colorV1[RED];
				leftColor[GREEN] = colorV1[GREEN];
				leftColor[BLUE] = colorV1[BLUE];
			
				rightColor[RED] = colorV1[RED];
				rightColor[GREEN] = colorV1[GREEN];
				rightColor[BLUE] = colorV1[BLUE];

				thirdColor[RED] = colorV2[RED];
				thirdColor[GREEN] = colorV2[GREEN];
				thirdColor[BLUE] = colorV2[BLUE];

			}else if(render->interp_mode == GZ_NORMALS){

				leftNormal[X] = norms[vert_index[0]][X];
				leftNormal[Y] = norms[vert_index[0]][Y];
				leftNormal[Z] = norms[vert_index[0]][Z];

				rightNormal[X] = norms[vert_index[0]][X];
				rightNormal[Y] = norms[vert_index[0]][Y];
				rightNormal[Z] = norms[vert_index[0]][Z];

				thirdNormal[X] = norms[vert_index[1]][X];
				thirdNormal[Y] = norms[vert_index[1]][Y];
				thirdNormal[Z] = norms[vert_index[1]][Z];

			}
		}else{
			/* TYPE2 - tris with V2[x] > V3[x] */
			left1 = edge2;
			right1 = edge1;
			right2 = edge3;

			if(render->interp_mode == GZ_COLOR){

				leftColor[RED] = colorV1[RED];
				leftColor[GREEN] = colorV1[GREEN];
				leftColor[BLUE] = colorV1[BLUE];

				rightColor[RED] = colorV1[RED];
				rightColor[GREEN] = colorV1[GREEN];
				rightColor[BLUE] = colorV1[BLUE];

				thirdColor[RED] = colorV2[RED];
				thirdColor[GREEN] = colorV2[GREEN];
				thirdColor[BLUE] = colorV2[BLUE];

			}else if(render->interp_mode == GZ_NORMALS){

				leftNormal[X] = norms[vert_index[0]][X];
				leftNormal[Y] = norms[vert_index[0]][Y];
				leftNormal[Z] = norms[vert_index[0]][Z];

				rightNormal[X] = norms[vert_index[0]][X];
				rightNormal[Y] = norms[vert_index[0]][Y];
				rightNormal[Z] = norms[vert_index[0]][Z];

				thirdNormal[X] = norms[vert_index[1]][X];
				thirdNormal[Y] = norms[vert_index[1]][Y];
				thirdNormal[Z] = norms[vert_index[1]][Z];

			}
		}
	}else{
		/* TYPE3 - tris with a horizontal top edge */
		if(h_top == 1){
			/* compare V1[x] & V2[x] to find the left & right edges */
			if(((*verts)[vert_index[0]][0]) < ((*verts)[vert_index[1]][0])){
				left1 = edge2;
				right1 = edge3;
				
				if(render->interp_mode == GZ_COLOR){

					leftColor[RED] = colorV1[RED];
					leftColor[GREEN] = colorV1[GREEN];
					leftColor[BLUE] = colorV1[BLUE];

					rightColor[RED] = colorV2[RED];
					rightColor[GREEN] = colorV2[GREEN];
					rightColor[BLUE] = colorV2[BLUE];

				}else if(render->interp_mode == GZ_NORMALS){
					
					leftNormal[X] = norms[vert_index[0]][X];
					leftNormal[Y] = norms[vert_index[0]][Y];
					leftNormal[Z] = norms[vert_index[0]][Z];

					rightNormal[X] = norms[vert_index[1]][X];
					rightNormal[Y] = norms[vert_index[1]][Y];
					rightNormal[Z] = norms[vert_index[1]][Z];
				}
			}else{
				left1 = edge3;
				right1 = edge2;

				if(render->interp_mode == GZ_COLOR){	

					leftColor[RED] = colorV2[RED];
					leftColor[GREEN] = colorV2[GREEN];
					leftColor[BLUE] = colorV2[BLUE];

					rightColor[RED] = colorV1[RED];
					rightColor[GREEN] = colorV1[GREEN];
					rightColor[BLUE] = colorV1[BLUE];

				}else if(render->interp_mode == GZ_NORMALS){

					leftNormal[X] = norms[vert_index[1]][X];
					leftNormal[Y] = norms[vert_index[1]][Y];
					leftNormal[Z] = norms[vert_index[1]][Z];

					rightNormal[X] = norms[vert_index[0]][X];
					rightNormal[Y] = norms[vert_index[0]][Y];
					rightNormal[Z] = norms[vert_index[0]][Z];

				}
			}	
		}
		/* TYPE4 - tris with a horizontal bottom edge */
		if(h_bottom == 1){
			/* compare V2[x] & V3[x] to find the left & right edges */
			if(((*verts)[vert_index[1]][0]) < ((*verts)[vert_index[2]][0])){
				left1 = edge1;
				right1 = edge2;

				if(render->interp_mode == GZ_COLOR){	

					leftColor[RED] = colorV1[RED];
					leftColor[GREEN] = colorV1[GREEN];
					leftColor[BLUE] = colorV1[BLUE];

					rightColor[RED] = colorV1[RED];
					rightColor[GREEN] = colorV1[GREEN];
					rightColor[BLUE] = colorV1[BLUE];

				}else if(render->interp_mode == GZ_NORMALS){

					leftNormal[X] = norms[vert_index[0]][X];
					leftNormal[Y] = norms[vert_index[0]][Y];
					leftNormal[Z] = norms[vert_index[0]][Z];

					rightNormal[X] = norms[vert_index[0]][X];
					rightNormal[Y] = norms[vert_index[0]][Y];
					rightNormal[Z] = norms[vert_index[0]][Z];

				}
			}else{
				left1 = edge2;
				right1 = edge1;

				if(render->interp_mode == GZ_COLOR){		

					leftColor[RED] = colorV1[RED];
					leftColor[GREEN] = colorV1[GREEN];
					leftColor[BLUE] = colorV1[BLUE];

					rightColor[RED] = colorV1[RED];
					rightColor[GREEN] = colorV1[GREEN];
					rightColor[BLUE] = colorV1[BLUE];
				
				}else if(render->interp_mode == GZ_NORMALS){

					leftNormal[X] = norms[vert_index[0]][X];
					leftNormal[Y] = norms[vert_index[0]][Y];
					leftNormal[Z] = norms[vert_index[0]][Z];

					rightNormal[X] = norms[vert_index[0]][X];
					rightNormal[Y] = norms[vert_index[0]][Y];
					rightNormal[Z] = norms[vert_index[0]][Z];

				}
			}
		}
	}/* end of tris TYPES */

	/* Advance the edges (left1 and right1) to the top scan line */
	deltay = (ceil((*verts)[vert_index[0]][1])) - ((*verts)[vert_index[0]][1]);
	left1.current[0] += (left1.slopex * deltay);
	left1.current[1] += deltay;
	left1.current[2] += (left1.slopez * deltay);

	right1.current[0] += (right1.slopex * deltay);
	right1.current[1] += deltay;
	right1.current[2] += (right1.slopez * deltay);
	
	/* If no horizontal top/bottom edge then 
	advance the third edge which will be swapped with the second edge 
	*/
	if((h_top == 0) && (h_bottom == 0)){
		if(x > ((*verts)[vert_index[1]][0])){
			deltay = (ceil(left2.start[1]))-(left2.start[1]);
			left2.current[0] += (left2.slopex * deltay);
			left2.current[1] += deltay;
			left2.current[2] += (left2.slopez * deltay);
		}else{
			deltay = (ceil(right2.start[1]))-(right2.start[1]);
			right2.current[0] += (right2.slopex * deltay);
			right2.current[1] += deltay;
			right2.current[2] += (right2.slopez * deltay);
		}
	}
	

//----------------------------------------------------------------------------------
// RENDER THE FIRST HALF OF THE TRIANGLE
//----------------------------------------------------------------------------------
	/* execute the scan lines until end of left/right edge is reached */

	while((left1.current[1] <= left1.end[1]) && (right1.current[1] <= right1.end[1])){
		// setup the span DDA for every scanline
	
		SPAN.start[0] = left1.current[0];
		SPAN.start[1] = left1.current[2];
	
		SPAN.end[0] = right1.current[0];
		SPAN.end[1] = right1.current[2];
	
		SPAN.current[0] = SPAN.start[0];
		SPAN.current[1] = SPAN.start[1];
	
		SPAN.slopez = ((SPAN.end[1])-(SPAN.start[1])) / ((SPAN.end[0])-(SPAN.start[0]));
		
		if(render->interp_mode == GZ_COLOR){
			// width / dcolor   -> (color = r/g/b)
			w = ((SPAN.end[0])-(SPAN.start[0]));
			rPerPixel = ((rightColor[RED] - leftColor[RED])) / w;
			gPerPixel = ((rightColor[GREEN] - leftColor[GREEN])) / w;
			bPerPixel = ((rightColor[BLUE] - leftColor[BLUE])) / w;
		
		}else if(render->interp_mode == GZ_NORMALS){

			w = ((SPAN.end[0])-(SPAN.start[0]));
			xPerPixel = (rightNormal[X] - leftNormal[X]) / w;
			yPerPixel = (rightNormal[Y] - leftNormal[Y]) / w;
			zPerPixel = (rightNormal[Z] - leftNormal[Z]) / w;

		}

		// advance the span DDA to the left most pixel on the current scan line 
		deltax = (ceil(SPAN.start[0]))-(SPAN.start[0]);
	
		SPAN.current[0] += deltax;
		SPAN.current[1] += (deltax * SPAN.slopez);	
		
		if(render->interp_mode == GZ_COLOR){
			//reset the colors for every scan line
			r = leftColor[RED];
			g = leftColor[GREEN];
			b = leftColor[BLUE];

		}else if(render->interp_mode == GZ_NORMALS){

			Xval = leftNormal[X];
			Yval = leftNormal[Y];
			Zval = leftNormal[Z];

		}
		for(indexx = SPAN.current[0]; indexx <= SPAN.end[0]; indexx++){

			if(render->interp_mode == GZ_COLOR){
			// GOURAND SHADING 

				finalR = (short)(r);
				finalG = (short)(g);
				finalB = (short)(b);

			}else if(render->interp_mode == GZ_NORMALS){
			// PHONG SHADING	
				
				interpolatedNormal[X] = Xval;
				interpolatedNormal[Y] = Yval;
				interpolatedNormal[Z] = Zval;
				
				// normalise the normal vector
				val = sqrt(pow(interpolatedNormal[X], 2) + pow(interpolatedNormal[Y], 2) + pow(interpolatedNormal[Z], 2));
				for(index = 0; index <= 2; index++)
					interpolatedNormal[index] /= val;

				phongColor = FindColor(render, interpolatedNormal); 

				for(index = 0; index <= 2; index++){
					if(((*phongColor)[index]) > 1)
						((*phongColor)[index]) = 1;

					((*phongColor)[index]) *= 4095;
				}

				finalR = (short)((*phongColor)[RED]);
				finalG = (short)((*phongColor)[GREEN]);
				finalB = (short)((*phongColor)[BLUE]);

			}else{
				// fill the pixel with flat shading
				r = ctoi(render->flatcolor[0]);
				g =	ctoi(render->flatcolor[1]);
				b = ctoi(render->flatcolor[2]);
			}

			// Z-buffer to remove hidden surfaces - smaller value of z wins
			deltaz = (ceil(SPAN.current[1]))-(SPAN.current[1]);
		
			z = (int)(SPAN.current[1] + deltaz);
			GzPutDisplay(render->display, (int)SPAN.current[0], (int)left1.current[1], finalR, finalG, finalB, a, z);
			
			// advance the scan line along the x for the next pixel
			SPAN.current[0] += 1;
			SPAN.current[1] += (1 * SPAN.slopez);  

			if(render->interp_mode == GZ_COLOR){
			
				r += rPerPixel;
				g += gPerPixel;
				b += bPerPixel;
			
			}else if(render->interp_mode == GZ_NORMALS){

				Xval += xPerPixel;
				Yval += yPerPixel;
				Zval += zPerPixel;
			}
		}
		
		// advance the y value to start the next scan line 
		left1.current[0] += (left1.slopex * 1);
		left1.current[1] += 1;
		left1.current[2] += (left1.slopez * 1);
	
		right1.current[0] += (right1.slopex * 1);
		right1.current[1] += 1;
		right1.current[2] += (right1.slopez * 1);

		if(render->interp_mode == GZ_COLOR){
			// Increment the color values at both the ends of every scan line
			// LEFT EDGE COLOR 
			leftColor[RED] += left1.rPerScanLine;
			leftColor[GREEN] += left1.gPerScanLine;
			leftColor[BLUE] += left1.bPerScanLine;
			// RIGHT EDGE COLOR
			rightColor[RED] += right1.rPerScanLine;
			rightColor[GREEN] += right1.gPerScanLine;
			rightColor[BLUE] += right1.bPerScanLine;

		}else if(render->interp_mode == GZ_NORMALS){

			leftNormal[X] += left1.xPerScanLine;
			leftNormal[Y] += left1.yPerScanLine;
			leftNormal[Z] += left1.zPerScanLine;

			rightNormal[X] += right1.xPerScanLine;
			rightNormal[Y] += right1.yPerScanLine;
			rightNormal[Z] += right1.zPerScanLine;

		}
	}

	// check for the end of the second edge 
	if((h_top == 0) && (h_bottom == 0)){
		if(left1.current[1] > left1.end[1]){
			left1 = left2;
			
			if(render->interp_mode == GZ_COLOR){

				leftColor[RED] = thirdColor[RED];
				leftColor[GREEN] = thirdColor[GREEN];
				leftColor[BLUE] = thirdColor[BLUE];

			}else if(render->interp_mode == GZ_NORMALS){

				leftNormal[X] = thirdNormal[X];
				leftNormal[Y] = thirdNormal[Y];
				leftNormal[Z] = thirdNormal[Z];
				
			}
		}
		else if(right1.current[1] > right1.end[1]){
			right1 = right2;  
			
			if(render->interp_mode == GZ_COLOR){

				rightColor[RED] = thirdColor[RED];
				rightColor[GREEN] = thirdColor[GREEN];
				rightColor[BLUE] = thirdColor[BLUE];

			}else if(render->interp_mode == GZ_NORMALS){

				rightNormal[X] = thirdNormal[X];
				rightNormal[Y] = thirdNormal[Y];
				rightNormal[Z] = thirdNormal[Z];
				
			}
		}
	} 
//-----------------------------------------------------------------------------------
//	RENDERING SECOND HALF OF TRIANGLE
//-----------------------------------------------------------------------------------
	
	while((left1.current[1] <= left1.end[1]) && (right1.current[1] <= right1.end[1])){
		// setup the span DDA for every scanline 
		
		SPAN.start[0] = left1.current[0];
		SPAN.start[1] = left1.current[2];
	
		SPAN.end[0] = right1.current[0];
		SPAN.end[1] = right1.current[2];
	
		SPAN.current[0] = SPAN.start[0];
		SPAN.current[1] = SPAN.start[1];
	
		SPAN.slopez = ((SPAN.end[1])-(SPAN.start[1])) / ((SPAN.end[0])-(SPAN.start[0]));
		
		if(render->interp_mode == GZ_COLOR){
			// width / dcolor/dx -> (color = r/g/b)
			w = ((SPAN.end[0])-(SPAN.start[0]));
			rPerPixel = ((rightColor[RED] - leftColor[RED])) / w;
			gPerPixel = ((rightColor[GREEN] - leftColor[GREEN])) / w;
			bPerPixel = ((rightColor[BLUE] - leftColor[BLUE])) / w;

		}else if(render->interp_mode == GZ_NORMALS){

			w = ((SPAN.end[0])-(SPAN.start[0]));
			xPerPixel = (rightNormal[X] - leftNormal[X]) / w;
			yPerPixel = (rightNormal[Y] - leftNormal[Y]) / w;
			zPerPixel = (rightNormal[Z] - leftNormal[Z]) / w;

		}

		// advance the span DDA to the left most pixel on the current scan line 
		deltax = (ceil(SPAN.start[0]))-(SPAN.start[0]);
	
		SPAN.current[0] += deltax;
		SPAN.current[1] += (deltax * SPAN.slopez);	
		
		if(render->interp_mode == GZ_COLOR){	
			//reset the colors for every scan line
			r = leftColor[RED];
			g = leftColor[GREEN];
			b = leftColor[BLUE];

		}else if(render->interp_mode == GZ_NORMALS){

			Xval = leftNormal[X];
			Yval = leftNormal[Y];
			Zval = leftNormal[Z];

		}
		for(indexx = SPAN.current[0]; indexx <= SPAN.end[0]; indexx++){	
			if(render->interp_mode == GZ_COLOR){
			// GOURAND SHADING 

				finalR = (short)(r);
				finalG = (short)(g);
				finalB = (short)(b);

			}else if(render->interp_mode == GZ_NORMALS){
				// PHONG SHADING	
				
				interpolatedNormal[X] = Xval;
				interpolatedNormal[Y] = Yval;
				interpolatedNormal[Z] = Zval;

				// normalise the normal vector
				val = sqrt(pow(interpolatedNormal[X], 2) + pow(interpolatedNormal[Y], 2) + pow(interpolatedNormal[Z], 2));
				for(index = 0; index <= 2; index++)
					interpolatedNormal[index] /= val;

				phongColor = FindColor(render, interpolatedNormal); 
				
				for(index = 0; index <= 2; index++){
					if(((*phongColor)[index]) > 1)
						((*phongColor)[index]) = 1;

					((*phongColor)[index]) *= 4095;
				}

				finalR = (short)((*phongColor)[RED]);
				finalG = (short)((*phongColor)[GREEN]);
				finalB = (short)((*phongColor)[BLUE]);

			}else{
				// fill the pixel with flat shading
				r = ctoi(render->flatcolor[0]);
				g =	ctoi(render->flatcolor[1]);
				b = ctoi(render->flatcolor[2]);
			}

			// Z-buffer to remove hidden surfaces - smaller value of z wins
			deltaz = (ceil(SPAN.current[1])) - (SPAN.current[1]);
			z = (int)(SPAN.current[1] + deltaz);
			GzPutDisplay(render->display, (int)SPAN.current[0], (int)left1.current[1], finalR, finalG, finalB, a, z);
			
			// advance the scan line along the x for the next pixel 
			SPAN.current[0] += 1;
			SPAN.current[1] += (1 * SPAN.slopez);  
			
			if(render->interp_mode == GZ_COLOR){
			
				r += rPerPixel;
				g += gPerPixel;
				b += bPerPixel;
			
			}else if(render->interp_mode == GZ_NORMALS){

				Xval += xPerPixel;
				Yval += yPerPixel;
				Zval += zPerPixel;
			}
		}
				
		// advance the y value to start the next scan line
		left1.current[0] += (left1.slopex * 1);
		left1.current[1] += 1;
		left1.current[2] += (left1.slopez * 1);
			
		right1.current[0] += (right1.slopex * 1);
		right1.current[1] += 1;
		right1.current[2] += (right1.slopez * 1);

		if(render->interp_mode == GZ_COLOR){
			// Increment the color values at both the ends of every scan line
			// LEFT EDGE COLOR 
			leftColor[RED] += left1.rPerScanLine;
			leftColor[GREEN] += left1.gPerScanLine;
			leftColor[BLUE] += left1.bPerScanLine;
			// RIGHT EDGE COLOR
			rightColor[RED] += right1.rPerScanLine;
			rightColor[GREEN] += right1.gPerScanLine;
			rightColor[BLUE] += right1.bPerScanLine;

		}else if(render->interp_mode == GZ_NORMALS){

			leftNormal[X] += left1.xPerScanLine;
			leftNormal[Y] += left1.yPerScanLine;
			leftNormal[Z] += left1.zPerScanLine;

			rightNormal[X] += right1.xPerScanLine;
			rightNormal[Y] += right1.yPerScanLine;
			rightNormal[Z] += right1.zPerScanLine;

		}

	}
return GZ_SUCCESS;
}

// END OF CODE