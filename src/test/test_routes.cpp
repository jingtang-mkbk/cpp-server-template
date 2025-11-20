#include "test_routes.h"
#include <httplib.h>
#include <string>

void handle_test([[maybe_unused]] const httplib::Request &req,
                 httplib::Response &res) {
  const std::string body = "{\"data\":\"success test\"}";
  res.set_content(body, "application/json; charset=utf-8");
}

// 配置测试相关路由
void configure_test_routes(httplib::Server &server) {
  server.Get("/api/test", handle_test);
}
