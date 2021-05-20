#ifndef NETSPEAK_UTIL_LFU_CACHE_HPP
#define NETSPEAK_UTIL_LFU_CACHE_HPP

#include <algorithm>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


namespace netspeak {
namespace util {

/**
 * A simple and fast object cache with LFU policy.
 */
template <typename T>
class LfuCache {
public:
  typedef std::string key_type;
  typedef T value_type;

private:
  typedef std::shared_ptr<value_type> shared_value;
  typedef std::pair<key_type, unsigned> key_priority_pair;
  typedef std::vector<key_priority_pair*> policy_type;
  typedef std::pair<shared_value, key_priority_pair*> entry_type;
  typedef std::unordered_map<key_type, entry_type> storage_type;
  typedef std::pair<typename storage_type::iterator, bool> insert_result;

  struct greater {
    bool operator()(const key_priority_pair* lhs,
                    const key_priority_pair* rhs) {
      return lhs->second > rhs->second;
    }
  };

public:
  typedef typename storage_type::const_iterator const_iterator;

  LfuCache(size_t capacity = 0)
      : storage_(capacity), acc_count_(0), hit_count_(0) {
    policy_.reserve(capacity);
  }
  LfuCache(const LfuCache&) = delete;
  ~LfuCache() {
    clear();
  }

  size_t access_count() const {
    return acc_count_;
  }

  const_iterator begin() const {
    return storage_.begin();
  }

  const_iterator end() const {
    return storage_.end();
  }

  double hit_rate() const {
    return acc_count_ == 0 ? 0 : static_cast<double>(hit_count_) / acc_count_;
  }

  std::ostream& list(std::ostream& os,
                     size_t n = std::numeric_limits<size_t>::max()) const {
    policy_type sorted_policy(policy_);
    std::sort_heap(sorted_policy.begin(), sorted_policy.end(), greater());
    for (unsigned i(0); i != sorted_policy.size() && i != n; ++i) {
      os << sorted_policy[i]->second << '\t' << sorted_policy[i]->first << '\n';
    }
    return os;
  }

  bool insert(const key_type& key, const std::shared_ptr<value_type>& value) {
    if (policy_.capacity() == 0)
      return false;
    std::lock_guard<std::mutex> lg(mutex_);
    return insert_(key, value).second;
  }

  bool update(const key_type& key, const std::shared_ptr<value_type>& value) {
    if (policy_.capacity() == 0)
      return false;
    std::lock_guard<std::mutex> lg(mutex_);
    const insert_result result(insert_(key, value));
    if (!result.second) { // overwrite old value and reset priority
      result.first->second.first = value;      // overwrite value
      result.first->second.second->second = 1; // overwrite priority
      std::make_heap(policy_.begin(), policy_.end(), greater());
    }
    return true;
  }

  unsigned priority(const key_type& key) const {
    typename storage_type::const_iterator it(storage_.find(key));
    return it == storage_.end() ? 0 : it->second.second->second;
  }

  std::shared_ptr<value_type> find(const key_type& key) {
    shared_value value;
    std::lock_guard<std::mutex> lg(mutex_);
    typename storage_type::iterator it(storage_.find(key));
    if (it != storage_.end()) {
      it->second.second->second++;
      std::make_heap(policy_.begin(), policy_.end(), greater());
      value = it->second.first;
      ++hit_count_;
    }
    ++acc_count_;
    return value;
  }

  void erase(const key_type& key) {
    std::lock_guard<std::mutex> lg(mutex_);
    typename storage_type::iterator it(storage_.find(key));
    if (it != storage_.end()) {
      it->second.second->second = 0; // set priority to minimum
      std::make_heap(policy_.begin(), policy_.end(), greater()); // bubble
      remove_top_();
    }
  }

  void reserve(size_t capacity) {
    if (capacity > policy_.capacity()) {
      storage_.rehash(capacity);
      policy_.reserve(capacity);
    }
  }

  size_t capacity() const {
    return policy_.capacity();
  }

  size_t size() const {
    return policy_.size();
  }

  bool empty() const {
    return policy_.empty();
  }

  void clear() {
    // delete priorities
    std::lock_guard<std::mutex> lg(mutex_);
    for (const auto& pair : storage_) {
      delete pair.second.second;
    }
    storage_.clear();
    policy_.clear();
  }

private:
  // Lock mutex before calling these methods
  const insert_result insert_(const key_type& key,
                              const std::shared_ptr<value_type>& value) {
    const entry_type entry(value, static_cast<key_priority_pair*>(NULL));
    insert_result result(storage_.insert(std::make_pair(key, entry)));
    if (result.second) { // The key-value pair was inserted
      if (size() == capacity()) {
        remove_top_(); // Policy responsibility
      }
      policy_.push_back(new key_priority_pair(key, 1));
      result.first->second.second = policy_.back();
      std::push_heap(policy_.begin(), policy_.end(), greater());
    }
    return result;
  }

  void remove_top_() {
    if (policy_.empty())
      return;
    std::pop_heap(policy_.begin(), policy_.end(), greater());
    typename storage_type::iterator it(storage_.find(policy_.back()->first));
    delete it->second.second; // Delete priority
    storage_.erase(it);
    policy_.pop_back();
  }

private:
  policy_type policy_;
  storage_type storage_;
  uint64_t acc_count_;
  uint64_t hit_count_;
  std::mutex mutex_;
};


} // namespace util
} // namespace netspeak

#endif // NETSPEAK_LFU_CACHE_HPP
