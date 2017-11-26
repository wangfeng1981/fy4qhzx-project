

reset
#scatter
set terminal png size 800, 600
####################################################################
set output "E:/testdata/noaa-ndvi/out1009-xiawu/qc6mon005.scat.png"
unset key
###########
fxa=1.1979
###########
fxb=-0.0054
####################################
leq="fit eq:y=1.1979*x+(-0.0054)"
##############################
correlation="Corr coef:0.947589"
set label 1 leq at screen 0.1,0.90  
set label 2 correlation at screen 0.1,0.85  
##############################################################
plot 'E:/testdata/noaa-ndvi/out1009-xiawu/qc6mon005.scat.tmp' using 1:2 , fxa*x+fxb 
unset output

