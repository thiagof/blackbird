#include "curl_fun.h"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

json_t* getJsonFromUrl(Parameters& params, std::string url, std::string postFields) {
  curl_easy_setopt(params.curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(params.curl, CURLOPT_CONNECTTIMEOUT, 10L);
  curl_easy_setopt(params.curl, CURLOPT_TIMEOUT, 20L);
  if (!postFields.empty()) {
    curl_easy_setopt(params.curl, CURLOPT_POSTFIELDS, postFields.c_str());
  }
  curl_easy_setopt(params.curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  std::string readBuffer;
  curl_easy_setopt(params.curl, CURLOPT_WRITEDATA, &readBuffer);
  curl_easy_setopt(params.curl, CURLOPT_DNS_CACHE_TIMEOUT, 3600);
  // WORKAROUND FOR BITFINEX: CHANGE TO GET REQUEST
  if (strcmp(url.c_str(), "https://api.bitfinex.com/v1/book/btcusd") == 0) {
    curl_easy_setopt(params.curl, CURLOPT_CUSTOMREQUEST, "GET");
  }
  CURLcode resCurl = curl_easy_perform(params.curl);
  while (resCurl != CURLE_OK) {
    *params.logFile << "Error with cURL: " << curl_easy_strerror(resCurl) << std::endl;
    *params.logFile << "  URL: " << url << std::endl;
    *params.logFile << "  Retry in 2 sec..." << std::endl;
    sleep(2.0);
    readBuffer = "";
    curl_easy_setopt(params.curl, CURLOPT_DNS_CACHE_TIMEOUT, 0);
    resCurl = curl_easy_perform(params.curl);
  }

  const char* gfinance = "http://www.google.com/finance";
  if (strcmp(url.substr(0, strlen(gfinance)).c_str(), gfinance) == 0) {
    readBuffer = readBuffer.substr(3, -1);
  }

  json_t* root;
  json_error_t error;
  root = json_loads(readBuffer.c_str(), 0, &error);

  while (!root) {
    *params.logFile << "Error with JSON:\n" << error.text << std::endl;
    *params.logFile << "Buffer:\n" << readBuffer.c_str() << std::endl;
    *params.logFile << "Retrying..." << std::endl;
    sleep(2.0);
    readBuffer = "";
    resCurl = curl_easy_perform(params.curl);
    while (resCurl != CURLE_OK) {
      *params.logFile << "Error with cURL: " << curl_easy_strerror(resCurl) << std::endl;
      *params.logFile << "  URL: " << url << std::endl;
      *params.logFile << "  Retry in 2 sec..." << std::endl;
      sleep(2.0);
      readBuffer = "";
      resCurl = curl_easy_perform(params.curl);
    }
    root = json_loads(readBuffer.c_str(), 0, &error);
  }

  return root;
}

