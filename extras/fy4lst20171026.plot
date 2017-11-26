# 陆表温度绘图 plot 等经纬度格网 2017-10-26
reset
set terminal png noenhanced size 1200,1050 
set output '{{{OUTFILE}}}'
set encoding utf8
set multiplot
#title lables ########## font
set label 1 "{{{TITLE1}}}"" at screen 0.5, screen 0.96 font 'Arial,16' center 
set label 2 "{{{TITLE2}}}"  at screen 0.5, screen 0.93 font 'Arial,16' center
#plot1

unset key 
set xtics 
set ytics 


unset ztics
set format x "%.0f\260"
set format y "%.0f\260"
set format x2 "%.0f\260"
show grid
set grid front 
set xrange[20:180]
set x2range[20:180]
set yrange[-50:75]
set cbrange[256:320]
set format cb "%.0fK"
set pal maxcolors 16
set palette defined (  \
 0.00 '#e8e8e888', 6.25 '#e8e8e888', \
 6.25 '#4d96b488', 12.5 '#4d96b488', \
12.50 '#6fb3ae88', 18.75 '#6fb3ae88', \
18.75 '#91cba9', 25 '#91cba9', \
25.00 '#b1e09f', 31.25 '#b1e09f', \
31.25 '#c7e98b', 37.5 '#c7e98b', \
37.50 '#def277', 43.75 '#def277', \
43.75 '#f4fb63', 50 '#f4fb63', \
50.00 '#fff55a', 56.25 '#fff55a', \
56.25 '#ffdf5c', 62.5 '#ffdf5c', \
62.50 '#fec95e', 68.75 '#fec95e', \
68.75 '#feb460', 75 '#feb460', \
75.00 '#f69053', 81.25 '#f69053', \
81.25 '#ec6841', 87.5 '#ec6841', \
87.50 '#e2402e', 93.75 '#e2402e', \
93.75 '#d7191c', 100 '#d7191c'\
)

set zrange[0:2] ;
set view 0,0
set origin -0.27,-0.2
set size square 1.4,1.4
set colorbox user origin 0.88,0.125 size 0.05,0.76
set cbtics (  'Land' 256 , '<260K' 260, '272K' 272 , '284K' 284,'296K' 296, '308K' 308,'>320K' 320 )
set ytics ( "40\260S" -40, "20\260S" -20, "0\260" 0, "20\260N" 20, "40\260N" 40, "60\260N" 60 )
set xtics ( "20\260E" 20, "40\260E" 40, "60\260E" 60, "80\260E" 80, "100\260E" 100, "120\260E" 120 , "140\140E" 140 , "160\260E" 160 , "180\260" 180 )


####################################
splot 'E:/coding/fy4qhzx-project/extras/land_0125half.llv.txt' u 1:2:(0.5):3  with pm3d ,  '{{{INFILE}}}' using 1:2:(0.5):( ($3<260)*260 + ($3>=260)*$3 ) with pm3d , 'E:/coding/fy4qhzx-project/extras/world_110m.txt' u 1:2:(0.501) with lines lt 1 lc 'grey' , 'E:/coding/fy4qhzx-project/extras/china.dat' u 1:2:(0.501) with lines lt 1 lc '#ccbbdd'  

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