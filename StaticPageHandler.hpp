#ifndef PAEKDUSAN_STATIC_PAGE_HANDLER_H
#define PAEKDUSAN_STATIC_PAGE_HANDLER_H

#include <memory>
#include <cstdio>
#include <cstdlib>
#include "IHttpRequestHandler.hpp"

namespace Paekdusan {
    const string BASE_DIR = "htdocs/";
    const string DEFAULT_FILE = "index.html";
    const int READ_FILE_BUFFER_SIZE = 2048;

    class StaticPageHandler : public IHttpRequestHandler {
    public:
        virtual string handle(const HttpRequest& httpRequest) const {
            if (httpRequest.isPOST()) return ResponseFactory::makeResponse(METHOD_NOT_IMPLEMENTED);
            
            string URI = httpRequest.getURI();
            size_t pos = URI.find('?');
            string contentType, content;

            if (pos != string::npos) return ResponseFactory::makeResponse(METHOD_NOT_IMPLEMENTED);

            if (serveFile(URI, contentType, content)) {
                string connection;
                if (httpRequest.getHeaderByKey("CONNECTION", connection)) connection = "keep-alive";
                return ResponseFactory::makeResponse(OK, connection.c_str(), contentType, content);
            } else {
                return ResponseFactory::makeResponse(NOT_FOUND);
            }
        }

    private:
        string getContentType(const string& suffix) const {
            if (suffix == "html" || suffix == "css") {
                return "text/" + suffix;
            } else if (suffix == "png" || suffix == "jpg" || suffix == "gif") {
                return "image/" + suffix;
            } else if (suffix == "js") {
                return "application/javascript";
            } else if (suffix == "pdf") {
                return "application/pdf";
            }
            return "text/plain";
        }

        bool serveFile(const string& URI, string& contentType, string& content) const {
            if (URI.length() == 0) return false;
            
            string filename = BASE_DIR + URI;
            if (URI[URI.length() - 1] == '/') filename += DEFAULT_FILE;

            LogDebug("read file: %s", filename.c_str());

            auto deleter = [](FILE *p) {fclose(p); };
            unique_ptr<FILE, decltype(deleter)> pFile(fopen(filename.c_str(), "r"), deleter);
            if (pFile == nullptr) return false;
            
            contentType = getContentType(getFileSuffix(filename));
            
            content.clear();
            char buff[READ_FILE_BUFFER_SIZE];

            fgets(buff, READ_FILE_BUFFER_SIZE - 1, pFile.get());
            while (! feof(pFile.get())) {
                content += buff;
                fgets(buff, READ_FILE_BUFFER_SIZE - 1, pFile.get());
            }

            return true;
        }
    };
}

#endif
