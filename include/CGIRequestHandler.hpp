#ifndef PAEKDUSAN_CGI_REQUEST_HANDLER_H
#define PAEKDUSAN_CGI_REQUEST_HANDLER_H

#include <memory>
#include <cstdio>
#include <cstdlib>
#include "IHttpRequestHandler.hpp"

namespace Paekdusan {
    const string BASE_DIR = "htdocs/";
    const string DEFAULT_FILE = "index.html";
    const int READ_FILE_BUFFER_SIZE = 2048;

    class CGIRequestHandler : public IHttpRequestHandler {
    public:
        virtual string handle(const HttpRequest& httpRequest) const {
            if (httpRequest.isGET()) {
                string URI = httpRequest.getURI();
                size_t pos = URI.find('?');
                string contentType, content;
                if (pos == string::npos) {
                    if (serveFile(URI, contentType, content)) {
                        return ResponseFactory::makeResponse(OK, contentType, content);
                    } else {
                        return ResponseFactory::makeResponse(NOT_FOUND);
                    }
                }
            }

            if (httpRequest.isPOST()) {
            }

            return ResponseFactory::makeResponse(METHOD_NOT_IMPLEMENTED);
        }

    private:
        bool serveFile(const string& URI, string& contentType, string& content) const {
            if (URI.length() == 0) return false;
            
            string filename = BASE_DIR + URI;
            if (URI[URI.length() - 1] == '/') filename += DEFAULT_FILE;

            LogDebug("read file: %s", filename.c_str());

            auto deleter = [](FILE *p) {fclose(p); };
            unique_ptr<FILE, decltype(deleter)> pFile(fopen(filename.c_str(), "r"), deleter);

            if (pFile == nullptr) return false;
            
            string suffix = getFileSuffix(filename);
            if (suffix == "html") {
                contentType = "text/html";
            }

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
