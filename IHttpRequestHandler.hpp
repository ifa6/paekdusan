#ifndef PAEKDUSAN_I_HTTP_REQUEST_HANDLER_H 
#define PAEKDUSAN_I_HTTP_REQUEST_HANDLER_H

#include "HttpRequest.hpp"
#include "Utils/StrUtils.hpp"

namespace Paekdusan {
    class IHttpRequestHandler {
    public:
        virtual string handle(const HttpRequest& httpRequest) const = 0;
        virtual ~IHttpRequestHandler() {};
    };

    const char* const HTTP_RESPONSE = "HTTP/1.1 %d %s\r\nConnection: close\r\nServer: Paekdusan/1.0.0\r\nContent-Type: %s; charset=UTF-8\r\nContent-Length: %d\r\n\r\n%s";
    
    enum STATUS_CODE {
        OK = 200,
        NOT_FOUND = 404,
        METHOD_NOT_IMPLEMENTED = 501,
        BAD_REQUEST = 400,
    };

    class ResponseFactory {
    public:
        static string makeResponse(int statusCode, const string& contentType = "text/html", const string& content = "") {
            string codeDescription;
            if (statusCode == OK) {
                return stringFormat(HTTP_RESPONSE, OK, "OK", contentType.c_str(), content.length(), content.c_str());
            } else if (statusCode == NOT_FOUND) {
                return notFound();
            } else if (statusCode == METHOD_NOT_IMPLEMENTED) {
                return unimplemented();
            } else if (statusCode == BAD_REQUEST) {
                return badRequest();
            }

            LogError("Unkown status code: %d", statusCode);
            return unimplemented();
        }

    private:
        static string badRequest() {
            string content = "<HTML><TITLE>Bad Request</TITLE>\n<BODY><P>Bad Request</P>\n</BODY></HTML>\n";
            return stringFormat(HTTP_RESPONSE, 400, "BAD REQUEST", "text/html", content.length(), content.c_str());
        }

        static string notFound() {
            string content = "<HTML><TITLE>Not Found</TITLE>\n<BODY><P>NOT FOUND</P>\n</BODY></HTML>\n";
            return stringFormat(HTTP_RESPONSE, 404, "NOT FOUND", "text/html", content.length(), content.c_str());
        }

        static string unimplemented() {
            string content = "<HTML><TITLE>Method Not Implemented</TITLE>\n<BODY><P>Method Not Implemented</P>\n</BODY></HTML>\n";
            return stringFormat(HTTP_RESPONSE, 501, "METHOD NOT IMPLEMENTED", "text/html", content.length(), content.c_str());
        }
    };
}

#endif