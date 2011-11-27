#!/usr/bin/env python
from Tkinter import *
from tkFileDialog import asksaveasfile, askdirectory

import os
import select
import socket
import struct
import random
import time
import traceback

from Tkinter import *
import os

class NothingToReadException(Exception):
    pass

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

def currentTimeMillis():
    """Current system time in milliseconds"""
    return long(time.time() * 1000)


class ServiceInfo(object):
    def __init__(self, name, desc, type, address, port):
        self.name = name
        self.desc = desc
        self.type = type
        self.address = address
        self.port = port
        self.cookie = 0
        self.index = -1
        self.maxService = 0

    def __eq__(self, rhs):
        return self.name == rhs.name and self.desc == rhs.name and \
               self.type == rhs.type and self.address == rhs.address and \
               self.port == rhs.port

    def __cmp__(self, rhs):
        if type(other) != ServiceInfo:
            raise TypeError("ServiceInfo cannot be compared to %s"% str(type(other)))
        return cmp(self.index, rhs.index)

    def __hash__(sefl):
        hc = hash(self.name)
        hc ^= hash(self.desc)
        hc ^= hash(self.type)
        hc ^= hash(self.address)
        hc ^= hash(self.port)
        return hc

    def __repr__(self):
        return str((self.name, self.type, self.address, self.port))

class ServiceProvider(object):
    def __init__(self, sm, addr):
        self.sm = sm
        self.addr = addr
        self.name = ''
        self.services = {}
        self.cookie = 0
        self.pendingServices = []
        self.timestamp = 0
        self.updateTS = 0

    def setCookie(self, cookie):
        if self.cookie != cookie:
            self.sm.sendQueryInfo(self.addr)
            self.sm.sendQuery(self.addr)
        self.cookie = cookie

    def getAddr(self):
        return addr

    def getServices(self):
        return self.services

    def servicesChanged(self):
        old_services = self.services
        self.services = {}
        for si in self.pendingServices:
            si.provider = self
            self.services[si.name] = si
        self.pendingServices = []
        self.sm.servicesChanged(self, old_services)

    def parseDict(self, msg, start, end = None):
        if end is None:
            end = len(msg)

        d = {}
        while start < end:
            l = struct.unpack('>H', msg[start:start + 2])[0]
            key = msg[start + 2:start + 2 + l]
            start += 2 + l
            #print 'key:', key

            l = struct.unpack('>H', msg[start:start + 2])[0]
            value = msg[start + 2:start + 2 + l]
            start += 2 + l
            #print 'value:', value

            d[key] = value
        return d

    def parseService(self, msg):
        #hexdump(msg)
        #print

        start = 16
        maxService = struct.unpack('>I', msg[start:start + 4])[0]
        if not maxService:
            return None
        start += 4

        index = struct.unpack('>I', msg[start:start + 4])[0]
        start += 4

        d = self.parseDict(msg, start)

        name = d['name']
        desc = d['desc']
        srvtype = d['type']
        address = d['addr']
        port = d['port']

        si = ServiceInfo(name, desc, srvtype, address, port)
        si.index = index
        si.maxService = maxService
        return si

    def processInfoPacket(self, msg, tag, seqno):
        cookie = struct.unpack('>I', msg[12:16])[0]
        if self.cookie != cookie:
            return
        start = 16
        # skip the service count
        start += 4
        d = self.parseDict(msg, start)
        if 'name' in d:
            name = d['name']
            if name != self.name:
                self.name = name
                self.sm.providerInfoChanged(self)

    def processPacket(self, msg, tag, seqno):
        if tag == 'QRRP':
            cookie = struct.unpack('>I', msg[12:16])
            service = self.parseService(msg)
            if service:
                service.cookie = cookie
                if self.pendingServices:
                    ps = self.pendingServices[0]
                    if ps.cookie != cookie:
                        self.pendingServices.clear()
                for si in self.pendingServices:
                    if si.index == service.index:
                        return
                self.pendingServices.append(service)
                if service.maxService == len(self.pendingServices):
                    self.servicesChanged()
                elif service.maxService == len(self.pendingServices) - 1:
                    self.sm.sendQueryInfo(self.addr)
            else:
                self.pendingServices = []
                self.servicesChanged()
        elif tag == 'QIRP':
            self.processInfoPacket(msg, tag, seqno)

    def setTimestamp(self, timestamp):
        self.timestamp = timestamp

    def isExpired(self, timestamp):
        if abs(timestamp - self.timestamp) > 5000:
            return True
        return False

    def update(self, current):
        if abs(current - self.updateTS) > 10000:
            self.sm.sendQuery(self.addr)
            self.sm.sendQueryInfo(self.addr)
            self.updateTS = current

class ServiceManager(object):
    def __init__(self):
        self.providers = {}
        self.timestamp = 0 #currentTimeMillis()
        self.sock = None
        self.pkseq = 0
        self.addr = '224.0.0.251'
        self.port = 11053
        self.callbacks = {}

    def start(self):
        self.close()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_TTL, 255)
        self.sock.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_LOOP, 1)
        try:
            self.sock.bind(('', 0))
        except:
            traceback.print_exc();
        self.sock.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_IF,
                             socket.inet_aton('0.0.0.0'))
        self.sock.setsockopt(socket.SOL_IP, socket.IP_ADD_MEMBERSHIP,
                             socket.inet_aton(self.addr) + socket.inet_aton('0.0.0.0'))
        self.pkseq = 0

    def close(self):
        try:
            self.sock.close();
        except:
            self.sock = None

    def getProviders(self):
        return self.providers

    def bind(self, event, callback, replace = False):
        valid_events = (
            '<providersAdded>',
            '<providersRemoved>',
            '<providerInfoChanged>',
            '<servicesChanged>',
        )
        if event not in valid_events:
            raise Exception, 'No such event'
        if not event in self.callbacks:
            self.callbacks[event] = []
        callbacks = self.callbacks[event]
        if replace:
            callbacks.clear()
        callbacks.append(callback)

    def emitEvents(self, event, *args, **kwargs):
        if not event in self.callbacks:
            return
        callbacks = self.callbacks[event]
        try:
            for callback in callbacks:
                callback(*args, **kwargs)
        except:
            traceback.print_exc()

    def providersAdded(self, providers):
        self.emitEvents('<providersAdded>', providers)

        ## print 'Provider added', providers

    def providersRemoved(self, providers):
        self.emitEvents('<providersRemoved>', providers)

        ## print 'Provider removed', providers

    def providerInfoChanged(self, provider):
        self.emitEvents('<providerInfoChanged>', provider)

    def servicesChanged(self, provider, oldServices = {}):
        self.emitEvents('<servicesChanged>', provider, oldServices)

        ## print provider.addr, 'servicesChanged()'
        ## services = provider.getServices()
        ## print 'Number of services:', len(services)
        ## i = 0
        ## keys = services.keys()
        ## keys.sort()
        ## for key in keys:
        ##     si = services[key]
        ##     print '%3d)  %s: %s %s:%s' % (i, si.name, si.type, si.address, si.port)
        ##     i += 1
        ## print

    def send(self, packet, addr = None):
        #print 'Sending a packet'
        self.pkseq += 1
        if addr is None:
            addr = (self.addr, self.port)
        self.sock.sendto(packet, 0, addr)

    def broadcastEcho(self):
        # 4b tag 'ECHO'
        # 4b seqno
        # 4b payload length 0
        pk = 'ECHO'
        pk += struct.pack('>I', self.pkseq)
        pk += struct.pack('>I', 0)
        for i in range(10):
            addr = (self.addr, self.port + i)
            self.send(pk, addr)

    def sendQueryInfo(self, addr):
        #print 'Sending a query info packet to', addr
        #print

        # 4b tag 'QRIF'
        # 4b seqno
        # 4b payload length 0
        pk = 'QRIF'
        pk += struct.pack('>I', self.pkseq)
        pk += struct.pack('>I', 0)
        self.send(pk, addr)

    def sendQuery(self, addr):
        #print 'Sending query packet to', addr
        #print

        # 4b tag 'QURY'
        # 4b seqno
        # 4b payload length 0
        pk = 'QURY'
        pk += struct.pack('>I', self.pkseq)
        pk += struct.pack('>I', 0)
        self.send(pk, addr)

    def processEchoReplyPacket(self, msg, tag, seqno, addr):
        (cookie,) = struct.unpack('>I', msg[12:16])
        #print 'cookie:', cookie, addr, len(msg)
        changed = False
        if addr not in self.providers:
            self.providers[addr] = ServiceProvider(self, addr)
            self.providers[addr].setTimestamp(currentTimeMillis())
            changed = True
        self.providers[addr].setCookie(cookie)
        if changed:
            self.providersAdded([self.providers[addr]])

    def processQueryReplyPacket(self, msg, tag, seqno, addr):
        #print 'Received query reply packet from', addr
        if addr in self.providers:
            self.providers[addr].processPacket(msg, tag, seqno)

    def processQueryInfoReplyPacket(self, msg, tag, seqno, addr):
        #print 'Received query reply packet from', addr
        if addr in self.providers:
            self.providers[addr].processPacket(msg, tag, seqno)

    def processPacket(self, msg, addr):
        if addr in self.providers:
            self.providers[addr].setTimestamp(currentTimeMillis())

        tag = msg[0:4]
        (seqno,) = struct.unpack('>I', msg[4:8])
        #print tag, seqno, addr, len(msg)
        if tag == 'ECRP':
            self.processEchoReplyPacket(msg, tag, seqno, addr)
        elif tag == 'QRRP':
            self.processQueryReplyPacket(msg, tag, seqno, addr)
        elif tag == 'QIRP':
            self.processQueryInfoReplyPacket(msg, tag, seqno, addr)
        else:
            print 'Unhandled packet', tag, seqno

    def processPackets(self):
        while self.sock:
            ret = select.select([self.sock], [], [], 0)
            if not ret[0]:
                return
            msg, addr = self.sock.recvfrom(512)
            self.processPacket(msg, addr)

    def update(self):
        if not self.sock:
            return
        current = currentTimeMillis()
        if abs(current - self.timestamp) > 2000:
            self.broadcastEcho()
            self.timestamp = current
        self.processPackets()

        ### removed expired providers
        expired = []
        for addr in self.providers:
            provider = self.providers[addr]
            if provider.isExpired(current):
                expired.append(addr)

        providers = []
        for addr in expired:
            providers.append(self.providers.pop(addr))

        if providers:
            self.providersRemoved(providers)

        for addr in self.providers:
            provider = self.providers[addr]
            provider.update(current)

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

class Application(Frame):
    def __init__(self, master = None):
        Frame.__init__(self, master)
        self.lastSockAddr = ()
        self.sock = None
        self.listen = False
        self.after_id = -1
        self.pack(fill = BOTH, expand = 1)
        self.createWidgets()
        #self.createSocket()
        self.serviceManager = ServiceManager()
        self.serviceManager.start()
        self.after(100, self.updateService)
        self.logServers = []
        self.serviceManager.bind('<providersAdded>', self.providersAdded)
        self.serviceManager.bind('<providersRemoved>', self.providersRemoved)
        self.serviceManager.bind('<providerInfoChanged>', self.providerInfoChanged)
        self.serviceManager.bind('<servicesChanged>', self.servicesChanged)
        self.syncServerMenu()

    def __del__(self):
        self.serviceManager.close()

    def createWidgets(self):
        ### top frame
        frame = LabelFrame(self, text = 'Configuration:')
        frame.pack(side = TOP, padx = 4, pady = 4, fill = X)

        label = Label(frame, text = 'Server address:')
        label.grid(column = 0, row = 0)
        entry = Entry(frame)
        entry.insert(END, 'localhost')
        entry.grid(column = 1, row = 0)
        self.addr_entry = entry

        label = Label(frame, text = 'Server port:')
        label.grid(column = 2, row = 0)
        entry = Entry(frame)
        entry.insert(END, '11035')
        entry.grid(column = 3, row = 0)
        self.port_entry = entry

        menuBtn = Button(frame, text = 'Select a server',
                         command = self.selectServer)
        menuBtn.grid(column = 4, row = 0)
        self.menuBtn = menuBtn

        menu = Menu(self.master, tearoff = 0)
        self.serverMenu = menu

        buttons = [
            ['Start/Stop', 'Clear', 'Save to...'],
        ]

        frame = LabelFrame(self, text = 'Controll:')
        frame.pack(side = TOP, padx = 4, pady = 4, fill = X)

        self.buttons = {}
        i = 0
        for row in buttons:
            j = 0
            for btn in row:
                if btn:
                    button = Button(frame, text = btn,
                                    command = ButtonCallback(self.buttonPressed, btn),
                                    width = 8)
                    self.buttons[btn] = button
                else:
                    button = Button(frame, text = "    ", width = 8)
                    button.config(state = DISABLED)
                button.grid(column = j, row = i)
                j += 1
            i += 1

        frame = LabelFrame(self, text = "Log:")
        frame.pack(side = BOTTOM, padx = 4, pady = 4, fill = BOTH, expand = 1)

        txt = Text(frame, bg = "white")
        txt.pack(side = LEFT, padx = 4, pady = 4, fill = BOTH, expand = 1)
        self.logText = txt

        scrollbar = Scrollbar(frame)
        scrollbar.pack(side = RIGHT, fill = Y)
        scrollbar.config(command = txt.yview)
        txt.config(yscrollcommand = scrollbar.set)
        self.txtScrollbar = scrollbar
        self.logText.yview(END)

    def createSocket(self):
        host, port = self.getAddress()
        if self.sock and self.lastSockAddr == (host, port):
            return

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            self.sock.settimeout(3)
            self.sock.connect((host, port))
            self.sock.settimeout(None)
        except:
            traceback.print_exc()
            self.sock.close()
            self.sock = None
            self.listen = False
            self.appendLog('Failed to onnect to %s:%d\n' % (str(host), port))
            self.after_id = -1
            return

        self.after_id = self.after(10, self.updateLog)
        self.appendLog('Connected to %s:%d\n' % (str(host), port))

    def updateService(self):
        try:
            self.serviceManager.update()
        except:
            traceback.print_exc()
        self.after(100, self.updateService)

    def getAddress(self):
        host = self.addr_entry.get()
        port = self.port_entry.get()
        if not host:
            host = 'localhost'
        if not port:
            port = '11035'
        return (host, int(port))

    def appendLog(self, msg):
        self.logText.config(state = NORMAL)
        self.logText.insert(END, msg)
        self.logText.config(state = DISABLED)
        if self.txtScrollbar.get()[1] == 1.0:
            self.logText.yview(END)

    def getLevelName(self, level):
        level_map = {
            0 : 'DEBUG',
            1 : 'INFO',
            2 : 'WARN',
            3 : 'ERROR'
        }

        if level in level_map:
            return level_map[level]
        return 'UNKNOWN'

    def read(self, sock, pklen):
        payload = ''
        while len(payload) != pklen:
            ret = sock.recv(pklen - len(payload))
            if not ret:
                raise NothingToReadException, 'Nothing to read'
            payload += ret
        return payload

    def readLogPacket(self, sock, payload):
        pid, timestamp, level, taglen, bodylen = struct.unpack(">IIHHI", payload[0:16])
        start = 4 + 4 + 2 + 2 + 4
        tag = payload[start:start + taglen]
        msg = payload[start + taglen:]
        levelName = self.getLevelName(level)
        peer = sock.getpeername()
        self.appendLog('%s:%s: %d %d.%03d %s %s: %s\n' % \
                       (peer[0], peer[1], pid, timestamp / 1000, timestamp % 1000,
                        levelName, tag, msg))

    def readPacket(self, sock):
        import struct
        pktag = self.read(sock, 4)
        pklenstr = self.read(sock, 4)
        pklen = struct.unpack('>I', pklenstr)[0]
        assert (pklen > 8)
        payload = self.read(sock, pklen - 8)
        if pktag == 'LGBD':
            self.readLogPacket(sock, payload)

    def closeLog(self):
        if self.sock:
            self.sock.close()
            self.sock = None
        self.listen = False
        self.appendLog('Connection closed\n')

    def updateLog(self):
        count = 0
        while self.sock and count < 50:
            count += 1
            ret = select.select([self.sock], [], [self.sock], 0)
            if not ret[0] and not ret[1] and not ret[2]:
                break
            if ret and ret[2]:
                self.closeLog()
            elif ret and ret[0]:
                try:
                    self.readPacket(self.sock)
                except:
                    traceback.print_exc()
                    self.closeLog()


        if self.listen:
            self.after_id = self.after(10, self.updateLog)
        else:
            self.after_id = -1

    def filterService(self, services):
        for s in services:
            si = services[s]
            if si.name == 'sexytcplog' and si.type == 'tcp':
                return si
        return None

    def providersAdded(self, providers):
        for provider in providers:
            print 'ProvidersAdded:', provider.addr
            srv = self.filterService(provider.getServices())
            if srv and srv not in self.logServers:
                self.logServers.append(srv)
        print

        self.syncServerMenu()

    def providersRemoved(self, providers):
        for provider in providers:
            print 'ProvidersRemoved:', provider.addr
            srv = self.filterService(provider.getServices())
            if srv and srv in self.logServers:
                self.logServers.remove(srv)
        self.syncServerMenu()

    def providerInfoChanged(self, provider):
        self.syncServerMenu()

    def servicesChanged(self, provider, oldServices):
        print 'ServicesChanged:', provider.addr

        srv = self.filterService(oldServices)
        if srv:
            self.logServers.remove(srv)
        srv = self.filterService(provider.getServices())
        if srv:
            self.logServers.append(srv)

        print self.logServers
        print

        self.syncServerMenu()

    def buttonPressed(self, btn):
        if btn == 'Clear':
            self.clearLog()
        elif btn == 'Start/Stop':
            self.startLog()
        else:
            self.saveLog()

    def startLog(self):
        self.listen = not self.listen
        if self.sock:
            try:
                self.sock.shutdown(2)
            except:
                pass
            try:
                self.sock.close()
            except Exception, e:
                pass
            self.sock = None
        if self.listen:
            self.createSocket()
        else:
            self.after_cancel(self.after_id)
            self.after_id = -1
            self.appendLog("Stopped\n")

    def clearLog(self):
        self.logText.config(state = NORMAL)
        self.logText.delete(1.0, END)
        self.logText.config(state = DISABLED)

    def saveLog(self):
        f = asksaveasfile()
        if not f:
            return

        log = self.logText.get('0.0', END)
        f.write(log)
        f.close()

    def syncServerMenu(self):
        if not self.logServers:
            self.serverMenu.unpost()
            self.serverMenu.delete(0, END)
            self.menuBtn.config(state = DISABLED)
        else:
            self.menuBtn.config(state = NORMAL)
            self.serverMenu.delete(0, END)
            self.serverMenu.add_command(label = 'Cancel')
            for srv in self.logServers:
                name = srv.provider.name or 'Unknown'
                addr = srv.provider.addr[0]
                port = srv.port
                text = '%s (%s:%s)' % (name, str(addr), str(port))
                self.serverMenu.add_command(label = text,
                                            command =
                                            ButtonCallback(self.menuSelectServer, srv));

    def menuSelectServer(self, srv):
        if not srv:
            return

        addr = srv.provider.addr[0]
        port = srv.port
        self.addr_entry.delete(0, END)
        self.addr_entry.insert(0, addr)
        self.port_entry.delete(0, END)
        self.port_entry.insert(0, port)

    def selectServer(self):
        self.serverMenu.unpost()
        self.syncServerMenu()
        self.serverMenu.post(self.menuBtn.winfo_rootx(),
                             self.menuBtn.winfo_rooty())

app = Application()
app.master.title("Log viewer for SexyFramework")
app.mainloop()
