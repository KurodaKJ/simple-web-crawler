# Simple-web-crawler

To compile and run use:
```
g++ -std=c++17 -o simple_web_crawler simple_web_crawler.cpp -I/usr/local/include/bsoncxx/v_noabi -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc -I/usr/include/gumbo -L/usr/local/lib -lmongocxx -lbsoncxx -lcurl -lgumbo
```
Then

```
./simple_web_crawler
```
This is the sample output:
![image](https://github.com/KurodaKJ/simple-web-crawler/assets/70169841/0fec9ed5-e3a8-48ca-89ed-091b41429330)
