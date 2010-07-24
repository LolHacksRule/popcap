//
//  EAGLView.m
//  GLES
//
//  Created by Luo Jinghua on 10-6-17.
//  Copyright home 2010. All rights reserved.
//

#import "EAGLView.h"

@implementation EAGLView

@synthesize touchDelegate;

// You must implement this method
+ (Class)layerClass
{
  return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame
{    
  if ((self = [super initWithFrame:frame])) {
    // Get the layer
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

    eaglLayer.opaque = TRUE;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
						     [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
						 kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];


    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];

    if (!context || ![EAGLContext setCurrentContext:context]) {
      if (context)
	[context release];
      [self release];
      return nil;
    }

    // Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
    glGenFramebuffersOES(1, &framebuffer);
    glGenRenderbuffersOES(1, &colorRenderbuffer);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);

    [self resizeFromLayer];
  }

  return self;
}

- (BOOL)resizeFromLayer
{
    // Allocate color buffer backing based on the current layer size
    //NSLog(@"resizeFromLayer(%@)", self.layer);

    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(id<EAGLDrawable>)self.layer];
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &width);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &height);
    if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
    {
        NSLog(@"Failed to make complete framebuffer object %x (%dx%d)",
	      glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES), width, height);
        return NO;

    }

    // This application only creates a single default framebuffer which is already bound at this point.
    // This call is redundant, but needed if dealing with multiple framebuffers.
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebuffer);
    return YES;
}

- (void)swapBuffers
{
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
  [context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (void)layoutSubviews
{
  [self resizeFromLayer];
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
  if (touchDelegate)
    [touchDelegate touchesBegan:touches withEvent:event];
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
  if (touchDelegate)
    [touchDelegate touchesMoved:touches withEvent:event];
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
  if (touchDelegate)
    [touchDelegate touchesEnded:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
  if (touchDelegate)
    [touchDelegate touchesCancelled:touches withEvent:event];
}

- (void)dealloc
{
  [EAGLContext setCurrentContext:nil];
  [context release];

  [super dealloc];
}

@end
