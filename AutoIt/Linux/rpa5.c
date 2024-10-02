#include <gtk/gtk.h>

static void on_button_clicked(GtkWidget *widget, gpointer data) {
    // Intentionally left empty to do nothing when a button is pressed
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 260, 250);
    
    // Create a vertical box to hold the buttons and entry
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create the entry for displaying the calculation
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "0.");
    gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);

    // Create buttons
    const char *buttons[] = {
        "MC", "MR", "MS", "M+",
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "0", ".", "C", "+",
        "1/x", "Sqrt", "%", "=",
        "Backspace", "CE"
    };

    // Create button grid
    GtkWidget *grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);

    for (int i = 0; i < 24; i++) {
        GtkWidget *button = gtk_button_new_with_label(buttons[i]);
        g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);
        gtk_grid_attach(GTK_GRID(grid), button, i % 4, i / 4, 1, 1);
    }

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
