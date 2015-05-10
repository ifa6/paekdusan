#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <thread>

#include "HttpRequest.hpp"
#include "StrTools.hpp"
#include "Queue.hpp"
#include "ThreadPool.hpp"
#include "ThreadPoolServer.hpp"

using namespace std;
using namespace Paekdusan;

int main() {
    ThreadPoolServer threadPoolServer(4, 128, 128, 12306);
    threadPoolServer.start();
    return 0;
}

