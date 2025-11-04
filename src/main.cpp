#include "routes.h"
#include <httplib.h>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

#define PORT 8080

int main() {
  // 创建服务器实例
  httplib::Server server;

  // 配置所有路由
  configure_routes(server);

  // 配置 404 错误处理器（请求不存在的接口）
  server.set_error_handler([](const httplib::Request &req,
                              httplib::Response &res) {
    if (res.status == 404) {
      json error = {{"error", "API endpoint not found"},
                    {"path", req.path},
                    {"method", req.method}};
      res.set_content(error.dump(2), "application/json; charset=utf-8");
    } else if (res.status == 405) {
      json error = {{"error", "Method not allowed"},
                    {"path", req.path},
                    {"method", req.method}};
      res.set_content(error.dump(2), "application/json; charset=utf-8");
    } else {
      json error = {{"error", "Internal server error"}, {"status", res.status}};
      res.set_content(error.dump(2), "application/json; charset=utf-8");
    }
  });

  // 启动服务器
  std::cout << "HTTP server listening on http://0.0.0.0:" << PORT << std::endl;
  server.listen("0.0.0.0", PORT);

  return 0;
}
