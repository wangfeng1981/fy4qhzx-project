#输出直方图
reset
set terminal png size 800, 600
set output "fy4sst0807.NC.hist.png"
set xrange[-5:5]
binwidth = 5
plot 'fy4sst0807.NC.biashist.txt' smooth freq with boxes
unset output
#输出散点图
reset
set terminal pngcairo size 800,600
set output "fy4sst0807.NC.scatter.png"
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
#绘制分布图
reset
set terminal pngcairo size 1000,800
set output "fy4sst0807.NC.heatmap.png"
set mapping spherical
set angles degrees
set hidden3d front
set parametric
set view 90,195
set isosamples 30
set xyplane at -1
set origin -0.26,-0.38
set size 1.4,1.75
unset xtics
unset ytics
unset ztics
#隐藏坐标轴
unset border 
#设置colorbar位置
set colorbox vertical user origin 0.9 , .1 size .04,.8
set urange[0:360]
set vrange[-90:90]
set cbrange[-5:5]
set xrange[-0.95:0.95]
set yrange[-1:1]
set zrange[-1:1]
set pal maxcolors 10
set palette defined ( 0 "0x5e4fa2",10 "0x5e4fa2" , 10 "0x3288bd",20 "0x3288bd" , 20 "0x66c2a5",30 "0x66c2a5" ,  30 "0xabdda4",40 "0xabdda4" , 40 "0xe6f598",50 "0xe6f598" , 50 "0xffffbf",60 "0xffffbf" ,  60 "0xfee08b",70 "0xfee08b" ,  70 "0xfdae61",80 "0xfdae61" ,  80 "0xf46d43",90 "0xf46d43" ,  90 "0x9e0142",100 "0x9e0142" )
r = 0.99
splot "fy4sst0807.NC.lonlatbias.txt" using 1:2:(1):3 with pm3d,r*cos(v)*cos(u),r*cos(v)*sin(u),r*sin(v) with lines ls 0,'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2:(1) with lines ls 2
unset output
