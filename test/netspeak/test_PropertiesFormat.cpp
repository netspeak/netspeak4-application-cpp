#include <map>
#include <sstream>
#include <string>

#include <boost/test/unit_test.hpp>

#include "netspeak/util/PropertiesFormat.hpp"

namespace std {
std::ostream& operator<<(std::ostream& out,
                         const std::pair<std::string, std::string>& pair) {
  return out << "pair<\"" << pair.first << "\", \"" << pair.second << "\">";
}
} // namespace std

namespace netspeak {

BOOST_AUTO_TEST_SUITE(PropertiesFormat)

BOOST_AUTO_TEST_CASE(test_read) {
  std::string data = R"(
    # comment
    ! also comment

    # different delimiters
    key0 = value
    key1: value
    key2   :   value
    key3 value

    # empty value
    empty_value=

    # And now, all the stuff from Wikipedia
    # https://en.wikipedia.org/wiki/.properties

# You are reading the ".properties" entry.
! The exclamation mark can also mark text as comments.
# The key characters =, and : should be written with
# a preceding backslash to ensure that they are properly loaded.
# However, there is no need to precede the value characters =, and : by a
# backslash.
website = https://en.wikipedia.org/
language = English
# The backslash below tells the application to continue reading
# the value onto the next line.
message = Welcome to \
          Wikipedia!
# But if the number of backslashes at the end of the line is even, the next line
# is not included in the value. In the following example, the value for "key" is
# "valueOverOneLine\"
key = valueOverOneLine\\
# This line is not included in the value for "key"
# Add spaces to the key
key\ with\ spaces = This is the value that could be looked up with the key "key with spaces".
# The characters = and : in the key must be escaped as well:
key\:with\=colonAndEqualsSign = This is the value for the key "key:with=colonAndEqualsSign"
# Unicode
tab : \u0009
# If you want your property to include a backslash, it should be escaped by
# another backslash
path=c:\\wiki\\templates
# However, some editors will handle this automatically
  )";

  std::vector<std::pair<std::string, std::string>> actual;
  std::stringstream str(data);
  util::read_properties(str, std::back_inserter(actual));

  std::vector<std::pair<std::string, std::string>> expected{
    { "key0", "value" },
    { "key1", "value" },
    { "key2", "value" },
    { "key3", "value" },

    { "empty_value", "" },

    { "website", "https://en.wikipedia.org/" },
    { "language", "English" },
    { "message", "Welcome to Wikipedia!" },
    { "key", "valueOverOneLine\\" },
    { "key with spaces",
      "This is the value that could be looked up with the key \"key with "
      "spaces\"." },
    { "key:with=colonAndEqualsSign",
      "This is the value for the key \"key:with=colonAndEqualsSign\"" },
    { "tab", "\t" },
    { "path", "c:\\wiki\\templates" },
  };

  BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(),
                                actual.begin(), actual.end());
}

BOOST_AUTO_TEST_CASE(test_write) {
  std::map<std::string, std::string> data{
    { "foo", "" },
    { "foo.bar", "baz\n" },
    { "foo bar", " bizzle bazzle" },
    { "#foo bar=:\t", "\t\r\n\x01\\ foo" },
  };

  std::stringstream str;
  util::write_properties(str, data.cbegin(), data.cend());
  std::map<std::string, std::string> copy;
  util::read_properties(str, std::inserter(copy, copy.end()));

  BOOST_CHECK_EQUAL_COLLECTIONS(data.begin(), data.end(), copy.begin(),
                                copy.end());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace netspeak
