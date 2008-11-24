/******************************************************************************
 * Name         : egltypes.h
 * Author       : Ben Bowman
 * Created      : 04/11/2003
 *
 * Copyright    : 2003-2006 by Imagination Technologies Limited.
 *              : All rights reserved. No part of this software, either
 *              : material or conceptual may be copied or distributed,
 *              : transmitted, transcribed, stored in a retrieval system or
 *              : translated into any human or computer language in any form
 *              : by any means, electronic, mechanical, manual or otherwise,
 *              : or disclosed to third parties without the express written
 *              : permission of Imagination Technologies Limited, 
 *              : Home Park Estate, Kings Langley, Hertfordshire,
 *              : WD4 8LZ, U.K.
 *
 * Platform     : ANSI
 *
 * $Date: 2006/02/27 13:52:47 $ $Revision: 1.3 $
 * $Log: egltypes.h $
 *****************************************************************************/

#ifndef _egltypes_h_
#define _egltypes_h_

#if defined(SUPPORT_X11)

#include <X11/Xlib.h>

typedef Display* NativeDisplayType;
typedef Window NativeWindowType;
typedef Pixmap NativePixmapType;

#else


#if defined(__SYMBIAN32__)

#include <e32def.h>

typedef TInt NativeDisplayType;

/* 
	Declare these as void although they points to classes - we can't
	include	a C++ header file as the EGL files are all written in C.
*/
#define NativeWindowType void* /* Is really an RWindow* */
#define NativePixmapType void* /* Is really a CFbsBitmap* */

#else

#if defined(_WIN32)

#include <windows.h>
typedef HDC NativeDisplayType;
typedef HWND NativeWindowType;
typedef HBITMAP NativePixmapType;


#else

typedef int NativeDisplayType;
typedef void *NativeWindowType;
typedef void *NativePixmapType;

#endif
#endif
#endif

#ifndef APIENTRY
#define APIENTRY
#endif


/*
** Types and resources
*/
typedef int EGLBoolean;
typedef int EGLint;
typedef void *EGLDisplay;
typedef void *EGLConfig;
typedef void *EGLSurface;
typedef void *EGLContext;

/*
** EGL and native handle values
*/
#define EGL_DEFAULT_DISPLAY ((NativeDisplayType)0)
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_NO_DISPLAY ((EGLDisplay)0)
#define EGL_NO_SURFACE ((EGLSurface)0)

#endif /* _egltypes_h_ */

/******************************************************************************
 End of file (egltypes.h)
******************************************************************************/

