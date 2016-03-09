#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <math.h>
#include <sys/time.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include "base64.h"
#include <jansson.h>
#include "foxbit.h"
#include "curl_fun.h"

/**
 * https://vbtc.vn/apidoc
 */

namespace Foxbit {

double getQuote(Parameters& params, bool isBid) {
  json_t* root = getJsonFromUrl(params, "https://api.blinktrade.com/api/v1/BRL/ticker?crypto_currency=BTC", "");
  double quoteValue;

  if (isBid) {
    quoteValue = json_real_value(json_object_get(root, "buy"));
  } else {
    quoteValue = json_real_value(json_object_get(root, "sell"));
  }

  json_decref(root);

  return convertToUsd(params, quoteValue);
}

double convertToUsd(Parameters& params, double brlQuote) {
  return brlQuote / getUsdQuote(params);
}

double getUsdQuote(Parameters& params) {
  // curl_easy_setopt(params.curl, CURLOPT_CUSTOMREQUEST, "GET");
  // json_t* root = getJsonFromUrl(params, "http://query.yahooapis.com/v1/public/yql?q=select * from yahoo.finance.xchange where pair=\"USDBRL\"&format=json&diagnostics=false&env=store://datatables.org/alltableswithkeys", "");
  // quote = json_string_value(json_object_get(json_object_get(json_object_get(json_object_get(root, "query"), "results"), "rate"), "Ask"));
  
  json_t* root = getJsonFromUrl(params, "http://www.google.com/finance/info?q=CURRENCY:USDBRL", "");

  const char* quote;
  double quoteValue;

  quote = json_string_value(json_object_get(json_array_get(root, 0), "l"));

  if (quote != NULL) {
    quoteValue = atof(quote);
  } else {
    quoteValue = 0.0;
  }

  return quoteValue;
}

double getAvail(Parameters& params, std::string currency) {
  // TODO
  // This function needs to be implemented
  // The code below is given as a general guideline but needs to be rewritten
  // to match the Poloniex API
  json_t* root = authRequest(params, "https://poloniex.com/tradingApi", "returnBalances", "");
  double availability = 0.0;
  json_decref(root);
  return availability;
}

int sendLongOrder(Parameters& params, std::string direction, double quantity, double price) {
  // TODO
  // This function needs to be implemented
  // The code below is given as a general guideline but needs to be rewritten
  // to match the Poloniex API
  *params.logFile << "<Poloniex> Trying to send a \"" << direction << "\" limit order: " << quantity << "@$" << price << "..." << std::endl;
  std::ostringstream oss;
  std::string options = oss.str();
  json_t* root = authRequest(params, "https://poloniex.com/tradingApi", "marginBuy", options);
  int orderId = json_integer_value(json_object_get(root, "order_id"));
  *params.logFile << "<Poloniex> Done (order ID: " << orderId << ")\n" << std::endl;
  json_decref(root);
  return orderId;
}

int sendShortOrder(Parameters& params, std::string direction, double quantity, double price) {
  // TODO
  // This function needs to be implemented
  // The code below is given as a general guideline but needs to be rewritten
  // to match the Poloniex API
  *params.logFile << "<Poloniex> Trying to send a \"" << direction << "\" limit order: " << quantity << "@$" << price << "..." << std::endl;
  std::ostringstream oss;
  std::string options = oss.str();
  json_t* root = authRequest(params, "https://poloniex.com/tradingApi", "marginSell", options);
  int orderId = json_integer_value(json_object_get(root, "order_id"));
  *params.logFile << "<Poloniex> Done (order ID: " << orderId << ")\n" << std::endl;
  json_decref(root);
  return orderId;
}

bool isOrderComplete(Parameters& params, int orderId) {
  // TODO
  // This function needs to be implemented
  // The code below is given as a general guideline but needs to be rewritten
  // to match the Poloniex API
  if (orderId == 0) {
    return true;
  }
  std::ostringstream oss;
  oss << "\"order_id\":" << orderId;
  std::string options = oss.str();
  json_t* root = authRequest(params, "https://poloniex.com/tradingApi", "order/status", options);
  bool isComplete = !json_boolean_value(json_object_get(root, "is_live"));
  json_decref(root);
  return isComplete;
}

double getActivePos(Parameters& params) {
  // TODO
  // This function needs to be implemented
  // The code below is given as a general guideline but needs to be rewritten
  // to match the Poloniex API
  json_t* root = authRequest(params, "https://poloniex.com/tradingApi", "positions", "");
  double position;
  if (json_array_size(root) == 0) {
    *params.logFile << "<Poloniex> WARNING: BTC position not available, return 0.0" << std::endl;
    position = 0.0;
  } else {
    position = atof(json_string_value(json_object_get(json_array_get(root, 0), "amount")));
  }
  json_decref(root);
  return position;
}

double getLimitPrice(Parameters& params, double volume, bool isBid) {
  // TODO
  // This function needs to be implemented
  // The code below is given as a general guideline but needs to be rewritten
  // to match the Poloniex API
  json_t* root;
  if (isBid) {
    root = json_object_get(getJsonFromUrl(params, "", ""), "bids");
  } else {
    root = json_object_get(getJsonFromUrl(params, "", ""), "asks");
  }
  *params.logFile << "<Poloniex> Looking for a limit price to fill " << fabs(volume) << " BTC..." << std::endl;
  double tmpVol = 0.0;
  double p;
  double v;
  int i = 0;
    // loop on volume
  while (tmpVol < fabs(volume) * params.orderBookFactor) {
    p = atof("");
    v = atof("");
    *params.logFile << "<Poloniex> order book: " << v << "@$" << p << std::endl;
    tmpVol += v;
    i++;
  }
  double limPrice = 0.0;
  limPrice = atof(json_string_value(json_object_get(json_array_get(root, i-1), "price")));
  json_decref(root);
  return limPrice;
}

json_t* authRequest(Parameters& params, std::string url, std::string request, std::string options) {
  // TODO
  // This function needs to be implemented
  // The code below is given as a general guideline but needs to be rewritten
  // to match the Poloniex API
  struct timeval tv;
  gettimeofday(&tv, NULL);
  unsigned long long nonce = (tv.tv_sec * 1000.0) + (tv.tv_usec * 0.001) + 0.5;
  std::ostringstream oss;
  oss << nonce << params.bitstampClientId << params.bitstampApi;
  unsigned char* digest;
  digest = HMAC(EVP_sha256(), params.bitstampSecret.c_str(), strlen(params.bitstampSecret.c_str()), (unsigned char*)oss.str().c_str(), strlen(oss.str().c_str()), NULL, NULL);
  char mdString[SHA256_DIGEST_LENGTH+100];  // FIXME +100
  for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
    sprintf(&mdString[i*2], "%02X", (unsigned int)digest[i]);
  }
  oss.clear();
  oss.str("");
  oss << "key=" << params.bitstampApi << "&signature=" << mdString << "&nonce=" << nonce << "&" << options;
  std::string postParams = oss.str().c_str();
  CURLcode resCurl;
  if (params.curl) {
    std::string readBuffer;
    curl_easy_setopt(params.curl, CURLOPT_POST,1L);
    curl_easy_setopt(params.curl, CURLOPT_POSTFIELDS, postParams.c_str());
    curl_easy_setopt(params.curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(params.curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(params.curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(params.curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(params.curl, CURLOPT_CONNECTTIMEOUT, 10L);
    resCurl = curl_easy_perform(params.curl);
    json_t* root;
    json_error_t error;
    while (resCurl != CURLE_OK) {
      *params.logFile << "<Poloniex> Error with cURL. Retry in 2 sec..." << std::endl;
      sleep(2.0);
      readBuffer = "";
      resCurl = curl_easy_perform(params.curl);
    }
    root = json_loads(readBuffer.c_str(), 0, &error);
    while (!root) {
      *params.logFile << "<Poloniex> Error with JSON:\n" << error.text << std::endl;
      *params.logFile << "<Poloniex> Buffer:\n" << readBuffer.c_str() << std::endl;
      *params.logFile << "<Poloniex> Retrying..." << std::endl;
      sleep(2.0);
      readBuffer = "";
      resCurl = curl_easy_perform(params.curl);
      while (resCurl != CURLE_OK) {
        *params.logFile << "<Poloniex> Error with cURL. Retry in 2 sec..." << std::endl;
        sleep(2.0);
        readBuffer = "";
        resCurl = curl_easy_perform(params.curl);
      }
      root = json_loads(readBuffer.c_str(), 0, &error);
    }
    curl_easy_reset(params.curl);
    return root;
  } else {
    *params.logFile << "<Poloniex> Error with cURL init." << std::endl;
    return NULL;
  }
}

}

