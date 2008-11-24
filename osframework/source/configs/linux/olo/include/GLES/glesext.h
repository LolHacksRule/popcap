/******************************************************************************
 Name : glesext.h
 Date : June 2004

 $Revision: 1.2 $

 Description : GL extensions.

 This file is part of the TOOLS library (PVRTools.lib) used
 with the PowerVR SDK demos.

 Copyright 2004-2006 by Imagination Technologies Ltd. All rights reserved.
 Information and source code samples contained herein are
 provided "as-is", without representations or warranties, and
 are subject to change without notice. The author cannot support
 modifications or derivative works created from the sample source
 code provided. You may use, reproduce, and modify portions or
 entire sections of the sample source code for the purposes of
 creating applications. Distribution is limited to executable
 or binary portions of the sample source code unless you gain
 written permission from the author.
******************************************************************************/
#ifndef _GLESEXT_H_
#define _GLESEXT_H_


#ifndef APIENTRY
#define APIENTRY
#endif

/**************************************************************************
****************************** IMG GL EXTENSIONS ******************************
**************************************************************************/

#define GL_MODULATE_COLOR_IMG						0x8C04
#define GL_RECIP_ADD_SIGNED_ALPHA_IMG				0x8C05
#define GL_TEXTURE_ALPHA_MODULATE_IMG				0x8C06
#define GL_FACTOR_ALPHA_MODULATE_IMG				0x8C07
#define GL_FRAGMENT_ALPHA_MODULATE_IMG				0x8C08
#define GL_ADD_BLEND_IMG							0x8C09


/******************************************
**    GL_IMG_texture_compression_pvrtc   **
******************************************/
/* Tokens */
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG			0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG			0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG			0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG			0x8C03

/* IMG_texture_stream */
#define GL_TEXTURE_STREAM_IMG						0x8C0D
#define GL_TEXTURE_NUM_STREAM_DEVICES_IMG			0x8C0E
#define GL_TEXTURE_STREAM_DEVICE_WIDTH_IMG			0x8C0F
#define GL_TEXTURE_STREAM_DEVICE_HEIGHT_IMG			0x8C10
#define GL_TEXTURE_STREAM_DEVICE_FORMAT_IMG			0x8C11
#define GL_TEXTURE_STREAM_DEVICE_NUM_BUFFERS_IMG	0x8C12

/*******************************
**   GL_IMG_vertex_program    **
*******************************/
/* Tokens */
#define GL_VERTEX_PROGRAM_ARB						0x8620
#define GL_PROGRAM_STRING_ARB						0x8628
#define GL_PROGRAM_ERROR_STRING_ARB					0x8874
#define GL_MAX_PROGRAM_ATTRIBS_ARB					0x88AD
#define GL_MAX_PROGRAM_PARAMETERS_ARB				0x88A9
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB			0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB			0x88B5
#define GL_MAX_VERTEX_ATTRIBS_ARB					0x8869
#define GL_MAX_PROGRAM_MATRICES_ARB					0x862F
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB		0x862E
#define GL_MATRIX0_ARB								0x88C0
#define GL_MATRIX1_ARB								0x88C1
#define GL_MATRIX2_ARB								0x88C2
#define GL_MATRIX3_ARB								0x88C3
#define GL_MATRIX4_ARB								0x88C4
#define GL_MATRIX5_ARB								0x88C5
#define GL_MATRIX6_ARB								0x88C6
#define GL_MATRIX7_ARB								0x88C7
#define GL_PROGRAM_FORMAT_BINARY_IMG				0x8C0B
#define GL_UNSIGNED_BYTE_NORM_IMG					0x8C0C

/* Typedefs */
typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERARB) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYARB) (GLuint index);
typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYARB) (GLuint index);
typedef void (APIENTRY * PFNGLPROGRAMSTRINGARB) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef void (APIENTRY * PFNGLBINDPROGRAMARB) (GLenum target, GLuint program);
typedef void (APIENTRY * PFNGLDELETEPROGRAMSARB) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * PFNGLGENPROGRAMSARB) (GLsizei n, GLuint *programs);
typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4FARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4FVARB) (GLenum target, GLuint index, const GLfloat *params);
typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4FARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4FVARB) (GLenum target, GLuint index, const GLfloat *params);
typedef GLboolean (APIENTRY * PFNGLISPROGRAMARB) (GLuint program);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4FVARB) (GLuint index, const float *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4XVIMG) (GLuint index, const GLfixed *v);
typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4XIMG) (GLenum target, GLuint index, GLfixed x, GLfixed y, GLfixed z, GLfixed w);
typedef void (APIENTRY * PFNGLPROGRAMLOCALPARAMETER4XVIMG) (GLenum target, GLuint index, const GLfixed *params);
typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4XIMG) (GLenum target, GLuint index, GLfixed x, GLfixed y, GLfixed z, GLfixed w);
typedef void (APIENTRY * PFNGLPROGRAMENVPARAMETER4XVIMG) (GLenum target, GLuint index, const GLfixed *params);

/* Function pointers */
PFNGLVERTEXATTRIBPOINTERARB			myglVertexAttribPointerARB;
PFNGLENABLEVERTEXATTRIBARRAYARB		myglEnableVertexAttribArrayARB;
PFNGLDISABLEVERTEXATTRIBARRAYARB	myglDisableVertexAttribArrayARB;
PFNGLPROGRAMSTRINGARB				myglProgramStringARB;
PFNGLBINDPROGRAMARB					myglBindProgramARB;
PFNGLDELETEPROGRAMSARB				myglDeleteProgramsARB;
PFNGLGENPROGRAMSARB					myglGenProgramsARB;
PFNGLISPROGRAMARB					myglIsProgramARB;
PFNGLPROGRAMENVPARAMETER4FARB		myglProgramEnvParameter4fARB;
PFNGLPROGRAMENVPARAMETER4FVARB		myglProgramEnvParameter4fvARB;
PFNGLPROGRAMLOCALPARAMETER4FARB		myglProgramLocalParameter4fARB;
PFNGLPROGRAMLOCALPARAMETER4FVARB	myglProgramLocalParameter4fvARB;
PFNGLVERTEXATTRIB4FVARB				myglVertexAttrib4fvARB;
PFNGLVERTEXATTRIB4XVIMG				myglVertexAttrib4xvIMG;
PFNGLPROGRAMLOCALPARAMETER4XIMG		myglProgramLocalParameter4xIMG;
PFNGLPROGRAMLOCALPARAMETER4XVIMG	myglProgramLocalParameter4xvIMG;
PFNGLPROGRAMENVPARAMETER4XIMG		myglProgramEnvParameter4xIMG;
PFNGLPROGRAMENVPARAMETER4XVIMG		myglProgramEnvParameter4xvIMG;

/* IMG_TEXTURE_STREAM */
typedef void (APIENTRY * PFNGLGETTEXSTREAMDEVICEATTRIBIVIMG)(GLint device, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLTEXBINDSTREAMIMG)(GLint device, GLint deviceoffset);
typedef const GLubyte * (APIENTRY * PFNGLGETTEXSTREAMDEVICENAMEIMG)(GLint device);

PFNGLGETTEXSTREAMDEVICEATTRIBIVIMG	myglGetTexStreamDeviceAttributeivIMG;
PFNGLTEXBINDSTREAMIMG				myglTexBindStreamIMG;
PFNGLGETTEXSTREAMDEVICENAMEIMG		myglGetTexStreamDeviceNameIMG;
#endif /* _GLESEXT_H_ */

/******************************************************************************
 End of file (glesext.h)
******************************************************************************/
