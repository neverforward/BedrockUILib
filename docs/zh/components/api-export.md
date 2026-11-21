# API 导出

库是带 C++ 导出 API 的 DLL：所有公共类和函数都带 `BUL_API`（`bul/Export.h`），构建时在 dll 旁边生成导入库。使用方模组需要：

- 把本仓库的 `src/` 和 `src-client/` 加进 include 路径；
- 链接 `BedrockUILib.lib`；
- 用相同的编译器工具链和 LeviLamina SDK 版本构建——C++ ABI 要跨 dll 边界。
