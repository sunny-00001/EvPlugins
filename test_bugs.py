"""
WPSRefresh3.py 路径 BUG 深度验证
"""
import os
import urllib.parse

SEP = "=" * 60

# ------------------------------------------------------------------
print(SEP)
print("BUG-5: check_path_navigable 的空路径行为")
print(SEP)
empty_exists = os.path.exists("")
abs_empty = os.path.abspath("")
print(f"  os.path.exists('') = {empty_exists}")
print(f"  os.path.abspath('') = {repr(abs_empty)}")
if not empty_exists:
    print("  空字符串路径在 Windows 下 exists=False，安全")
else:
    print("  ⚠️ BUG确认: 空路径 exists=True，会通过导航前检查传入 Navigate2")

# ------------------------------------------------------------------
print()
print(SEP)
print("BUG-6: _location_url_to_path 四斜杠 UNC 路径处理错误")
print(SEP)
url4 = "file:////server/share"
print(f"  输入 URL: {url4!r}")
print(f"  startswith('file://///') = {url4.startswith('file://////')}")
print(f"  startswith('file:///') = {url4.startswith('file:///')}")

# 实际会落入 file:/// 分支
after_protocol = url4[7:]  # '/server/share'
local = after_protocol[1:] if after_protocol.startswith("/") else after_protocol
local = urllib.parse.unquote(local).replace("/", "\\")
print(f"  当前输出: {repr(local)}")
print(f"  正确应为: {repr(r'\\server\share')}")
is_bug = local != r"\\server\share"
print(f"  ⚠️ BUG {'确认' if is_bug else '不存在'}: file:////server/share 被误解析为 {repr(local)}")

# ------------------------------------------------------------------
print()
print(SEP)
print("BUG-3: UNC effective_len 计算偏差1字节")
print(SEP)
# 代码: effective_len = len("\\") + len(abs_path[8:])   # 只有1个反斜杠
# 正确: effective_len = len("\\\\") + len(abs_path[8:]) # 应为2个反斜杠（UNC前缀是 \\）
unc_sample = "\\\\?\\UNC\\server\\share\\" + "x" * 240
code_len   = len("\\")  + len(unc_sample[8:])  # 代码实际用的
correct_len = len("\\\\") + len(unc_sample[8:])  # 正确值
print(f"  样例 \\\\?\\UNC\\... 总长: {len(unc_sample)}")
print(f"  代码 effective_len = {code_len}  (1个\\  + rest)")
print(f"  正确 effective_len = {correct_len}  (2个\\\\ + rest)")
print(f"  差值: {correct_len - code_len} 字节 —— 在临界值260附近会导致误判")

# ------------------------------------------------------------------
print()
print(SEP)
print("BUG-7(新发现): navigate_folder_with_retry 路径比对 — \\\\?\\ 前缀未统一去除")
print(SEP)
# 场景：navigate 的目标路径 nav_path 已经过 strip_long_path_prefix，是普通路径
# norm_target = os.path.normpath(os.path.abspath(nav_path)) => 普通路径
# 但 _get_current_path 返回的 Shell.Folder.Self.Path 对超长路径可能带 \\?\ 前缀
# 代码在比对前也 strip 了:
#   bare_current = strip_long_path_prefix(current_path)
#   norm_current = os.path.normpath(os.path.abspath(bare_current))
# 逻辑上是对的，但验证一下 abspath 对 \\?\C:\foo 的行为：
weird_path = "\\\\?\\C:\\Users\\test"
absp = os.path.abspath(weird_path)
print(f"  abspath('\\\\\\\\?\\\\C:\\\\Users\\\\test') = {repr(absp)}")
# 如果 abspath 保留了 \\?\ 前缀，则 norm_current 也会含该前缀，比对会失败
has_prefix_after_abspath = absp.startswith("\\\\?\\")
print(f"  abspath 后仍含 \\\\?\\ 前缀: {has_prefix_after_abspath}")
if has_prefix_after_abspath:
    print("  ⚠️ BUG确认: Shell.Path返回 \\\\?\\C:\\... 时，abspath 不会去掉前缀，")
    print("             导致 norm_current 含前缀，而 norm_target 不含，比对永远失败！")
    print("             受影响路径: 超过259字符的任何目录导航后均无法通过验证")
else:
    print("  abspath 正确去除了前缀，无此 Bug")

# ------------------------------------------------------------------
print()
print(SEP)
print("BUG-8(新发现): check_path_navigable 双重存在性检查逻辑错误")
print(SEP)
# 代码流程（第961~987行）：
# if not path.startswith("\\\\?\\"):
#     abs_path = os.path.abspath(path)
#     ...计算 effective_len, check_path...
#     if effective_len > 260:
#         ...可能修改 check_path...
#         if not os.path.exists(check_path): return False, ...
#     else:
#         if not os.path.exists(path):        <--- 这里用 path 而非 abs_path！
#             return False, ...
# 当 path 是绝对路径时 os.path.exists(path) == os.path.exists(abs_path)，无问题
# 当 path 含尾部斜杠时可能有差异
path_with_trail = os.path.join(os.getcwd(), "test") + "\\"
path_no_trail   = os.path.join(os.getcwd(), "test")
print(f"  尾部\\路径 exists: {os.path.exists(path_with_trail)}")
print(f"  无尾部\\路径 exists: {os.path.exists(path_no_trail)}")

# else 分支用 os.path.exists(path)，path 是原始参数，没有做 abspath
# 若调用方传入 ".\test" 且工作目录正确，exists 能返回正确值，OK
# 但代码已经把 check_path 设为 abs_path，逻辑不一致: 计算用 abs_path，检查用 path
print(f"  代码问题: else分支用 os.path.exists(path) 而非 os.path.exists(check_path)")
print(f"            check_path 已设为 abs_path，检查时却没有使用，逻辑不一致")

# ------------------------------------------------------------------
print()
print(SEP)
print("BUG-9(新发现): get_all_subfolders_with_progress 深度计算在特殊路径下偏差")
print(SEP)
# current_depth = root[len(folder_path):].count(os.sep)
# 如果 folder_path 没有 normpath（但代码开头已经 normpath 了），一般无问题
# 但如果路径中含 os.sep 字符的重复（如 multi  spaces  path 中无问题，但
# 如果 folder_path 末尾有 os.sep，depth 从1开始而非0）
folder_with_sep = r"C:\Users\Administrator\Desktop\WPSRefresh\test" + "\\"
folder_no_sep   = r"C:\Users\Administrator\Desktop\WPSRefresh\test"
# 模拟 os.walk 的第一个 root
root_sample = r"C:\Users\Administrator\Desktop\WPSRefresh\test\中文路径"
depth_with_sep = root_sample[len(folder_with_sep):].count(os.sep)
depth_no_sep   = root_sample[len(folder_no_sep):].count(os.sep)
print(f"  folder_path 末尾有\\ 时深度 = {depth_with_sep}")
print(f"  folder_path 末尾无\\ 时深度 = {depth_no_sep}")
# 代码开头有 folder_path = os.path.normpath(folder_path)，normpath 会去掉尾斜杠
# 所以实际上安全，但 tkinter filedialog 在 Windows 上返回的路径带不带 / 取决于选择方式
print(f"  代码用 os.path.normpath 处理了 folder_path，此处安全")

# ------------------------------------------------------------------
print()
print(SEP)
print("总结")
print(SEP)
bugs = [
    ("BUG-3", "低",  "UNC effective_len 少算1字节，临界值259-260时误判，UNC场景极少"),
    ("BUG-6", "中",  "_location_url_to_path: file:////server/share 被误解析为相对路径而非UNC"),
    ("BUG-7", "高",  "Shell.Path返回\\\\?\\前缀路径时，abspath不去前缀，路径比对永久失败"),
    ("BUG-8", "低",  "check_path_navigable else分支用原始path而非check_path做exists检查，逻辑不一致"),
]
for bid, severity, desc in bugs:
    print(f"  [{severity}] {bid}: {desc}")
