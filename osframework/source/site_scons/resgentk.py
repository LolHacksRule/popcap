#!/bin/env python
from Tkinter import *
from tkFileDialog import askopenfilename, askdirectory

import os
import resgen

(
    COLUMN_PROJECT,
    COLUMN_XML,
    COLUMN_CODE,
    COLUMN_PREFIX
) = range(4)

path = os.path.expanduser('~/.config/resgengtk/projects.pickle')

from Tkinter import *
import os

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

class Project(Dialog):
    def __init__(self, master = None, title = None):
        Dialog.__init__(self, master, title)

    def body(self, frame):
        label = Label(frame, text = 'Project name:')
        label.grid(column = 0, row = 0)
        entry = Entry(frame)
        entry.grid(column = 1, row = 0, sticky = W)
        self.name_entry = entry

        label = Label(frame, text = 'Resouce xml:')
        label.grid(column = 0, row = 1)
        entry = Entry(frame, width = 50)
        entry.grid(column = 1, row = 1, sticky = W)
        button = Button(frame, text = "Browse...", command = self.selectResouceXml)
        button.grid(column = 2, row = 1)
        self.resource_entry = entry

        label = Label(frame, text = 'Output directory:')
        label.grid(column = 0, row = 2)
        entry = Entry(frame, width = 50)
        entry.grid(column = 1, row = 2, sticky = W)
        button = Button(frame, text = "Browse...", command = self.selectOutputDir)
        button.grid(column = 2, row = 2)
        self.output_entry = entry

        label = Label(frame, text = 'Prefix:')
        label.grid(column = 0, row = 3)
        entry = Entry(frame)
        entry.grid(column = 1, row = 3, sticky = W)
        self.prefix_entry = entry

        return frame

    def validate(self):
        xml = self.resource_entry.get()
        if not xml:
            return False
        return True

    def apply(self):
        name = self.name_entry.get()
        xml = self.resource_entry.get()
        output = self.output_entry.get()
        prefix = self.prefix_entry.get()
        if not xml:
            return

        self.result = (name, xml, output, prefix)

    def selectResouceXml(self, *args):
        result = askopenfilename()
        if result:
            self.resource_entry.delete(0, END)
            self.resource_entry.insert(END, result)

    def selectOutputDir(self, *args):
        result = askdirectory()
        if result:
            self.output_entry.delete(0, END)
            self.output_entry.insert(END, result)

    def setup(self, item):
        self.name_entry.insert(END, item[COLUMN_PROJECT])
        self.resource_entry.insert(END, item[COLUMN_XML])
        self.output_entry.insert(END, item[COLUMN_CODE])
        self.prefix_entry.insert(END, item[COLUMN_PREFIX])

        self.resource_entry.xview(END)
        self.output_entry.xview(END)

class Application(Frame):
    def __init__(self, master = None):
        Frame.__init__(self, master)
        self.loadProjects()
        self.pack()
        self.createWidgets()

    def createWidgets(self):
        ### top frame
        frame = Frame(self)
        frame.pack(side = TOP, fill = BOTH, padx = 4, pady = 4)

        self.list = Listbox(frame, width = 50, height = 10)
        self.list.pack(fill = BOTH, side = LEFT)

        yscroll = Scrollbar(frame, command = self.list.yview, orient = VERTICAL)
        yscroll.pack(fill = BOTH, side = LEFT)
        self.list.configure(yscrollcommand = yscroll.set)

        txtframe = Frame(frame)
        txtframe.pack(fill = BOTH, side = LEFT)

        label = Label(txtframe, text = 'Project name:')
        label.grid(column = 0, row = 0)
        entry = Entry(txtframe)
        entry.grid(column = 1, row = 0, sticky = W)
        self.name_entry = entry

        label = Label(txtframe, text = 'Resouce xml:')
        label.grid(column = 0, row = 1)
        entry = Entry(txtframe, width = 50)
        entry.grid(column = 1, row = 1, sticky = W)
        self.resource_entry = entry

        label = Label(txtframe, text = 'Output directory:')
        label.grid(column = 0, row = 2)
        entry = Entry(txtframe, width = 50)
        entry.grid(column = 1, row = 2, sticky = W)
        self.output_entry = entry

        label = Label(txtframe, text = 'Prefix:')
        label.grid(column = 0, row = 3)
        entry = Entry(txtframe)
        entry.grid(column = 1, row = 3, sticky = W)
        self.prefix_entry = entry

        self.updateProjectList()
        self.updateProjectInfo()

        self.list.bind('<ButtonRelease-1>', self.listSelectionChanged)

        frame = Frame(self)
        frame.pack(side = BOTTOM, padx = 4, pady = 4)

        self.newProjectButton = Button (frame,
                                        text = 'New Project',
                                        command = self.newProject)
        self.newProjectButton.pack(side = LEFT)

        self.modifyProjectButton = Button (frame,
                                           text = 'Modify Project',
                                           command = self.modifyProject)
        self.modifyProjectButton.pack(side = LEFT)

        self.deleteProjectButton = Button (frame,
                                           text = 'Delete Project',
                                           command = self.deleteProject)
        self.deleteProjectButton.pack(side = LEFT)

        self.genCodeButton = Button (frame,
                                     text = 'Generate Code',
                                     command = self.genCode)
        self.genCodeButton.pack(side = LEFT)

        self.quitButton = Button (frame,
                                  text = 'Quit',
                                  command = self.quit)
        self.quitButton.pack(side = LEFT)

    def updateProjectList(self):
        for item in self.projects:
            self.list.insert(END, item[0])

    def updateProjectInfo(self, index = 0):
        if index >= 0 and index < len(self.projects):
            item = self.projects[index]
        else:
            item = ('', '', '', '')

        self.name_entry.config(state = NORMAL)
        self.resource_entry.config(state = NORMAL)
        self.output_entry.config(state = NORMAL)
        self.prefix_entry.config(state = NORMAL)

        self.name_entry.delete(0, END)
        self.resource_entry.delete(0, END)
        self.output_entry.delete(0, END)
        self.prefix_entry.delete(0, END)

        self.name_entry.insert(END, item[COLUMN_PROJECT])
        self.resource_entry.insert(END, item[COLUMN_XML])
        self.output_entry.insert(END, item[COLUMN_CODE])
        self.prefix_entry.insert(END, item[COLUMN_PREFIX])

        self.resource_entry.xview(END)
        self.output_entry.xview(END)

        self.name_entry.config(state = DISABLED)
        self.resource_entry.config(state = DISABLED)
        self.output_entry.config(state = DISABLED)
        self.prefix_entry.config(state = DISABLED)

    def newProject(self, *args):
        dialog = Project(self)
        result = dialog.run()
        if not result:
            return
        self.projects.append(result)
        self.list.insert(END, self.projects[-1][COLUMN_PROJECT])
        self.saveProjects()

    def modifyProject(self, *args):
        if not self.list.curselection():
            return
        index = int(self.list.curselection()[0])

        dialog = Project(self)
        dialog.setup(self.projects[index])
        result = dialog.run()
        if not result:
            return

        self.projects[index] = result
        self.list.delete(index)
        self.list.insert(index, result[COLUMN_PROJECT])
        self.updateProjectInfo(index)
        self.saveProjects()

    def deleteProject(self, *args):
        if not self.list.curselection():
            return

        index = int(self.list.curselection()[0])
        del self.projects[index]
        self.list.delete(index)
        self.saveProjects()

    def genCode(self, *args):
        if not self.list.curselection():
            return
        index = int(self.list.curselection()[0])
        item = self.projects[index]
        try:
            gen = resgen.ResGen()
            gen.parse(item[COLUMN_XML])
            output = os.path.join(item[COLUMN_CODE], item[COLUMN_PREFIX])
            gen.write(output)
        except Exception, e:
            print e

    def loadProjects(self):
        import pickle

        self.projects = []
        if not os.path.exists(path):
            return self.projects

        try:
            self.projects = pickle.load(file(path, 'rb'))
        except Exception, e:
            print 'Failed to load existing projects', e
        return self.projects

    def saveProjects(self):
        import pickle

        try:
            if not os.path.exists(os.path.dirname(path)):
                os.makedirs(os.path.dirname(path))
            pickle.dump(self.projects, file(path, 'wb'))
        except:
            pass

    def listSelectionChanged(self, *args):
        # get selected line index
        if self.list.curselection():
            index = self.list.curselection()[0]
            self.updateProjectInfo(int(index))

app = Application()
app.master.title("Resource code generator")
app.mainloop()
