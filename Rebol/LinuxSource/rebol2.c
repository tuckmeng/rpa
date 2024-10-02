#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// Function to evaluate a postfix expression
double evaluate_postfix(const char *expression) {
    double stack[100];
    int stack_pos = 0;

    const char *ptr = expression;
    while (*ptr) {
        if (isdigit(*ptr) || (*ptr == '.')) {
            double number;
            sscanf(ptr, "%lf", &number);
            stack[stack_pos++] = number;

            // Move the pointer past the number
            while (isdigit(*ptr) || *ptr == '.') ptr++;
        } else if (strchr("+-*/", *ptr)) {
            double b = stack[--stack_pos];
            double a = stack[--stack_pos];
            switch (*ptr) {
                case '+': stack[stack_pos++] = a + b; break;
                case '-': stack[stack_pos++] = a - b; break;
                case '*': stack[stack_pos++] = a * b; break;
                case '/':
                    if (b != 0) {
                        stack[stack_pos++] = a / b; 
                    } else {
                        return NAN; // Handle division by zero
                    }
                    break;
            }
            ptr++; // Move past the operator
        } else {
            ptr++; // Skip other characters
        }
    }

    return stack[0];
}

// Function to convert infix expression to postfix
char *infix_to_postfix(const char *infix) {
    static char postfix[256];
    char stack[100];
    int stack_pos = 0;
    int output_pos = 0;

    const char *ptr = infix;

    while (*ptr) {
        if (isdigit(*ptr) || (*ptr == '.')) {
            // If it's a number, add it to the output
            while (isdigit(*ptr) || *ptr == '.') {
                postfix[output_pos++] = *ptr++;
            }
            postfix[output_pos++] = ' '; // Separate numbers with space
        } else if (*ptr == '+' || *ptr == '-') {
            while (stack_pos > 0 && (stack[stack_pos - 1] == '*' || stack[stack_pos - 1] == '/')) {
                postfix[output_pos++] = stack[--stack_pos];
                postfix[output_pos++] = ' ';
            }
            stack[stack_pos++] = *ptr++;
        } else if (*ptr == '*' || *ptr == '/') {
            stack[stack_pos++] = *ptr++;
        } else {
            ptr++; // Skip other characters
        }
    }

    // Pop all the operators from the stack
    while (stack_pos > 0) {
        postfix[output_pos++] = stack[--stack_pos];
        postfix[output_pos++] = ' ';
    }

    postfix[output_pos] = '\0'; // Null-terminate the string
    return postfix;
}

static gboolean last_was_result = FALSE;

static void on_button_clicked(GtkButton *button, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(user_data);
    const gchar *button_label = gtk_button_get_label(button);
    
    // Check if the entry shows "Error"
    if (g_strcmp0(gtk_entry_get_text(entry), "Error") == 0) {
        gtk_entry_set_text(entry, ""); // Clear the entry if it shows "Error"
    }

    if (g_strcmp0(button_label, "=") == 0) {
        const gchar *expression = gtk_entry_get_text(entry);
        char *postfix = infix_to_postfix(expression);
        double result = evaluate_postfix(postfix);
        
        if (isnan(result)) {
            gtk_entry_set_text(entry, "Error");
        } else {
            gtk_entry_set_text(entry, g_strdup_printf("%g", result));
            last_was_result = TRUE; // Mark that the last action was a result
        }
    } else {
        if (last_was_result) {
            gtk_entry_set_text(entry, ""); // Clear the entry if the last was a result
            last_was_result = FALSE; // Reset the flag
        }
        const gchar *current_text = gtk_entry_get_text(entry);
        gchar *new_text = g_strconcat(current_text, button_label, NULL);
        gtk_entry_set_text(entry, new_text);
        g_free(new_text);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 188, 185); // Set the window size

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    GtkWidget *entry = gtk_entry_new();
    
    // Set CSS for the entry
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider, "entry { font-size: 25px; }", -1, NULL);
    gtk_style_context_add_provider(
        gtk_widget_get_style_context(entry),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 4, 1);

    const char *buttons[16] = {
        "1", "2", "3", "+",
        "4", "5", "6", "-",
        "7", "8", "9", "*",
        "0", ".", "/", "="
    };

    for (int i = 0; i < 16; i++) {
        GtkWidget *button = gtk_button_new_with_label(buttons[i]);
        g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), entry);
        gtk_grid_attach(GTK_GRID(grid), button, i % 4, (i / 4) + 1, 1, 1);
    }

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
