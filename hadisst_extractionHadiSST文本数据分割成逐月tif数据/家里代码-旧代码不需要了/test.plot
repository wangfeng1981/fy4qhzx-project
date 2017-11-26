reset
set terminal png size 800 , 350  
set output 'heatmap2d.png'
 
# color definitions
set border lw 1.5
set style line 1 lc rgb 'black' lt 1 lw 2

set rmargin screen 0.85

unset key
#tics表示图例和坐标轴上的刻度线
set tics scale 5
#unset xtics
#unset ytics
#设置图像显示的最大颜色阶段
set pal maxcolors 10
set cbrange [-10:40]
set xrange[-179:179]
set yrange[-89:89]

set palette defined (  \
0 "0x222255",10 "0x222255" , \
10 "0x4575b4",20 "0x4575b4" , \
20 "0x74add1",30 "0x74add1" , \
30 "0xabd9e9",40 "0xabd9e9" , \
40 "0xe0f3f8",50 "0xe0f3f8" , \
50 "0xffffbf",60 "0xffffbf" , \
60 "0xfee090",70 "0xfee090" , \
70 "0xfdae61",80 "0xfdae61" , \
80 "0xf46d43",90 "0xf46d43" , \
90 "0xd73027",100 "0xd73027" \
)

plot 'HadISST1_SST_2017.txt.2017-1-1.tif.dat' u 1:2:3 w image, \
     'E:/coding/fy4qhzx-project/extras/world_110m.txt' with lines linestyle 1


