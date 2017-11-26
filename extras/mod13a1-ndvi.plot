#draw modis ndvi 
#2017-11-23 wf
reset
#heatmap
set terminal png noenhanced size 2000,1390
set output '{{{OUTFILE}}}'
set encoding utf8
set multiplot

#plot1
set origin 0.03,0.12
set size  0.95,0.85
unset key 
unset xtics 
unset ytics 
unset x2tics

set format x "%.0f"
set format y "%.0f"

set xrange[-3000000:3000000]
set yrange[1800000:6000000]
set cbrange[-0.1:0.8]

set cbtics ( -0.1 , 0.0 , 0.1 , 0.2 ,0.3 , 0.4 , 0.5 , 0.6 , 0.7 , 0.8 )


#set pal maxcolors 14
set palette defined (  \
0 '#713E3A',   \
1.11 '#83533C' , \
2.22 '#AC8327',   \
3.33 '#D1AF18',   \
4.44 '#EFE900',   \
5.55 '#84E200',   \
6.66 '#05B514',   \
7.77 '#00811C',   \
8.88 '#015020',   \
10 '#001F18'    \
)

set label 1 "MOD13A1 NDVI {{{TITLE}}} 500m 16-Days " at screen 0.5, screen 0.1 font 'Arial,32' center 

set label 2 "Produced:{{{PTIME}}}" at screen 0.1, screen 0.05 font 'Arial,12' left 


set object 1 rect from graph 0.86,0.0 to graph 1.0,0.3
set object 1 rect fc rgb 'white' fillstyle solid  0.0 noborder front
plot \
'{{{INFILE}}}' using 1:2:3 with image , \
'E:/coding/fy4qhzx-project/extras/world_110m-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'E:/coding/fy4qhzx-project/extras/china-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'E:/coding/fy4qhzx-project/extras/sheng-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'E:/coding/fy4qhzx-project/extras/river-albers-final.txt' u 1:2 with lines lt 1 lc '#0E35A6',\
'E:/coding/fy4qhzx-project/extras/grid-albers-final.txt' u 1:2 with lines lt 1 lc '#cccccc'



#nan hai
unset key 
unset xtics 
unset ytics 
set size 0.14,0.28
set origin 0.73,0.122
set xrange[40000:1800000]
set yrange[100000:2500000]
unset colorbox
unset object 1
plot \
'{{{INFILE}}}' using 1:2:3 with image , \
'E:/coding/fy4qhzx-project/extras/world_110m-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'E:/coding/fy4qhzx-project/extras/china-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'E:/coding/fy4qhzx-project/extras/sheng-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'E:/coding/fy4qhzx-project/extras/river-albers-final.txt' u 1:2 with lines lt 1 lc '#0E35A6' ,\
'E:/coding/fy4qhzx-project/extras/grid-albers-final.txt' u 1:2 with lines lt 1 lc '#cccccc'


#logos
set origin 0.75,-0.00
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