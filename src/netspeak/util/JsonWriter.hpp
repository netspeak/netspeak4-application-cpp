#ifndef NETSPEAK_UTIL_JSON_WRITER_HPP
#define NETSPEAK_UTIL_JSON_WRITER_HPP

#include <stdint.h>

#include <string>

#include "google/protobuf/util/json_util.h"

namespace netspeak {
namespace util {

static char to_hex_digit(char value) {
  if (value < 10) {
    return '0' + value;
  } else {
    return 'a' - 10 + value;
  }
}

template <class T>
static void print_json_string(T& out, const std::string& str) {
  out << '"';
  for (const auto c : str) {
    switch (c) {
      case '\r':
        out << "\\r";
        break;
      case '\n':
        out << "\\n";
        break;
      case '\t':
        out << "\\t";
        break;
      case '\\':
        out << "\\\\";
        break;
      case '"':
        out << "\\\"";
        break;
      default: {
        const auto unsigned_c = static_cast<unsigned char>(c);
        if (unsigned_c < ' ') {
          out << "\\u00" << to_hex_digit(c >> 4) << to_hex_digit(c & 0x0F);
        } else {
          out << c;
        }
      }
    }
  }
  out << '"';
}

class JsonWriter {
  JsonWriter() = delete;
  JsonWriter(const JsonWriter&) = delete;
  JsonWriter(JsonWriter&&) = delete;

public:
  template <class S>
  class Done {
    S out_;

  public:
    explicit Done(S out) : out_(out) {}
    Done() = delete;

    S done() {
      return out_;
    }
  };

  template <class S, class R>
  class ObjectStart;
  template <class S, class R>
  class ArrayStart;

#define BOILERPLATE(name)                     \
private:                                      \
  S out_;                                     \
                                              \
public:                                       \
  explicit name(S out) : out_(out) {}         \
  name() = delete;                            \
  name(name<S, R>&&) = default;               \
  name<S, R>& operator=(name<S, R>&& other) { \
    if (this != &other) {                     \
      this->out_ = std::move(other.out_);     \
    }                                         \
    return *this;                             \
  }

  template <class S, class R>
  class Value {
    BOILERPLATE(Value)

  public:
    R null() {
      out_ << "null";
      return R(out_);
    }

    R boolean(bool value) {
      out_ << (value ? "true" : "false");
      return R(out_);
    }

    R number(int16_t value) {
      out_ << value;
      return R(out_);
    }
    R number(uint16_t value) {
      out_ << value;
      return R(out_);
    }
    R number(int32_t value) {
      out_ << value;
      return R(out_);
    }
    R number(uint32_t value) {
      out_ << value;
      return R(out_);
    }
    R number(int64_t value) {
      out_ << value;
      return R(out_);
    }
    R number(uint64_t value) {
      out_ << value;
      return R(out_);
    }

    R string(const std::string& value) {
      print_json_string(out_, value);
      return R(out_);
    }

    R raw_json(const std::string& json) {
      out_ << json;
      return R(out_);
    }

    R protobuf_message(const google::protobuf::Message& message) {
      std::string json;
      google::protobuf::util::MessageToJsonString(message, &json);
      return raw_json(json);
    }

    ObjectStart<S, R> object() {
      return ObjectStart<S, R>(out_);
    }
    ArrayStart<S, R> array() {
      return ArrayStart<S, R>(out_);
    }
  };

  template <class S, class R>
  class ObjectContinue {
    BOILERPLATE(ObjectContinue)

  public:
    Value<S, ObjectContinue<S, R>> prop(const std::string& key) {
      out_ << ',';
      print_json_string(out_, key);
      out_ << ':';
      return Value<S, ObjectContinue<S, R>>(out_);
    }

    R endObject() {
      out_ << '}';
      return R(out_);
    }
  };
  template <class S, class R>
  class ObjectStart {
    BOILERPLATE(ObjectStart)

  public:
    Value<S, ObjectContinue<S, R>> prop(const std::string& key) {
      out_ << '{';
      print_json_string(out_, key);
      out_ << ':';
      return Value<S, ObjectContinue<S, R>>(out_);
    }

    R endObject() {
      out_ << '{' << '}';
      return R(out_);
    }
  };

  template <class S, class R>
  class ArrayContinue {
    BOILERPLATE(ArrayContinue)

  public:
    Value<S, ArrayContinue<S, R>> value() {
      out_ << ',';
      return Value<S, ArrayContinue<S, R>>(out_);
    }

    R endArray() {
      out_ << ']';
      return R(out_);
    }
  };
  template <class S, class R>
  class ArrayStart {
    BOILERPLATE(ArrayStart)

  public:
    Value<S, ArrayContinue<S, R>> value() {
      out_ << '[';
      return Value<S, ArrayContinue<S, R>>(out_);
    }

    R endArray() {
      out_ << '[' << ']';
      return R(out_);
    }
  };


  template <class S>
  static Value<S&, Done<S&>> create(S out) {
    return Value<S&, Done<S&>>(out);
  }

  struct string_stream_wrapper {
  private:
    std::string* s_;

  public:
    string_stream_wrapper() = delete;
    string_stream_wrapper(const string_stream_wrapper&) = default;
    string_stream_wrapper(string_stream_wrapper&&) = default;
    string_stream_wrapper(std::string& s) : s_(&s) {}
    string_stream_wrapper& operator=(string_stream_wrapper&& other) {
      if (this != &other) {
        this->s_ = other.s_;
      }
      return *this;
    };

    inline string_stream_wrapper& operator<<(const std::string& other) {
      s_->append(other);
      return *this;
    }
    inline string_stream_wrapper& operator<<(char other) {
      s_->push_back(other);
      return *this;
    }
    string_stream_wrapper& operator<<(int8_t other) {
      return *this << std::to_string(other);
    }
    string_stream_wrapper& operator<<(uint8_t other) {
      return *this << std::to_string(other);
    }
    string_stream_wrapper& operator<<(int16_t other) {
      return *this << std::to_string(other);
    }
    string_stream_wrapper& operator<<(uint16_t other) {
      return *this << std::to_string(other);
    }
    string_stream_wrapper& operator<<(int32_t other) {
      return *this << std::to_string(other);
    }
    string_stream_wrapper& operator<<(uint32_t other) {
      return *this << std::to_string(other);
    }
    string_stream_wrapper& operator<<(int64_t other) {
      return *this << std::to_string(other);
    }
    string_stream_wrapper& operator<<(uint64_t other) {
      return *this << std::to_string(other);
    }
  };

  static Value<string_stream_wrapper, Done<string_stream_wrapper>> create(
      std::string& str) {
    string_stream_wrapper out(str);
    return Value<string_stream_wrapper, Done<string_stream_wrapper>>(out);
  }
};

#undef BOILERPLATE

} // namespace util
} // namespace netspeak

#endif
