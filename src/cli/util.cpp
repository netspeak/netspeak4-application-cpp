#include "cli/util.hpp"

#include <iomanip>

#include "netspeak/util/service.hpp"

namespace cli {

using namespace netspeak;


std::string localhost(uint16_t port) {
  std::string local("localhost:");
  local.append(std::to_string(port));
  return local;
}

service::CorporaResponse getCorpora(service::NetspeakService::Stub& stub,
                                    const std::string& stub_address) {
  service::CorporaRequest corporaReq;
  service::CorporaResponse corporaRes;
  grpc::ClientContext context;
  auto corpora_status = stub.GetCorpora(&context, corporaReq, &corporaRes);

  if (!corpora_status.ok()) {
    std::stringstream what;
    what << "Couldn't get corpora";
    if (!stub_address.empty()) {
      what << " from " << stub_address;
    }
    what << "\n" << corpora_status;
    throw std::logic_error(what.str());
  }

  return corporaRes;
}


const std::string RESET("\x1b[0m");

#define BLACK "0m"
#define RED "1m"
#define GREEN "2m"
#define YELLOW "3m"
#define BLUE "4m"
#define MAGENTA "5m"
#define CYAN "6m"
#define WHITE "7m"

#define FG "\x1b[3"
#define FG_BRIGHT "\x1b[9"
#define BG "\x1b[4"
#define BG_BRIGHT "\x1b[10"

const std::string FG_BLACK(FG BLACK);
const std::string FG_RED(FG RED);
const std::string FG_GREEN(FG GREEN);
const std::string FG_YELLOW(FG YELLOW);
const std::string FG_BLUE(FG BLUE);
const std::string FG_MAGENTA(FG MAGENTA);
const std::string FG_CYAN(FG CYAN);
const std::string FG_WHITE(FG WHITE);

const std::string FG_BRIGHT_BLACK(FG_BRIGHT BLACK);
const std::string FG_BRIGHT_RED(FG_BRIGHT RED);
const std::string FG_BRIGHT_GREEN(FG_BRIGHT GREEN);
const std::string FG_BRIGHT_YELLOW(FG_BRIGHT YELLOW);
const std::string FG_BRIGHT_BLUE(FG_BRIGHT BLUE);
const std::string FG_BRIGHT_MAGENTA(FG_BRIGHT MAGENTA);
const std::string FG_BRIGHT_CYAN(FG_BRIGHT CYAN);
const std::string FG_BRIGHT_WHITE(FG_BRIGHT WHITE);

const std::string BG_BLACK(BG BLACK);
const std::string BG_RED(BG RED);
const std::string BG_GREEN(BG GREEN);
const std::string BG_YELLOW(BG YELLOW);
const std::string BG_BLUE(BG BLUE);
const std::string BG_MAGENTA(BG MAGENTA);
const std::string BG_CYAN(BG CYAN);
const std::string BG_WHITE(BG WHITE);

const std::string BG_BRIGHT_BLACK(BG_BRIGHT BLACK);
const std::string BG_BRIGHT_RED(BG_BRIGHT RED);
const std::string BG_BRIGHT_GREEN(BG_BRIGHT GREEN);
const std::string BG_BRIGHT_YELLOW(BG_BRIGHT YELLOW);
const std::string BG_BRIGHT_BLUE(BG_BRIGHT BLUE);
const std::string BG_BRIGHT_MAGENTA(BG_BRIGHT MAGENTA);
const std::string BG_BRIGHT_CYAN(BG_BRIGHT CYAN);
const std::string BG_BRIGHT_WHITE(BG_BRIGHT WHITE);


} // namespace cli
