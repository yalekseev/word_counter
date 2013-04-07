#ifndef SYNCHRONIZED_QUEUE_H
#define SYNCHRONIZED_QUEUE_H

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <deque>

/*! \brief A thread-safe blocking queue wrapper based on std::deque. */
template<typename T>
class SynchronizedQueue {
public:
  explicit SynchronizedQueue(size_t max_size) : MAX_SIZE(max_size) { }

  /*! \brief Remove all elements from the queue. */
  void clear();

  /*! \brief Check if the queue is empty. */
  bool empty() const;

  /*! \brief Take an element from the queue in a blocking FIFO mode. */
  void pop(T &);

  /*! \brief Add an element to the queue in a blocking FIFO mode. */
  void push(const T &);

  /*! Return the number of elements in the queue. */
  size_t size() const;

private:
  std::deque<T> m_queue;
  mutable boost::mutex m_mutex;

  const size_t MAX_SIZE;

  boost::condition_variable m_cv_not_full;
  boost::condition_variable m_cv_not_empty;
};

template <typename T>
void SynchronizedQueue<T>::clear() {
  boost::mutex::scoped_lock lock(m_mutex);
  m_queue.clear();
}

template <typename T>
bool SynchronizedQueue<T>::empty() const {
  boost::mutex::scoped_lock lock(m_mutex);
  return m_queue.empty();
}

template <typename T>
void SynchronizedQueue<T>::pop(T & t) {
  boost::mutex::scoped_lock lock(m_mutex);
  m_cv_not_empty.wait(lock, [&]{return !m_queue.empty();});
  t = m_queue.front();
  m_queue.pop_front();
  m_cv_not_full.notify_one();
}

template <typename T>
void SynchronizedQueue<T>::push(const T & t) {
  boost::mutex::scoped_lock lock(m_mutex);
  m_cv_not_full.wait(lock, [&]{return m_queue.size() < MAX_SIZE;});
  m_queue.push_back(t);
  m_cv_not_empty.notify_one();
}

template<typename T>
size_t SynchronizedQueue<T>::size() const {
  boost::mutex::scoped_lock lock(m_mutex);
  return m_queue.size();
};

#endif // SYNCHRONIZED_QUEUE_H
