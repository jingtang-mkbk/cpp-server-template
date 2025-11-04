#include "test_handlers.h"

// 处理 /api/test 请求
void handle_test([[maybe_unused]] const httplib::Request &req,
                 httplib::Response &res) {
  const std::string body = "{\"data\":\"success\"}";
  res.set_content(body, "application/json; charset=utf-8");
}
