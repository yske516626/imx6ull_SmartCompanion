import jieba

inFile = 'data/data.txt'
outFile = 'data/cutdata.txt'

# 使用 with 语句自动管理文件资源，避免手动关闭文件
with open(inFile, 'r', encoding='utf-8') as f, open(outFile, 'w', encoding='utf-8') as writer:
    for line in f:
        # 去除行末的换行符，并根据空白字符分割行内容
        splitor = line.strip().split(maxsplit=1)  # 只分割一次，确保保留标签部分
        if len(splitor) != 2:
            print(f"Warning: line format error - {line}")
            continue

        text, label = splitor
        # 使用 jieba 进行分词
        seg_list = jieba.cut_for_search(text)
        segmented_text = " ".join(seg_list)

        # 写入处理后的文本和标签到输出文件
        writer.write(f"{segmented_text} __label__{label}\n")
