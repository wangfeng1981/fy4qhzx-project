#南海绘图 2017-11-16
reset
set terminal png noenhanced size 2000,500 
set output '{{{OUTFILE}}}'
set multiplot
set encoding utf8

set bmargin screen 0.3

set timefmt "%Y-%m-%d"
set xdata time
set xrange ["{{{X0}}}":"{{{X1}}}"]
set format x "%m/%d"
set yrange [50:450]
set y2range [-10:10]
set y2tics
unset key
####################################
plot '{{{INFILE}}}' using 1:2 with lines lc '#ff0000' axes x1y1 ,'' using 1:2 with points axes x1y1, '' using 1:3 with lines lc '#0000ff' axes x1y2, '' using 1:3 with points axes x1y2, 250 lc "#000000" lw 2 axes x1y1



#title lables
set label 1 'Variation of Zonal wind and FY4A OLR over monitoring region' at screen 0.15,0.15 left font 'Arial,16' 
set label 2 "from {{{TIME0}}} to {{{TIME1}}}" at screen 0.15,0.1 left font 'Arial,15' 
set label 3 "Produced at {{{MAKETIME}}}" at screen 0.15,0.01 left font 'Arial,10' 

#logos
set bmargin screen 0
unset cbrange
unset xtics
unset ytics
unset y2tics
unset key
unset border
set origin 0,0
set size 0.11,0.2
set xrange [0:256] 
set yrange [0:128] 

unset title
plot 'E:/coding/fy4qhzx-project/extras/cmalogo.png' binary filetype=png dx=1 dy=1 origin=(0,0)  with rgbimage  ,'E:/coding/fy4qhzx-project/extras/ncclogo.png' binary filetype=png dx=1 dy=1 origin=(128,0)  with rgbimage 


unset multiplot
unset output