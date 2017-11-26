# 2017-11-10 季风水汽和风场叠加绘图
reset
set terminal png noenhanced size 1300,800
set output 'jifeng-1.png'
set encoding utf8
set multiplot
#title lables
set label 1 'FY4A TPW WITH NCEP WIND FIELD {{{TITLE}}}' at screen 0.5, screen 0.96 font 'Arial,16' center 


unset key 
set xtics 
set ytics 

unset ztics
set format x "%.0f\260"
set format y "%.0f\260"
set format x2 "%.0f\260"
show grid
set grid front 
set xrange[30:180]
set x2range[30:180]
set yrange[-20:50]
set cbrange[0:10]
#set format cb "%.01fcm"
set pal maxcolors 10
set palette defined (  \
 0 '#000096',   1 '#000096', \
 1 '#2B3CBE',   2 '#2B3CBE', \
 2 '#5779E6',   3 '#5779E6', \
 3 '#64A2BE',   4 '#64A2BE', \
 4 '#6AC583',   5 '#6AC583', \
 5 '#8CA499',   6 '#8CA499', \
 6 '#BD5FD9',   7 '#BD5FD9', \
 7 '#CA2CEB',   8 '#CA2CEB', \
 8 '#A016B5',   9 '#A016B5', \
 9 '#750080',   10 '#750080' \
)

set zrange[0:2] ;
set view 0,0
set origin 0.0,0.1
set size 0.92,0.83
set colorbox user origin 0.92,0.24 size 0.03,0.5
set cbtics ( 0, 2 , 4 , 6 , 8 , 10 )
set ytics ( "40\260S" -40, "20\260S" -20, "0\260" 0, "20\260N" 20, "40\260N" 40, "60\260N" 60 )
set xtics ( "20\260E" 20, "40\260E" 40, "60\260E" 60, "80\260E" 80, "100\260E" 100, "120\260E" 120 , "140\140E" 140 , "160\260E" 160 , "180\260" 180 )

set style arrow 1 head nofilled size screen 0.03,10 lc '#FF4848'

plot  'jifeng.txt' using 1:2:3 with image ,'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2 with lines  lt 1 lc '#eaea55' ,  'E:/coding/fy4qhzx-project/extras/china2.txt' u 1:2  with lines lt 1 lc '#55ff55' ,'wind.txt' u 1:2:($3/5):($4/5) w vec arrowstyle 1


#logos
set origin 0.72,-0.02
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
