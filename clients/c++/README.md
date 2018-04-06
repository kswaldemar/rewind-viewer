Usage example:
```c++
#include "RewindClient.h"

int main() {
  auto &rewind = RewindClient::instance();
  
  for (int i = 0; i < 10; i++) {
    rewind.message("Hello, %s! %d", "world", i);
    rewind.end_frame();
  }
}
```

Mention `&` before the `rewind` variable. If you forget it, nothing will work properly.
