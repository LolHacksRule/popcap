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

@protocol EAGLTouchDelegate <NSObject>
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event;
@end

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView
{    
@private
  id<EAGLTouchDelegate> touchDelegate;
  EAGLContext *context;

  // The pixel dimensions of the CAEAGLLayer
  GLint width;
  GLint height;

    // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view
  GLuint framebuffer;
  GLuint colorRenderbuffer;
}


/** touch delegate */
@property(nonatomic,readwrite,assign) id<EAGLTouchDelegate> touchDelegate;

- (id)initWithFrame:(CGRect)frame;
- (BOOL)resizeFromLayer;
- (void)swapBuffers;

@end
