#fy4 snow cover plot 等经纬度格网
#变量 {{{OUTPNGFILE}}} {{{TITLE1}}} {{{TITLE2}}} {{{LONLATVALTXTFILE}}}
reset
set terminal png noenhanced size 1000,1050 
set output '{{{OUTPNGFILE}}}'
set encoding utf8
set multiplot
#title lables
set label 1 '{{{TITLE1}}}' at screen 0.5, screen 0.96 font 'Arial,16' center 
set label 2 "{{{TITLE2}}}"  at screen 0.5, screen 0.93 font 'Arial,16' center
#plot1
set origin 0.03,0.00
set size square 1,1
unset key 
set xtics 
set ytics 
set x2tics
set format x "%.0f\260"
set format y "%.0f\260"
set format x2 "%.0f\260"
show grid
set grid front 
set xrange[25:175]
set x2range[25:175]
set yrange[-50:75]
set cbrange[0:255]
set format cb "%.0fK"
set pal maxcolors 16
set palette defined (  \
0 '#2b83ba', 6.25 '#2b83ba', \
6.25 '#4d96b4', 12.5 '#4d96b4', \
12.5 '#6fb3ae', 18.75 '#6fb3ae', \
18.75 '#91cba9', 25 '#91cba9', \
25 '#b1e09f', 31.25 '#b1e09f', \
31.25 '#c7e98b', 37.5 '#c7e98b', \
37.5 '#def277', 43.75 '#def277', \
43.75 '#f4fb63', 50 '#f4fb63', \
50 '#fff55a', 56.25 '#fff55a', \
56.25 '#ffdf5c', 62.5 '#ffdf5c', \
62.5 '#fec95e', 68.75 '#fec95e', \
68.75 '#feb460', 75 '#feb460', \
75 '#f69053', 81.25 '#f69053', \
81.25 '#ec6841', 87.5 '#ec6841', \
87.5 '#e2402e', 93.75 '#e2402e', \
93.75 '#d7191c', 100 '#d7191c'\
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
