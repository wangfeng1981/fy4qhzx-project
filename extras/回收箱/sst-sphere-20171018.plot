#fy4海表温度绘图sst plot
reset
set terminal png noenhanced size 800,1000 
set output '{{{OUTFILE}}}'
set multiplot
set encoding utf8
set mapping spherical
set angles degrees

set parametric
set view 90,195
set isosamples 30
set xyplane at -1
set origin -0.30,-0.2
set size 1.6,1.4
unset xtics
unset ytics
unset ztics
unset border
unset key 



set colorbox horizontal user origin 0.1 , 0.055 size 0.8,.05
set urange[0:360]
set vrange[-90:90]
set cbrange[11:35]
set cbtics ("Land" 11,\
"<14\260C" 14,\
"20\260C" 20 ,\
"26\260C" 26 ,\
"30.5\260C" 30.5,\
">33.5\260C" 35 \
)

set hidden3d front



#set xrange[-0.95:0.95] bug cause a circle
#set yrange[-1:1] bug cause a circle
#set zrange[-1:1] bug cause a circle
set pal maxcolors 16
set palette defined (\
 0.00 '#f1f1f1', 6.25 '#f1f1f1',\
 6.25 '#0854c2', 12.50 '#0854c2',\
12.50 '#1464d2', 18.75 '#1464d2',\
18.75 '#1e6eeb', 25.00 '#1e6eeb',\
25.00 '#2882f0' ,31.25 '#2882f0' ,\
31.25 '#3c96f5', 37.50 '#3c96f5',\
37.50 '#50a5f5', 43.75 '#50a5f5',\
43.75 '#78b9fa', 50.00 '#78b9fa',\
50.00 '#96d2fa' ,56.25 '#96d2fa',\
56.25 '#b4f0fa', 62.25 '#b4f0fa',\
62.50 '#e1ffff', 68.75 '#e1ffff',\
68.75 '#fffaaa', 75.00 '#fffaaa',\
75.00 '#ffc03c' ,81.25 '#ffc03c',\
81.25 '#ff6000', 87.50 '#ff6000',\
87.50 '#e11400', 93.75 '#e11400',\
93.75 '#a50000', 100.00 '#a50000'\
)
r = 1.01
set pm3d interpolate 2,2
####################################
splot 'E:/coding/fy4qhzx-project/extras/global_20.llv.txt' using 1:2:(1):3 with pm3d , "{{{INFILE}}}" using 1:2:(1):( ($3<13)*13 + ($3>=13)*$3 ) with pm3d, r*cos(v)*cos(u),r*cos(v)*sin(u),r*sin(v) with lines lt 0 lc 'grey' ,'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2:(1.01) with lines lt 1 lc 'grey' , 'E:/coding/fy4qhzx-project/extras/china.dat' u 1:2:(1.01) with lines lt 1 lc '#ccbbdd'  



#title lables
set label 1 '{{{TITLE1}}}' at screen 0.5,0.95 center font 'Arial,16' 
set label 2 '{{{TITLE2}}}'  at -0.60,0.64 font 'Arial,15' 

#logos
unset cbrange
set xrange [-1:1] ;
set yrange [-1:1] ;
unset title
plot 'E:/coding/fy4qhzx-project/extras/cmalogo.png' binary filetype=png dx=0.00125 dy=0.001 origin=(0.18,0.48)  with rgbimage  ,'E:/coding/fy4qhzx-project/extras/ncclogo.png' binary filetype=png dx=0.001 dy=0.001 origin=(0.36,0.48)  with rgbimage 


unset multiplot
unset output