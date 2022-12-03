set terminal png
set output "WirelessFairnessPlot.png"
set title "Plot of TCP Fairness Index against Packet Size"
set xlabel "Packet Size"
set ylabel "Fairness Index"

set xrange [0:1550]
plot "-"  title "Index Of Fairness " with linespoints
40 0.768203
44 0.760125
48 0.752639
52 0.745692
60 0.730718
552 0.549224
576 0.547819
628 0.543996
1420 0.520618
1500 0.51957
e
