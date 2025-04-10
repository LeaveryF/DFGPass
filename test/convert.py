from graphviz import Source

# Source.from_file('HLS_output/dot/icrc1/HLS_STGraph.dot').view()
# Source.from_file('HLS_output/dot/icrc1/HLS_STGraph.dot').render("output_graph", format="svg", cleanup=True)

import os
import sys

def convert_dot_to_svg(directory):
  """
  递归遍历目录，将所有 .dot 文件转换为同名的 .svg 文件
  :param directory: 目标目录路径
  """
  for root, dirs, files in os.walk(directory):
    for file in files:
      if file.endswith(".dot"):
        dot_path = os.path.join(root, file)  # 获取 .dot 文件的完整路径
        svg_path = os.path.splitext(dot_path)[0] + ".svg"  # 生成对应的 .svg 文件路径

        print(f"Converting {dot_path} to {svg_path}...")

        # 使用 graphviz 渲染 .dot 文件为 .svg
        try:
          with open(dot_path, "r", encoding="utf-8") as f:
            dot_content = f.read()
          graph = Source(dot_content)
          graph.render(filename=os.path.splitext(svg_path)[0], format="svg", cleanup=True)
        except Exception as e:
          print(f"Failed to convert {dot_path}: {e}")

def main():
  target_directory = "./"
  if len(sys.argv) > 2:
    print("Usage: python3 convert.py <target-dir>")
    sys.exit(1)
  elif len(sys.argv) == 2:
    target_directory = sys.argv[1]

  convert_dot_to_svg(target_directory)

if __name__ == "__main__":
  main()
