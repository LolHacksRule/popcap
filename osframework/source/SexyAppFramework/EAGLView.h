//
//  EAGLView.h
//  GLES
//
//  Created by Luo Jinghua on 10-6-17.
//  Copyright home 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView
{    
@private
  EAGLContext *context;

  // The pixel dimensions of the CAEAGLLayer
  GLint width;
  GLint height;

    // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view
  GLuint framebuffer;
  GLuint colorRenderbuffer;
}

- (id)initWithFrame:(CGRect)frame;
- (BOOL)resizeFromLayer;
- (void)swapBuffers;

@end
