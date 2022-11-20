#pragma once

#include <iostream>
#include <unordered_map>

#include "moFileReader.hpp"
#if __cplusplus == 201402L
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#else
    #include <filesystem>
    namespace fs = std::filesystem;
#endif

#ifdef _WIN32
#include <codecvt>
#include <winnls.h>
#endif

namespace LocalMan {
    struct LMLocale {
        std::string language;
        std::string country;
        std::string fullname;
    };
    std::unordered_map<std::string, fs::path> localesMap{};
    std::string currentLocale = "";
    const fs::path basePath{"./locale/compiled"};

    const char* getDisplayName(const char* name) {
        // On Windows we can query built-in APIs to get Localized Name from our input
        // But I'm not particularly happy about this method, or the split
        // Throw argument back I suppose
        return name;
    }

    void updateLocales() {
        for (auto dircont : fs::directory_iterator{basePath}) {
            if (!fs::is_regular_file(dircont)) continue;
            if (dircont.path().extension() != ".mo") continue;
            localesMap.insert({dircont.path().stem().string(), dircont.path()});
        }
    }

    void changeLocale(const char* locale) {
        try {
            const fs::path cataloguePath = localesMap.at(locale);
            moFileLib::moFileReader::eErrorCode errorCode = moFileLib::moFileReader::eErrorCode::EC_SUCCESS;
            errorCode = moFileLib::moReadMoFile(cataloguePath.c_str());
            if (errorCode != moFileLib::moFileReader::EC_SUCCESS) {
                throw std::runtime_error(moFileLib::moFileGetErrorDescription());
            }
            currentLocale = locale;
            std::cout << "Set locale to " << locale
                      << " from " << cataloguePath
                      << std::endl;
        } catch (std::exception &err) {
            std::cerr << "Unable to change locale to " << locale
                      << ". what(): " << err.what()
                      << std::endl;
        }
    }

    void changeLocale(std::string locale) {
        changeLocale(locale.c_str());
    }

    LMLocale getLocale(const char* hint);
    LMLocale getLocale(std::string hint);
    LMLocale getLocale();

    #if defined(_WIN32)
    LMLocale getLocale(const wchar_t* hint) {
        LMLocale out;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        // 9 is a magic number. Please avoid magic numbers and read
        // https://learn.microsoft.com/en-us/windows/win32/intl/locale-siso-constants instead
        // That won't make you feel better, but it'll certainly explain why '9'.
        // (it's not defined anywhere i can find, so... magic it is)
        // ((per Windows tradition))
        WCHAR currentIdentifier[9];

        // BEGIN MADNESS
        // -------------
        // "No need for error checking since there can't *possibly* be a wrong argument."
        //                                              - me, 18-NOV-2022, 19:mm:ss GMT
        // "Oh let's make it dynamic to aid testing"
        //                                              - me, 18-NOV-2022, 20:mm:ss GMT
        // "...well now good thing I added those timestamps"
        //                                              - me, 18-NOV-2022, 20:38:ss GMT
        GetLocaleInfoEx(localeName, LOCALE_SISO639LANGNAME, currentIdentifier, 9);
        out.language = converter.to_bytes(currentIdentifier);
        GetLocaleInfoEx(localeName, LOCALE_SISO3166CTRYNAME, currentIdentifier, 9);
        out.country = converter.to_bytes(currentIdentifier);
        out.fullname = out.language;
        if (out.country) {
            out.fullname = out.fullname.append("_").append(out.country);
        }
        // END MADNESS
        return out;
    }

    LMLocale getLocale(const char* hint) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        LMLocale out = getLocale(converter.from_bytes(hint).c_str());
        return out;
    }

    LMLocale getLocale() {
        std::unique_ptr<WCHAR[]> hint(new WCHAR[LOCALE_NAME_MAX_LENGTH]);
        GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, hint.get(), LOCALE_NAME_MAX_LENGTH);
        LMLocale out = getLocale(hint.get());
        return out;
    }
    #else
    LMLocale getLocale(std::string hint) {
        LMLocale out;

        // Don't bother
        if (hint == "C") { return out; }

        // Cleanup the provided locale
        size_t index;
        if ((index = hint.find_first_of(".")) != std::string::npos) {
            hint.erase(hint.begin() + index, hint.end());
        }
        out.fullname = hint;

        // and now split
        if ((index = hint.find_first_of("_")) != std::string::npos) {
            out.language.assign(hint.begin(), hint.begin() + index);
            out.country.assign(hint.begin() + index + 1, hint.end());
        } else {
            out.language = hint;
        }

        return out;
    }

    LMLocale getLocale(const char* hint) {
        return getLocale(std::string(hint));
    }

    LMLocale getLocale() {
        return getLocale(setlocale(LC_CTYPE, 0));
    }
    #endif

    void setToDefault() {
        LMLocale locale = getLocale();

        if (localesMap.find(locale.fullname) != localesMap.end()) {
            changeLocale(locale.fullname);
        } else {
            changeLocale(locale.language);
        }
    }
}