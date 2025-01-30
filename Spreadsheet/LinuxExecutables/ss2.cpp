// GPT version

#include <gtk/gtk.h>
#include <fstream>
#include <vector>
#include <string>
#include <zip.h>

#define DEFAULT_ROWS 5
#define DEFAULT_COLS 5

std::vector<std::vector<GtkWidget*>> cells;
GtkWidget *grid;
GtkWidget *row_entry, *col_entry;

void update_grid(GtkWidget *widget, gpointer data) {
    int rows = atoi(gtk_entry_get_text(GTK_ENTRY(row_entry)));
    int cols = atoi(gtk_entry_get_text(GTK_ENTRY(col_entry)));
    
    for (auto &row : cells) {
        for (auto &cell : row) {
            gtk_widget_destroy(cell);
        }
    }
    cells.clear();
    
    for (int i = 0; i < rows; ++i) {
        std::vector<GtkWidget*> row;
        for (int j = 0; j < cols; ++j) {
            GtkWidget *entry = gtk_entry_new();
            gtk_grid_attach(GTK_GRID(grid), entry, j, i, 1, 1);
            row.push_back(entry);
        }
        cells.push_back(row);
    }
    gtk_widget_show_all(grid);
}

void create_xlsx() {
    std::ofstream content_types("[Content_Types].xml");
    content_types << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    content_types << "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\n";
    content_types << "    <Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n";
    content_types << "    <Default Extension=\"xml\" ContentType=\"application/xml\"/>\n";
    content_types << "    <Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>\n";
    content_types << "    <Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>\n";
    content_types << "    <Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>\n";
    content_types << "</Types>\n";
    content_types.close();

    std::ofstream rels("_rels/.rels");
    rels << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    rels << "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n";
    rels << "    <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>\n";
    rels << "</Relationships>\n";
    rels.close();

    struct zip_t *zip = zip_open("spreadsheet.xlsx", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    zip_entry_open(zip, "[Content_Types].xml");
    zip_entry_fwrite(zip, "[Content_Types].xml");
    zip_entry_close(zip);
    zip_entry_open(zip, "_rels/.rels");
    zip_entry_fwrite(zip, "_rels/.rels");
    zip_entry_close(zip);
    zip_close(zip);
}

void save_to_xlsx(GtkWidget *widget, gpointer data) {
    create_xlsx();
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK Spreadsheet");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), controls, FALSE, FALSE, 5);
    
    row_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(row_entry), "5");
    gtk_box_pack_start(GTK_BOX(controls), row_entry, FALSE, FALSE, 5);
    
    col_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(col_entry), "5");
    gtk_box_pack_start(GTK_BOX(controls), col_entry, FALSE, FALSE, 5);
    
    GtkWidget *update_button = gtk_button_new_with_label("Update Size");
    g_signal_connect(update_button, "clicked", G_CALLBACK(update_grid), NULL);
    gtk_box_pack_start(GTK_BOX(controls), update_button, FALSE, FALSE, 5);
    
    GtkWidget *save_button = gtk_button_new_with_label("Save as XLSX");
    g_signal_connect(save_button, "clicked", G_CALLBACK(save_to_xlsx), NULL);
    gtk_box_pack_start(GTK_BOX(controls), save_button, FALSE, FALSE, 5);
    
    grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 5);
    
    update_grid(NULL, NULL);
    
    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}
