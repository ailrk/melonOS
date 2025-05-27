#include "sys.h"


int main() {
    for(;;) {
        write(1, "sh\n", 3);
    }
    return 0;
}
