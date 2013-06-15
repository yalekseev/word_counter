#include <string>
#include <thread>
#include <map>

#include "word_counter.h"

void print_top(const std::multimap<size_t, std::string> & top) {
  for (auto it = top.rbegin(); it != top.rend(); ++it) {
    std::cout << it->second << " : " << it->first << std::endl;
  }
}

void print_usage(const std::string & program) {
  std::cout << program << " directory top_size" << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    print_usage(argv[0]);
    return 1;
  }

  std::string directory_name = argv[1];

  size_t num_workers = std::thread::hardware_concurrency();
  if (num_workers == 0) {
    num_workers = 1;
  }

  WordCounter word_counter;
  word_counter.process_directory(directory_name, num_workers);

  size_t top_size = std::stoul(argv[2]);

  std::multimap<size_t, std::string> top;
  word_counter.get_top(top, top_size);

  print_top(top);

  return 0;
}
