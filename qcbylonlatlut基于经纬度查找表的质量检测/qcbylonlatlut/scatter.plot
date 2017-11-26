#输出散点图
reset
set terminal pngcairo size 800,600
set output "scatter-test.png"

#拟合曲线
f(x)=a*x+b
a=1.
b=1.
fit f(x) "fy4sst0807.NC.scatter.txt" using 1:2 via a,b
set xlabel 'input'
set ylabel 'ref'
fiteq=sprintf('f(x)=%.2f*x+%.2f' , a , b) ;
set label 1 fiteq at graph 0.1 , 0.9

#设置高宽比
#set size ratio 0.9
plot "fy4sst0807.NC.scatter.txt" using 1:2 , f(x) with lines linecolor 7
unset output