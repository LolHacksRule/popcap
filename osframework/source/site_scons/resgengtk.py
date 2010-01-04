#!/usr/bin/env python
import gobject
import gtk
import pango
import os

(
    COLUMN_PROJECT,
    COLUMN_XML,
    COLUMN_CODE,
    COLUMN_PREFIX
) = range(4)

class ProjectDialog(gtk.Dialog):
    def __init__(self, title, parent = None):
        gtk.Dialog.__init__(self, title, parent, 0,
                            (gtk.STOCK_OK, gtk.RESPONSE_OK,
                             gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))

        hbox = gtk.HBox(False, 8)
        hbox.set_border_width(8)
        self.vbox.pack_start(hbox, False, False, 0)

        table = gtk.Table(4, 4)
        table.set_row_spacings(4)
        table.set_col_spacings(4)
        hbox.pack_start(table, True, True, 0)

        label = gtk.Label('Project name:')
        table.attach(label, 0, 1, 0, 1, 0, 0)
        entry = gtk.Entry()
        table.attach(entry, 1, 2, 0, 1)
        self.name_entry = entry

        label = gtk.Label('Resource xml:')
        table.attach(label, 0, 1, 1, 2, 0, 0)
        entry = gtk.Entry()
        table.attach(entry, 1, 2, 1, 2)
        button = gtk.Button('Browse...')
        button.connect('clicked', self.open_xml_dialog)
        table.attach(button, 2, 3, 1, 2, 0, 0)
        self.resource_entry = entry

        label = gtk.Label('Output directory:')
        table.attach(label, 0, 1, 2, 3, 0, 0)
        entry = gtk.Entry()
        table.attach(entry, 1, 2, 2, 3)
        button = gtk.Button('Browse...')
        button.connect('clicked', self.open_dir_dialog)
        table.attach(button, 2, 3, 2, 3, 0, 0)
        self.output_entry = entry

        label = gtk.Label('Prefix:')
        table.attach(label, 0, 1, 3, 4, 0, 0)
        entry = gtk.Entry()
        table.attach(entry, 1, 2, 3, 4)
        self.prefix_entry = entry

    def open_file_dialog(self, widget, title = None, patterns = None,
                         action = gtk.FILE_CHOOSER_ACTION_OPEN):
        chooser = gtk.FileChooserDialog(title = title,
                                        action = action,
                                        buttons = (gtk.STOCK_CANCEL,
                                                   gtk.RESPONSE_CANCEL,
                                                   gtk.STOCK_OPEN,
                                                   gtk.RESPONSE_OK))
        chooser.set_default_response(gtk.RESPONSE_OK)
        for p in patterns or []:
            filter = gtk.FileFilter()
            filter.set_name(p[0])
            filter.add_pattern(p[1])
            chooser.add_filter(filter)
            chooser.set_filter(filter)
        response = chooser.run()

        result = None
        if response == gtk.RESPONSE_OK:
            result = chooser.get_filename()

        chooser.destroy()
        return result

    def open_xml_dialog(self, widget):
        result = self.open_file_dialog(widget, 'Choose a xml',
                                       [('resource xml', '*.xml')])
        if result:
            self.resource_entry.set_text(result)

    def open_dir_dialog(self, widget):
        result = self.open_file_dialog(widget, 'Choose a directory', None,
                                       gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER)
        if result:
            self.output_entry.set_text(result)

    def set_values(self, result):
        self.name_entry.set_text(result[0])
        self.resource_entry.set_text(result[1])
        self.output_entry.set_text(result[2])
        self.prefix_entry.set_text(result[3])

    def get_result(self):
        name = self.name_entry.get_text()
        xml = self.resource_entry.get_text()
        output = self.output_entry.get_text()
        prefix = self.prefix_entry.get_text()
        if not prefix:
            prefix = 'Res'
        return (name, xml, output, prefix)

class ProjectListStore(gtk.Window):
    def __init__(self, parent=None):
        # create window, etc
        gtk.Window.__init__(self)
        try:
            self.set_screen(parent.get_screen())
        except AttributeError:
            self.connect('destroy', lambda *w: gtk.main_quit())
        self.set_title(self.__class__.__name__)

        self.set_border_width(8)
        self.set_default_size(480, 320)

        vbox = gtk.VBox(False, 8)
        self.add(vbox)

        sw = gtk.ScrolledWindow()
        sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
        sw.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
        vbox.pack_start(sw)

        # create tree model
        model = self.__create_model()

        # create tree view
        treeview = gtk.TreeView(model)
        treeview.set_rules_hint(True)
        treeview.set_search_column(COLUMN_PREFIX)
        treeview.connect("row_activated", self.on_row_activated)

        sw.add(treeview)
        self.treeview = treeview

        # add columns to the tree view
        self.__add_columns(treeview)

        ### add buttons
        hbox = gtk.HBox(False, 8)
        vbox.pack_start(hbox, False, False)

        button = gtk.Button(label = 'New Project..')
        button.connect('clicked', self.on_new_project)
        hbox.pack_start(button, False, False)

        button = gtk.Button(label = 'Modify Project..')
        button.connect('clicked', self.on_modify_project)
        hbox.pack_start(button, False, False)

        button = gtk.Button(label = 'Delete Project')
        button.connect('clicked', self.on_delete_project)
        hbox.pack_start(button, False, False)

        button = gtk.Button(label = 'Generate Code')
        button.connect('clicked', self.on_generate_code)
        hbox.pack_start(button, False, False)

        self.show_all()

    def __create_model(self):
        lstore = gtk.ListStore(
            gobject.TYPE_STRING,
            gobject.TYPE_STRING,
            gobject.TYPE_STRING,
            gobject.TYPE_STRING)

        self.load_projects()
        for item in self.projects:
            iter = lstore.append()
            lstore.set(iter,
                COLUMN_PROJECT, item[COLUMN_PROJECT],
                COLUMN_XML, item[COLUMN_XML],
                COLUMN_CODE, item[COLUMN_CODE],
                COLUMN_PREFIX, item[COLUMN_PREFIX])
        return lstore

    def fixed_toggled(self, cell, path, model):
        # get toggled iter
        iter = model.get_iter((int(path),))
        fixed = model.get_value(iter, COLUMN_PROJECT)

        # do something with the value
        fixed = not fixed

        # set new value
        model.set(iter, COLUMN_PROJECT, fixed)

    def __add_columns(self, treeview):
        model = treeview.get_model()

        # column for project
        column = gtk.TreeViewColumn('project', gtk.CellRendererText(),
                                    text = COLUMN_PROJECT)

        treeview.append_column(column)

        # column for resource xml
        renderer = gtk.CellRendererText()
        renderer.set_property('ellipsize', pango.ELLIPSIZE_START)
        column = gtk.TreeViewColumn('resource xml', renderer,
                                    text = COLUMN_XML)
        column.set_resizable(True)
        column.set_sort_column_id(COLUMN_XML)
        treeview.append_column(column)

        # columns for output directory
        renderer = gtk.CellRendererText()
        renderer.set_property('ellipsize', pango.ELLIPSIZE_START)
        column = gtk.TreeViewColumn('output directory', renderer,
                                    text = COLUMN_CODE)
        column.set_resizable(True)
        column.set_sort_column_id(COLUMN_CODE)
        treeview.append_column(column)

        # column for prefix
        column = gtk.TreeViewColumn('prefix', gtk.CellRendererText(),
                                     text = COLUMN_PREFIX)
        column.set_sort_column_id(COLUMN_PREFIX)
        treeview.append_column(column)

    path = os.path.expanduser('~/.config/resgengtk/projects.pickle')
    def load_projects(self):
        import pickle

        self.projects = []
        if not os.path.exists(ProjectListStore.path):
            return self.projects

        try:
            self.projects = pickle.load(file(ProjectListStore.path, 'rb'))
        except Exception, e:
            print 'Failed to load existing projects', e
        return self.projects

    def save_projects(self):
        import pickle

        try:
            if not os.path.exists(os.path.dirname(ProjectListStore.path)):
                os.makedirs(os.path.dirname(ProjectListStore.path))
            pickle.dump(self.projects, file(ProjectListStore.path, 'wb'))
        except:
            pass

    def on_new_project(self, *args):
        dialog = ProjectDialog('New Project', self)
        dialog.show_all()
        response = dialog.run()
        dialog.destroy()

        if response != gtk.RESPONSE_OK:
            return

        item = dialog.get_result()
        if not item[1]:
            return

        self.projects.append(item)
        self.save_projects()

        model = self.treeview.get_model()
        iter = model.append()
        model.set(iter,
                  COLUMN_PROJECT, item[COLUMN_PROJECT],
                  COLUMN_XML, item[COLUMN_XML],
                  COLUMN_CODE, item[COLUMN_CODE],
                  COLUMN_PREFIX, item[COLUMN_PREFIX])

    def on_modify_project(self, *args):
        selection = self.treeview.get_selection ()
        model, rows = selection.get_selected_rows ()
        if not rows:
            return

        dialog = ProjectDialog('Modify Project', self)
        iter = model.get_iter (rows[0])
        dialog.set_values(model.get(iter, 0, 1, 2, 3))
        dialog.show_all()
        response = dialog.run()
        dialog.destroy()

        if response != gtk.RESPONSE_OK:
            return

        item = dialog.get_result()
        if not item[1]:
            return

        self.projects[rows[0][0]] = item
        model = self.treeview.get_model()
        iter = model.get_iter(rows[0])
        model.set(iter,
                  COLUMN_PROJECT, item[COLUMN_PROJECT],
                  COLUMN_XML, item[COLUMN_XML],
                  COLUMN_CODE, item[COLUMN_CODE],
                  COLUMN_PREFIX, item[COLUMN_PREFIX])

        self.save_projects()

    def on_row_activated(self, treeview, path, view_column):
        self.on_modify_project()

    def on_delete_project(self, *args):
        selection = self.treeview.get_selection ()
        model, rows = selection.get_selected_rows ()
        if not rows:
            return

        rows.reverse ()
        for row in rows:
            iter = model.get_iter (row)
            del self.projects[row[0]]
            del model[row]

    def on_generate_code(self, *args):
        import resgen
        selection = self.treeview.get_selection ()
        model, rows = selection.get_selected_rows ()
        if not rows:
            return

        rows.reverse ()
        for row in rows:
            iter = model.get_iter (row)
            item = self.projects[row[0]]

            try:
                gen = resgen.ResGen()
                gen.parse(item[COLUMN_XML])
                output = os.path.join(item[COLUMN_CODE], item[COLUMN_PREFIX])
                gen.write(output)
            except Exception, e:
                print e

def main():
    ProjectListStore()
    gtk.main()

if __name__ == '__main__':
    main()

