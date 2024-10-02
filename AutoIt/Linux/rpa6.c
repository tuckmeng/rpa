#include <gtk/gtk.h>
#include <time.h>

// Function to update the clock
gboolean update_clock(GtkLabel *label) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    // Format the time as HH:MM:SS
    char time_string[9]; // HH:MM:SS + null terminator
    strftime(time_string, sizeof(time_string), "%H:%M:%S", local);

    // Update the label with the current time
    gtk_label_set_text(label, time_string);

    return TRUE; // Keep the timeout active
}

// Function to create the main window and start the clock
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Digital Clock");
    gtk_window_set_default_size(GTK_WINDOW(window), 200,50);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE); // Disable resizing
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a box to manage layout
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Create a label to display the time
    GtkWidget *label = gtk_label_new("");

    // Set CSS for the label
    gtk_widget_set_name(label, "clockLabel");
    
    // Add the label to the box
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);

    // Load CSS
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        "#clockLabel { "
        "   font-size: 32px; "
        "   color: black; "            // Set text color to black
        "   background-color: white; " // Set background color to white
        "   padding: 3px; "
        "}",
        -1, NULL);
    
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    g_object_unref(css_provider); // Unreference the CSS provider

    // Show all widgets
    gtk_widget_show_all(window);

    // Update the clock every second
    g_timeout_add_seconds(1, (GSourceFunc)update_clock, label);

    // Start the main loop
    gtk_main();

    return 0;
}
