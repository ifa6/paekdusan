#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <thread>

#include "ThreadPoolServer.hpp"
#include "CGIRequestHandler.hpp"

using namespace std;
using namespace Paekdusan;

int main() {
    CGIRequestHandler requestHandler;
    ThreadPoolServer threadPoolServer(4, 128, 128, 12306, requestHandler);
    threadPoolServer.start();
    return 0;
}

