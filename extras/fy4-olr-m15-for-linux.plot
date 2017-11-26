#fy4 OLR plot 等经纬度格网
#2017-10-13
#变量 OUTPNGFILE TITLE1 TITLE2 LONLATVALTXTFILE
reset
set terminal png noenhanced size 1050,1050 font '/root/ncc-fy4-project/extras/libsans-reg.ttf'
set output '{{{OUTPNGFILE}}}'
set encoding utf8
set multiplot
#title lables
set label 1 '{{{TITLE1}}}' at screen 0.5, screen 0.96 font '/root/ncc-fy4-project/extras/libsans-reg.ttf,18' center 
set label 2 "{{{TITLE2}}}"  at screen 0.5, screen 0.93 font '/root/ncc-fy4-project/extras/libsans-reg.ttf,18' center
#plot1
set origin 0.03,0.10
set size 0.95,0.8
unset key 
set xtics 
set ytics 
set x2tics
set format x "%.0f\260"
set format y "%.0f\260"
set format x2 "%.0f\260"
show grid
set grid front 
set xrange[20:190]
set x2range[20:190]
set yrange[-80:80]

#new
set xtics ("20\260E" 20, "40\260E" 40,"60\260E" 60,"80\260E" 80,"100\260E" 100,"120\260E" 120, "140\260E" 140,"160\260E" 160, "180\260" 180)
set x2tics ("20\260E" 20, "40\260E" 40,"60\260E" 60,"80\260E" 80,"100\260E" 100,"120\260E" 120, "140\260E" 140,"160\260E" 160, "180\260" 180)
set ytics ("-80\260S" -80, "-60\260S" -60,"-40\260S" -40,"-20\260S" -20,"0\260" 0,"20\260N" 20, "40\260N" 40,"60\260N" 60, "80\260N" 80)
set cbrange[50:450]


set format cb "%.0fW/m^2"
set pal maxcolors 16
set palette defined (  \
0 '#2b83ba', 6.25 '#2b83ba', \
6.25 '#4d96b4', 12.5 '#4d96b4', \
12.5 '#6fb3ae', 18.75 '#6fb3ae', \
18.75 '#91cba9', 25 '#91cba9', \
25 '#b1e09f', 31.25 '#b1e09f', \
31.25 '#c7e98b', 37.5 '#c7e98b', \
37.5 '#def277', 43.75 '#def277', \
43.75 '#f4fb63', 50 '#f4fb63', \
50 '#fff55a', 56.25 '#fff55a', \
56.25 '#ffdf5c', 62.5 '#ffdf5c', \
62.5 '#fec95e', 68.75 '#fec95e', \
68.75 '#feb460', 75 '#feb460', \
75 '#f69053', 81.25 '#f69053', \
81.25 '#ec6841', 87.5 '#ec6841', \
87.5 '#e2402e', 93.75 '#e2402e', \
93.75 '#d7191c', 100 '#d7191c'\
)
plot '{{{LONLATVALTXTFILE}}}' using 1:2:3 with image \
, '/root/ncc-fy4-project/extras/world_110m.txt' u 1:2 with lines lt 1 lc 'grey' \
, '/root/ncc-fy4-project/extras/china.dat' u 1:2 with lines lt 1 lc '#ccbbdd' 
#logos
set origin 0.75,-0.02
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
plot '/root/ncc-fy4-project/extras/cmalogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(0,0)  with rgbimage , '/root/ncc-fy4-project/extras/ncclogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(64,0.5)  with rgbimage 
unset multiplot
unset output
