import fasttext
import jieba

classify = fasttext.load_model("model/classify.model")

input="向右转弯"
text = jieba.cut_for_search(input)
text = " ".join(text)

result = classify.predict(text)

print("预测词：" + input + "\n")
print("预测结果：")
label, _ = result  # 解包元组，_ 用来忽略概率数组
cleaned_label = label[0][len('__label__'):]  # 切片去掉前缀
print(cleaned_label)
