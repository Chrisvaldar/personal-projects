from sklearn import tree

X = [
    [160, 50, 38],
    [165, 55, 39],
    [170, 60, 41],
    [175, 70, 42],
    [180, 80, 44],
    [185, 85, 45],
    [190, 90, 46],
    [195, 100, 47],
    [200, 110, 48],
    [155, 45, 37]
]

Y = [
    "female",
    "female",
    "male",
    "male",
    "male",
    "male",
    "male",
    "male",
    "male",
    "female"
]

clf = tree.DecisionTreeClassifier()

clf = clf.fit(X,Y)

prediction = clf.predict([[190,70,43]])

print(prediction)