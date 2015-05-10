#ifndef PAEKDUSAN_HTTP_REQUEST_H
#define PAEKDUSAN_HTTP_REQUEST_H

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <unordered_map>

#include "Logger.hpp"
#include "StrTools.hpp"

namespace Paekdusan {
    using namespace std;

    class HttpRequest {
    public:
        HttpRequest() : _readRequestLineFinished(false), _readHeaderFinished(false), _readBodyFinished(false), _contentLength(0) {}
        
        int parse(const string& rawData) {
            size_t parsedLength = 0;
            size_t start = 0, pos = 0;

            if (!_readRequestLineFinished) {
                pos = rawData.find("\r\n", start);
                if (pos == string::npos) return parsedLength;

                if (!_parseAndSetRequestLine(rawData.substr(start, pos - start))) return -1;
                
                parsedLength += (pos - start + 2);
                _readRequestLineFinished = true;
                start = pos + 2;
            }

            if (!_readHeaderFinished) {
                pos = rawData.find("\r\n", start);
                string headerLine = rawData.substr(start, pos - start);

                while (pos != string::npos && !headerLine.empty()) {
                    parsedLength += (pos - start + 2);

                    size_t colonPos = headerLine.find(':');
                    if (colonPos == string::npos) {
                        LogError("parse header line failed: %s", headerLine.c_str());
                        return -1;
                    }

                    _header[upper(headerLine.substr(0, colonPos))] = headerLine.substr(colonPos);
                    start = pos + 2;
                    pos = rawData.find("\r\n", start);
                    headerLine = rawData.substr(start, pos - start);
                }

                if (pos == string::npos) return parsedLength;

                start = pos + 2;
                parsedLength += 2;
                _readHeaderFinished = true;
                if (existKeyInHeader("CONTENT-LENGTH")) _contentLength = atoi(_header["CONTENT-LENGTH"].c_str());

                string transferCoding;
                if (getHeaderByKey("TRANSFER-CODING", transferCoding) && transferCoding == "chunked") {
                    LogError("can't process chunked request");
                    return -1;
                }
            }

            if (!_readBodyFinished) {
                if (isGET() || _contentLength == 0 && existKeyInHeader("CONTENT-LENGTH")) {
                    _readBodyFinished = true;
                    return parsedLength;
                }

                size_t bodyRemain = min(_contentLength - _body.length(), rawData.length() - start);
                _body.append(rawData, start, bodyRemain);
                parsedLength += bodyRemain;

                if (_contentLength == _body.length()) _readRequestLineFinished = true;
            }
            return parsedLength;
        }

        bool isGET() const {
            return _readRequestLineFinished && _method == "GET";
        }

        bool isPOST() const {
            return _readRequestLineFinished && _method == "POST";
        }

        bool readRequestLineFinished() const {
            return _readRequestLineFinished;
        }

        bool readHeaderFinished() const {
            return _readHeaderFinished;
        }

        bool readBodyFinished() const {
            return _readBodyFinished;
        }

        bool existKeyInHeader(const string& key) const {
            return _header.find(key) == _header.end();
        }

        bool getHeaderByKey(const string& key, string& val) const {
            auto itr = _header.find(key);
            if (itr == _header.end()) return false;

            val = itr->second;
            return true;
        }

    private:
        bool _parseAndSetRequestLine(const string& requestLine) {
            if (requestLine.empty()) {
                LogError("parse an empty requestLine");
                return false;
            }

            vector<string> buff = splitBySingle(requestLine, " ");
            if (buff.size() != 3) {
                LogError("parse first line failed: %s", requestLine.c_str());
                return false;
            }

            _method = upper(buff[0]);
            _URI = URLDecode(buff[1]);
            _version = buff[2];

            return true;
        }

        bool _readRequestLineFinished;
        bool _readHeaderFinished;
        bool _readBodyFinished;

        string _method;
        string _URI;
        string _version;

        unordered_map<string, string> _header;
        size_t _contentLength;

        string _body;
    };
}

#endif