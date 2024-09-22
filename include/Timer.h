#include <chrono>
#include <iostream>

class Timer {
public:
  Timer(const std::string &sectionName)
      : name(sectionName), start(std::chrono::high_resolution_clock::now()) {}

  ~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();
    std::cout << "Time taken for \"" << name << "\": " << duration
              << " microseconds.\n";
  }

private:
  std::string name;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
};
