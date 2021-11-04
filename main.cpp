#include <iostream>
#include "cJSON.h"

int main(int, char**) {
    const char* data = "";
    cJSON* cjson_node = cJSON_Parse(data);
    if (cjson_node == NULL) {
        goto cleanup;
    }
    std::cout << "Hello, world!\n";
cleanup:
    std::cout << "error: " << std::endl;
}
