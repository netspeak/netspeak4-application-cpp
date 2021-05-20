#include <sstream>
#include <string>

#include <boost/test/unit_test.hpp>

#include "netspeak/util/LfuCache.hpp"

namespace netspeak {

using namespace util;

typedef std::shared_ptr<std::string> shared_value;

BOOST_AUTO_TEST_SUITE(lfu_cache)

void fill_cache(LfuCache<std::string>& cache) {
  for (unsigned i = 0; i != cache.capacity(); ++i) {
    shared_value value(new std::string("value" + std::to_string(i)));
    BOOST_REQUIRE(cache.insert("key" + std::to_string(i), value));
  }
  // check size
  BOOST_REQUIRE_EQUAL(cache.size(), cache.capacity());
}

BOOST_AUTO_TEST_CASE(test_explicit_ctor) {
  unsigned capacity(0);
  LfuCache<std::string> cache(capacity);
  BOOST_REQUIRE_EQUAL(cache.capacity(), capacity);
  BOOST_REQUIRE_EQUAL(cache.size(), 0);

  capacity = 10;
  cache.reserve(capacity);
  BOOST_REQUIRE_EQUAL(cache.capacity(), capacity);
  BOOST_REQUIRE_EQUAL(cache.size(), 0);
}

BOOST_AUTO_TEST_CASE(test_insert) {
  LfuCache<std::string> cache(0);
  BOOST_REQUIRE_EQUAL(cache.size(), 0);
  BOOST_REQUIRE_EQUAL(cache.capacity(), 0);

  // try to insert
  BOOST_REQUIRE(!cache.insert("key", shared_value(new std::string("value"))));

  // increase capacity
  cache.reserve(1);
  BOOST_REQUIRE_EQUAL(cache.size(), 0);
  BOOST_REQUIRE_EQUAL(cache.capacity(), 1);

  // try to insert same key
  BOOST_REQUIRE(cache.insert("key1", shared_value(new std::string("value"))));
  BOOST_REQUIRE(!cache.insert("key1", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.capacity(), 1);
  BOOST_REQUIRE_EQUAL(cache.size(), 1);

  // try to insert new key
  BOOST_REQUIRE(cache.insert("key2", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.capacity(), 1);
  BOOST_REQUIRE_EQUAL(cache.size(), 1);

  // increase capacity
  cache.reserve(2);
  BOOST_REQUIRE_EQUAL(cache.size(), 1);
  BOOST_REQUIRE_EQUAL(cache.capacity(), 2);

  // try to insert same key
  BOOST_REQUIRE(!cache.insert("key2", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.capacity(), 2);
  BOOST_REQUIRE_EQUAL(cache.size(), 1);

  // try to insert new key
  BOOST_REQUIRE(cache.insert("key3", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.capacity(), 2);
  BOOST_REQUIRE_EQUAL(cache.size(), 2);

  // try to insert new key
  BOOST_REQUIRE(cache.insert("key4", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.capacity(), 2);
  BOOST_REQUIRE_EQUAL(cache.size(), 2);
}

BOOST_AUTO_TEST_CASE(test_update) {
  LfuCache<std::string> cache(0);

  // try to update
  BOOST_REQUIRE(!cache.update("key", shared_value(new std::string("value"))));

  // increase capacity
  cache.reserve(1);
  BOOST_REQUIRE_EQUAL(cache.size(), 0);
  BOOST_REQUIRE_EQUAL(cache.capacity(), 1);

  // try to update same key
  BOOST_REQUIRE(cache.update("key1", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.update("key1", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.capacity(), 1);
  BOOST_REQUIRE_EQUAL(cache.size(), 1);

  // try to update new key
  BOOST_REQUIRE(cache.update("key2", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.capacity(), 1);
  BOOST_REQUIRE_EQUAL(cache.size(), 1);

  // increase capacity
  cache.reserve(2);
  BOOST_REQUIRE_EQUAL(cache.size(), 1);
  BOOST_REQUIRE_EQUAL(cache.capacity(), 2);

  // try to update same key
  BOOST_REQUIRE(cache.update("key2", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.capacity(), 2);
  BOOST_REQUIRE_EQUAL(cache.size(), 1);

  // try to update new key
  BOOST_REQUIRE(cache.update("key3", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.capacity(), 2);
  BOOST_REQUIRE_EQUAL(cache.size(), 2);

  // try to update new key
  BOOST_REQUIRE(cache.update("key4", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.capacity(), 2);
  BOOST_REQUIRE_EQUAL(cache.size(), 2);
}

BOOST_AUTO_TEST_CASE(test_find) {
  LfuCache<std::string> cache(3);

  // try to find unknown key
  BOOST_REQUIRE(!cache.find("key0"));

  // insert key and find it
  BOOST_REQUIRE(cache.insert("key1", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.find("key1"));
  BOOST_REQUIRE(!cache.find("key0"));
  BOOST_REQUIRE_EQUAL(cache.size(), 1);

  // update key and find it
  BOOST_REQUIRE(cache.update("key2", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.find("key2"));
  BOOST_REQUIRE(!cache.find("key0"));
  BOOST_REQUIRE_EQUAL(cache.size(), 2);
}

BOOST_AUTO_TEST_CASE(test_priority) {
  LfuCache<std::string> cache(3);
  BOOST_REQUIRE_EQUAL(cache.size(), 0);
  BOOST_REQUIRE_EQUAL(cache.capacity(), 3);

  // insert three keys
  BOOST_REQUIRE(cache.insert("key1", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.insert("key2", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.insert("key3", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.priority("key1"), 1);
  BOOST_REQUIRE_EQUAL(cache.priority("key2"), 1);
  BOOST_REQUIRE_EQUAL(cache.priority("key3"), 1);

  // increase priorities
  BOOST_REQUIRE(cache.find("key3"));
  BOOST_REQUIRE(cache.find("key3"));
  BOOST_REQUIRE(cache.find("key2"));
  BOOST_REQUIRE_EQUAL(cache.priority("key0"), 0);
  BOOST_REQUIRE_EQUAL(cache.priority("key1"), 1);
  BOOST_REQUIRE_EQUAL(cache.priority("key2"), 2);
  BOOST_REQUIRE_EQUAL(cache.priority("key3"), 3);

  // inserting present key does not change priority
  BOOST_REQUIRE(!cache.insert("key2", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.priority("key2"), 2);

  // updating present keys resets its priority
  BOOST_REQUIRE(cache.update("key3", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.priority("key3"), 1);
}

BOOST_AUTO_TEST_CASE(test_erase) {
  LfuCache<std::string> cache(3);

  // insert three keys
  BOOST_REQUIRE(cache.insert("key1", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.insert("key2", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.insert("key3", shared_value(new std::string("value"))));
  cache.erase("key1");
  cache.erase("key2");

  // try to find erased keys
  BOOST_REQUIRE(!cache.find("key1"));
  BOOST_REQUIRE(!cache.find("key2"));

  // try to find present key
  BOOST_REQUIRE(cache.find("key3"));
  BOOST_REQUIRE_EQUAL(cache.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_reserve) {
  LfuCache<std::string> cache(3);

  // insert three keys
  BOOST_REQUIRE(cache.insert("key1", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.insert("key2", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.insert("key3", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.size(), 3);

  // increase capacity
  cache.reserve(10);
  BOOST_REQUIRE_EQUAL(cache.capacity(), 10);
  BOOST_REQUIRE_EQUAL(cache.size(), 3);
}

BOOST_AUTO_TEST_CASE(test_clear) {
  LfuCache<std::string> cache(3);

  // insert three keys
  BOOST_REQUIRE(cache.insert("key1", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.insert("key2", shared_value(new std::string("value"))));
  BOOST_REQUIRE(cache.insert("key3", shared_value(new std::string("value"))));
  BOOST_REQUIRE_EQUAL(cache.size(), 3);

  // clear cache
  cache.clear();
  // BOOST_REQUIRE_EQUAL(cache.capacity(), 3);
  //  BOOST_REQUIRE_EQUAL(cache.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace netspeak
