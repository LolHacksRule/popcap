/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 1.0 code

#include "GameLauncher.h"

#include <android/log.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define  LOG_TAG    "libGameMain"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
	     = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
static GLvoid InitGL(GLsizei Width, GLsizei Height)
{
    glEnable (GL_BLEND);
    glLineWidth (1.0);
    glDisable (GL_LIGHTING);
    glDisable (GL_DEPTH_TEST);
    glDisable (GL_NORMALIZE);
    glDisable (GL_CULL_FACE);
    glShadeModel (GL_FLAT);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

    glClearColor (0.0, 0.0, 0.0, 0.0);

    glViewport(0, 0, Width, Height);
    glMatrixMode (GL_PROJECTION) ;
    glLoadIdentity ();

    glOrthof (0.0f, Width, Height, 0.0f, -1.0f, 1.0f);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
}

static bool setupGraphics(int w, int h)
{
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);
    LOGI("Screen size: %dx%d\n", w, h);

    InitGL(w, h);
    return true;
}

static void FillRect(float x, float y, float w, float h)
{
    GLfloat verts[4 * 2];

    verts[0] = x;     verts[1] = y;
    verts[2] = x;     verts[3] = y + h;
    verts[4] = x + w; verts[5] = y;
    verts[6] = x + w; verts[7] = y + h;

    glVertexPointer (2, GL_FLOAT, 0, verts);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

static int ncols = 25;
static int nrows = 25;
static int win_width;
static int win_height;

static void DrawGrids(int width, int height)
{
    int i, j, w, h;

    win_width = width;
    win_height = height;
    w = width / ncols;
    h = height / nrows;
    for (i = 0; i < nrows; i++)
    {
        for (j = 0; j < ncols; j++)
        {
            if ((i & 1) ^ (j & 1))
                glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
            else
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            FillRect(j * w, i * h, w, h);
        }
    }
}

static void renderFrame()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    DrawGrids(win_width, win_height);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

extern "C" int GameInit()
{
    GameLauncher *launcher = GameLauncher::getInstance();

    win_width = launcher->getViewWidth();
    win_height = launcher->getViewHeight();
    setupGraphics(launcher->getViewWidth(), launcher->getViewHeight());
    return 0;
}

extern "C" int GameRender()
{
    renderFrame();
    return 1;
}

extern "C" int GameUninit()
{
    LOGI("uninitialized.");
    return 0;
}

