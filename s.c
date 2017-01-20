#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../inih/ini.h"

typedef struct {
  const char* name;
  const char* pswd;
  const char* device;
} configuration;

static int handler(void* user, const char* section, const char* name,
  const char* value) {
  configuration* pconfig = (configuration*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
  if (MATCH("user", "name")) {
    pconfig->name = strdup(value);
  }
  else if (MATCH("user", "pswd")) {
    pconfig->pswd = strdup(value);
  }
  else if (MATCH("user", "device")) {
    pconfig->device = strdup(value);
  }
  else {
    return 0;
  }

  return 1;
}

int main(int argc, char *argv[]) {
  /**
   * config.ini.
   */
  configuration config;
  if (ini_parse("s.ini", handler, &config) < 0) {
    printf("Can't load 's.ini'\n");
    return 1;
  }

  /**
   * libcurl http-post.
   */
  CURL *curl;
  CURLcode res;

  /**
   * Let's start printing some stupid stuff.
   */
  printf("\n");

  if ( argc > 1 && strcmp(argv[1], "send") == 0 ) {
    /**
     * Initialize the fucking libcurl.
     */
    curl = curl_easy_init();
    if(curl) {
      char strparams[128];
      snprintf(strparams, sizeof(strparams), "email=%s&password=%s&device=%s&number=09099999010&message=test+from+using+libcurl+in+c+plus+config+ini+file \"",
          config.name, config.pswd, config.device);
      curl_easy_setopt(curl, CURLOPT_URL, "http://smsgateway.me/api/v3/messages/send");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strparams);
      res = curl_easy_perform(curl);
      if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
      curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
  }
  else if(argc > 1 &&
    (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
    printf("version beta-1.0\n");
    printf("by tae et. al.\n");
  }
  else if(argc == 1 ||
    (argc > 1 && (strcmp(argv[1], "help") == 0 ||
    strcmp(argv[1], "--help") == 0))) {
    printf("             ____  __  __ ____\n");
    printf("            / ___||  \\/  / ___|\n");
    printf("            \\___ \\| |\\/| \\___ \\\n");
    printf("             ___) | |  | |___) |\n");
    printf(" Welcome to |____/|_|  |_|____/ !!!\n");
  }
  else {
    printf("Invalid argument(s).\n");
    printf("Command \"%s\" not found.\n", argv[1]);
  }

  return 0;
}
