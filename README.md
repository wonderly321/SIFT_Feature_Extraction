### 一 算法流程



1. 读入需要特征匹配的两张bmp图像

   本步骤主要是将格式的图片以像素和宽高等信息读入并转化为灰度图。

2. 构造尺度空间层

   本步骤主要是将原始图像通过上下采样构造成不同尺度空间下的原始图像并进一步构建高斯金字塔和DoG金字塔。

   2.1  通过采样构造图像的尺度空间的层，本项目中设置了六层不同的尺度空间大小。

   2.2  构造高斯金字塔

   ​	（1）首先采用不同尺度因子的高斯核对图像进行卷积以得到图像的不同尺度空间，将这一组图像作为金子塔图像的第一层；

   ​	（2）接着对第一层图像中的2倍尺度图像（相对于该层第一幅图像的2倍尺度）以2倍像素距离进行下采样来得到金子塔图像的第二层中的第一幅图像，对该图像采用不同尺度因子的高斯核进行卷积，以获得金字塔图像中第二层的一组图像；

   ​	（3）再以金字塔图像中第二层中的2倍尺度图像（相对于该层第一幅图像的2倍尺度）以2倍像素距离进行下采样来得到金字塔图像的第三层中的第一幅图像，对该图像采用不同尺度因子的高斯核进行卷积，以获得金字塔图像中第三层的一组图像。这样依次类推，从而获得了金字塔图像的每一层中的一组图像

   2.3  构造DoG（差分）金字塔
   	对上图得到的每一层相邻的高斯图像相减，就得到了高斯差分图像，所有的层得到的图像就构成了差分金字塔

3. 检测关键点

   为了寻找尺度空间的极值点，每一个采样点都要和它所有的26个邻域（同尺度8个，上下相邻尺度各9个）比较，看其是否比它的图像域和尺度域的相邻点都大或者都小。若该点为局部极值点，则保存为候选关键点。但是这样产生的极值点不是稳定的极值点，需要对关键点进行精确定位，去除不稳定的极值点。通过对尺度空间DoG函数进行曲线拟合，计算其极值点，从而实现关键点的精确定位。利用Hessian矩阵判断关键点是否位于边缘。

4. 提取描述符

   经过以上步骤，对于每一个关键点，拥有三个信息：位置、尺度以及方向。接下来就是为每个关键点建立描述符，使其不随各种变化而变化，比如光照变化、视角变化等等。并且描述符应该有较高的独特性，以便于提高关键点正确匹配的概率。  

   ​	（1）首先将坐标旋转为关键点的方向，以保证旋转不变形。
   
   ​	（2）接下来以关键点为中心取8×8的窗口。
   
<img src="https://github.com/wonderly321/SIFT_Feature_Extraction/blob/master/img/4_1.png" width="50%" />

   如上图中左部分的中央黑点为当前关键点的位置，每个小格代表关键点邻域所在尺度空间的一个像素，箭头方向代表该像素的梯度方向，箭头长度代表梯度模值，图中蓝色的圈代表高斯加权的范围（越靠近关键点的像素梯度方向信息贡献越大）。

   ​	（3）然后在每4×4的小块上计算8个方向的梯度方向直方图，绘制每个梯度方向的累加值，即可形成一个种子点，如图5右部分所示。此图中一个关键点由2×2共4个种子点组成，每个种子点有8个方向向量信息。这种邻域方向性信息联合的思想增强了算法抗噪声的能力，同时对于含有定位误差的特征匹配也提供了较好的容错性。

   在实际实现过程中，为了增强匹配的稳健性，对每个关键点使用4×4共16个种子点来描述，这样对于一个关键点就可以产生128个数据，即最终形成128维的SIFT特征向量。此时SIFT特征向量已经去除了尺度变化、旋转等几何变形因素的影响，再继续将特征向量的长度归一化，则可以进一步去除光照变化的影响。

5. 匹配两张图片上标记出的关键点

   使用暴力的方法依次匹配两幅图上的关键点

6. 画出匹配结果图并保存本地

   将两幅图中检测出的关键点分别标记在图上。并在匹配的关键点之间连线。保存匹配结果。

### 二  环境要求

Windows 系统/OS系统 	VS2013及以上版本 	C语言

### 三 实验结果

输入图片：

<img src="https://github.com/wonderly321/SIFT_Feature_Extraction/blob/master/img/test1.bmp" width="40%" /><img src="https://github.com/wonderly321/SIFT_Feature_Extraction/blob/master/img/test2.bmp" width="40%" />


输出结果：

<img src="https://github.com/wonderly321/SIFT_Feature_Extraction/blob/master/SIFT_v3/A_B_matching.bmp" width="80%" />

<img src="https://github.com/wonderly321/SIFT_Feature_Extraction/blob/master/SIFT_v3/A_keypoints.bmp" width="40%" /><img src="https://github.com/wonderly321/SIFT_Feature_Extraction/blob/master/SIFT_v3/B_keypoints.bmp" width="40%" />

