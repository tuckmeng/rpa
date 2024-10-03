#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // Include for isspace
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

typedef struct {
    char *data;
    size_t size;
} MemoryStruct;

// Callback function for writing the fetched data
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, MemoryStruct *userp) {
    size_t realsize = size * nmemb;
    userp->data = realloc(userp->data, userp->size + realsize + 1);
    if (userp->data == NULL) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0; // Out of memory!
    }
    memcpy(&(userp->data[userp->size]), contents, realsize);
    userp->size += realsize;
    userp->data[userp->size] = 0;  // Null-terminate
    return realsize;
}

// Function to trim whitespace from a string
char* trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Null terminate after the last non-space character
    *(end + 1) = '\0';

    return str;
}

// Function to extract links from HTML
void extract_links(const char *html, const char *base_url, const char *filter) {
    htmlDocPtr doc = htmlReadMemory(html, strlen(html), NULL, NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse HTML\n");
        return;
    }

    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((xmlChar *)"//a", xpathCtx);

    if (xpathObj == NULL) {
        fprintf(stderr, "Failed to evaluate XPath\n");
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return;
    }

    for (int i = 0; i < xpathObj->nodesetval->nodeNr; i++) {
        xmlNodePtr node = xpathObj->nodesetval->nodeTab[i];
        xmlChar *href = xmlGetProp(node, (const xmlChar *)"href");
        xmlChar *text = xmlNodeGetContent(node);

        if (href && text) {
            // Trim whitespace from the text
            char *formatted_text = trim_whitespace((char *)text);

            // Check if the href is a relative link
            char full_url[512];
            if (href[0] == '/') {
                // Absolute path (relative to base URL)
                snprintf(full_url, sizeof(full_url), "%s%s", base_url, href);
            } else {
                // Already a full URL or relative path, use as is
                snprintf(full_url, sizeof(full_url), "%s", href);
            }

            // Print all links if no filter is provided
            if (filter == NULL) {
                printf("%s, %s\n", formatted_text, full_url);
            } else if (strstr(formatted_text, filter) != NULL) {
                printf("%s, %s\n", formatted_text, full_url);
            }

            xmlFree(href);
            xmlFree(text);
        }
    }

    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
}

void print_help(const char *program_name) {
    printf("Usage: %s <URL> [filter_text]\n", program_name);
    printf("Extract links from the specified webpage and filter by text if provided.\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return EXIT_FAILURE;
    }

    const char *url = argv[1];
    const char *filter = (argc > 2) ? argv[2] : NULL; // Set filter to NULL if not provided
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {0};

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res == CURLE_OK) {
            extract_links(chunk.data, url, filter);
        } else {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // Cleanup
        curl_easy_cleanup(curl);
        free(chunk.data);
    }

    curl_global_cleanup();
    return EXIT_SUCCESS;
}
