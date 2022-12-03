set terminal png
set output "wiredTCPthroughput.png"
set title "Throughput vs Packet Size"
set xlabel "Packet Size"
set ylabel "Throughput"

set xrange [0:1550]
plot "-"  title "Throughput Data" with linespoints
40 1.02849
44 1.12054
48 1.202
52 1.3047
60 1.49945
250 5.92056
300 7.08738
552 12.9621
576 13.5358
628 14.7539
1420 30.575
1500 30.5465
e
