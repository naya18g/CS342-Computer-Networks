set terminal png
set output "wiredTCPfairness.png"
set title "Fairness Index vs Packet Size"
set xlabel "Packet Size"
set ylabel "Fairness Index"

set xrange [0:1550]
plot "-"  title "Fairness Index Data" with linespoints
40 0.590428
44 0.582498
48 0.57655
52 0.579436
60 0.576561
250 0.551018
300 0.550465
552 0.547247
576 0.547729
628 0.54755
1420 0.545793
1500 0.545837
e
