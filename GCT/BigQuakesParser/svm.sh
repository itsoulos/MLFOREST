CLASSPATH=/usr/share/java/weka.jar:/usr/share/java/libsvm.jar
export CLASSPATH
DATAFILE=$1
java -Xmx4096m  weka.classifiers.functions.LibSVM -t $1 


