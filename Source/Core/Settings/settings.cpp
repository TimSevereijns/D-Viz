#include "Settings/settings.h"
#include "constants.h"

#include <fstream>
#include <ostream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <spdlog/spdlog.h>

namespace
{
    void LogJsonParsingError(
        const std::experimental::filesystem::path& path, rapidjson::ParseErrorCode errorCode)
    {
        auto message = "Error parsing the JSON file found at: " + path.string() + ". ";

        switch (errorCode) {
            case rapidjson::ParseErrorCode::kParseErrorDocumentEmpty: {
                message += "Could not locate the file.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorDocumentRootNotSingular: {
                message += "The JSON root may not be followed by other values.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorValueInvalid: {
                message += "Encountered an invalid value.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorObjectMissName: {
                message += "Missing a name for an object member.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorObjectMissColon: {
                message += "Missing a colon after an object member name.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorObjectMissCommaOrCurlyBracket: {
                message += "Missing a comma or '}' after an object member.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorArrayMissCommaOrSquareBracket: {
                message += "Missing a comma or ']' after an array member.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorStringUnicodeEscapeInvalidHex: {
                message += "Incorrect hex digit after \\u escape in string.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorStringUnicodeSurrogateInvalid: {
                message += "The surrogate pair in string is invalid.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorStringEscapeInvalid: {
                message += "Invalid escape character in string.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorStringMissQuotationMark: {
                message += "Missing a closing quotation mark in string.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorStringInvalidEncoding: {
                message += "Invalid encoding in string.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorNumberTooBig: {
                message += "Number too big to be stored in double.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorNumberMissFraction: {
                message += "Missing fractional part in number.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorNumberMissExponent: {
                message += "Missing exponent in number.";
                break;
            }
            case rapidjson::ParseErrorCode::kParseErrorTermination: {
                message += "Parsing was terminated.";
                break;
            }
            default: {
                message += "Unspecific syntax error.";
                break;
            }
        }

        const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
        log->error(message);
    }
} // namespace

namespace Settings
{
    JsonDocument LoadFromDisk(const std::experimental::filesystem::path& path)
    {
        std::ifstream fileStream{ path.string() };
        rapidjson::IStreamWrapper streamWrapper{ fileStream };

        JsonDocument document;
        document.ParseStream<rapidjson::kParseDefaultFlags, rapidjson::UTF8<char>>(streamWrapper);

        if (document.HasParseError()) {
            LogJsonParsingError(path, document.GetParseError());
        }

        return document;
    }

    bool SaveToDisk(const JsonDocument& document, const std::experimental::filesystem::path& path)
    {
        std::ofstream fileStream{ path.string() };
        rapidjson::OStreamWrapper streamWrapper{ fileStream };

        rapidjson::PrettyWriter<decltype(streamWrapper), rapidjson::UTF16<wchar_t>> writer{
            streamWrapper
        };

        const auto success = document.Accept(writer);

        if (!success) {
            const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
            log->error("Encountered error writing JSON document to \"{}\".", path.string());
        }

        return success;
    }
} // namespace Settings
