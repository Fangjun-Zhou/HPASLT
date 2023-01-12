#include <sstream>
#include <iostream>
#include <string>
#include <regex>

#include "console.h"
#include "logger/logger.h"

#include "commands/commands.h"

namespace hpaslt {

eventpp::CallbackList<void(bool)> Console::s_onEnable;

Console::Console() : ImGuiObject("Console") {
  m_enabled = false;

  Commands::setConsoleSystem(&(m_consoleWindow.System()));

  // Listen to the change event.
  m_enableCallbackHandle =
      s_onEnable.append([this](bool enabled) { this->setEnabled(enabled); });
}

Console::~Console() {
  s_onEnable.remove(m_enableCallbackHandle);

  Commands::resetConsoleSystem();
}

void Console::render() {
  // TODO: Use seperate thread to parse log level.
  // Convert the console ostringstream to istringstream.
  std::unique_ptr<std::istringstream> iss = nullptr;
  {
    auto oss = logger->getConsoleOutputStream().lock();
    if (oss) {
      std::string rawLine = oss->str();
      oss->str("");
      iss = std::make_unique<std::istringstream>(rawLine);
    }
  }
  // Disaply all log.
  if (iss) {
    std::string line;
    while (std::getline(*iss, line)) {
      // Default log type.
      csys::ItemType logLevel = csys::ItemType::LOG;

      std::regex levelRegex(R"(\[((debug)|(info)|(warning)|(error))\])");
      std::smatch match;
      if (std::regex_search(line, match, levelRegex)) {
        if (match[1].str() == "debug") {
          logLevel = csys::ItemType::LOG;
        } else if (match[1].str() == "info") {
          logLevel = csys::ItemType::INFO;
        } else if (match[1].str() == "warning") {
          logLevel = csys::ItemType::WARNING;
        } else if (match[1].str() == "error") {
          logLevel = csys::ItemType::ERROR;
        }
      }

      m_consoleWindow.System().Log(logLevel) << line << csys::endl;
    }
  }

  m_consoleWindow.Draw();
}

}  // namespace hpaslt