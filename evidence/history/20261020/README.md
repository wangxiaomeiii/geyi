# 历史记录 REQ-315 电脑端证据（2026-10-20）

这些文件由仓库内工具在开发机生成，**不是开发板照片**，不能作为板端验收证据。

生成方式（PowerShell，需 SDK 与 MSVC；本机 SDK 在 `D:\SiFli-SDK\v2.5.1`）：

```powershell
$env:SIFLI_SDK='D:\SiFli-SDK\v2.5.1'
python tools/generate_ui_font.py     # 132 汉字 / 16744 位图字节
python tools/run_host.py             # 主机单测：核心 27246、输入 60、历史 105 断言
python tools/build_desktop.py        # 桌面 LVGL 构建并跑 --test
```

| 文件 | 内容 |
|---|---|
| `desktop-test.log` | 桌面 LVGL `--test` 完整输出，含 TC-315/314/313 的 PASS 行 |
| `host_test.log` | 主机单测完整输出，含 TC-315 的 105 断言与核心/输入回归 |
| `home.png` | 首页“历史记录”已从禁用的占位改为可用按钮 |
| `initial.png` | 对局页右上角显示计时 `00:00` |
| `timer.png` | `Sleep(1100)` + `lv_timer_handler()` 后显示 `00:01`，随后认输记录的时长 ≥1000ms |
| `history.png` | 两条已结束对局，最新在前，显示结果、时长、步数 |
| `history-detail.png` | 详情页：结果、用时、步数、结束方式、种子 |
| `history-empty.png` | 清空后显示“暂无记录” |

PNG 由 `build/desktop/*.bmp`（LVGL 输出的 BGRA 帧缓冲）转换而来，只做通道转换与底色叠加，未修改像素内容。

覆盖范围仅单机同板双人；记录只存内存，重启即清空。板端固件本轮未编译、未烧录（开发机没有 arm-none-eabi-gcc），
真机列表与用时需要学生烧录后另行留证。
