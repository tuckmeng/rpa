#include <gtk/gtk.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Function to write data from a CURL request to a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, char** userp) {
    size_t totalSize = size * nmemb;
    *userp = realloc(*userp, strlen(*userp) + totalSize + 1); // Resize buffer
    if (*userp == NULL) {
        return 0; // Out of memory
    }
    strncat(*userp, contents, totalSize);
    return totalSize;
}

// Function to read content from a URL
char* readFromUrl(const char* url) {
    CURL* curl;
    CURLcode res;
    char* readBuffer = malloc(1);
    readBuffer[0] = '\0'; // Initialize to empty string

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return readBuffer;
}

// Function to save data to a URL (HTTP PUT)
int saveToUrl(const char* url, const char* data) {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READDATA, data);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return (res == CURLE_OK) ? 0 : 1;
    }
    curl_global_cleanup();
    return 1;
}

// Function to save data to a local file
void saveToLocalFile(const char* filename, const char* data) {
    FILE* outFile = fopen(filename, "w");
    if (outFile) {
        fputs(data, outFile);
        fclose(outFile);
    }
}

// Button callback functions
void on_download_button_clicked(GtkWidget* widget, gpointer user_data) {
    GtkEntry* entry = GTK_ENTRY(user_data);
    const char* url = gtk_entry_get_text(entry);

    GtkTextView* html_text_view = GTK_TEXT_VIEW(gtk_builder_get_object(gtk_builder_get_object(gtk_builder_new(), "vbox"), "html_text_view"));
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(html_text_view);

    char* htmlContent = readFromUrl(url);
    gtk_text_buffer_set_text(buffer, htmlContent, -1);
    free(htmlContent);
}

void on_load_file_button_clicked(GtkWidget* widget, gpointer user_data) {
    GtkTextView* html_text_view = GTK_TEXT_VIEW(user_data);
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(html_text_view);
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(gtk_widget_get_toplevel(widget)), GTK_FILE_CHOOSER_ACTION_OPEN, 
                                                   "Cancel", GTK_RESPONSE_CANCEL, 
                                                   "Open", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        FILE* file = fopen(filename, "r");
        if (file) {
            fseek(file, 0, SEEK_END);
            long filesize = ftell(file);
            fseek(file, 0, SEEK_SET);
            char* content = malloc(filesize + 1);
            fread(content, 1, filesize, file);
            content[filesize] = '\0'; // Null-terminate the string
            gtk_text_buffer_set_text(buffer, content, -1);
            free(content);
            fclose(file);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void on_save_to_url_button_clicked(GtkWidget* widget, gpointer user_data) {
    GtkEntry* entry = GTK_ENTRY(user_data);
    const char* url = gtk_entry_get_text(entry);

    GtkTextView* html_text_view = GTK_TEXT_VIEW(gtk_builder_get_object(gtk_builder_get_object(gtk_builder_new(), "vbox"), "html_text_view"));
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(html_text_view);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char* content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (saveToUrl(url, content) == 0) {
        g_print("Changes saved to the web site.\n");
    } else {
        g_print("Failed to save changes.\n");
    }
    g_free(content);
}

void on_save_to_local_file_button_clicked(GtkWidget* widget, gpointer user_data) {
    GtkTextView* html_text_view = GTK_TEXT_VIEW(user_data);
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(html_text_view);
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(gtk_widget_get_toplevel(widget)), GTK_FILE_CHOOSER_ACTION_SAVE, 
                                                   "Cancel", GTK_RESPONSE_CANCEL, 
                                                   "Save", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        char* content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        saveToLocalFile(filename, content);
        g_print("Changes saved to local file.\n");
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Web Page Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
    
    // Create a vertical box
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create URL entry
    GtkEntry* url_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_text(url_entry, "https://www.nparks.gov.sg/");
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(url_entry), FALSE, FALSE, 0);

    // Create HTML text view
    GtkTextView* html_text_view = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_widget_set_size_request(GTK_WIDGET(html_text_view), 600, 440);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(html_text_view), TRUE, TRUE, 0);

    // Create buttons
    GtkWidget* download_button = gtk_button_new_with_label("Download HTML Page");
    gtk_box_pack_start(GTK_BOX(vbox), download_button, FALSE, FALSE, 0);
    g_signal_connect(download_button, "clicked", G_CALLBACK(on_download_button_clicked), url_entry);

    GtkWidget* load_button = gtk_button_new_with_label("Load Local HTML File");
    gtk_box_pack_start(GTK_BOX(vbox), load_button, FALSE, FALSE, 0);
    g_signal_connect(load_button, "clicked", G_CALLBACK(on_load_file_button_clicked), html_text_view);

    GtkWidget* save_to_url_button = gtk_button_new_with_label("Save Changes to Web Site");
    gtk_box_pack_start(GTK_BOX(vbox), save_to_url_button, FALSE, FALSE, 0);
    g_signal_connect(save_to_url_button, "clicked", G_CALLBACK(on_save_to_url_button_clicked), url_entry);

    GtkWidget* save_to_local_file_button = gtk_button_new_with_label("Save Changes to Local File");
    gtk_box_pack_start(GTK_BOX(vbox), save_to_local_file_button, FALSE, FALSE, 0);
    g_signal_connect(save_to_local_file_button, "clicked", G_CALLBACK(on_save_to_local_file_button_clicked), html_text_view);

    // Connect the destroy signal
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Show the window
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
