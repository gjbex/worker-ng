#include <cstdlib>
#include "worker_exception.h"

namespace worker {

    void exit(Error err) {
        std::exit(static_cast<int>(err));
    }

}
