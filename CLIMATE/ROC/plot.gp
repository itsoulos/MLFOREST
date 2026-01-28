set terminal png size 900,700
set key bottom right
set output 'roc.png'
set title 'TURKEY - ROC Curves 
set xlabel 'False Positive Rate'
set ylabel 'True Positive Rate'
plot 'roc1.dat' with lines lw 2 title 'RBF(AUC=0.903692)','roc2.dat' with lines lw 2 title 'NNC(AUC=0.912654)','roc3.dat' with lines lw 2 title 'GENCLASS(AUC=0.990938)'

set output 'pr.png'
set key bottom right
set title 'TURKEY - Precision-Recall Curve 
set xlabel 'Recall'
set ylabel 'Precision'
plot 'pr1.dat' with lines lw 2 title 'RBF(AP=0.903692)','pr2.dat' with lines lw 2 title 'NNC(AP=0.912654)','pr3.dat' with lines lw 2 title 'GENCLASS(AP=0.990938)'

