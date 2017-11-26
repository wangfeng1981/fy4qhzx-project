#fy4 水汽含量绘图 plot
reset
set terminal png noenhanced size 800,1000 
set output 'tpw.heatmap.png'
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
set cbrange[-1:10]
set cbtics ("Land" -1,\
"0g/kg" 0.02,\
"2.0g/kg" 2.1 ,\
"5.0g/kg" 5 ,\
"7.0g/kg" 7 ,\
"10.0g/kg" 10 \
)

set hidden3d front

set pal maxcolors 11
set palette defined (  \
0 '#f1f1f1', 9.09 '#f1f1f1', \
9.09 '#FFBE00',18.18 '#FFBE00',\
18.18 '#FDE000',27.27 '#FDE000',\
27.27 '#FEFE00',36.36 '#FEFE00',\
36.36 '#65E600',45.45 '#65E600',\
45.45 '#01CC00',54.55 '#01CC00',\
54.55 '#1AFEB1',63.64 '#1AFEB1',\
63.64 '#18C6FB',72.73 '#18C6FB',\
72.73 '#1890FE',81.82 '#1890FE',\
81.82 '#1918A2',90.91 '#1918A2',\
90.91 '#FB00FD',100.00 '#FB00FD'\
)
r = 1.01
set pm3d interpolate 2,2
####################################
splot 'global_20.llv.txt' using 1:2:(1):3 with pm3d , "tpw.llv.txt" using 1:2:(1):3 with pm3d, r*cos(v)*cos(u),r*cos(v)*sin(u),r*sin(v) with lines lt 0 lc 'grey' ,'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2:(1.01) with lines lt 1 lc 'grey' , 'E:/coding/fy4qhzx-project/extras/china.dat' u 1:2:(1.01) with lines lt 1 lc '#ccbbdd'  



#title lables
set label 1 'FY4A 20170821 Total Percipitive Water Daily 4km QC:all' at screen 0.5,0.95 center font 'Arial,16' 
set label 2 ""  at -0.60,0.64 font 'Arial,15' 

#logos
unset cbrange
set xrange [-1:1] ;
set yrange [-1:1] ;
unset title
plot 'E:/coding/fy4qhzx-project/extras/cmalogo.png' binary filetype=png dx=0.00125 dy=0.001 origin=(0.18,0.48)  with rgbimage  ,'E:/coding/fy4qhzx-project/extras/ncclogo.png' binary filetype=png dx=0.001 dy=0.001 origin=(0.36,0.48)  with rgbimage 


unset multiplot
unset output