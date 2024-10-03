#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <windows.h>
#include <wininet.h>

#define BUFFER_SIZE 4096

// Function to trim whitespace from a string
std::string trim_whitespace(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    return (first == std::string::npos) ? "" : str.substr(first, (last - first + 1));
}

// Function to extract links from HTML
void extract_links(const std::string& html, const std::string& base_url, const std::string& filter) {
    const std::string a_tag = "<a ";
    const std::string href_attr = "href=\"";
    size_t pos = 0;

    while ((pos = html.find(a_tag, pos)) != std::string::npos) {
        size_t href_start = html.find(href_attr, pos);
        if (href_start != std::string::npos) {
            href_start += href_attr.length();
            size_t href_end = html.find('\"', href_start);
            if (href_end != std::string::npos) {
                std::string link = trim_whitespace(html.substr(href_start, href_end - href_start));

                std::string full_url;
                if (link[0] == '/') {
                    // Absolute path (relative to base URL)
                    full_url = base_url + link;
                } else {
                    // Already a full URL
                    full_url = link;
                }

                // Get the link text
                size_t text_start = href_end + 1;
                size_t text_end = html.find("</a>", text_start);
                if (text_end != std::string::npos) {
                    std::string formatted_text = trim_whitespace(html.substr(text_start, text_end - text_start));

                    // Print all links if no filter is provided
                    if (filter.empty() || formatted_text.find(filter) != std::string::npos) {
                        std::cout << formatted_text << ", " << full_url << std::endl;
                    }
                }

                pos = href_end; // Move position for the next search
            }
        }
    }
}

void print_help(const std::string& program_name) {
    std::cout << "Usage: " << program_name << " <URL> [filter_text]" << std::endl;
    std::cout << "Extract links from the specified webpage and filter by text if provided." << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    std::string url = argv[1];
    std::string filter = (argc > 2) ? argv[2] : ""; // Set filter to empty if not provided

    // Initialize WinINet
    HINTERNET hInternet = InternetOpen("MyUserAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        std::cerr << "InternetOpen failed: " << GetLastError() << std::endl;
        return EXIT_FAILURE;
    }

    HINTERNET hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        std::cerr << "InternetOpenUrl failed: " << GetLastError() << std::endl;
        InternetCloseHandle(hInternet);
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    std::string html;
    DWORD bytesRead;

    // Read the webpage content
    while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead != 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the buffer
        html += buffer; // Append to the full HTML
    }

    // Extract links from the HTML content
    extract_links(html, url, filter);

    // Cleanup
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return EXIT_SUCCESS;
}
