#include "netspeak/util/PropertiesFormat.hpp"

#include <istream>
#include <ostream>


namespace netspeak {
namespace util {
namespace __properties_format_impl {

inline bool is_space(int c) {
  return c == ' ' || c == '\t';
}
inline bool is_line_break(int c) {
  return c == '\r' || c == '\n';
}
inline bool is_delimiter(int c) {
  return c == '=' || c == ':';
}

/**
 * @brief Consumes any number of spaces and tabs.
 *
 * @param in
 */
void consume_spaces(std::istream& in) {
  int c = in.peek();
  while (c != EOF) {
    if (is_space(c)) {
      // consume character
      in.get();
      c = in.peek();
    } else {
      break;
    }
  }
}
/**
 * @brief Consumes a comment.
 *
 * @param in
 */
void consume_comment(std::istream& in) {
  int c;
  do {
    c = in.get();
  } while (c != EOF && !is_line_break(c));
}
/**
 * @brief Consumes all spaces, empty lines, can comments before a key.
 *
 * @param in
 */
void consume_before_key(std::istream& in) {
  int c = in.peek();
  while (c != EOF) {
    if (is_space(c) || is_line_break(c)) {
      // consume character
      in.get();
      c = in.peek();
    } else if (c == '#' || c == '!') {
      // consume comment
      consume_comment(in);
      c = in.peek();
    } else {
      break;
    }
  }
}
void consume_delimiter(std::istream& in) {
  // A key-value delimiter is either a ':' or '=' surrounded by any number of
  // spaces or at least one space followed by any number of spaces. (Spaces
  // means ' ' and '\t' here.)

  int c = in.get();
  if (c == EOF) {
    throw format_properties_error("Unexpected EOF after key at " +
                                  std::to_string(in.tellg()));
  }
  if (is_delimiter(c)) {
    // just consume the following spaces
    consume_spaces(in);
  } else if (is_space(c)) {
    // consume the following spaces
    consume_spaces(in);
    c = in.peek();
    if (is_delimiter(c)) {
      in.get(); // consume delimiter
      // consume the following spaces
      consume_spaces(in);
    }
  } else {
    throw format_properties_error("Unexpected character after key at " +
                                  std::to_string(in.tellg()));
  }
}

void read_escaped(std::istream& in, std::string& consumer) {
  int c = in.get();
  if (c == EOF) {
    throw format_properties_error("Unexpected EOF during escape at " +
                                  std::to_string(in.tellg()));
  } else if (c == 'u') {
    // unicode escpae
    uint16_t escaped = 0;
    for (size_t i = 0; i < 4; i++) {
      c = in.get();
      uint16_t digit;
      if (c == EOF) {
        throw format_properties_error(
            "Unexpected EOF during Unicode escape at " +
            std::to_string(in.tellg()));
      } else if (c >= '0' && c <= '9') {
        digit = c - '0';
      } else if (c >= 'A' && c <= 'F') {
        digit = 10 + c - 'A';
      } else if (c >= 'a' && c <= 'f') {
        digit = 10 + c - 'a';
      } else {
        throw format_properties_error(
            "Unexpected character during Unicode escape at " +
            std::to_string(in.tellg()));
      }
      escaped = (escaped << 4) | digit;
    }
    if (escaped >= 128) {
      throw format_properties_error(
          "Right now, only ASCII characters can be escape at " +
          std::to_string(in.tellg()));
    }
    consumer.push_back((char)escaped);
  } else if (c == '\n') {
    // skip and number of spaces after "\\\n"
    consume_spaces(in);
  } else if (c == '\r') {
    // skip and number of spaces after "\\\r" or "\\\r\n"
    c = in.peek();
    if (c == '\n') {
      in.get();
    }
    consume_spaces(in);
  } else {
    // literal escape
    consumer.push_back(c);
  }
}

bool has_next_key(std::istream& in) {
  consume_before_key(in);
  return in.good();
}
std::string read_key(std::istream& in) {
  std::string key;

  int c;
  while (true) {
    c = in.peek();
    if (c == EOF) {
      throw format_properties_error("Unexpected EOF after key at " +
                                    std::to_string(in.tellg()));
    } else if (c == '\\') {
      in.get();
      read_escaped(in, key);
    } else if (is_line_break(c)) {
      throw format_properties_error("Unexpected line break after key at " +
                                    std::to_string(in.tellg()));
    } else if (is_delimiter(c) || is_space(c)) {
      // reached end of key
      break;
    } else {
      in.get();
      key.push_back((char)c);
    }
  }

  return key;
}
std::string read_value(std::istream& in) {
  std::string value;

  int c;
  while (true) {
    c = in.peek();
    if (c == EOF || is_line_break(c)) {
      // reached end of value
      break;
    } else if (c == '\\') {
      in.get();
      read_escaped(in, value);
    } else {
      in.get();
      value.push_back((char)c);
    }
  }

  return value;
}


void write_hex_digit(std::ostream& out, char c) {
  if (c < 10) {
    out << (char)('0' + c);
  } else {
    out << (char)('A' - (char)10 + c);
  }
}
void write_unicode_char(std::ostream& out, char c) {
  if (c < 0) {
    throw tracable_logic_error(
        "Right now, only ASCII characters can be escape.");
  }
  out << '\\' << 'u' << '0' << '0';
  write_hex_digit(out, c >> 4);
  write_hex_digit(out, c & 0xF);
}
void write_key(std::ostream& out, const std::string& key) {
  for (auto c : key) {
    switch (c) {
      case '\\':
      case '=':
      case ':':
      case '#':
      case '!':
      case ' ':
        out << '\\' << c;
        break;
      default:
        if (c < ' ') {
          // all control characters
          write_unicode_char(out, c);
        } else {
          out << c;
        }
        break;
    }
  }
}
void write_value(std::ostream& out, const std::string& value) {
  auto it = value.begin();
  if (it == value.end()) {
    // empty string
    return;
  }
  if (*it == ' ') {
    // we have to escape the first space (if any)
    it++;
    out << '\\' << ' ';
  }

  for (; it != value.end(); it++) {
    auto c = *it;
    switch (c) {
      case '\\':
        out << '\\' << c;
        break;
      default:
        if (c < ' ') {
          // all control characters
          write_unicode_char(out, c);
        } else {
          out << c;
        }
        break;
    }
  }
}

} // namespace __properties_format_impl
} // namespace util
} // namespace netspeak
