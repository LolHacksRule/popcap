#!/usr/bin/env python
from Tkinter import *
from tkFileDialog import askopenfilename, askdirectory

import os
import socket
import struct
import random

EVENT_KEY_DOWN		     = 1
EVENT_KEY_UP		     = 2
EVENT_MOUSE_BUTTON_PRESS     = 3
EVENT_MOUSE_BUTTON_RELEASE   = 4
EVENT_MOUSE_WHEEL_UP	     = 5
EVENT_MOUSE_WHEEL_DOWN	     = 6
EVENT_MOUSE_MOTION	     = 7
EVENT_ACTIVE		     = 8
EVENT_EXPOSE		     = 9
EVENT_QUIT		     = 10
EVENT_DEVICE_SEARCHING       = 11
EVENT_DEVICE_LOST            = 12
EVENT_DEVICE_DISCOVERED      = 13
EVENT_ACC                    = 14

EVENT_FLAGS_AXIS       = 1 << 0
EVENT_FLAGS_REL_AXIS   = 1 << 1
EVENT_FLAGS_BUTTON     = 1 << 2
EVENT_FLAGS_KEY_CODE   = 1 << 3
EVENT_FLAGS_KEY_CHAR   = 1 << 4
EVENT_FLAGS_AXIS_RANGE = 1 << 5

KEYCODE_UNKNOWN	      = 0x00
KEYCODE_LBUTTON	      = 0x01
KEYCODE_RBUTTON       = 0x02
KEYCODE_CANCEL        = 0x03
KEYCODE_MBUTTON       = 0x04
KEYCODE_BACK          = 0x08
KEYCODE_TAB           = 0x09
KEYCODE_CLEAR         = 0x0C
KEYCODE_RETURN        = 0x0D
KEYCODE_SHIFT         = 0x10
KEYCODE_CONTROL       = 0x11
KEYCODE_MENU          = 0x12
KEYCODE_PAUSE         = 0x13
KEYCODE_CAPITAL       = 0x14
KEYCODE_KANA          = 0x15
KEYCODE_HANGEUL       = 0x15
KEYCODE_HANGUL        = 0x15
KEYCODE_JUNJA         = 0x17
KEYCODE_FINAL         = 0x18
KEYCODE_HANJA         = 0x19
KEYCODE_KANJI         = 0x19
KEYCODE_ESCAPE        = 0x1B
KEYCODE_CONVERT       = 0x1C
KEYCODE_NONCONVERT    = 0x1D
KEYCODE_ACCEPT        = 0x1E
KEYCODE_MODECHANGE    = 0x1F
KEYCODE_SPACE         = 0x20
KEYCODE_PRIOR         = 0x21
KEYCODE_NEXT          = 0x22
KEYCODE_END           = 0x23
KEYCODE_HOME          = 0x24
KEYCODE_LEFT          = 0x25
KEYCODE_UP            = 0x26
KEYCODE_RIGHT         = 0x27
KEYCODE_DOWN          = 0x28
KEYCODE_SELECT        = 0x29
KEYCODE_PRINT         = 0x2A
KEYCODE_EXECUTE       = 0x2B
KEYCODE_SNAPSHOT      = 0x2C
KEYCODE_INSERT        = 0x2D
KEYCODE_DELETE        = 0x2E
KEYCODE_HELP          = 0x2F
KEYCODE_ASCIIBEGIN    = 0x30
KEYCODE_ASCIIEND      = 0x5A
KEYCODE_LWIN          = 0x5B
KEYCODE_RWIN          = 0x5C
KEYCODE_APPS          = 0x5D
KEYCODE_NUMPAD0       = 0x60
KEYCODE_NUMPAD1       = 0x61
KEYCODE_NUMPAD2       = 0x62
KEYCODE_NUMPAD3       = 0x63
KEYCODE_NUMPAD4       = 0x64
KEYCODE_NUMPAD5       = 0x65
KEYCODE_NUMPAD6       = 0x66
KEYCODE_NUMPAD7       = 0x67
KEYCODE_NUMPAD8       = 0x68
KEYCODE_NUMPAD9       = 0x69
KEYCODE_MULTIPLY      = 0x6A
KEYCODE_ADD           = 0x6B
KEYCODE_SEPARATOR     = 0x6C
KEYCODE_SUBTRACT      = 0x6D
KEYCODE_DECIMAL       = 0x6E
KEYCODE_DIVIDE        = 0x6F
KEYCODE_F1            = 0x70
KEYCODE_F2            = 0x71
KEYCODE_F3            = 0x72
KEYCODE_F4            = 0x73
KEYCODE_F5            = 0x74
KEYCODE_F6            = 0x75
KEYCODE_F7            = 0x76
KEYCODE_F8            = 0x77
KEYCODE_F9            = 0x78
KEYCODE_F10           = 0x79
KEYCODE_F11           = 0x7A
KEYCODE_F12           = 0x7B
KEYCODE_F13           = 0x7C
KEYCODE_F14           = 0x7D
KEYCODE_F15           = 0x7E
KEYCODE_F16           = 0x7F
KEYCODE_F17           = 0x80
KEYCODE_F18           = 0x81
KEYCODE_F19           = 0x82
KEYCODE_F20           = 0x83
KEYCODE_F21           = 0x84
KEYCODE_F22           = 0x85
KEYCODE_F23           = 0x86
KEYCODE_F24           = 0x87
KEYCODE_NUMLOCK       = 0x90
KEYCODE_SCROLL        = 0x91
KEYCODE_ASCIIBEGIN2   = 0xB3 #ASCII + 0x80
KEYCODE_ASCIIEND2     = 0xE0

from Tkinter import *
import os

keymap = {
    "UP"    : KEYCODE_UP,
    "DOWN"  : KEYCODE_DOWN,
    "LEFT"  : KEYCODE_LEFT,
    "RIGHT" : KEYCODE_RIGHT,
    "ENTER" : KEYCODE_RETURN,
    "BACKSPACE" : KEYCODE_BACK,
    "ESC" : KEYCODE_ESCAPE,
    "F1" : KEYCODE_F1,
    "F2" : KEYCODE_F2,
    "F3" : KEYCODE_F3,
    "F4" : KEYCODE_F4
}

for c in range(ord('A'), ord('Z')):
    keymap[chr(c)] = c

def hexdump(data):
    length  = len(data)
    width   = 16
    for idx in range(length):
        mod         = idx % width
        rest        = width - mod -1
        s_offset    = idx - mod
        e_offset    = idx
        # offset
        if ( 0 == mod ):
            print "0x%08x" % idx,
        # hex dump
        print hex(ord(data[idx]))[2:].zfill(2),
        # padding dump
        if ( idx == len(data)-1 ):
            for k in range(rest):
                print "  ",
        # char dump
        if ( mod == (width-1) or idx == len(data)-1 ):
            print "\t",
            #print "from %d to %d" % (s_off, e_off)
            for j in range(s_offset, e_offset+1):
                if ( ord(data[j]) < 31 and ord(data[j]) > 126 ):
                    sys.stdout.write(data[j])
                else:
                    sys.stdout.write(".")
            print "\n",
        idx = idx+1

class Dialog(Toplevel):
    def __init__(self, parent, title = None):
        Toplevel.__init__(self, parent)
        self.transient(parent)

        if title:
            self.title(title)

        self.parent = parent
        self.result = None

        body = Frame(self)
        self.initial_focus = self.body(body)
        body.pack(padx=5, pady=5)

        self.buttonbox()
        self.grab_set()

        if not self.initial_focus:
            self.initial_focus = self

        self.protocol("WM_DELETE_WINDOW", self.cancel)
        self.geometry("+%d+%d" % (parent.winfo_rootx() + 50,
                                  parent.winfo_rooty() + 50))

        self.initial_focus.focus_set()

    def run(self):
        self.wait_window(self)
        return self.result

    #
    # construction hooks
    def body(self, master):
        # create dialog body.  return widget that should have
        # initial focus.  this method should be overridden
        pass

    def buttonbox(self):
        # add standard button box. override if you don't want the
        # standard buttons
        box = Frame(self)

        w = Button(box, text="OK", width=10, command=self.ok, default=ACTIVE)
        w.pack(side=LEFT, padx=5, pady=5)
        w = Button(box, text="Cancel", width=10, command=self.cancel)
        w.pack(side=LEFT, padx=5, pady=5)

        self.bind("<Return>", self.ok)
        self.bind("<Escape>", self.cancel)

        box.pack()

    #
    # standard button semantics
    def ok(self, event=None):
        if not self.validate():
            self.initial_focus.focus_set() # put focus back
            return

        self.withdraw()
        self.update_idletasks()

        self.apply()

        self.cancel()

    def cancel(self, event=None):
        # put focus back to the parent window
        self.parent.focus_set()
        self.destroy()

    #
    # command hooks
    def validate(self):
        return 1 # override

    def apply(self):
        pass # override

class ButtonCallback(object):
    def __init__(self, callback, *args):
        self.callback = callback
        self.args = args

    def __call__(self):
        return self.callback(*self.args)

class AccState(object):
    def __init__(self):
        self.swing = False
        self.currentAcc = 0
        self.acc = 0
        self.accAcc = 0

class Application(Frame):
    def __init__(self, master = None):
        Frame.__init__(self, master)
        self.accState = AccState()
        self.mousePos = (0, 0)
        self.pack()
        self.createWidgets()

    def createWidgets(self):
        ### top frame
        frame = Frame(self)
        frame.pack(side = TOP, padx = 4, pady = 4)

        label = Label(frame, text = 'Target IP address:')
        label.grid(column = 0, row = 0)
        entry = Entry(frame)
        entry.grid(column = 1, row = 0)
        self.addr_entry = entry

        buttons = [
            ["F1",   "F2",        "F3"],
            ['A',    'B',         'C'],
            ['K',    'L',         'N'],
            [None,   "UP",        "QUIT"],
            ["LEFT", "DOWN",      "RIGHT"],
            ["ESC",  "ENTER", "BACKSPACE"],
            ["ACC50",  "ACC100", "ACC200"],
            ["ACC400",  "ACC800", "ACC1600"]
        ]

        frame = Frame(self)
        frame.pack(side = TOP, padx = 4, pady = 4)

        self.buttons = {}
        i = 0
        for row in buttons:
            j = 0
            for btn in row:
                if btn:
                    button = Button(frame, text = btn,
                                    command = ButtonCallback(self.buttonPressed, btn),
                                    width = 12)
                    self.buttons[btn] = button
                else:
                    button = Button(frame, text = "    ", width = 12)
                    button.config(state = DISABLED)
                button.grid(column = j, row = i)
                j += 1
            i += 1

        topframe = Frame(self)
        topframe.pack(side = BOTTOM, padx = 4, pady = 4)

        frame = Frame(topframe, width = 640, height = 480, bg = "white")
        self.relativeMouse = IntVar()
        button = Checkbutton(frame, text = "Send relative mouse movements",
                             variable = self.relativeMouse)
        button.grid(column = 0, row = 0)

        self.sendKeepalive = IntVar()
        button = Checkbutton(frame, text = "Send keep alive events",
                             variable = self.sendKeepalive,
                             command = self.onSendKeepalive)
        button.grid(column = 1, row = 0)
        self.keepaliveId = -1

        frame.pack()

        frame = Frame(topframe, width = 640, height = 480, bg = "white")
        frame.pack(side = BOTTOM, padx = 4, pady = 4)
        frame.bind("<Motion>", self.mouseMoved)
        frame.bind("<Enter>", self.mouseIn)
        frame.bind("<Leave>", self.mouseOut)
        frame.bind("<Button-1>", self.button1Pressed)
        frame.bind("<ButtonRelease-1>", self.button1Released)
        frame.bind("<Button-2>", self.button2Pressed)
        frame.bind("<ButtonRelease-2>", self.button2Released)
        self.mouseFrame = frame

    def getRemoteAddress(self):
        host = self.addr_entry.get()
        port = '2799'

        return (host, port)

    def sendData(self, host, port, data):
        if not host:
            host = '127.0.0.1'
        if not port:
            port = '2799'

        fd = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
        fd.sendto (data, (host, int(port)))

    def genKeyEvent(self, keycode, down = True):
        if down:
            evttype = EVENT_KEY_DOWN
        else:
            evttype = EVENT_KEY_UP
        flags = EVENT_FLAGS_KEY_CODE
        if keycode >= ord('A') and keycode <= ord('Z'):
            flags |= EVENT_FLAGS_KEY_CHAR
            keychar = keycode
        else:
            keychar = 0
        s = struct.pack(">IIIIIIIIII",
                        evttype, flags, 0, 0, 0, keycode,
                        keychar, 0, 0, 0)
        return s

    def genQuitEvent(self):
        evttype = EVENT_QUIT
        s = struct.pack(">IIIIIIIIII",
                        evttype, 0, 0, 0, 0, 0,
                        0, 0, 0, 0)
        return s

    def sendKey(self, btn):
        #print btn, "pressed"
        host, port = self.getRemoteAddress()

        if btn == 'QUIT':
            data = self.genQuitEvent()
            self.sendData(host, port, data)
            return

        if btn[0:3] == 'ACC':
            self.sendAcc(int(btn[3:]))
            return

        if btn not in keymap:
            return
        down = self.genKeyEvent(keymap[btn], True)
        up = self.genKeyEvent(keymap[btn], False)
        self.sendData(host, port, down)
        self.sendData(host, port, up)

    def genMouseMotionEvent(self, x, y, relative = True,
                            maxx = 0, maxy = 0):
        evttype = EVENT_MOUSE_MOTION
        flags = 0
        if relative:
            flags |= EVENT_FLAGS_REL_AXIS
        else:
            flags |= EVENT_FLAGS_AXIS
            if maxx != 0 and maxy != 0:
                flags |= EVENT_FLAGS_AXIS_RANGE
        s = struct.pack(">IIiiiiiiii",
                        evttype, flags, x, y, 0, maxx,
                        maxy, 0, 0, 0)
        return s

    def genMouseButtonEvent(self, button, pressed = True):
        if pressed:
            evttype = EVENT_MOUSE_BUTTON_PRESS
        else:
            evttype = EVENT_MOUSE_BUTTON_RELEASE
        flags = 0
        s = struct.pack(">IIIIIIIIII",
                        evttype, flags, 0, 0, button, 0,
                        0, 0, 0, 0)
        return s

    def genAccEvent(self, acc):
        evttype = EVENT_ACC
        flags = 0
        s = struct.pack(">IIIIIIIIII",
                        evttype, flags, acc, 0, 0, 0,
                        0, 0, 0, 0)
        return s

    def sendMouseMotion(self, x, y, relative = True, maxx = 0, maxy = 0):
        host, port = self.getRemoteAddress()
        motion = self.genMouseMotionEvent(x, y, relative, maxx, maxy)
        self.sendData(host, port, motion)

    def sendMouseButton(self, button, pressed = True):
        host, port = self.getRemoteAddress()
        button = self.genMouseButtonEvent(button, pressed)
        self.sendData(host, port, button)

    def buttonPressed(self, btn):
        self.sendKey(btn)

    def mouseIn(self, event):
        self.mouseFrame.grab_set()
        self.initialPos = False

    def mouseOut(self, event):
        self.mouseFrame.grab_release()
        self.initialPos = False

    def mouseMoved(self, event):
        if not self.initialPos:
            self.mousePos = (event.x, event.y)
            self.initialPos = True
        if self.relativeMouse.get():
            x = event.x - self.mousePos[0]
            y = event.y - self.mousePos[1]
        else:
            x = event.x
            y = event.y
        self.mousePos = (event.x, event.y)
        self.sendMouseMotion(x, y, self.relativeMouse.get(),
                             self.mouseFrame.winfo_width(),
                             self.mouseFrame.winfo_height())

    def button1Pressed(self, event):
        self.sendMouseButton(1, True)

    def button2Pressed(self, event):
        self.sendMouseButton(2, True)

    def button1Released(self, event):
        self.sendMouseButton(1, False)

    def button2Released(self, event):
        self.sendMouseButton(2, False)

    def SendAccProc(self):
        host, port = self.getRemoteAddress()

        acc = self.accState.currentAcc
        data = self.genAccEvent(acc)
        self.sendData(host, port, data)

        #print 'sent a acc event:', acc, self.accState.accAcc

        if self.accState.currentAcc == self.accState.acc:
            self.accState.accAcc *= -2
        elif acc == 0 and self.accState.accAcc < 0:
            self.accState.swing = False
            return

        self.accState.currentAcc += self.accState.accAcc
        # add some noise
        self.accState.currentAcc += \
                                 random.randint(-abs(self.accState.accAcc / 2),
                                                abs(self.accState.accAcc / 2))
        if self.accState.currentAcc < 0:
            self.accState.currentAcc = 0
        if self.accState.currentAcc > self.accState.acc:
            self.accState.currentAcc = self.accState.acc
        self.after(8, self.SendAccProc)

    def getAccAcc(self, acc):
        strength = (acc / 50 / 4) or 1
        return 16 * strength

    def sendAcc(self, acc):
        if self.accState.swing:
            if acc > self.accState.acc:
                self.accState.acc = acc
            if self.accState.accAcc > 0:
                self.accState.accAcc += self.getAccAcc(acc)
            else:
                self.accState.accAcc = self.getAccAcc(acc)
            return

        self.accState.swing = True
        self.accState.acc = acc
        self.accState.accAcc = self.getAccAcc(acc)
        self.accState.currentAcc = 0

        self.SendAccProc();

    def onSendKeepalive(self):
        if self.keepaliveId >= 0:
            self.after_cancel(self.keepaliveId)
            self.keepaliveId = -1
        if self.sendKeepalive.get():
            self.keepaliveId = self.after(1000, self.sendKeepaliveEvent)

    def sendKeepaliveEvent(self):
        if not self.sendKeepalive.get():
            return
        self.sendMouseMotion(0, 0, True,
                             self.mouseFrame.winfo_width(),
                             self.mouseFrame.winfo_height())
        self.after(1000, self.sendKeepaliveEvent)

app = Application()
app.master.title("RemoteController for SexyFramework")
app.mainloop()
