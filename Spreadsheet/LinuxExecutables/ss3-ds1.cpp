// DS version
#include <gtk/gtk.h>
#include <zip.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

struct AppState {
    GtkWidget *window;  // Added to hold the main window reference
    GtkWidget *grid;
    GtkSpinButton *row_spin;
    GtkSpinButton *col_spin;
    std::vector<std::vector<GtkEntry*>> cells;
};

std::string escape_xml(const std::string& str) {
    std::string output;
    for (char c : str) {
        switch (c) {
            case '&':  output += "&amp;";  break;
            case '<':  output += "&lt;";   break;
            case '>':  output += "&gt;";   break;
            case '"':  output += "&quot;"; break;
            case '\'': output += "&apos;"; break;
            default:   output += c;        break;
        }
    }
    return output;
}

std::string get_column_name(int n) {
    std::string name;
    while (n > 0) {
        int remainder = (n - 1) % 26;
        name = static_cast<char>('A' + remainder) + name;
        n = (n - 1) / 26;
    }
    return name;
}

void update_grid(AppState* state) {
    int rows = gtk_spin_button_get_value_as_int(state->row_spin);
    int cols = gtk_spin_button_get_value_as_int(state->col_spin);

    // Clear existing cells
    for (auto& row : state->cells) {
        for (GtkEntry* entry : row) {
            gtk_widget_destroy(GTK_WIDGET(entry));
        }
    }
    state->cells.clear();

    // Create new grid
    state->cells.resize(rows, std::vector<GtkEntry*>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
            gtk_entry_set_width_chars(entry, 8);
            gtk_grid_attach(GTK_GRID(state->grid), GTK_WIDGET(entry), j, i, 1, 1);
            state->cells[i][j] = entry;
        }
    }
    gtk_widget_show_all(GTK_WIDGET(state->grid));
}

void save_xlsx(AppState* state, const std::string& filename) {
    int rows = state->cells.size();
    if (rows == 0) return;
    int cols = state->cells[0].size();

    // Create ZIP archive with the provided filename
    int error = 0;
    zip_t* zip = zip_open(filename.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!zip) return;

    // Helper function to add files to ZIP
    auto add_file = [&](const std::string& filename, const std::string& content) {
        zip_source_t* source = zip_source_buffer(zip, content.c_str(), content.size(), 0);
        zip_file_add(zip, filename.c_str(), source, ZIP_FL_ENC_UTF_8);
    };

    // Create directory structure and files
    add_file("[Content_Types].xml",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\n"
        "    <Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n"
        "    <Default Extension=\"xml\" ContentType=\"application/xml\"/>\n"
        "    <Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>\n"
        "    <Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>\n"
        "    <Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>\n"
        "</Types>");

    add_file("_rels/.rels",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
        "    <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>\n"
        "</Relationships>");

    add_file("xl/workbook.xml",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">\n"
        "    <sheets>\n"
        "        <sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/>\n"
        "    </sheets>\n"
        "</workbook>");

    add_file("xl/_rels/workbook.xml.rels",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
        "    <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>\n"
        "    <Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>\n"
        "</Relationships>");

    add_file("xl/styles.xml",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">\n"
        "    <fonts count=\"1\"><font/></fonts>\n"
        "    <fills count=\"1\"><fill/></fills>\n"
        "    <borders count=\"1\"><border/></borders>\n"
        "    <cellStyleXfs count=\"1\"><xf/></cellStyleXfs>\n"
        "    <cellXfs count=\"1\"><xf numFmtId=\"0\"/></cellXfs>\n"
        "</styleSheet>");

    // Generate worksheet XML
    std::ostringstream worksheet;
    worksheet << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              << "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"\n"
              << "           xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">\n"
              << "    <dimension ref=\"A1:" << get_column_name(cols) << rows << "\"/>\n"
              << "    <sheetViews>\n"
              << "        <sheetView workbookViewId=\"0\"/>\n"
              << "    </sheetViews>\n"
              << "    <sheetFormatPr defaultRowHeight=\"15\"/>\n"
              << "    <sheetData>\n";

    for (int i = 0; i < rows; ++i) {
        worksheet << "<row r=\"" << (i+1) << "\">";
        for (int j = 0; j < cols; ++j) {
            const gchar* text = gtk_entry_get_text(state->cells[i][j]);
            std::string value = escape_xml(text);
            std::string col_name = get_column_name(j+1);
            
            worksheet << "<c r=\"" << col_name << (i+1) << "\" t=\"inlineStr\">"
                      << "<is><t>" << value << "</t></is>"
                      << "</c>";
        }
        worksheet << "</row>\n";
    }

    worksheet << "    </sheetData>\n</worksheet>";
    add_file("xl/worksheets/sheet1.xml", worksheet.str());

    // Close ZIP archive
    zip_close(zip);
}

void on_save_clicked(GtkWidget* widget, gpointer data) {
    AppState* state = static_cast<AppState*>(data);

    // Create save dialog
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File",
                                                    GTK_WINDOW(state->window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel",
                                                    GTK_RESPONSE_CANCEL,
                                                    "_Save",
                                                    GTK_RESPONSE_ACCEPT,
                                                    NULL);

    // Add .xlsx filter
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Excel Files (*.xlsx)");
    gtk_file_filter_add_pattern(filter, "*.xlsx");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Set default name and overwrite confirmation
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "spreadsheet.xlsx");
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    // Run dialog
    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        save_xlsx(state, filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void on_update_clicked(GtkWidget* widget, gpointer data) {
    AppState* state = static_cast<AppState*>(data);
    update_grid(state);
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Spreadsheet To Enter Data Into");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    AppState state;
    state.window = window;  // Store the main window reference

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // Controls
    GtkWidget* controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    state.row_spin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 100, 1));
    state.col_spin = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 100, 1));
    gtk_spin_button_set_value(state.row_spin, 5);
    gtk_spin_button_set_value(state.col_spin, 5);

    GtkWidget* update_btn = gtk_button_new_with_label("Update Size");
    GtkWidget* save_btn = gtk_button_new_with_label("Save as XLSX");

    gtk_box_pack_start(GTK_BOX(controls), gtk_label_new("Rows:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(controls), GTK_WIDGET(state.row_spin), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(controls), gtk_label_new("Columns:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(controls), GTK_WIDGET(state.col_spin), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(controls), update_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(controls), save_btn, FALSE, FALSE, 0);

    // Grid
    GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);
    state.grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(state.grid), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(state.grid), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), state.grid);

    gtk_box_pack_start(GTK_BOX(box), controls, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);

    g_signal_connect(update_btn, "clicked", G_CALLBACK(on_update_clicked), &state);
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_clicked), &state);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    update_grid(&state);
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
