#! python
# -*- coding: utf-8 -*-
import random

def draw():
    a = random.randint(1, 17721088)
    if a == 1:
        return 1, 5000000
    elif a<=16:
        return 2, 100000
    elif a<=178:
        return 3, 3000
    elif a<=7873:
        return 4, 200
    elif a<=145348:
        return 5, 10
    elif a<=1188988:
        return 6, 5
    else:
        return 0, 0

A_balance = 64000*10000*10000
B_balance = 4000*10000*10000
step = 1
for i in range(1, 100000):
    if A_balance - step*2 < 0:
        print("小明只剩%d元，买不起%d注彩票，小明孤注一掷，把所有的钱买了%d注彩票"%(A_balance, step, A_balance//2))
        step = A_balance//2
        A_balance = 0
        B_balance += step * 2
    else:
        A_balance -= step * 2
        B_balance += step * 2
    
    prize, amount = draw()
    if amount != 0:
        print("第%d天，小明买了%d注彩票，中了%d等奖。小明赢了%d元"%(i, step, prize, step * amount))
        if B_balance - step * amount < 0:
            print("中福彩只有%d元，无法兑换奖金，中福彩破产了"%(B_balance))
            break
        else:
            A_balance += step * amount
            B_balance -= step * amount
            step = 1
    else:
        print("第%d天，小明买了%d注彩票，没有中奖，因此亏了%d元"%(i, step, step*2))
        step *= 2
    if A_balance == 0:
        print("小明破产了")
        break

