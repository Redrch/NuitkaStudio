# coding: utf-8

import xml.etree.ElementTree as ET
from deep_translator import GoogleTranslator
import time
import os

def translate_with_retry(translator, text, retries=3):
    for i in range(retries):
        try:
            return translator.translate(text)
        except Exception as e:
            wait_time = (i + 1) * 2
            print(f"遇到网络抖动，等待 {wait_time} 秒后重试...")
            time.sleep(wait_time)
    return None

def translate_ts_file(input_file, output_file):
    translator = GoogleTranslator(source='zh-CN', target='en')

    if not os.path.exists(input_file):
        print(f"错误: 找不到文件 {input_file}")
        return

    # 解析 XML
    tree = ET.parse(input_file)
    root = tree.getroot()

    print(f"{input_file} -> {output_file}")

    messages = root.findall(".//message")
    total = len(messages)
    success_count = 0
    fail_count = 0

    for index, message in enumerate(messages):
        source_node = message.find("source")
        translation_node = message.find("translation")

        if source_node is not None and source_node.text:
            original_text = source_node.text

            # 断点续传
            if translation_node is not None and translation_node.text and len(translation_node.text.strip()) > 0:
                if "That’s an error" not in translation_node.text:
                    continue

            try:
                # 执行翻译
                translated_text = translate_with_retry(translator, original_text)

                if translation_node is not None:
                    translation_node.text = translated_text
                    # 移除 unfinished 标签
                    if 'type' in translation_node.attrib:
                        del translation_node.attrib['type']

                success_count += 1
                print(f"[{index+1}/{total}] 成功: {original_text} -> {translated_text}")

                time.sleep(0.5)

            except Exception as e:
                print(f"[{index+1}/{total}] 失败: {original_text}, 错误: {e}")
                fail_count += 1
                tree.write(output_file, encoding='utf-8', xml_declaration=True)
                print("...已保存当前进度到文件...")
                time.sleep(2)

    # 最终保存
    tree.write(output_file, encoding='utf-8', xml_declaration=True)
    print(f"\n✅ 处理完成！本次新翻译了 {success_count} 条，结果保存至: {output_file}")
    print(f"本次翻译成功 {success_count} 条，失败 {fail_count} 条，总计 {total} 条")

if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    # 生成TS文件
    os.system("lupdate src -ts lang/zh_CN.ts")
    os.system("lupdate src -ts lang/en_US.ts")

    input_filename = "../lang/zh_CN.ts"
    output_filename = "../lang/en_US.ts"
    translate_ts_file(input_filename, output_filename)

    # 生成QM文件
    os.system("lrelease lang/zh_CN.ts -qm lang/zh_CN.qm")
    os.system("lrelease lang/en_US.ts -qm lang/en_US.qm")
