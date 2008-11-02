# -*- coding: utf-8 -*-
# -*- python -*-
# Author: Luo Jinghua

import configs.linux
import configs.linux.opengl
import configs.linux.directfb

def AddOptions(opts):
    configs.linux.directfb.AddOptions(opts)
    configs.linux.opengl.AddOptions(opts)

def Configure(env):
    configs.linux.directfb.Configure(env)
    configs.linux.opengl.Configure(env)
