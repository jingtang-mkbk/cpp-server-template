#include "routes.h"
#include <httplib.h>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

#define PORT 8080

int main() {
  // 创建服务器实例
  httplib::Server server;

  // 配置CORS支持 - 在所有响应后添加CORS头
  server.set_post_routing_handler(
      []([[maybe_unused]] const httplib::Request &req, httplib::Response &res) {
        // 允许的源（可以根据需要修改为特定域名，或使用
        // req.get_header_value("Origin")）
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods",
                       "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers",
                       "Content-Type, Authorization, X-Requested-With");
        res.set_header("Access-Control-Max-Age", "3600");
      });

  // 处理OPTIONS预检请求
  server.Options(".*", []([[maybe_unused]] const httplib::Request &req,
                          httplib::Response &res) { res.status = 200; });

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
