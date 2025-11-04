#ifndef TEST_HANDLERS_H
#define TEST_HANDLERS_H

#include <httplib.h>

// 测试接口处理函数

// 处理 /api/test 请求
void handle_test(const httplib::Request &req, httplib::Response &res);

#endif // TEST_HANDLERS_H
