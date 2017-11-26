reset
r=0.99
set multiplot
#set terminal png
#set output 'test2.png'
set size 1.0, 1.0
unset key
splot "fy4sst3.txt" using 1:2:(1):3 with pm3d,  'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2:(1) with lines lt 1 lc 'grey' , 'E:/coding/fy4qhzx-project/extras/china.dat' u 1:2:(1) with lines lt 1 lc '#ccbbdd'
#logos
unset xtics
unset ytics
unset border
set size 0.1,0.1
set origin 0.9,0.9
plot 'cmalogo.png' binary filetype=png origin=(0,0) dx=0.1 dy=0.1 with rgbimage 
