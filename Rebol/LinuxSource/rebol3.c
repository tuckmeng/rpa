#include <gtk/gtk.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h> // For GThread

// Function prototypes
size_t WriteCallback(void* contents, size_t size, size_t nmemb, char** userp);
char* readFromUrl(const char* url);
int saveToUrl(const char* url, const char* data);
void saveToLocalFile(const char* filename, const char* data);

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

    // Check for successful response and return the buffer
    if (res == CURLE_OK) {
        return readBuffer; // Return the downloaded content
    } else {
        free(readBuffer); // Free the memory if there's an error
        return NULL; // Return NULL if failed
    }
}

// Structure to hold URL and GtkTextView
typedef struct {
    const char* url;
    GtkTextView* html_text_view;
} DownloadData;

// Function to update the text view with downloaded content
gboolean set_text_from_downloaded_content(DownloadData* data) {
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(data->html_text_view);
    gtk_text_buffer_set_text(buffer, data->url, -1); // Set the downloaded content
    free((void*)data->url); // Free the memory allocated for URL content
    free(data); // Free the DownloadData structure
    return G_SOURCE_REMOVE; // Remove the source
}

// Function to download content from a URL in a separate thread
void* downloadContent(void* user_data) {
    DownloadData* data = (DownloadData*)user_data;
    char* content = readFromUrl(data->url);
    
    // Schedule the update of the text view in the main thread
    if (content) {
        // Create a copy of the content to pass to the text view
        DownloadData* new_data = malloc(sizeof(DownloadData));
        new_data->url = content; // The content to be displayed
        new_data->html_text_view = data->html_text_view; // Pass the text view

        g_idle_add((GSourceFunc)set_text_from_downloaded_content, new_data);
    } else {
        g_print("Failed to download content from %s.\n", data->url);
        free(data); // Free the DownloadData structure if failed
    }
    
    return NULL;
}

// Button callback functions
void on_download_button_clicked(GtkWidget* widget, gpointer user_data) {
    gpointer* data = (gpointer*)user_data;  // Get both url_entry and html_text_view
    GtkEntry* entry = GTK_ENTRY(data[0]);
    GtkTextView* html_text_view = GTK_TEXT_VIEW(data[1]);

    const char* url = gtk_entry_get_text(entry);
    DownloadData* download_data = malloc(sizeof(DownloadData));
    download_data->url = strdup(url); // Duplicate the URL string
    download_data->html_text_view = html_text_view;

    // Start the download in a new thread
    g_thread_new("DownloadThread", downloadContent, download_data);
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

    GtkTextView* html_text_view = GTK_TEXT_VIEW(gtk_widget_get_parent(widget)); // Get parent of the button
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

void saveToLocalFile(const char* filename, const char* data) {
    FILE* outFile = fopen(filename, "w");
    if (outFile) {
        fputs(data, outFile);
        fclose(outFile);
    }
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

    // Create a scrolled window for HTML text view
    GtkWidget* scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    // Create HTML text view
    GtkTextView* html_text_view = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_widget_set_size_request(GTK_WIDGET(html_text_view), 600, 440);
    gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(html_text_view));

    // Create buttons
    GtkWidget* download_button = gtk_button_new_with_label("Download HTML Page");
    gtk_box_pack_start(GTK_BOX(vbox), download_button, FALSE, FALSE, 0);
    
    gpointer data[2] = {url_entry, html_text_view};  // Pass both url_entry and html_text_view
    g_signal_connect(download_button, "clicked", G_CALLBACK(on_download_button_clicked), data);

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
