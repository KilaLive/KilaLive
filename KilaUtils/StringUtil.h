#ifndef H_STRING_UTIL_H
#define H_STRING_UTIL_H
#include <string>
#include <vector>
#include "KilaUtils.h"
class KILAUTILS_API CStringUtil final
{
public:
    //根据pattern字符对原始字符串str进行分割
    static std::vector<std::string> split(std::string str, std::string pattern);

    //移除字符串两侧的空白字符
    static std::string& trim(std::string& s);

    static std::string& replace_str(std::string& str, const std::string& to_replaced, const std::string& newchars);

    static std::string base64_decode(std::string const& encoded_string);

    static std::string base64_encode(std::string const& input_string);

    static std::string wstring2string(std::wstring wstr);

    static std::wstring MBTOW(const std::string& astr);

    static std::string utf16toAnsi(const std::wstring& utf16);

    static std::string utf16_to_utf8(const std::wstring& source);

private:
    static inline bool is_base64(unsigned char c);

private:
    static std::string base64_chars;
};

#endif