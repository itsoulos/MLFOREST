#include <QCoreApplication>
# include <QFile>
# include <QTextStream>
# include <QDebug>
struct Quake
{
    int year;
    int epoch;
    int day;
    int time;
    double lat;
    double prev_lat;
    double lon;
    double prev_lon;
    int depthcode;
    int magcode;
    int prevmagcode;
    int regioncode;
    int days_since;
    int lithcode;
    QString datetime;
    int kpcode;
};

QVector<Quake> originalQuake;
QVector<Quake> finalQuake;
long timeInterval=0;//secs


long makeDateInSecs(QString t)
{
    QStringList alist=t.split(" ");
    QStringList dateList = alist[0].split("/");
    QStringList timeList = alist[1].split(":");
    long sum1 = dateList[0].toInt()* 24*60*60;
    long sum2 = (dateList[1].toInt())*30 * 24 *60 * 60;
    long sum3 = (dateList[2].toInt()-1970)*12 * 30 * 24 *60*60;
    long sum4 = timeList[0].toInt()*60*60;
    long sum5 = timeList[1].toInt()*60;
    return sum1+sum2+sum3+sum4+sum5;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

   setlocale(LC_ALL,"C");
    QString filename=argv[1];
    QFile fp(filename);
    fp.open(QIODevice::ReadOnly);
    QTextStream st(&fp);
    int icount=0;
    while(!st.atEnd())
    {
        QString line = st.readLine();
        if(icount==0) {icount=1; continue;}
        if(line.length()<1) continue;
        icount++;
        QStringList list=line.split(";");
        Quake record;
        record.year=list[0].toInt();
        record.epoch=list[1].toInt();
        record.day=list[2].toInt();
        record.time=list[3].toInt();
        record.lat=list[4].toDouble();
        record.prev_lat=list[5].toDouble();
        record.lon=list[6].toDouble();
        record.prev_lon=list[7].toDouble();
        record.depthcode=list[8].toInt();
        record.magcode=list[9].toInt();
        record.prevmagcode=list[10].toInt();
        record.regioncode=list[11].toInt();
        record.days_since=list[12].toInt();
        record.lithcode=list[13].toInt();
        record.datetime=list[14];
        record.kpcode=list[15].toInt();
        if(record.magcode<=3) continue;
        originalQuake<<record;
    }
    fp.close();

    int N=originalQuake.size();
    for(int i=0;i<N;i++)
    {
        long minDistanceinSecs =1000000;
        for(int j=0;j<finalQuake.size();j++)
        {
            QString t1 = originalQuake[i].datetime;
            QString t2 = finalQuake[j].datetime;
            long l1 = makeDateInSecs(t1);
            long l2 = makeDateInSecs(t2);
            if(abs(l1-l2)<minDistanceinSecs)
                minDistanceinSecs=abs(l1-l2);
        }
        if(minDistanceinSecs<timeInterval) {
   //         printf("Discard record %d  from %d \n",i,N);
            continue;
        }
        finalQuake.append(originalQuake[i]);
        if(finalQuake.size()%500==0)
        printf("Iteration: %d Now Size in final %d Original size: %d  \n",i,
                   finalQuake.size(),N);
    }

    QFile fout("output.data");
    fout.open(QIODevice::WriteOnly);
    QTextStream s2(&fout);
    s2<<"10\n"<<finalQuake.size()<<"\n";
    for(int i=0;i<finalQuake.size();i++)
    {
        Quake record = finalQuake[i];
        if(record.magcode>=5) record.magcode=5;
        s2<<(record.year-1970)*1.0/(2025.0-1970.0)<<" "<<record.epoch<<" "<<
            record.day<<" "<<record.time<<" "<<
            record.lat<<" "<<
            record.lon<<" "<<
            record.depthcode<<" "<<
            record.prevmagcode<<" "<<record.regioncode<<" "<<
            record.lithcode<<" "<<
            record.magcode<<"\n";

    }
    fout.close();
    return 0;
}
