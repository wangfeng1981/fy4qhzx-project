#绘制地图
reset
set terminal pngcairo size 1000,800
set output "heatmap-test.png"

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
set palette defined (0 "#151b6d",25 "#30ffed", \
	50 "#04ff00" , 75 "#fff830" , 100 "#ff0000" , 101 "grey" )
r=0.99
splot "fy4sst0807.NC.lonlatbias.txt" using 1:2:(1):3 with pm3d , \
	r*cos(v)*cos(u),r*cos(v)*sin(u),r*sin(v) with lines ls 0, \
   'world_110m.txt' u 1:2:(1) with lines ls 2

unset output

