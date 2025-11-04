#include "file_routes.h"
#include "file_handlers.h"

// 配置文件操作相关路由
void configure_file_routes(httplib::Server &server) {
  // 设置文件上传大小限制
  server.set_payload_max_length(MAX_FILE_SIZE);

  server.Get("/api/file-get", handle_file_get);
  server.Get("/api/file-list", handle_file_list);
  server.Get("/api/file-preview", handle_file_preview);
  server.Post("/api/file-upload", handle_file_upload);
  server.Delete("/api/file-delete", handle_file_delete_by_code);
}
