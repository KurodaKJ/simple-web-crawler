#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <unordered_set>
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
        s->append((char*)contents, newLength);  // Append the received data to the string
    } catch (bad_alloc &e) {
        return 0;  // Return 0 on failure
    }
    return newLength;  // Return the number of bytes processed
}

// Function to fetch HTML content from a given URL
string fetchHtml(const string& url) {
    CURL* curl;
    CURLcode res;
    string html;

    curl = curl_easy_init();  // Initialize a CURL session
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());  // Set the URL to fetch
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);  // Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);  // Set the string to write the data to
        res = curl_easy_perform(curl);  // Perform the request
        curl_easy_cleanup(curl);  // Clean up the CURL session
    }

    if (res != CURLE_OK) {
        cerr << "Failed to fetch URL: " << url << " - " << curl_easy_strerror(res) << endl;  // Print error message
        return "";  // Return an empty string on failure
    }

    return html;  // Return the fetched HTML content
}

// Helper function to traverse Gumbo nodes
void searchForLinks(GumboNode* node, vector<string>& links) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;  // If the node is not an element, return
    }

    GumboAttribute* href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        links.push_back(href->value);  // If the node is an <a> tag with an href attribute, add the URL to the links vector
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        searchForLinks(static_cast<GumboNode*>(children->data[i]), links);  // Recursively search for links in child nodes
    }
}

// Function to extract URLs from HTML content
vector<string> extractUrls(const string& html) {
    vector<string> links;
    GumboOutput* output = gumbo_parse(html.c_str());  // Parse the HTML content
    searchForLinks(output->root, links);  // Search for links in the parsed HTML
    gumbo_destroy_output(&kGumboDefaultOptions, output);  // Clean up the Gumbo parser output
    return links;  // Return the extracted URLs
}

// Function to store URLs in MongoDB
void storeUrls(mongocxx::collection& collection, const string& baseUrl, const vector<string>& urls) {
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::document;
    using bsoncxx::builder::basic::array;

    document builder{};
    array urlArray{};

    for (const auto& url : urls) {
        urlArray.append(url);  // Add each URL to the BSON array
    }

    builder.append(
        kvp("base_url", baseUrl),  // Add the base URL to the document
        kvp("found_urls", urlArray)  // Add the found URLs to the document
    );

    collection.insert_one(builder.view());  // Insert the document into the MongoDB collection
}

// Function to crawl URLs recursively
void crawl(mongocxx::collection& collection, const string& startUrl, int maxPages) {
    queue<string> urlQueue;  // Queue to manage URLs to be crawled
    unordered_set<string> visited;  // Set to track visited URLs
    int pagesCrawled = 0;  // Counter to limit the number of pages crawled

    urlQueue.push(startUrl);  // Add the starting URL to the queue
    visited.insert(startUrl);  // Mark the starting URL as visited

    while (!urlQueue.empty() && pagesCrawled < maxPages) {
        string url = urlQueue.front();  // Get the next URL from the queue
        urlQueue.pop();  // Remove the URL from the queue

        cout << "Crawling: " << url << endl;  // Log the URL being crawled

        string html = fetchHtml(url);  // Fetch the HTML content of the URL
        if (html.empty()) {
            continue;  // If the HTML content is empty, continue to the next URL
        }

        vector<string> urls = extractUrls(html);  // Extract URLs from the HTML content
        storeUrls(collection, url, urls);  // Store the URLs in MongoDB

        for (const auto& foundUrl : urls) {
            if (visited.find(foundUrl) == visited.end()) {  // If the URL has not been visited
                urlQueue.push(foundUrl);  // Add the URL to the queue
                visited.insert(foundUrl);  // Mark the URL as visited
            }
        }

        pagesCrawled++;  // Increment the counter
    }
}

int main() {
    // Initialize MongoDB client
    mongocxx::instance instance{};  // Initialize MongoDB instance
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};  // Connect to MongoDB
    mongocxx::database db = client["crawler_db"];  // Access the database
    mongocxx::collection coll = db["webpages"];  // Access the collection

    // URL to start crawling
    string startUrl = "http://example.com";  // Replace with the URL you want to start crawling
    int maxPages = 10;  // Set the limit on the number of pages to crawl

    crawl(coll, startUrl, maxPages);  // Start crawling

    cout << "Crawling complete." << endl;  // Print completion message

    return 0;
}
