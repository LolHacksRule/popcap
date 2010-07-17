//
//  EAGLView.h
//  GLES
//
//  Created by Luo Jinghua on 10-6-17.
//  Copyright home 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#import "EAGLRenderer.h"

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView
{    
@private
    id <EAGLRenderer> renderer;
}

- (void)present:(id)sender;

@end
