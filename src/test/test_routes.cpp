#include "test_routes.h"
#include "test_handlers.h"

// 配置测试相关路由
void configure_test_routes(httplib::Server &server) {
  server.Get("/api/test", handle_test);
}
