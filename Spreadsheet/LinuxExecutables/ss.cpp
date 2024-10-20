#include <gtk/gtk.h>
#include <iostream>
#include <vector>
#include <string>
#include "xlsxwriter.h"

const int INITIAL_ROWS = 100;
const int INITIAL_COLS = 26;

GtkWidget *window;
GtkWidget *grid;
GtkWidget *scrolled_window;
std::vector<std::vector<GtkWidget*>> cells;
int current_row = 0;
int current_col = 0;

void save_data_xlsx(const char* filename) {
    lxw_workbook  *workbook  = workbook_new(filename);
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);

    for (size_t i = 0; i < cells.size(); ++i) {
        for (size_t j = 0; j < cells[i].size(); ++j) {
            const char* text = gtk_entry_get_text(GTK_ENTRY(cells[i][j]));
            worksheet_write_string(worksheet, i, j, text, NULL);
        }
    }

    workbook_close(workbook);
}

void add_row() {
    int new_row = cells.size();
    cells.push_back(std::vector<GtkWidget*>());
    for (size_t j = 0; j < cells[0].size(); ++j) {
        GtkWidget* entry = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(entry), 10);
        gtk_grid_attach(GTK_GRID(grid), entry, j, new_row, 1, 1);
        cells[new_row].push_back(entry);
    }
    gtk_widget_show_all(grid);
}

void add_column() {
    int new_col = cells[0].size();
    for (size_t i = 0; i < cells.size(); ++i) {
        GtkWidget* entry = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(entry), 10);
        gtk_grid_attach(GTK_GRID(grid), entry, new_col, i, 1, 1);
        cells[i].push_back(entry);
    }
    gtk_widget_show_all(grid);
}

void move_focus(int row, int col) {
    if (row < 0 || col < 0) return;
    
    while (static_cast<size_t>(row) >= cells.size()) add_row();
    while (static_cast<size_t>(col) >= cells[0].size()) add_column();

    gtk_widget_grab_focus(cells[row][col]);
    current_row = row;
    current_col = col;

    // Scroll to make the focused cell visible
    GtkAdjustment *adj;
    adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    gtk_adjustment_clamp_page(adj, row * 30.0, (row + 1) * 30.0);
    adj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    gtk_adjustment_clamp_page(adj, col * 80.0, (col + 1) * 80.0);
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    switch(event->keyval) {
        case GDK_KEY_Up:
            move_focus(current_row - 1, current_col);
            return TRUE;
        case GDK_KEY_Down:
            move_focus(current_row + 1, current_col);
            return TRUE;
        case GDK_KEY_Left:
            move_focus(current_row, current_col - 1);
            return TRUE;
        case GDK_KEY_Right:
            move_focus(current_row, current_col + 1);
            return TRUE;
        default:
            return FALSE;
    }
}

void on_menu_file_new(GtkWidget *widget, gpointer data) {
    for (auto &row : cells) {
        for (auto &cell : row) {
            gtk_entry_set_text(GTK_ENTRY(cell), "");
        }
    }
    move_focus(0, 0);
}

void on_menu_file_save_as(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Save File",
                                         GTK_WINDOW(window),
                                         action,
                                         "_Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "_Save",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    chooser = GTK_FILE_CHOOSER(dialog);

    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
    
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.xlsx");
    gtk_file_filter_set_name(filter, "Excel files");
    gtk_file_chooser_add_filter(chooser, filter);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(chooser);
        
        std::string filenameStr(filename);
        if (filenameStr.substr(filenameStr.length() - 5) != ".xlsx") {
            filenameStr += ".xlsx";
        }
        
        save_data_xlsx(filenameStr.c_str());
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

gboolean on_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
    save_data_xlsx("auto_save.xlsx");
    gtk_main_quit();
    return FALSE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Simple Spreadsheet");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "delete-event", G_CALLBACK(on_delete_event), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Menu bar
    GtkWidget *menubar = gtk_menu_bar_new();
    GtkWidget *fileMenu = gtk_menu_new();
    GtkWidget *fileMi = gtk_menu_item_new_with_label("File");
    GtkWidget *newMi = gtk_menu_item_new_with_label("New");
    GtkWidget *saveAsMi = gtk_menu_item_new_with_label("Save As");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), newMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), saveAsMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi);

    g_signal_connect(G_OBJECT(newMi), "activate", G_CALLBACK(on_menu_file_new), NULL);
    g_signal_connect(G_OBJECT(saveAsMi), "activate", G_CALLBACK(on_menu_file_save_as), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    // Scrolled window for the grid
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    // Grid for cells
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), grid);

    // Create initial cells
    for (int i = 0; i < INITIAL_ROWS; ++i) {
        cells.push_back(std::vector<GtkWidget*>());
        for (int j = 0; j < INITIAL_COLS; ++j) {
            GtkWidget* entry = gtk_entry_new();
            gtk_entry_set_width_chars(GTK_ENTRY(entry), 10);
            gtk_grid_attach(GTK_GRID(grid), entry, j, i, 1, 1);
            cells[i].push_back(entry);
        }
    }

    gtk_widget_show_all(window);
    move_focus(0, 0);  // Set initial focus
    gtk_main();

    return 0;
}
