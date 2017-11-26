#fy4海表温度绘图sst plot 等经纬度格网 
#2017-10-13
#变量 OUTPNGFILE TITLE1 TITLE2 LONLATVALTXTFILE
reset
set terminal png noenhanced size 1000,1050 
set output '{{{OUTPNGFILE}}}'
set encoding utf8
set multiplot
#title lables
set label 1 '{{{TITLE1}}}' at screen 0.5, screen 0.96 font 'Arial,16' center 
set label 2 "{{{TITLE2}}}"  at screen 0.5, screen 0.93 font 'Arial,16' center
#plot1
set origin 0.03,0.10
set size 0.95,0.8
unset key 
set xtics 
set ytics 
set x2tics
set format x "%.0f\260"
set format y "%.0f\260"
set format x2 "%.0f\260"
show grid
set grid front 
set xrange[20:190]
set x2range[20:190]
set yrange[-80:80]

#new
set xtics ("20\260E" 20, "40\260E" 40,"60\260E" 60,"80\260E" 80,"100\260E" 100,"120\260E" 120, "140\260E" 140,"160\260E" 160, "180\260" 180)
set x2tics ("20\260E" 20, "40\260E" 40,"60\260E" 60,"80\260E" 80,"100\260E" 100,"120\260E" 120, "140\260E" 140,"160\260E" 160, "180\260" 180)
set ytics ("-80\260S" -80, "-60\260S" -60,"-40\260S" -40,"-20\260S" -20,"0\260" 0,"20\260N" 20, "40\260N" 40,"60\260N" 60, "80\260N" 80)

set cbtics (0,1.25,2.5,3.75,5,6.25,7.5,8.75,10)


set cbrange[0:10]
set format cb "%.02fg/k"
set pal maxcolors 16
set palette defined (  \
0 '#9918ba', 6.25 '#9918ba', \
6.25 '#1918a2', 12.5 '#1918a2', \
12.5 '#1890fe', 18.75 '#1890fe', \
18.75 '#18c6fb', 25 '#18c6fb', \
25 '#34fffc', 31.25 '#34fffc', \
31.25 '#1afeb1', 37.5 '#1afeb1', \
37.5 '#01cc00', 43.75 '#01cc00', \
43.75 '#65e600', 50 '#65e600', \
50 '#fefe00', 56.25 '#fefe00', \
56.25 '#fde000', 62.5 '#fde000', \
62.5 '#ffbe00', 68.75 '#ffbe00', \
68.75 '#fe7e00', 75 '#fe7e00', \
75 '#fe3d02', 81.25 '#fe3d02', \
81.25 '#fe0000', 87.5 '#fe0000', \
87.5 '#b4b4b4', 93.75 '#b4b4b4', \
93.75 '#dadada', 100 '#dadada'\
)
plot '{{{LONLATVALTXTFILE}}}' using 1:2:3 with image \
, 'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2 with lines lt 1 lc 'grey' \
, 'E:/coding/fy4qhzx-project/extras/china.dat' u 1:2 with lines lt 1 lc '#ccbbdd' 
#logos
set origin 0.75,-0.02
set size square 0.24,0.24
set xrange[0:128]
set yrange[0:128]
unset key 
unset xtics
unset x2tics
unset ytics
unset border
unset cbrange
unset pm3d
plot 'E:/coding/fy4qhzx-project/extras/cmalogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(0,0)  with rgbimage , 'E:/coding/fy4qhzx-project/extras/ncclogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(64,0.5)  with rgbimage 
unset multiplot
unset output
