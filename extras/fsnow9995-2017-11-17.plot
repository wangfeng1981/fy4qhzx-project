#fy4融合积雪绘图 2017-11-17
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



unset colorbox
set urange[0:360]
set vrange[-90:90]
set cbrange[0:10]

set hidden3d front

#set xrange[-0.95:0.95] bug cause a circle
#set yrange[-1:1] bug cause a circle
#set zrange[-1:1] bug cause a circle
set pal maxcolors 15
set palette defined (\
0 '#000000', 1.1 '#000000',\
1.1 '#888888', 2.1 '#888888',\
2.1 '#1C4B9B', 3.1 '#1C4B9B',\
3.1 '#e8e8e8', 4.1 '#e8e8e8',\
4.1 '#ff0000', 5.1 '#ff0000',\
5.1 '#e8e800', 6.1 '#e8e800',\
6.1 '#f1f1f1', 7 '#f1f1f1',\
7 '#f1f1f1', 8 '#f1f1f1',\
8 '#f1f1f1', 9 '#f1f1f1',\
9 '#f1f1f1', 10 '#f1f1f1'\
)
r = 1.01
#set pm3d interpolate 2,2
set pm3d corners2color min

######### legend rect ##############

set object 1 rect from screen 0.05,screen 0.07 to screen 0.1,screen 0.1 fc  "#000000"
set label "Night" at screen 0.075,screen 0.04 center font "Arial,16"

set object 2 rect from screen 0.15,screen 0.07 to screen 0.2,screen 0.1 fc  "#888888"
set label "Land" at screen 0.175,screen 0.04 center font "Arial,16"

set object 3 rect from screen 0.25,screen 0.07 to screen 0.3,screen 0.1 fc  "#1C4B9B"
set label "Water" at screen 0.275,screen 0.04 center font "Arial,16"

set object 4 rect from screen 0.35,screen 0.07 to screen 0.4,screen 0.1 fc  "#e8e8e8"
set label "Cloud" at screen 0.375,screen 0.04 center font "Arial,16"

set object 5 rect from screen 0.45,screen 0.07 to screen 0.5,screen 0.1 fc  "#ff0000"
set label "Snow" at screen 0.475,screen 0.04 center font "Arial,16"

set object 6 rect from screen 0.55,screen 0.07 to screen 0.6,screen 0.1 fc  "#e8e800"
set label "C-Snow" at screen 0.575,screen 0.04 center font "Arial,16"



####################################
splot  "{{{INFILE}}}" using 1:2:(1):(($3==3)*1+($3==8)*2+($3==7)*3+($3==6)*4+($3==10)*5+($3==11)*6) with pm3d, r*cos(v)*cos(u),r*cos(v)*sin(u),r*sin(v) with lines lt 0 lc 'grey' ,'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2:(1.01) with lines lt 1 lc 'grey' , 'E:/coding/fy4qhzx-project/extras/china2.txt' u 1:2:(1.01) with lines lt 1 lc '#ccbbdd'  

#title lables
set label 1 'FY4A-SNC And FY3C-SD Combination Snow Cover Daily 4km' at screen 0.5, screen 0.95 center font 'Arial,16' 
set label 2 "{{{DATE}}}"  at screen 0.5, screen 0.92 font 'Arial,16' 

#logos
unset cbrange
set xrange [-1:1] ;
set yrange [-1:1] ;
unset title
plot 'E:/coding/fy4qhzx-project/extras/cmalogo.png' binary filetype=png dx=0.00125 dy=0.001 origin=(0.18,0.48)  with rgbimage  ,'E:/coding/fy4qhzx-project/extras/ncclogo.png' binary filetype=png dx=0.001 dy=0.001 origin=(0.36,0.48)  with rgbimage 


unset multiplot
unset output