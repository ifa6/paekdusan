#ifndef PAEKDUSAN_STR_TOOLS_H
#define PAEKDUSAN_STR_TOOLS_H

#include <string>
#include <vector>

namespace Paekdusan {
    using namespace std;
    using BYTE = unsigned char;

    inline BYTE toHex(BYTE x) {
        return x > 9 ? x - 10 + 'A' : x + '0';
    }

    inline BYTE fromHex(BYTE x) {
        if (isdigit(x)) return x - '0';
        return isupper(x) ? x - 'A' + 10 : x - 'a' + 10;
    }

    inline string getFileSuffix(const string& filename) {
        size_t pos = filename.rfind('.');
        string suffix;

        if (pos == string::npos) {
            suffix = "";
        } else {
            suffix = filename.substr(pos+1);
        }

        return suffix;
    }

    inline string upper(const string& src) {
        string ret = src;
        for_each(ret.begin(), ret.end(), [](char c) { return toupper(c); });
        return ret;
    }

    /*return value optimization will be applied*/
    inline vector<string> splitBySingle(const string& src, const string& pattern, size_t limit = 0) {
        vector<string> res;
        if (src.empty()) return res;

        size_t start = 0;
        size_t end = 0;

        while (start < src.size() && (limit == 0 || res.size() < limit)) {
            end = src.find_first_of(pattern, start);
            if (string::npos == end) {
                res.push_back(src.substr(start));
                return res;
            }

            res.push_back(src.substr(start, end - start));

            start = end + 1;
        }
        return res;
    }

    /*return value optimization will be applied*/
    inline vector<string> splitByMulti(const string& src, const string& pattern, size_t limit = 0) {
        vector<string> res;
        if (src.empty()) return res;

        size_t start = 0;
        size_t end = 0;

        while (start < src.size() && (limit == 0 || res.size() < limit)) {
            end = src.find(pattern, start);
            if (string::npos == end) {
                res.push_back(src.substr(start));
                return res;
            }

            res.push_back(src.substr(start, end - start));

            start = end + pattern.length();
        }
        return res;
    }

    /*return value optimization will be applied*/
    inline string URLEncode(const string& urlIn) {
        string urlOut;
        for (size_t i = 0; i < urlIn.size(); i++) {
            BYTE buf[4];
            memset(buf, 0, sizeof(buf));

            if (isalnum((BYTE) urlIn[i])
                || (urlIn[i] == '-')
                || (urlIn[i] == '_')
                || (urlIn[i] == '.')
                || (urlIn[i] == '~')) {
                buf[0] = urlIn[i];
            } else if (urlIn[i] == ' ') {
                buf[0] = '+';
            } else {
                buf[0] = '%';
                buf[1] = toHex(((BYTE) urlIn[i]) >> 4);
                buf[2] = toHex(((BYTE) urlIn[i]) & 15);
            }
            urlOut += ((char*) buf);
        }
        return urlOut;
    }

    /*return value optimization will be applied*/
    inline string URLDecode(const string& urlIn) {
        string urlOut;
        for (size_t i = 0; i < urlIn.size(); i++) {
            BYTE ch = 0;

            if (urlIn[i] == '%') {
                ch = (fromHex(urlIn[i + 1]) << 4) | (fromHex(urlIn[i + 2]));
                i += 2;
            } else if (urlIn[i] == '+') {
                ch = ' ';
            } else {
                ch = urlIn[i];
            }

            urlOut += ((char) ch);
        }
        return urlOut;
    }

    inline string stringFormat(const char* fmt, ...) {
        int size = 256;
        string str;
        va_list ap;
        while (1) {
            str.resize(size);
            va_start(ap, fmt);
            int n = vsnprintf((char *) str.c_str(), size, fmt, ap);
            va_end(ap);
            if (n > -1 && n < size) {
                str.resize(n);
                return str;
            }
            if (n > -1)
                size = n + 1;
            else
                size *= 2;
        }
        return str;
    }
}


#endif