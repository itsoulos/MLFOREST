#!/bin/bash
CLASSPATH=/usr/share/java/weka.jar
export CLASSPATH
export LC_ALL=C
DATAFILE=$1
DATAPATH=./
rm -f $1.out


for i in {1..30}
do
  java weka.classifiers.trees.RandomForest  \
     -t $DATAPATH/$DATAFILE.train.arff -T $DATAPATH/$DATAFILE.test.arff  \
    -s $i \
    -i | awk '

    # --------- Macro Precision / Recall ----------
    /=== Detailed Accuracy By Class ===/ {flag=1; next}
    flag && /Weighted Avg\./ {flag=0}
    flag && NF > 0 && $1 !~ /TP/ {
        p += $4
        r += $5
        n++
    }

    # --------- Classification Error ----------
    /Incorrectly Classified Instances/ {
        err = $(NF-1)   # το ποσοστό πριν το %
    }

    END {
        if (n > 0)
            print p/n, r/n, err
    }'

done > results.txt
awk '{p+=$1; r+=$2; e+=$3} END {
  print "Avg Precision:", p/NR,
        "Avg Recall:", r/NR,
        "Avg Error:", e/NR
}' results.txt
