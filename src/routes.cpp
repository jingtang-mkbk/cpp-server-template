#include "routes.h"
#include "file/file_routes.h"
#include "test/test_routes.h"

// 配置所有 API 路由
void configure_routes(httplib::Server &server) {
  // 配置测试路由
  configure_test_routes(server);

  // 配置文件操作路由
  configure_file_routes(server);
}
