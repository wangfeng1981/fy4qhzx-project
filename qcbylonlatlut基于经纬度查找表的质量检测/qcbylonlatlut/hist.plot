#输出直方图
reset

set terminal pngcairo size 800,600
set output "hist-test.png"

set xrange [-5:5] 
binwidth=5
plot 'fy4sst0807.NC.biashist.txt' smooth freq with boxes

unset output