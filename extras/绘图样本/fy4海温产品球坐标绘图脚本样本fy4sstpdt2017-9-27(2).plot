#fy4海表温度绘图sst plot
reset
set terminal png noenhanced size 800,1000 
set output 'fy4sst.heatmap.png'
set multiplot
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



#set title 'FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_20170815000000_20170815001459_4000M_V0001.NC' offset -0,-12 font 'Arial,15' 
set colorbox horizontal user origin 0.1 , 0.055 size 0.8,.05
set urange[0:360]
set vrange[-90:90]
set cbrange[15:35]
set cbtics ("15\260C" 15,"20\260C" 20.3, "25\260C" 25, "30\260C" 29.85, "35\260C" 35)
set xrange[-0.95:0.95]
set yrange[-1:1]
set zrange[-1:1]
set pal maxcolors 15
set palette defined ( 0 '#0c3b7d', 6.7 '#0c3b7d',  6.7 '#1464d2', 13.4 '#1464d2', 13.4 '#1e6eeb', 20.1 '#1e6eeb',  20.1 '#2882f0' ,  26.8 '#2882f0' , 26.8 '#3c96f5', 33.5 '#3c96f5', 33.5 '#50a5f5', 40.2 '#50a5f5',  40.2 '#78b9fa', 46.9 '#78b9fa',  46.9 '#96d2fa' ,  53.6 '#96d2fa' ,53.6 '#b4f0fa', 60.3 '#b4f0fa',60.3 '#e1ffff', 67 '#e1ffff', 67 '#fffaaa', 73.7 '#fffaaa', 73.7 '#ffc03c' , 80.4 '#ffc03c' , 80.4 '#ff6000', 87.1 '#ff6000', 87.1 '#e11400', 93.8 '#e11400',  93.8 '#a50000',  100 '#a50000' )
r = 0.999
set pm3d interpolate 2,2
splot "fy4sst.txt" using 1:2:(1):3 with pm3d, r*cos(v)*cos(u),r*cos(v)*sin(u),r*sin(v) with lines lt 0 lc 'grey' ,'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2:(1) with lines lt 1 lc 'grey' , 'E:/coding/fy4qhzx-project/extras/china.dat' u 1:2:(1) with lines lt 1 lc '#ccbbdd' 

#title lables
set label 1 'FY4A-_AGRI--_N_DISK_1047E_L2-_SST-_MULT_NOM_20170815' at -0.8,0.68 font 'Arial,15'
set label 2 "000000_20170815001459_4000M_V0001.NC"  at -0.60,0.64 font 'Arial,15' 

#logos
unset cbrange
set xrange [-1:1] ;
set yrange [-1:1] ;
unset title
plot 'cmalogo.png' binary filetype=png dx=0.00125 dy=0.001 origin=(0.18,0.48)  with rgbimage  ,'ncclogo.png' binary filetype=png dx=0.001 dy=0.001 origin=(0.36,0.48)  with rgbimage 



