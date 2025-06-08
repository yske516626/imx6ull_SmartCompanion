import fasttext
import jieba

classifier = fasttext.train_supervised("data/cutdata.txt",epoch=10,ws=2 , lr=0.5, wordNgrams=1, dim=100,label=u"__label__",loss=u'softmax')

classifier.save_model("model/classify.model")


input="谢谢，再见。"
text = jieba.cut_for_search(input)
text = " ".join(text)
result = classifier.predict(text)
print("预测词：" + input + "\n")
print("预测结果：")
print( result)
