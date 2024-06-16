#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <gumbo.h>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>

using namespace std;

// Callback function for libcurl to write data to a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (bad_alloc &e) {
        return 0;
    }
    return newLength;
}

// Function to fetch HTML content from a given URL
string fetchHtml(const string& url) {
    CURL* curl;
    CURLcode res;
    string html;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    if (res != CURLE_OK) {
        cerr << "Failed to fetch URL: " << curl_easy_strerror(res) << endl;
        return "";
    }

    return html;
}

// Helper function to traverse Gumbo nodes
void searchForLinks(GumboNode* node, vector<string>& links) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }

    GumboAttribute* href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        links.push_back(href->value);
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        searchForLinks(static_cast<GumboNode*>(children->data[i]), links);
    }
}

// Function to extract URLs from HTML content
vector<string> extractUrls(const string& html) {
    vector<string> links;
    GumboOutput* output = gumbo_parse(html.c_str());
    searchForLinks(output->root, links);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    return links;
}

// Function to store URLs in MongoDB
void storeUrls(mongocxx::collection& collection, const string& baseUrl, const vector<string>& urls) {
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::document;
    using bsoncxx::builder::basic::array;

    document builder{};
    array urlArray{};

    for (const auto& url : urls) {
        urlArray.append(url);
    }

    builder.append(
        kvp("base_url", baseUrl),
        kvp("found_urls", urlArray)
    );

    collection.insert_one(builder.view());
}

int main() {
    // Initialize MongoDB client
    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
    mongocxx::database db = client["crawler_db"];
    mongocxx::collection coll = db["webpages"];

    // URL to crawl
    string url = "http://example.com";  // Replace with the URL you want to crawl
    string html = fetchHtml(url);

    if (html.empty()) {
        cerr << "Failed to fetch HTML content for URL: " << url << endl;
        return 1;
    }

    vector<string> urls = extractUrls(html);

    // Store URLs in MongoDB
    storeUrls(coll, url, urls);

    cout << "Stored " << urls.size() << " URLs from " << url << " into MongoDB." << endl;

    return 0;
}
