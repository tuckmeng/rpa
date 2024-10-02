#include <gtk/gtk.h>
#include <regex.h>
#include <stdio.h>

void show_message(const char *title, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title); // Set the title of the dialog
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    const char *input = "You used 36 of 279 pages.";
    const char *pattern = "([0-9]{1,3}) pages"; // Updated regex pattern
    const char *title = "SRE Example 6 Result"; // Title for the message box
    regex_t regex;
    regmatch_t matches[2];

    // Compile the regular expression
    int compile_status = regcomp(&regex, pattern, REG_EXTENDED);
    if (compile_status != 0) {
        char errbuf[100];
        regerror(compile_status, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "Could not compile regex: %s\n", errbuf);
        return 1;
    }

    // Execute the regular expression
    if (regexec(&regex, input, 2, matches, 0) == 0) {
        char result[100];
        snprintf(result, sizeof(result), "%.*s", (int)(matches[1].rm_eo - matches[1].rm_so), input + matches[1].rm_so);
        
        show_message(title, result); // Pass the title to the show_message function
    } else {
        show_message(title, "No match found."); // Show the same title for no match
    }

    // Free the compiled regular expression
    regfree(&regex);
    
    return 0;
}
