#ndvi_hist_image
reset
#fy3hist
set terminal png size 800, 600
set output "{{{IHISTPNG}}}"
set xrange[-0.3:1.0]
plot '{{{IHISTTXT}}}' using 1:2 smooth freq with boxes
unset output

reset
#noaahist
set terminal png size 800, 600
set output "{{{RHISTPNG}}}"
set xrange[-0.3:1.0]
plot '{{{RHISTTXT}}}' using 1:2 smooth freq with boxes
unset output

reset
#biashist
set terminal png size 800, 600
set output "{{{BHISTPNG}}}"
set xrange[-0.5:0.5]
plot '{{{BHISTTXT}}}' using 1:2 smooth freq with boxes
unset output

reset
#scatter
set terminal png size 800, 600
set output "{{{SCATTERPNG}}}"
unset key
plot '{{{SCATTERTXT}}}' using 1:2 
unset output

reset
#heatmap
set terminal png noenhanced size 1024,768 
set output '{{{BDISTPNG}}}'
set encoding utf8
set multiplot
#title lables
set label 1 '' at screen 0.5, screen 0.96 font 'Arial,16' center 
set label 2 ""  at screen 0.5, screen 0.93 font 'Arial,16' center
#plot1
set origin 0.03,0.15
set size  1,0.75
unset key 
set xtics 
set ytics 
set x2tics
set format x "%.0f\260"
set format y "%.0f\260"
set format x2 "%.0f\260"
show grid
set grid front 
set xrange[70:140]
set x2range[70:140]
set yrange[15:55]
set cbrange[-0.5:0.5]
set pal maxcolors 10
set palette defined (  \
0 '#2b83ba', 10 '#2b83ba', \
10 '#4d96b4', 20 '#4d96b4', \
20 '#6fb3ae', 30 '#6fb3ae', \
30 '#91cba9', 40 '#91cba9', \
40 '#f4fb63', 50 '#f4fb63', \
50 '#f4fb63', 60 '#f4fb63', \
60 '#f69053', 70 '#f69053', \
70 '#ec6841', 80 '#ec6841', \
80 '#e2402e', 90 '#e2402e', \
90 '#d7191c', 100 '#d7191c'\
)
plot '{{{BDISTTXT}}}' using 1:2:3 with image \
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

