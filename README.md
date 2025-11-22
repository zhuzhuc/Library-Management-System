# 图书管理系统 / Library System

一个基于 C++17 的图书管理系统，包含命令行示例程序与 Qt6 Widgets 图形界面。核心业务位于仓库根目录的实体类（`Book`, `Library`, `Borrower`, `Student`, `Teacher` 等），通过 `FileManager` 读写 TSV 数据，亦可选配 MySQL（`src/db/DBManager.cpp`）。GUI 位于 `src/gui/`，涵盖主窗口、借阅记录面板、主题设置等。

## 目录结构
```
.
├── Book.cpp / .h 等核心业务文件
├── main.cpp           # CLI 演示入口
├── src/cli/           # CLI 控制器与推荐服务
├── src/gui/           # Qt6 GUI 组件（支持 Translator）
├── src/db/            # MySQL 支持（可选）
├── docs/resources/    # 报告、UML、导出图
├── docs/sql/          # schema.sql
├── translations/      # Qt 语言包目录
├── books.tsv          # 默认图书数据
├── users.tsv          # 默认用户数据
├── gui.sh             # GUI 构建运行脚本
├── run.sh             # CLI 编译运行脚本
└── build/             # CMake 产物（勿提交）
```

## 快速开始
```bash
# 构建并运行 GUI（默认使用 Ninja 生成器，可设置 SKIP_GUI_RUN=1 仅构建）
./gui.sh

# 手动 CMake（可增加 -D USE_MYSQL=ON 打开数据库模式）
cmake -S . -B build
cmake --build build --target library_gui library_cli

# 运行命令行示例（复用 CMake 构建的 library_cli 目标，可设 SKIP_CLI_RUN=1 跳过运行）
./run.sh

# 运行自动化测试（核心逻辑 + Qt GUI 主题）
cmake --build build --target library_core_tests library_gui_tests
(cd build && ctest)
```
若已安装 MySQL 开发头文件及库，可通过 `-D USE_MYSQL=ON` 启用数据库功能；否则使用 TSV 文件作为持久化层。

## 常见问题
- 若 CMake 报告 build 目录属于其他工程（例如曾在不同路径使用相同 build/），重新运行 `./gui.sh` 会自动清理失效的 `CMakeCache.txt` 并重新配置；必要时手动删除整个 `build/` 目录后重试。

## 功能亮点
- 管理端：CLI 控制器集中处理菜单/鉴权，支持文件或 MySQL 双持久化、批量保存/加载、推荐统计打印。
- 用户端：登录、搜索、借阅与归还、借阅历史记录；借阅成功会刷新 `BookRecommendationService` 统计。
- GUI：提供图书列表、借阅管理、主题设置等多窗口体验，并通过 `translations/` 目录的 `.qm` 文件实现 Qt 国际化。
- 数据：默认读取 `books.tsv` / `users.tsv`，设计文档/流程图移至 `docs/resources/`，数据库建模脚本在 `docs/sql/`。

## 开发与贡献
- 代码风格遵循 4 空格缩进、头源文件配对、类名 PascalCase、函数 lowerCamelCase；提交前推荐运行 `clang-format`。
- 新增功能时更新对应 TSV 或数据库迁移说明，并在 PR 中附上运行 `./gui.sh`、`./run.sh` 和 `ctest` 的结果、关键界面截图与 MySQL 启用情况。
- 语言包生成：参见 `translations/README.md`，结合 `lupdate`/`lrelease` 维护 `app_<locale>.qm`，GUI 会在启动时自动加载。
- 所有测试通过 `ctest` 管理：`library_core_tests` 覆盖 `Library`/`FileManager`，`library_gui_tests` 验证 `UiTheme` 缓存与样式字符串。
