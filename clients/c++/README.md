Usage example:
```c++
#include "RewindClient.h"

int main() {
  auto &rewind = RewindClient::instance();

  // permanent frame
  rewind.set_options(1, true);
  rewind.circle(100, 100, 50, RewindClient::COLOR_RED, true);
  rewind.rectangle(200, 50, 300, 150, RewindClient::COLOR_GREEN, true);
  rewind.triangle(350, 150, 400, 50, 450, 150, RewindClient::COLOR_BLUE, true);
  rewind.line(0, 100, 200, 100, RewindClient::COLOR_YELLOW);

  std::vector<double> points = { 0, 100, 100, 200, 200, 100, 100, 0, 0, 100 };
  rewind.polyline(points, RewindClient::COLOR_WHITE);

  for (int i = 0; i < 10; ++i) {
    rewind.set_options(2);
    rewind.circle(100, 100 + i * 50, 50, RewindClient::COLOR_RED);
    rewind.popup(600, 100, 50, "popup message");
    rewind.message("Hello, %s! %d", "world", i);
    rewind.end_frame();
  }
}
```
