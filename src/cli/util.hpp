#ifndef CLI_UTIL_HPP
#define CLI_UTIL_HPP

#include <string>

#include "netspeak/service/NetspeakService.grpc.pb.h"

namespace cli {

/**
 * @brief Returns the address of the localhost with the given port.
 */
std::string localhost(uint16_t port);

/**
 * @brief Returns the corpora of the given stub.
 *
 * @param stub
 * @param stub_address The address of the stub or the empty string.
 */
netspeak::service::CorporaResponse getCorpora(
    netspeak::service::NetspeakService::Stub& stub,
    const std::string& stub_address = "");


extern const std::string RESET;

extern const std::string FG_BLACK;
extern const std::string FG_RED;
extern const std::string FG_GREEN;
extern const std::string FG_YELLOW;
extern const std::string FG_BLUE;
extern const std::string FG_MAGENTA;
extern const std::string FG_CYAN;
extern const std::string FG_WHITE;

extern const std::string FG_BRIGHT_BLACK;
extern const std::string FG_BRIGHT_RED;
extern const std::string FG_BRIGHT_GREEN;
extern const std::string FG_BRIGHT_YELLOW;
extern const std::string FG_BRIGHT_BLUE;
extern const std::string FG_BRIGHT_MAGENTA;
extern const std::string FG_BRIGHT_CYAN;
extern const std::string FG_BRIGHT_WHITE;

extern const std::string BG_BLACK;
extern const std::string BG_RED;
extern const std::string BG_GREEN;
extern const std::string BG_YELLOW;
extern const std::string BG_BLUE;
extern const std::string BG_MAGENTA;
extern const std::string BG_CYAN;
extern const std::string BG_WHITE;

extern const std::string BG_BRIGHT_BLACK;
extern const std::string BG_BRIGHT_RED;
extern const std::string BG_BRIGHT_GREEN;
extern const std::string BG_BRIGHT_YELLOW;
extern const std::string BG_BRIGHT_BLUE;
extern const std::string BG_BRIGHT_MAGENTA;
extern const std::string BG_BRIGHT_CYAN;
extern const std::string BG_BRIGHT_WHITE;

} // namespace cli

#endif
