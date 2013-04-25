#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <utility>
#include <fstream>
#include <string>
#include <map>

#include "word_counter.h"

WordCounter::WordCounter()
    : m_file_names_queue(MAX_QUEUE_SIZE) {
}

void WordCounter::process_directory(const std::string & path, size_t num_workers) {
  boost::thread_group threads;

  // Create a thread that reads and enqueues file names
  threads.create_thread(boost::bind(&WordCounter::search_file_names, this, path, num_workers));

  // Create threads that read files and update word counts
  for (size_t i = 0; i < num_workers; ++i) {
    threads.create_thread(boost::bind(&WordCounter::process_files, this));
  }

  // Wait for all threads to complete
  threads.join_all();
}

void WordCounter::search_file_names(const std::string & start, size_t end_of_queue_count) {
  try {
    boost::filesystem::recursive_directory_iterator dir(start), end;

    while (dir != end) {
      try {
        if (dir->path().extension() == ".txt" && boost::filesystem::is_regular_file(dir->path())) {
          m_file_names_queue.push(dir->path());
        }

        ++dir;
      } catch (const boost::filesystem::filesystem_error & e) {
        std::cerr << e.what() << std::endl;
        dir.no_push();
      }
    }
  } catch (const std::exception & e) {
    std::cerr << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception while reading directory: " << start << std::endl;
  }

  // Send #end_of_queue_count empty paths so that working threads can finish processing
  for (size_t i = 0; i < end_of_queue_count; ++i) {
    m_file_names_queue.push("");
  }
}

void WordCounter::process_files() {
  while (true) {
    boost::filesystem::path file_path;
    m_file_names_queue.pop(file_path);

    // Empty path designates the end of processing
    if (file_path.empty()) {
      return;
    }

    // Read words from file updating respective counts
    std::string word;
    std::ifstream file(file_path.c_str());
    for (std::istreambuf_iterator<char> it(file), end; it != end; ++it) {
      char ch = *it;

      if (std::isalnum(ch)) {
        ch = std::tolower(ch);
        word += ch;
      } else {
        if (!word.empty()) {
          {
            boost::mutex::scoped_lock lock(m_word_counts_mutex);
            ++m_word_counts[word];
          }
          word.clear();
        }
      }
    }

    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
  }
}

void WordCounter::get_top(std::multimap<size_t, std::string> & top, size_t max_top_size) const {
  top.clear();

  // Create a (count -> word) map
  {
    boost::mutex::scoped_lock lock(m_word_counts_mutex);
    for (const auto & word_count : m_word_counts) {
      top.insert(std::make_pair(word_count.second, word_count.first));
    }
  }

  // Keep only required number of elements, remove the rest
  auto it = top.rbegin();
  while (it != top.rend() && (0 < max_top_size)) {
    --max_top_size;
    ++it;
  }

  top.erase(top.begin(), it.base());
}

void WordCounter::clear() {
  m_file_names_queue.clear();

  boost::mutex::scoped_lock lock(m_word_counts_mutex);
  m_word_counts.clear();
}
