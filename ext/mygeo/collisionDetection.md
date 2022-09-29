## 质点角动量定理
$$
\vec{\tau}=\vec{r}\times \vec{f}=\frac{d\vec{L}}{dt}=\frac{d(\vec{r}\times\vec{P})}{dt}
$$
Proof:
$$
\frac{d\vec{L}}{dt}=\frac{d\vec{r}}{dt}\times\vec{P}+\vec{r}\times\frac{d\vec{P}}{dt}=\vec{r}\times\frac{d\vec{P}}{dt}=\vec{r}\times\vec{f}
$$

## 质点系角动量定理
$$
\vec{\tau}=\sum\tau_i=\sum \vec{r}_i\times\vec{f_i}=\sum\frac{d\vec{L}_i}{dt}
$$
Proof:
每个质点受到两部分力矩，内力矩和外力矩，其中内力矩成对出现，每对力大小相同方向相反，而对应的r和r'在质心到力线上的投影大小相同，因此力矩相互抵消，所以内力局之和为0，得证。

## 刚体角动量定理与惯性张量
Orientation Matrix 朝向矩阵 R(t)
$$
\dot{\vec{r}}(t)=\vec{\omega}(t)\times \vec{r}(t)\\
\vec{r}(t)=R(t)r_0
$$
代入R(t),有

$$
\frac{dR(t)}{dt}\vec{r}=\vec{\omega}(t)\times(R(t)\vec{r})
$$
简写：
$$
\dot{R}(t) \vec{r}=\vec{\omega}(t)\times(R(t)\vec{r})
$$
矩阵乘以向量的一种解读是矩阵的每一列乘以向量对应分量，然后相加得到结果向量，也就是说以向量分量作为系数对矩阵的列做了一个线性组合。  
因此我们有
$$
\dot{R}(t).col[i]=\vec{\omega}(t)\times R(t).col[i]
$$
即，朝向矩阵导数(矩阵)的第i列向量=角速度叉乘朝向矩阵第i列向量。  
上述表示方法比较啰嗦，为了简化使用下列trick:  
$$
\vec{a}\times\vec{b}=
\begin{pmatrix}
a_yb_z-a_zb_y\\
a_zb_x-a_xb_z\\ 
a_xb_y-a_yb_x
\end{pmatrix}=
\begin{pmatrix}
0 & -a_z & a_y \\
a_z & 0 & -a_x \\
-a_y & a_x & 0 
\end{pmatrix} 
\begin{pmatrix}
b_x\\
b_y\\
b_z
\end{pmatrix} 
$$
定义上述乘积中的矩阵为a*，则有
$$
a^*b=a\times b
$$
那么上面的朝向矩阵变化率则为
$$
\dot{R}(t)=\omega(t)^*R(t)
$$
这样便将叉乘也转化为了矩阵乘法！  
  
考虑一般情况，质心系有可能是非惯性系，推导刚体的角动量定理： 
根据牛二定理
$$
\dot{p}=m(\ddot{r}+\ddot{r_c})=f
$$
其中r是质心相对于某惯性系的位置，r_c为某点相对于质心系的位置。  
两边同乘：
$$
(r+r_c)^*f=(r+r_c)^*\dot{p}
$$
对刚体所有点求和:
$$
\sum (r_i+r_c)^*f_i=\sum(r_i+r_c)^*\dot{p_i}
$$
由于r_c可以提出求和号外，又fi=\dot{p_i}，故
$$
\sum(r_i)^*f_i=\sum(r_i)^*\dot{p}_i
$$
等式右边通项为(先不写m_i)
$$
r_i^*\ddot{r}_i=-r_i^*r_i^*\dot{\omega}-r_i^*\dot{r_i}^*\omega
$$
右边第一项
$$
-r_i^*r_i^*\dot{\omega}=(r_i^Tr_i\hat{I}-r_ir_i^T)\dot{\omega}
$$
又有
$$
\dot{r}_i^*r_i^*\omega=0
$$
给右边加上这一项不影响，可得：
$$
\dot{I}(t)\omega+I(t)\dot{\omega}=\dot{L}=\tau
$$
其中，惯性张量为
$$
I=\sum_im_i(r_i^Tr_i\hat{I}-r_ir_i^T)
$$


## 刚体模拟
### linear part
$$
f_i^{0}=f_i(r_i^{0},\dot{x}_i^{0}) \\
f^{0}=\sum f_i^{0}\\
v^{1}=v^{0}+\Delta t M^{-1}f^{0}\\
r^{1}=r^{0}+\Delta tv^{1}
$$

### angular part
$$
R^{0}=q^{0}.toMatrix()\\
\tau^{0}_i=(R^{0}r_i)\times f^{0}_i\\
\tau^{0}=\sum \tau^{0}_i\\

I^{0}=R^{0}I_{ref}(R^{0})^T\\
\omega^{1}=\omega^{0}+\Delta t(I^{0})^{-1}\tau^{0}\\
q^{1}=q^{0}+(0,\frac{\Delta t}{2}\omega^{1})\times q^{0}

$$
实际实现时，0和1步都在同一个变量中更新即可。重力不会产生力矩，因此只受重力的话，角速度不变。  
上述的角速度更新公式似乎不正确，因为角速度可能不守恒，应该更新角动量和惯性张量，然后间接计算出角速度！！！

$$
R^{0}=q^{0}.toMatrix()\\
\tau^{0}_i=(R^{0}r_i)\times f^{0}_i\\
\tau^{0}=\sum \tau^{0}_i\\

I^{0}=R^{0}I_{ref}(R^{0})^T\\
L^{1}=L^{0}+\Delta t\tau^{0}\\
\omega^{1}=inv(I^{0}) L^{1}\\
q^{1}=q^{0}+(0,\frac{\Delta t}{2}\omega^{1})\times q^{0}
$$
