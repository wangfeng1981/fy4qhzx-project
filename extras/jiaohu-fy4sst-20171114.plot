#fy4海表温度交互绘图sst plot
reset
set terminal png noenhanced size 800,1000 
set output '{{{OUTFILE}}}'
set multiplot
set encoding utf8

set bmargin 8

set xtics
set ytics
unset ztics
unset border
unset key 

set palette defined (  0.00 '#0854c2', 50.00 '#efef00' , 100.00 '#a50000' )

####################################
set xrange[{{{X0}}}:{{{X1}}}] 
set yrange[{{{Y0}}}:{{{Y1}}}] 
set cbrange[{{{VAL0}}}:{{{VAL1}}}]

plot  "{{{INFILE}}}" using 1:2:3 with image , 'E:/coding/fy4qhzx-project/extras/world_50m.txt' u 1:2 with filledcurves lt 1 lc 'grey'   , 'E:/coding/fy4qhzx-project/extras/china.dat' u 1:2  with lines lt 1 lc '#ccbbdd'  


#title lables
set label 1 '{{{TITLE1}}}' at screen 0.5,0.08 center font 'Arial,16' 
set label 2 "Produced {{{MAKETIME}}}" at screen 0.5,0.01 center font 'Arial,13' 

#logos
unset xtics
unset ytics
unset cbrange
set xrange [0:128] 
set yrange [0:128] 
set margin 0,0,0,0
unset title
set origin 0,0
set size 0.2,0.2
plot 'E:/coding/fy4qhzx-project/extras/cmalogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(0,0)  with rgbimage  ,'E:/coding/fy4qhzx-project/extras/ncclogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(64,0)  with rgbimage 


unset multiplot
unset output
