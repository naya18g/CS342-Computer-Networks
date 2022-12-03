set terminal png
set output "WirelessThroughputPlot.png"
set title "Plot of Throughput against Packetsize"
set xlabel "Packet Size"
set ylabel "Throughput"

set xrange [0:1550]
plot "-"  title "Throughput" with linespoints
40 5.11014
44 5.2915
48 5.41927
52 5.57351
60 5.86009
552 24.4639
576 25.3252
628 27.2633
1420 56.9658
1500 59.6267
e
