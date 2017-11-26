reset
set terminal png size 800,1000 
set output 'fy4sst.heatmap.png'
set encoding utf8
set mapping spherical
set angles degrees
set hidden3d front
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
set title 'FY4A SST 20170927 15:15' offset 0,-11 font ',30' 
set colorbox horizontal user origin 0.1 , 0.055 size 0.8,.05
set urange[0:360]
set vrange[-90:90]
set cbrange[15:35]
#set cblabel "{/Symbol \260}C" offset 0,1 font ',15'
set cbtics ("15{/Symbol \260}C" 15,"20{/Symbol \260}C" 20, "25{/Symbol \260}C" 25, "30{/Symbol \260}C" 30, "35{/Symbol \260}C" 35)
set xrange[-0.95:0.95]
set yrange[-1:1]
set zrange[-1:1]
set pal maxcolors 15
set palette defined ( 0 '#0c3b7d', 6.7 '#0c3b7d',  6.7 '#1464d2', 13.4 '#1464d2', 13.4 '#1e6eeb', 20.1 '#1e6eeb',  20.1 '#2882f0' ,  26.8 '#2882f0' , 26.8 '#3c96f5', 33.5 '#3c96f5', 33.5 '#50a5f5', 40.2 '#50a5f5',  40.2 '#78b9fa', 46.9 '#78b9fa',  46.9 '#96d2fa' ,  53.6 '#96d2fa' ,53.6 '#b4f0fa', 60.3 '#b4f0fa',60.3 '#e1ffff', 67 '#e1ffff', 67 '#fffaaa', 73.7 '#fffaaa', 73.7 '#ffc03c' , 80.4 '#ffc03c' , 80.4 '#ff6000', 87.1 '#ff6000', 87.1 '#e11400', 93.8 '#e11400',  93.8 '#a50000',  100 '#a50000' )
r = 0.999
set pm3d interpolate 2,2
splot "fy4sst3.txt" using 1:2:(1):3 with pm3d, r*cos(v)*cos(u),r*cos(v)*sin(u),r*sin(v) with lines lt 0 lc 'grey' ,'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2:(1) with lines lt 1 lc 'grey' , 'E:/coding/fy4qhzx-project/extras/china.dat' u 1:2:(1) with lines lt 1 lc '#ccbbdd' 
unset output






