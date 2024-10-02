#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>

void show_error_message(const char *message) {
    GtkWidget *dialog;
    GtkWidget *window;

    // Create a temporary window for the message dialog
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Error");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    // Create the message dialog
    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    gtk_widget_destroy(window);
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Check if xdotool exists
    if (system("command -v xdotool > /dev/null 2>&1") != 0) {
        // Show an alert using GTK message box
        show_error_message("Xdotool does not exist on this machine. Please install it.");
        return 1; // Exit if xdotool is not found
    }

    // Start the text editor (xed)
    system("xed &");

    // Wait for a brief moment to allow xed to start
    sleep(1);

    // Send keystrokes to type "This is some text."
    system("xdotool type 'This is some text.'");

    // Wait a moment to ensure the text is typed
    sleep(1);

    // Send Alt+F4 to close xed and trigger the Save dialog
    system("xdotool key alt+F4");

    // Wait for the Save dialog to appear
    sleep(1);

    // Use xdotool to simulate pressing the right arrow key twice
    system("xdotool key Left Left");

    // Add a delay before sending the space key
    sleep(1); // Adjust the delay as needed

    // Simulate pressing the space key
    system("xdotool key space");

    return 0;
}
