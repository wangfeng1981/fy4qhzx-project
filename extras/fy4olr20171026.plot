#fy4 长波辐射圆盘绘图sst plot 20171026
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
set cbrange[25:400]
set cbtics ("Land" 25,\
"<50" 50,\
"125" 125 ,\
"200" 200 ,\
"275" 275,\
"350" 350 ,\
">400 (W/m^2)" 400 \
)

set hidden3d front



#set xrange[-0.95:0.95] bug cause a circle
#set yrange[-1:1] bug cause a circle
#set zrange[-1:1] bug cause a circle
set pal maxcolors 15
set palette defined (\
 0.00 '#f1f1f1', 6.67 '#f1f1f1',\
 6.67 '#0854c2', 13.33 '#0854c2',\
13.33 '#1e6eeb', 20.00 '#1e6eeb',\
20.00 '#2882f0' ,26.67 '#2882f0' ,\
26.67 '#3c96f5', 33.33 '#3c96f5',\
33.33 '#50a5f5', 40.00 '#50a5f5',\
40.00 '#78b9fa', 46.67 '#78b9fa',\
46.67 '#96d2fa' ,53.33 '#96d2fa',\
53.33 '#b4f0fa', 60.00 '#b4f0fa',\
60.00 '#e1ffff', 66.67 '#e1ffff',\
66.67 '#fffaaa', 73.33 '#fffaaa',\
73.33 '#ffc03c' ,80.00 '#ffc03c',\
80.00 '#ff6000', 86.67 '#ff6000',\
86.67 '#e11400', 93.33 '#e11400',\
93.33 '#a50000', 100.00 '#a50000'\
)
r = 1.01
set pm3d interpolate 2,2
####################################
splot 'E:/coding/fy4qhzx-project/extras/land_0125half.llv.txt' using 1:2:(1):3 with pm3d , "{{{INFILE}}}" using 1:2:(1):( ($3<50)*50 + ($3>=50)*$3 ) with pm3d, r*cos(v)*cos(u),r*cos(v)*sin(u),r*sin(v) with lines lt 0 lc 'grey' ,'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2:(1.01) with lines lt 1 lc 'grey' , 'E:/coding/fy4qhzx-project/extras/china.dat' u 1:2:(1.01) with lines lt 1 lc '#ccbbdd'  



#title lables font should change.
set label 1 '{{{TITLE1}}}' at screen 0.5,0.95 center font 'Arial,16' 
set label 2 "{{{TITLE2}}}" at screen 0.5,0.92 center font 'Arial,15' 

#logos
unset cbrange
set xrange [-1:1] ;
set yrange [-1:1] ;
unset title
plot 'E:/coding/fy4qhzx-project/extras/cmalogo.png' binary filetype=png dx=0.00125 dy=0.001 origin=(0.18,0.48)  with rgbimage  ,'E:/coding/fy4qhzx-project/extras/ncclogo.png' binary filetype=png dx=0.001 dy=0.001 origin=(0.36,0.48)  with rgbimage 


unset multiplot
unset output