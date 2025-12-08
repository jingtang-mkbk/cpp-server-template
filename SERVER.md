
# 创建 systemd 服务文件
sudo nano /etc/systemd/system/simple-http-server.service

[Unit]
Description=Simple HTTP Server
After=network.target
Wants=network.target

[Service]
Type=simple
User=root
WorkingDirectory=/root/Desktop/cpp-server-template/bin
ExecStart=/root/Desktop/cpp-server-template/bin/simple_http_server
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target

# 确保可执行文件有执行权限
sudo chmod +x /root/Desktop/cpp-server-template/bin/simple_http_server

# 重新加载systemd配置
sudo systemctl daemon-reload

# 启用开机自启
sudo systemctl enable simple-http-server

# 立即启动服务
sudo systemctl start simple-http-server

# 检查服务状态
sudo systemctl status simple-http-server