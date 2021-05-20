// test_Indexer.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

namespace bfs = boost::filesystem;

// Note that the main functionality is already tested by ManagedIndexer.
// This test is only to check proper behaviour at too large records.

BOOST_AUTO_TEST_SUITE(test_Indexer);

// BOOST_AUTO_TEST_CASE(test_too_long_key)
//{
//  typedef uint64_t value_type;
//
//  const bfs::path index_dir("test_Indexer_test_too_long_key");
//  BOOST_REQUIRE(bfs::create_directory(index_dir));
//
//  Configuration config;
//  config.set_index_directory(index_dir.string());
//  config.set_key_sorting(util::key_sorting::unsorted);
//  config.set_value_sorting(util::value_sorting::disabled);
//
//  Indexer<value_type> indexer(config);
//  // Build key that is longer than std::numeric_limit<uint16_t>::max()
//  try
//  {
//    indexer.insert(Record<value_type>(std::string(100000, 'x'), 0));
//  }
//  catch (std::invalid_argument& e)
//  {
//    std::cerr << e.what() << std::endl;
//  }
//  indexer.index();
//  bfs::remove_all(index_dir);
//}
//
// BOOST_AUTO_TEST_CASE(test_too_long_value)
//{
//  typedef std::string value_type;
//
//  const bfs::path index_dir("test_Indexer_test_too_long_value");
//  BOOST_REQUIRE(bfs::create_directory(index_dir));
//
//  Configuration config;
//  config.set_index_directory(index_dir.string());
//  config.set_key_sorting(util::key_sorting::unsorted);
//  config.set_value_sorting(util::value_sorting::disabled);
//
//  Indexer<value_type> indexer(config);
//  // Build value that is longer than std::numeric_limit<uint16_t>::max()
//  try
//  {
//    indexer.insert(Record<value_type>("key", std::string(100000, 'x')));
//  }
//  catch (std::invalid_argument& e)
//  {
//    std::cerr << e.what() << std::endl;
//  }
//  indexer.index();
//  bfs::remove_all(index_dir);
//}

BOOST_AUTO_TEST_SUITE_END();
