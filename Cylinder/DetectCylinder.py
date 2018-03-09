"""本代码用于检测图片中的圆柱"""
import numpy as np
import matplotlib.pyplot as plt

#获取图片中圆柱体的中线
def getMid(Data):
    """Data是图片的灰度图"""
    data = Data
    row, col = data.shape
    x =np.arange(0,col,1)
    y = np.zeros(col)
    g = np.zeros(col)
    z = np.zeros(col)
    # 行采样间隔
    div = 10
    # 对采样得到的行数据做平均滤波的窗口值
    win = 5

    begin=int(row*0.1)
    end = int(row*0.9)

    for i in range(begin,end,div):
        y=data[i]

        smoothing(y,5)

        g=getGradient(y)

        smoothing(g,5)

        for j in range(col - 1):
            if (g[j] > 0 and g[j + 1] <= 0):
                z[j] += 1
            # if (g[j] < 0 and g[j + 1] >= 0):
            #     z[j] += 1
        plt.plot(x,y,label='g{0}'.format(i))
        # plt.plot(x,y,label='y{0}'.format(i))
    plt.legend()
    plt.show()

    # 统计平均的窗口,必须是偶数
    smoothing(z,2)
    plt.plot(x,z)
    plt.show()
    mid = np.where(z == np.max(z))[0][0]

    return mid

#获取圆柱中的盘面
def getPanel(Data):
    """Data是图片中圆柱中线所在列的灰度数据，是一个一维数组"""
    row=Data.shape[0]
    x = np.arange(0, row, 1)
    y = Data
    g = np.zeros(row)
    plt.plot(x,y)
    plt.show()
    for i in range(row):
        if i == 0:
            g[i] = y[i + 1] - y[i]
        elif i == row - 1:
            g[i] = y[i] - y[i - 1]
        else:
            g[i] = (y[i + 1] - y[i - 1]) / 2
        g[i] = np.abs(g[i])

    # 统计平均所用的窗口大小
    win = 5
    for i in range(row):
        #不推荐滤波，损失了太多信息
        if i < win:
            g[i] = np.mean(g[0:i +win+ 1])
        elif i>row-win-1:
            g[i]=np.mean(g[i-win:row])
        else:
            g[i] = np.mean(g[i - win:i + win+1])

    z = np.zeros(row)
    # 梯度门限值，高于此值认为出现大的明暗变化。
    threshold = 10
    for i in range(1, row):
        if g[i] == threshold:
            if g[i - 1] < threshold:
                z[i] = 1
            elif g[i - 1] > threshold:
                z[i] = -1
        elif (g[i] - threshold) * (g[i - 1] - threshold) < 0:
            if g[i] < g[i - 1]:
                z[i] = -1
            elif g[i] > g[i - 1]:
                z[i] = 1
    # plt.plot(x, z)
    # plt.show()
    peakIndex=[]
    isUP=False
    for i in range(row):
        if z[i]==1:
            begin=i
            isUP=True
        elif z[i]==-1:
            end=i
            if isUP:
                peak=np.where(np.max(g[begin:end])==g[begin:end])[0][0]
                peak+=begin
                peakIndex.append(peak)
            isUP=False

    candidates=[]
    for peak in peakIndex:
        candidate=np.zeros(5)
        candidate[2]=peak
        for j in range(peak,-1,-1):
            if j==peak and z[j]==-1:
                continue
            if z[j]==1:
                candidate[1]=j
            elif z[j]==-1:
                candidate[0]=j
                break

        for j in range(peak,row):
            if j==peak and z[j]==1:
                continue
            if z[j]==-1:
                candidate[3]=j
            elif z[j]==1:
                candidate[4]=j
                break
            elif j==row-1:
                candidate[4]=j
        candidates.append(candidate)

    #第一轮筛选，删除比例不正常的区域
    cylinders=[]
    for candidate in candidates:
        L1=candidate[1]-candidate[0]
        L2=candidate[3]-candidate[1]
        L3=candidate[4]-candidate[3]
        if L3>L1  and L1>L2 and L3>L2:
            cylinders.append(candidate)

    #第二轮筛选，选择区域占比最大的区域
    cylinder=np.zeros(5)+int(row/2)
    if len(cylinders)==0:
        return cylinder
    else:
        length=np.zeros(len(cylinders))
        for i in range(len(cylinders)):
            length[i]=cylinders[i][4]-cylinders[i][0]
        index=np.where(np.max(length)==length)[0][0]
        cylinder[:]=cylinders[index][:]
        return cylinder


    # x1=np.zeros(5)
    # for candidate in cylinders:
    #     plt.scatter(candidate, x1)
    #     x1+=5
    #     print(candidate)


    # plt.plot(x,g)
    # x1=np.zeros(len(peakIndex))+10
    # plt.scatter(peakIndex,x1)
    #
    #
    # plt.show()

#精细调整
def fineTune(data,row,col):
    row1=int((row[2]-row[0])/2)
    row2=int((row[4]-row[2])/2)
    y1=data[row1,:]
    y2=data[row2,:]
    a,b=data.shape
    #滤波窗口
    win=2
    for i in range(b):
        if i<win:
            y1[i]=np.mean(y1[0:i+win+1])
            y2[i]=np.mean(y2[0:i+win+1])
        elif i>b-1-win:
            y1[i]=np.mean(y1[i-win:b])
            y2[i]=np.mean(y2[i-win:b])
        else:
            y1[i] = np.mean(y1[i - win:i+win+1])
            y2[i] = np.mean(y2[i - win:i+win+1])
    g1=np.zeros(b)
    g2=np.zeros(b)
    for i in range(b):
        if i==0:
            g1[i]=y1[i+1]-y1[i]
            g2[i]=y2[i+2]-y2[i]
        elif i==b-1:
            g1[i]=y1[i]-y1[i-1]
            g2[i]=y2[i]-y2[i-1]
        else:
            g1[i]=(y1[i+1]-y1[i-1])/2
            g2[i]=(y2[i+1]-y2[i-1])/2
    #统计梯度曲线的过零点
    z1=np.zeros(b)
    z2=np.zeros(b)
    for i in range(1,b):
        if g1[i]==0:
            if g1[i-1]<0:
                z1[i]=1
            elif g1[i-1]>0:
                z1[i]=-1
        elif g1[i-1]*g1[i]<0:
            if g1[i-1]>g1[i]:
                z1[i]=-1
            else:
                z1[i]=1

        if g2[i] == 0:
            if g2[i - 1] < 0:
                z2[i] = 1
            elif g2[i - 1] > 0:
                z2[i] = -1
        elif g2[i - 1] * g2[i] < 0:
            if g2[i - 1] > g2[i]:
                z2[i] = -1
            else:
                z2[i] = 1

    #统计距离col最近的左右两侧各两个过零点
    candidate1=np.zeros(5)
    candidate2=np.zeros(5)
    # for i in range(col,b):




    x=np.arange(0,b,1)
    plt.plot(x,g1,label='g1')
    plt.plot(x,g2,label='g2')
    plt.plot(x,y1,label='y1')
    plt.plot(x,y2,label='y2')
    plt.scatter(col,10)
    plt.legend()
    plt.show()

def getZeros(data,win,point=None):
    """统计过零点，"""
def smoothing(data,win):

    len=data.shape[0]
    dat = np.zeros(len)
    dat[:]=data[:]
    if len<=win:
        return
    for i in range(len):
        if i < win:
            dat[i] = np.mean(data[0:i +win+ 1])
        elif i>len-win-1:
            dat[i]=np.mean(data[i-win:len])
        else:
            dat[i] = np.mean(data[i - win:i + win+1])
    data[:]=dat[:]
def getGradient(data):
    len=data.shape[0]
    g=np.zeros(len)
    for j in range(len):
        if j == 0:
            g[j] = data[j + 1] - data[j]
        elif j == len - 1:
            g[j] = data[j] - data[j - 1]
        else:
            g[j] = (data[j + 1] - data[j - 1]) / 2
    return g


if __name__=='__main__':
    data=np.loadtxt("F:/TestData/img.txt")
    # data[:,0:100]=0
    col=getMid(data)
    midData=data[:,col]
    row=getPanel(midData)
    print(row,col)
    data[int(row[2]),:]=0
    data[:,col]=0
    plt.imshow(data)
    plt.show()

    # fineTune(data,row,col)


