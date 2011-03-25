/*
 * Copyright (C) 2007 The Android Open Source Project
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

package org.jinghua;

// Wrapper for native library

public class GameJni {

     static {
         System.loadLibrary("GameLauncher");
     }

    public static native void    init(String sourcedir,
                                      String datadir,
                                      String filesdir,
				      GameView view,
                                      int    width,
                                      int    height);
    public static native boolean render();
    public static native void    uninit();
    public static native boolean pause();
    public static native boolean resume();

    public static native void    queueKeyEvent(int down,
					       long time,
					       int keycode,
					       int keychar);
    public static native void    queuePointerEvent(int   id,
						   int   action,
						   long  time,
						   int   flags,
						   float x,
						   float y,
						   float pressure);
    public static native void    readAudioData();

    public static native void    textInput(String text);
}
