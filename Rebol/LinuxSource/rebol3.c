#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for sleep
#include <string.h> // for string handling

// Callback for downloading HTML
void on_download_html(GtkWidget *widget, gpointer data) {
    GtkBuilder *builder = (GtkBuilder *)data;
    GtkTextView *text_view = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "the_html"));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    const gchar *url = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "page_to_read")));

    // Fetching HTML content using curl
    gchar *command = g_strdup_printf("curl -s '%s'", url); // Added quotes for URLs with special characters
    FILE *fp = popen(command, "r");

    if (fp == NULL) {
        gtk_text_buffer_set_text(buffer, "Error: Unable to run curl command.", -1);
        g_free(command);
        return;
    }

    // Read the output from curl
    gchar *line = NULL;
    size_t len = 0;
    GString *result = g_string_new("");

    while (getline(&line, &len, fp) != -1) {
        g_string_append(result, line);
    }
    
    // Clean up
    free(line);
    int status = pclose(fp);

    if (status == -1) {
        gtk_text_buffer_set_text(buffer, "Error: Unable to close curl command.", -1);
    } else {
        // Delay for 10 seconds
        sleep(10);

        // Set the content to the text area
        gtk_text_buffer_set_text(buffer, result->str, -1);
    }
    
    // Clean up
    g_string_free(result, TRUE);
    g_free(command);
}

// Callback for loading local HTML file
void on_load_local_file(GtkWidget *widget, gpointer data) {
    GtkBuilder *builder = (GtkBuilder *)data;
    GtkTextView *text_view = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "the_html"));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gchar *file_content = NULL;
        g_file_get_contents(filename, &file_content, NULL, NULL);
        gtk_text_buffer_set_text(buffer, file_content ? file_content : "Error loading file.", -1);
        g_free(file_content);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// Callback for saving changes to the web
void on_save_to_web(GtkWidget *widget, gpointer data) {
    GtkBuilder *builder = (GtkBuilder *)data;
    GtkTextView *text_view = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "the_html"));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    const gchar *url = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "page_to_read")));
    GtkTextIter start, end;
    gchar *text;

    gtk_text_buffer_get_bounds(buffer, &start, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    // Save using curl
    gchar *command = g_strdup_printf("curl -X PUT --data '%s' '%s'", text, url);
    system(command);
    g_free(command);
}

// Callback for saving changes to a local file
void on_save_local_file(GtkWidget *widget, gpointer data) {
    GtkBuilder *builder = (GtkBuilder *)data;
    GtkTextView *text_view = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "the_html"));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(text_view);
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Save", GTK_RESPONSE_ACCEPT,
                                                   NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        GtkTextIter start, end;
        gchar *text;

        gtk_text_buffer_get_bounds(buffer, &start, &end);
        text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        g_file_set_contents(filename, text, -1, NULL);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
    GtkBuilder *builder;
    GtkWidget *window;

    gtk_init(&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "rebol3.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));

    g_signal_connect(gtk_builder_get_object(builder, "download_button"), "clicked", G_CALLBACK(on_download_html), builder);
    g_signal_connect(gtk_builder_get_object(builder, "load_local_button"), "clicked", G_CALLBACK(on_load_local_file), builder);
    g_signal_connect(gtk_builder_get_object(builder, "save_web_button"), "clicked", G_CALLBACK(on_save_to_web), builder);
    g_signal_connect(gtk_builder_get_object(builder, "save_local_button"), "clicked", G_CALLBACK(on_save_local_file), builder);

    gtk_widget_show_all(window);
    gtk_main();

    g_object_unref(builder);
    return 0;
}
