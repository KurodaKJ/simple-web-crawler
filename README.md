# Simple Web Crawler
This is a simple web crawler that fetches and stores URLs from a starting webpage, using C++, libcurl, Gumbo, and MongoDB.

## Prerequisites
Before compiling and running the web crawler, ensure you have the following dependencies installed:
- C++ compiler supporting C++17
- libcurl
- Gumbo-parser
- MongoDB C++ Driver

### Installing Dependencies on Ubuntu
You can install the necessary dependencies using the following commands:
```sh
sudo apt update
sudo apt install g++ libcurl4-openssl-dev libgumbo-dev
```
To install the MongoDB C++ Driver, follow the official installation instructions from MongoDB's GitHub.

## Compilation and Execution
To compile and run the web crawler, use the following commands:
```sh
g++ -std=c++17 -o simple_web_crawler simple_web_crawler.cpp \
    -I/usr/local/include/bsoncxx/v_noabi \
    -I/usr/local/include/mongocxx/v_noabi \
    -I/usr/local/include/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc \
    -I/usr/include/gumbo \
    -L/usr/local/lib -lmongocxx -lbsoncxx -lcurl -lgumbo
```
Then, run the compiled executable:
```sh
./simple_web_crawler
```

### Sample Output
Here's a sample output of the web crawler:
![image](https://github.com/KurodaKJ/simple-web-crawler/assets/70169841/0fec9ed5-e3a8-48ca-89ed-091b41429330)

## Sample Websites to Crawl
You can start crawling from the following sample websites:

- http://example.com
- https://web-scraping.dev/

## Code Overview
The web crawler works as follows:

1. **Fetch HTML:** It uses libcurl to fetch the HTML content of the given URL.
2. **Parse HTML:** It uses Gumbo-parser to parse the HTML and extract URLs.
3. **Store URLs:** It stores the base URL and found URLs in a MongoDB database.
4. **Recursive Crawling:** It recursively crawls the found URLs up to a specified limit.

## Configuration
In the **`main`** function, you can configure:

**`startUrl`**: The initial URL to start crawling.
**`maxPages`**: The maximum number of pages to crawl.

Example:
```cpp
string startUrl = "http://example.com";  // Replace with your starting URL
int maxPages = 10;  // Set the limit on the number of pages to crawl
```
