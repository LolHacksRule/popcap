# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs

def AddOptions (opts):
    configs.ios.AddOptions (opts)
    configs.AudiereSoundAddOptions (opts)

def EnableEAGL(env):
    env.PrependUnique (FRAMEWORKS = ['OpenGLES', 'QuartzCore', 'CoreGraphics',
                                     'UIKit', 'Foundation'])

def Configure(env):
    configs.ios.Configure (env)
    env.AppendUnique (DRIVERS = ['EAGL'])
    eagl = {}
    eagl['ENABLE'] = EnableEAGL
    env.AppendUnique (EAGL = eagl)
    configs.AudiereSoundConfigure (env)
