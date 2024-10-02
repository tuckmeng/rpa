#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>

GtkWidget *field;
time_t start_time = 0, end_time = 0;

// Function to update the field with the difference in days
void calculate_days() {
    if (end_time > start_time) {
        double difference = difftime(end_time, start_time);
        int days = (int)(difference / (60 * 60 * 24)); // Convert seconds to days
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "%d", days);
        gtk_entry_set_text(GTK_ENTRY(field), buffer);
    } else {
        gtk_entry_set_text(GTK_ENTRY(field), "Invalid dates");
    }
}

// Callback function to handle date selection
void on_date_selected(GtkCalendar *calendar, gpointer user_data) {
    guint year, month, day;
    gtk_calendar_get_date(calendar, &year, &month, &day);
    struct tm tm = {0};
    tm.tm_year = year - 1900; // tm_year is years since 1900
    tm.tm_mon = month;        // Month is 0-11
    tm.tm_mday = day;         // Day of the month

    time_t selected_time = mktime(&tm);
    if (user_data == NULL) { // If user_data is NULL, it's for start date
        start_time = selected_time;
    } else { // Else it's for end date
        end_time = selected_time;
        calculate_days();
    }

    gtk_widget_destroy(gtk_widget_get_toplevel(GTK_WIDGET(calendar))); // Close dialog
}

// Function to open calendar for date selection
void open_calendar(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Select Date", 
                                                    NULL, 
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_ACCEPT,
                                                    "_Cancel", GTK_RESPONSE_REJECT,
                                                    NULL);
    
    GtkWidget *calendar = gtk_calendar_new();
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), calendar, TRUE, TRUE, 0);
    
    g_signal_connect(calendar, "day-selected", G_CALLBACK(on_date_selected), data);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
    
    gtk_widget_show_all(dialog);
}

// Main function to create the GUI
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Days Between");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create layout
    GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), layout);

    // Create Start button
    GtkWidget *start_button = gtk_button_new_with_label("Start Date");
    gtk_widget_set_margin_start(start_button, 1);
    gtk_widget_set_margin_end(start_button, 1);
    gtk_widget_set_margin_top(start_button, 1);
    gtk_widget_set_margin_bottom(start_button, 1);
    g_signal_connect(start_button, "clicked", G_CALLBACK(open_calendar), NULL);
    gtk_box_pack_start(GTK_BOX(layout), start_button, TRUE, TRUE, 0);

    // Create End button
    GtkWidget *end_button = gtk_button_new_with_label("End Date");
    gtk_widget_set_margin_start(end_button, 1);
    gtk_widget_set_margin_end(end_button, 1);
    gtk_widget_set_margin_top(end_button, 1);
    gtk_widget_set_margin_bottom(end_button, 1);
    g_signal_connect(end_button, "clicked", G_CALLBACK(open_calendar), (gpointer)1);
    gtk_box_pack_start(GTK_BOX(layout), end_button, TRUE, TRUE, 0);

    // Create horizontal box for label and entry field
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(layout), hbox, TRUE, TRUE, 0);

    // Create label for days between
    GtkWidget *label = gtk_label_new("Days Between:");
    gtk_widget_set_margin_start(label, 1);
    gtk_widget_set_margin_end(label, 1);
    gtk_widget_set_margin_top(label, 1);
    gtk_widget_set_margin_bottom(label, 1);
    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

    // Create field to display days
    field = gtk_entry_new();
    gtk_widget_set_margin_start(field, 1);
    gtk_widget_set_margin_end(field, 1);
    gtk_widget_set_margin_top(field, 1);
    gtk_widget_set_margin_bottom(field, 1);
    gtk_box_pack_start(GTK_BOX(hbox), field, TRUE, TRUE, 0);

    // Show all widgets
    gtk_widget_show_all(window);
    
    // Start the main loop
    gtk_main();

    return 0;
}
