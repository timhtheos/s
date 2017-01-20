#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ctype.h>
#include <unistd.h>
#include "../inih/ini.h"

int isMobileNumber(char number[]);

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
  char stfilePattern[] = "/tmp/s_send_XXXXXX";

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

  if (argc > 1 && strcmp(argv[1], "send") == 0) {
    // Check if the recipient's mobile number has been entered.
    if (argc == 2) {
      printf("Mobile number not entered.\n");
      printf("To send: s send <number> <msg>\n");
      return 1;
    }

    // Check if the mobile number is valid.
    if (isMobileNumber(argv[2]) == 0) {
      printf("Invalid mobile number.\n");
      printf("Valid characters: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9\n");
      printf("An optional leading + is also a valid character.\n");
      return 1;
    }

    // Check if there is a message.
    if (argc < 3 ) {
      printf("No message was found.");
      printf("To send: s send <number> <msg>\n");
      return 1;
    }
    else {
      char stfileCommand[128];

      // Generate temp file and open system's defualt editor.
      mkstemp(stfilePattern);
      snprintf(stfileCommand, sizeof(stfileCommand),
          "stfilePath=\"%s.tmp\" && $EDITOR $stfilePath;", stfilePattern);
      system(stfileCommand);

      // Check if the messaged has been saved.
      if(access(stfilePattern, F_OK ) == -1 ) {
        printf("Not found or invalid message.");
        printf("Sending message has been aborted.");
        return 1;
      }
    }

    /**
     * Initialize the fucking libcurl.
     *
     * @todo: Move into a separate function.
     */
    curl = curl_easy_init();
    if(curl) {
      char strparams[128];

      /**
       * Check if there is a message, and compose http post from arv,
       * otherwise, from stfilePattern.
       *
       * @todo: Improve using dry.
       */
      if (argc > 2) {
        snprintf(strparams, sizeof(strparams),
            "email=%s&password=%s&device=%s&number=%s&message=%s",
            config.name, config.pswd, config.device, argv[2], argv[3]);
      }
      else {
        snprintf(strparams, sizeof(strparams),
            "email=%s&password=%s&device=%s&number=%s&message=%s",
            config.name, config.pswd, config.device, argv[2],
            stfilePattern);
      }

      // Set endpoint.
      curl_easy_setopt(curl, CURLOPT_URL,
          "http://smsgateway.me/api/v3/messages/send");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strparams);

      // Send message.
      res = curl_easy_perform(curl);
      if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
      curl_easy_cleanup(curl);
    }

    // Let's do the routine cleanup.
    system("find /tmp/ -name \"s_send_*\" -exec rm {} \\;");
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

/**
 * Simple function to check if number is a valid number.
 */
int isMobileNumber(char number[]) {
  int i = 0;

  // Check if the number is negative.
  if (number[0] == '-') {
    i = 1;
  }

  // Check if the number has a leading + sign.
  if (number[0] == '+') {
    i = 1;
  }

  // Check each char if it is a number.
  for (; number[i] != 0; i++) {
    if (!isdigit(number[i])) {
      return 0;
    }
  }

  return 1;
}
