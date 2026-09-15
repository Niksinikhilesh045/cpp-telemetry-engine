#include "telemetry/parser.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace telemetry {
namespace {

std::string trim(std::string value) {
    const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

template <typename T, typename Converter>
bool parse_number(const std::string& input, T& output, Converter converter) {
    try {
        std::size_t consumed = 0;
        const auto value = converter(input, &consumed);
        if (consumed != input.size()) {
            return false;
        }
        output = static_cast<T>(value);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

}  // namespace

ParseResult TelemetryParser::parse(std::string_view payload, Transport transport) const {
    std::unordered_map<std::string, std::string> fields;
    std::size_t start = 0;

    while (start <= payload.size()) {
        const auto end = payload.find(';', start);
        const auto token_view = payload.substr(
            start, end == std::string_view::npos ? payload.size() - start : end - start);
        std::string token = trim(std::string(token_view));

        if (!token.empty()) {
            const auto separator = token.find('=');
            if (separator == std::string::npos || separator == 0 || separator == token.size() - 1) {
                return {std::nullopt, "invalid key=value token: " + token};
            }

            auto key = trim(token.substr(0, separator));
            auto value = trim(token.substr(separator + 1));
            if (!fields.emplace(std::move(key), std::move(value)).second) {
                return {std::nullopt, "duplicate telemetry field"};
            }
        }

        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }

    constexpr const char* required[] = {"device", "temperature", "pressure", "rpm", "timestamp"};
    for (const char* field : required) {
        if (fields.find(field) == fields.end()) {
            return {std::nullopt, std::string("missing required field: ") + field};
        }
    }

    TelemetryEvent event;
    event.device_id = fields.at("device");
    event.transport = transport;

    if (event.device_id.empty()) {
        return {std::nullopt, "device must not be empty"};
    }

    if (!parse_number(fields.at("temperature"), event.temperature,
                      [](const std::string& value, std::size_t* consumed) {
                          return std::stod(value, consumed);
                      })) {
        return {std::nullopt, "invalid temperature"};
    }

    if (!parse_number(fields.at("pressure"), event.pressure,
                      [](const std::string& value, std::size_t* consumed) {
                          return std::stod(value, consumed);
                      })) {
        return {std::nullopt, "invalid pressure"};
    }

    if (!parse_number(fields.at("rpm"), event.rpm,
                      [](const std::string& value, std::size_t* consumed) {
                          return std::stoi(value, consumed);
                      }) ||
        event.rpm < 0) {
        return {std::nullopt, "invalid rpm"};
    }

    if (!parse_number(fields.at("timestamp"), event.timestamp_ms,
                      [](const std::string& value, std::size_t* consumed) {
                          return std::stoll(value, consumed);
                      }) ||
        event.timestamp_ms <= 0) {
        return {std::nullopt, "invalid timestamp"};
    }

    return {std::move(event), {}};
}

}  // namespace telemetry
