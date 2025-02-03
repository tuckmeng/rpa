#include <gtk/gtk.h>
#include <zip.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

struct AppState {
    GtkWidget *window;
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

void add_file_to_zip(zip_t* zip, const std::string& filename, const std::string& content) {
    zip_source_t* source = zip_source_buffer(zip, content.c_str(), content.size(), 0);
    zip_int64_t index = zip_file_add(zip, filename.c_str(), source, ZIP_FL_ENC_UTF_8);
    zip_set_file_compression(zip, index, ZIP_CM_DEFLATE64, 9);  // Use DEFLATE64 compression
}

void add_directory_to_zip(zip_t* zip, const std::string& dirname) {
    zip_dir_add(zip, dirname.c_str(), ZIP_FL_ENC_UTF_8);
}

void save_xlsx(AppState* state, const std::string& filename) {
    int rows = state->cells.size();
    if (rows == 0) return;
    int cols = state->cells[0].size();

    int error = 0;
    zip_t* zip = zip_open(filename.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!zip) return;

    // Create necessary directories
    add_directory_to_zip(zip, "xl");
    add_directory_to_zip(zip, "_rels");
    add_directory_to_zip(zip, "xl/_rels");
    add_directory_to_zip(zip, "xl/worksheets");

    // Add [Content_Types].xml
    add_file_to_zip(zip, "[Content_Types].xml",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\n"
        "    <Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n"
        "    <Default Extension=\"xml\" ContentType=\"application/xml\"/>\n"
        "</Types>");

    // Add _rels/.rels
    add_file_to_zip(zip, "_rels/.rels",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
        "    <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>\n"
        "</Relationships>");

    // Add xl/workbook.xml
    add_file_to_zip(zip, "xl/workbook.xml",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">\n"
        "    <sheets>\n"
        "        <sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/>\n"
        "    </sheets>\n"
        "</workbook>");

    // Add xl/_rels/workbook.xml.rels
    add_file_to_zip(zip, "xl/_rels/workbook.xml.rels",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
        "    <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>\n"
        "    <Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>\n"
        "</Relationships>");

    // Add xl/styles.xml
    add_file_to_zip(zip, "xl/styles.xml",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">\n"
        "    <fonts count=\"1\">\n"
        "        <font>\n"
        "            <sz val=\"11\"/>\n"
        "            <color theme=\"1\"/>\n"
        "            <name val=\"Calibri\"/>\n"
        "            <family val=\"2\"/>\n"
        "            <scheme val=\"minor\"/>\n"
        "        </font>\n"
        "    </fonts>\n"
        "    <fills count=\"1\">\n"
        "        <fill>\n"
        "            <patternFill patternType=\"none\"/>\n"
        "        </fill>\n"
        "    </fills>\n"
        "    <borders count=\"1\">\n"
        "        <border>\n"
        "            <left/>\n"
        "            <right/>\n"
        "            <top/>\n"
        "            <bottom/>\n"
        "            <diagonal/>\n"
        "        </border>\n"
        "    </borders>\n"
        "    <cellXfs count=\"1\">\n"
        "        <xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" applyFont=\"1\" applyFill=\"1\" applyBorder=\"1\"/>\n"
        "    </cellXfs>\n"
        "</styleSheet>");

    // Add xl/worksheets/sheet1.xml
    std::ostringstream worksheet;
    worksheet << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              << "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">\n"
              << "<sheetData>\n";

    for (int i = 0; i < rows; ++i) {
        worksheet << "<row r=\"" << (i + 1) << "\">";
        for (int j = 0; j < cols; ++j) {
            const gchar* text = gtk_entry_get_text(state->cells[i][j]);
            std::string value = escape_xml(text);  // Only escape cell content
            worksheet << "<c t=\"inlineStr\"><is><t>" << value << "</t></is></c>";
        }
        worksheet << "</row>\n";
    }
    worksheet << "</sheetData></worksheet>";

    add_file_to_zip(zip, "xl/worksheets/sheet1.xml", worksheet.str());

    zip_close(zip);
}

void on_save_clicked(GtkWidget* widget, gpointer data) {
    AppState* state = static_cast<AppState*>(data);
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save File",
                                                    GTK_WINDOW(state->window),
                                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    "_Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "spreadsheet.xlsx");
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        save_xlsx(state, filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void update_grid(AppState* state) {
    int rows = gtk_spin_button_get_value_as_int(state->row_spin);
    int cols = gtk_spin_button_get_value_as_int(state->col_spin);

    const int CELL_WIDTH = 100;
    const int CELL_HEIGHT = 30;

    for (auto& row : state->cells) {
        for (GtkEntry* entry : row) {
            gtk_widget_destroy(GTK_WIDGET(entry));
        }
    }
    state->cells.clear();

    state->cells.resize(rows, std::vector<GtkEntry*>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            GtkEntry* entry = GTK_ENTRY(gtk_entry_new());
            gtk_entry_set_width_chars(entry, 8);
            gtk_widget_set_size_request(GTK_WIDGET(entry), CELL_WIDTH, CELL_HEIGHT);
            gtk_grid_attach(GTK_GRID(state->grid), GTK_WIDGET(entry), j, i, 1, 1);
            state->cells[i][j] = entry;
        }
    }

    int new_width = cols * CELL_WIDTH + 50;
    int new_height = rows * CELL_HEIGHT + 100;
    gtk_window_resize(GTK_WINDOW(state->window), new_width, new_height);

    gtk_widget_show_all(GTK_WIDGET(state->grid));
}

void on_update_clicked(GtkWidget* widget, gpointer data) {
    AppState* state = static_cast<AppState*>(data);
    update_grid(state);
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Spreadsheet Grid");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    AppState state;
    state.window = window;

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
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

    GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);
    state.grid = gtk_grid_new();
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
