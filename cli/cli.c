#include <stdlib.h>

static int kl_cli_arg_count = 0;
static char** kl_cli_arg_values = NULL;

void kl_cli_set_args(int argc, char** argv) {
    if (argc <= 1 || !argv) {
        kl_cli_arg_count = 0;
        kl_cli_arg_values = NULL;
        return;
    }

    kl_cli_arg_count = argc - 1;
    kl_cli_arg_values = argv + 1;
}

int kl_cli_argc(void) {
    return kl_cli_arg_count;
}

char* kl_cli_arg(int index) {
    if (index < 0 || index >= kl_cli_arg_count || !kl_cli_arg_values) {
        return "";
    }
    return kl_cli_arg_values[index];
}
