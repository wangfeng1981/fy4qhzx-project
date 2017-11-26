
#fyhist
set terminal png size 800, 600
##########################
set output "fy5.png"
set xrange[-0.3:1.0]
set xlabel "FY3B-NDVI"
set ylabel "percent(%)"
##########################
plot 'm5b.tif.ihist.tmp' using 1:2 smooth freq with boxes
unset output

#fy3corrhist
set terminal png size 800, 600
##########################
set output "fy5c.png"
set xrange[-0.3:1.0]
set xlabel "FY3B-NDVI(corrected)"
set ylabel "percent(%)"
##########################
plot 'm5cb.tif.ihist.tmp' using 1:2 smooth freq with boxes
unset output

#noaahist
set terminal png size 800, 600
##########################
set output "noaa5.png"
set xrange[-0.3:1.0]
set xlabel "AVHRR-NDVI"
set ylabel "percent(%)"
##########################
plot 'm5b.tif.rhist.tmp' using 1:2 smooth freq with boxes
unset output

#biashist
set terminal png size 800, 600
##########################
set output "bias5.png"
set xrange[-0.5:0.5]
set xlabel "FY3B vs AVHRR NDVI bias"
set ylabel "percent(%)"
##########################
plot 'm5b.tif.bhist.tmp' using 1:2 smooth freq with boxes
unset output


#corrected biashist
set terminal png size 800, 600
##########################
set output "cbias5.png"
set xrange[-0.5:0.5]
set xlabel "FY3B(corrected) vs AVHRR NDVI bias"
set ylabel "percent(%)"
##########################
plot 'm5cb.tif.bhist.tmp' using 1:2 smooth freq with boxes
unset output







