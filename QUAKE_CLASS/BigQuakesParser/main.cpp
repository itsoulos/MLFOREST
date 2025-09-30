#include <QCoreApplication>
# include <QFile>
# include <QTextStream>
# include <QDebug>
# include <math.h>
struct Quake
{
    int year;
    int month;
    int epoch;
    int day;
    int time;
    double lat;
    double lon;
    int depth;
    double prevmag;
    int regioncode;
    QString datetime;
    int same_region;
    int lithcode;
    double magnitude;

    int nearQuakesInThePast=0;
    double meanQuakesInThePast=0.0;
};

QVector<Quake> originalQuake;
QVector<Quake> finalQuake;


long timeInterval=2000;//secs


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


double latlondistance(double lat1,double lon1,double lat2,double lon2)
{
    const double R = 6371*1000;
    double phi1 = lat1 * M_PI/180; // φ, λ in radians
    double  phi2 = lat2 * M_PI/180;
    double df=(lat2-lat1) * M_PI/180;
    double dl=(lon2-lon1) * M_PI/180;
    double a = sin(df/2) * sin(df/2) +
               cos(phi1) * cos(phi2) *
                   sin(dl/2) * sin(dl/2);
    double c =  2 * atan2(sqrt(a), sqrt(1-a));
    double d = R *c;
    return d;
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
        int lcount=0;
        record.year=list[lcount++].toInt();
        record.month=list[lcount++].toInt();
        record.epoch=list[lcount++].toInt();
        record.day=list[lcount++].toInt();
        record.time=list[lcount++].toInt();
        record.lat=list[lcount++].toDouble();
        record.lon=list[lcount++].toDouble();
        record.depth=list[lcount++].toInt();
        record.prevmag=list[lcount++].toDouble();
        record.regioncode=list[lcount++].toInt();
        record.same_region=list[lcount++].toInt();
        record.datetime=list[lcount++];
        record.lithcode=list[lcount++].toInt();
        record.magnitude=list[lcount++].toDouble();
        if(record.magnitude<=4) continue;
        originalQuake<<record;
    }
    fp.close();


    int N=originalQuake.size();
    for(int i=0;i<N;i++)
    {
        double minDistance = 50000.0;//50km
        int iminDistance=0;
        double dminDistance=0.0;
        double dminMagnitude=0.0;
        for(int j=0;j<N;j++)
        {
            if(i==j) continue;
            double distance = latlondistance(
                originalQuake[i].lat/10000,
                originalQuake[i].lon/10000,
                originalQuake[j].lat/10000,
                originalQuake[j].lon/10000
                );
            QString t1 = originalQuake[i].datetime;
            QString t2 = originalQuake[j].datetime;
            long l1 = makeDateInSecs(t1);
            long l2 = makeDateInSecs(t2);
            if(i%100==0) printf("running %d \n",i);
            if(l2>l1) continue;
            if(distance<minDistance)
            {
                iminDistance++;
                dminDistance+=distance;
                dminMagnitude+=originalQuake[j].magnitude;
            }
        }
        if(iminDistance>0)
        {
            originalQuake[i].nearQuakesInThePast=iminDistance;
            originalQuake[i].meanQuakesInThePast=dminMagnitude/iminDistance;
        }
        else
        {
            originalQuake[i].nearQuakesInThePast=0;
            originalQuake[i].meanQuakesInThePast=0.0;
        }

    }
   /* for(int i=0;i<N;i++)
    {
        long minDistanceinSecs =1000000;
        for(int j=0;j<finalQuake.size();j++)
        {
            QString t1 = originalQuake[i].datetime;
            QString t2 = finalQuake[j].datetime;
            long l1 = makeDateInSecs(t1);
            long l2 = makeDateInSecs(t2);

            double distance = latlondistance(
                originalQuake[i].lat,
                originalQuake[i].lon,
                finalQuake[j].lat,
                finalQuake[j].lon
                );
           // qDebug()<<"Distance = "<<distance;
            if(distance<1000 &&
            abs(l1-l2)<minDistanceinSecs)
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
    }*/
    finalQuake = originalQuake;

   QFile fout("output.data");
    fout.open(QIODevice::WriteOnly);
    QTextStream s2(&fout);
    s2<<"13\n"<<finalQuake.size()<<"\n";
    for(int i=0;i<finalQuake.size();i++)
    {
        Quake record = finalQuake[i];
        s2<<(record.year-1970)*1.0<<" "<<
            record.epoch<<" "<<
            record.day<<" "<<
            record.time<<" "<<
            record.lat/10000.0<<" "<<
            record.lon/10000.0<<" "<<
            record.depth<<" "<<
            (round(record.prevmag)>=6.0?6.0:round(record.prevmag))<<" "<<
            record.regioncode<<" "<<
            record.same_region<<" "<<
            record.lithcode<<" "<<
            record.nearQuakesInThePast<<" "<<
            record.meanQuakesInThePast<<"  "<<
            (round(record.magnitude)>=6.0?6.0:round(record.magnitude))<<"\n";
    }
    fout.close();
    return 0;
}
