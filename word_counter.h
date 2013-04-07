#ifndef WORD_COUNTER_H
#define WORD_COUNTER_H

#include <boost/filesystem.hpp>
#include <string>
#include <map>

#include "synchronized_queue.h"

/*!
  \brief  A class that calculates and returns the number of occurrences of each
          word from text files located within a given directory and its subdirectories.
*/
class WordCounter {
private:
  enum { MAX_QUEUE_SIZE = 1024 };

public:
  WordCounter();

  /*! \brief Remove all elements from internal data structures. */
  void clear();

  /*! \brief Get the most frequent words with the number of their occurrences. */
  void get_top(std::multimap<size_t, std::string> & top, size_t max_top_size = 0) const;

  /*!
    \brief  Search for text files in a given directory (and its subdirectories).
            Extract words from the found files and update respective counts.
  */
  void process_directory(const std::string & path, size_t num_workers = 1);

private:
  /*! \brief Extract words from a given file and update respective counts. */
  void process_files();

  /*! \brief Search for text files in a given directory (and its subdirectories). */
  void search_file_names(const std::string & start, size_t end_of_queue_count);

private:
  mutable boost::mutex m_word_counts_mutex;
  std::map<std::string, size_t> m_word_counts;

  SynchronizedQueue<boost::filesystem::path> m_file_names_queue;
};

#endif // WORD_COUNTER_H
