#! python
# -*- coding: utf-8 -*-
import random

class SSQ:
    red_full = list(range(1,34))
    blue_full = list(range(1, 17))
    
    def __init__(self):
        self.round = 0
        self.gain = 0
        self.history = {0:0,5:0,10:0,200:0,3000:0,140000:0,7000000:0}
        
    def choice(self):
        a = sorted(random.sample(SSQ.red_full, 6))
        a.append(random.choice(SSQ.blue_full))
        return a
    
    def compare(self, a, b):
        c = len(set(a[0:6]) & set(b[0:6]))
        if a[6] == b[6]:
            if c == 6:
                return 7000000
            elif c == 5:
                return 3000
            elif c == 4:
                return 200
            elif c == 3:
                return 10
            else:
                return 5
        elif c == 6:
            return 140000
        elif c == 5:
            return 200
        elif c == 4:
            return 10
        else:
            return 0
    
    def draw(self):
        a = random.randint(1, 17721088)
        if a == 1:
            return 7000000
        elif a<=16:
            return 140000
        elif a<=178:
            return 3000
        elif a<=7873:
            return 200
        elif a<=145348:
            return 10
        elif a<=1188988:
            return 5
        else:
            return 0
    
    def simulate10days(self):
        print("小明开始每天买一张彩票，机选号码")
        for i in range(10):
            a = ssq.choice()
            b = ssq.choice()
            c = ssq.compare(a, b)
            self.history[c] += 1
            self.round += 1
            self.gain += c
            print("第%d天，小明买了"%(self.round),a,"开奖结果",b,"小明得到%d元"%(c))
        balance = self.gain - self.round*2
        if balance>0:
            print("小明这十天总共赚了%d元"%(balance))
        else:
            print("小明这十天总共亏了%d元"%(-balance))
                    
    def simulate1year(self):
        print("小明坚持每天都买一张彩票")
        for i in range(355):
            c = self.draw()
            if c>0:
                print("第%d天，小明中奖了，得到 %d元"%(self.round,c))
            self.history[c] += 1
            self.round += 1
            self.gain += c
        balance = self.gain - self.round*2
        if balance>0:
            print("小明这一年总共赚了%d元"%(balance))
        else:
            print("小明这一年总共亏了%d元"%(-balance))
    
    def simulate59year(self):
        for i in range(365*59):
            c = self.draw()
            self.history[c] += 1
            self.round += 1
            self.gain += c
        print("60年过去了，他总共买了%d张彩票"%(self.round))
        print("他一共中了%d次一等奖，%d次二等奖，%d次三等奖，%d次四等奖，%d次五等奖，%d次六等奖"%(
                self.history[7000000], self.history[140000], self.history[3000], self.history[200], self.history[10], self.history[5]))
        balance = self.gain - self.round*2
        if balance>0:
            print("小明这辈子总共赚了%d元"%(balance))
        else:
            print("小明这辈子总共亏了%d元"%(-balance))
    
    def simulate2life(self):
        print("-----------------")
        print("小明在他死的时候意外穿越了，回到过去。他决定这辈子还是每天买一次彩票")
        self.round = 0
        self.gain = 0
        self.history = {0:0,5:0,10:0,200:0,3000:0,140000:0,7000000:0}
        for i in range(365*60):
            c = self.draw()
            self.history[c] += 1
            self.round += 1*1
            self.gain += c*1
        print("60年又过去了，他总共买了%d张彩票"%(self.round))
        print("他一共中了%d次一等奖，%d次二等奖，%d次三等奖，%d次四等奖，%d次五等奖，%d次六等奖"%(
                self.history[7000000], self.history[140000], self.history[3000], self.history[200], self.history[10], self.history[5]))
        balance = self.gain - self.round*2
        if balance>0:
            print("小明这辈子总共赚了%d元"%(balance))
        else:
            print("小明这辈子总共亏了%d元"%(-balance))            
        
    def simulatenthlife(self):
        print("-----------------")
        print("小明每次死的时候都能意外穿越。他决定继续买")
        for r in range(3,101):
            for i in range(365*60):
                c = self.draw()
                if c == 7000000:
                    break
            if c == 7000000:
                print("终于在第%d次穿越的时候中了一等奖，恭喜小明！"%(r))
                return
        print("6000年过去了，他轮回了100次，但是一次一等奖也没有中过。小明厌倦了这个游戏，不想玩了")
        
ssq = SSQ()
ssq.simulate10days()
ssq.simulate1year()
ssq.simulate59year()
ssq.simulate2life()
ssq.simulatenthlife()