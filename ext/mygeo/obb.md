# OBB generation via Principal Component Analysis

图形引擎中，包围体可被用于view frustum culling，节省渲染时间。  
一般使用OBB作为包围盒，如何生成呢？
采用统计学和数值线性代数方法来解决该问题:

## 主成分分析 Principal Component Analysis
从线性系统中提取数据时，有三种confusion的来源：
1. 噪声，来源于测量中的不准确
2. 旋转，测量的变量可能是研究的变量的组合(最好是线性)
3. 冗余，测量的变量存在线性相关
PCA是一种使用正交变换(rotation)将一组可能相关的变量observations(如点云)转化为一些线性无关的变量(主成分)。它实现了三件事:
1. 隔绝了噪声
2. 消灭了旋转
3. 分离了冗余自由度

统计学中，两个变量的协变量表示了线性相关度，越小越无关
$$
cov(X_i,X_j)=E[(X_i-\mu_i)(X_j-\mu_j)]
$$


## dito算法， 比PCA快
1. 根据经验选取一些轴，根据这些轴得到一系列极限点对，可选取7个常用轴
2. 选取其中相距最远的点对，构成第一条边
3. 选取距离该条边所在直线最远的点，构成主三角形
4. 对主三角形的每条边，取该边，三角形面法线和二者的叉乘作为obb三轴，计算表面积
5. 取距离主三角形所在平面最远的两个点(上下)，又构成了6个三角形，对它们进行和主三角形相同的操作
6. 在所有7个三角形，共21种obb中选取表面积最小的OBB




 