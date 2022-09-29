# Quaternion 四元数
$$
q=(w,\vec{v})=w+xi+yj+zk
$$
- 加减法规则和向量相同，分量相加即可。
- 乘法
设q1,q2两个四元数如下
$$
q_1=w_1+x_1i+y_1j+z_1k\\
q_2=w_2+x_2i+y_2j+z_2k
$$
按照多项式乘法直接计算即可得到
$$
q_1\times q_2=w_1w_2-\vec{v}_1\cdot\vec{v}_2+w_1\vec{v}_2+w_2\vec{v}_1+\vec{v}_1\times\vec{v}_2\\
=(w_1w_2-\vec{v}_1\cdot\vec{v}_2,w_1\vec{v}_2+w_2\vec{v}_1+\vec{v}_1\times\vec{v}_2)
$$


