#include <QCoreApplication>
# include <QFile>
# include <QTextStream>
# include <QDebug>
# include <math.h>
struct Quake
{
    int 	year;
    int 	month;
    int 	epoch;
    int 	day;
    int 	time;
    double 	lat;
    double 	lon;
    double 	depth;
    double 	prevmag;
    int 	regioncode;
    QString 	datetime;
    int 	same_region;
    int 	lithcode;
    double 	magnitude;

    QString region;
    long timestamp;

    double      maxNearQuake=0.0;
    double      meanDistance=0.0;
    int         nearQuakesInThePast=0;
    double      meanQuakesInThePast=0.0;
    long        timeBetweenMajorQuake=0;
    double      distanceFromMajorQuake=0.0;
};

QVector<Quake> originalQuake;
QVector<Quake> finalQuake;


long timeInterval=2000;//secs

const  double minDistance = 5000.0;//10km
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
        if(rand()%20<19) continue;
        QStringList list=line.split(";");
        Quake record;
        int lcount=0;
        record.year=list[lcount++].toInt();
        record.month=list[lcount++].toInt();
        //record.epoch=list[lcount++].toInt();
        record.day=list[lcount++].toInt();
        record.datetime=list[lcount++];
        record.lat=list[lcount++].toDouble();
        record.lon=list[lcount++].toDouble();
        if(fabs(record.lat)>1000) record.lat/=10000.0;
        if(fabs(record.lon)>1000) record.lon/=10000.0;
        record.depth=list[lcount++].toDouble();
        //record.prevmag=list[lcount++].toDouble();
        record.magnitude=list[lcount++].toDouble();
        record.region=list[lcount++];
        record.timestamp=list[lcount++].toLong();

        //record.regioncode=list[lcount++].toInt();
        //record.same_region=list[lcount++].toInt();
        //record.datetime=list[lcount++];
        //record.lithcode=list[lcount++].toInt();

        //if(record.magnitude<=2) continue;
        originalQuake<<record;
    }
    fp.close();


    int N=originalQuake.size();
    for(int i=0;i<N;i++)
    {

        int iminDistance=0;
        double dminDistance=0.0;
        double dminMagnitude=0.0;
        originalQuake[i].maxNearQuake=0.0;
        originalQuake[i].timeBetweenMajorQuake=0;
        originalQuake[i].distanceFromMajorQuake=0.0;
        for(int j=0;j<N;j++)
        {
            if(i==j) continue;
	    int a = 1970 + originalQuake[i].timestamp / 31537970;
            double distance = latlondistance(
                originalQuake[i].lat/10000,
                originalQuake[i].lon/10000,
                originalQuake[j].lat/10000,
                originalQuake[j].lon/10000
                );
            QString t1 = originalQuake[i].datetime;
            QString t2 = originalQuake[j].datetime;
            long l1 = originalQuake[i].timestamp;// makeDateInSecs(t1);
            long l2 = originalQuake[j].timestamp;// makeDateInSecs(t2);
            if(i%100==0) printf("running %d from %d \n",i,N);
            if(l2>l1) continue;
            if(distance<minDistance)
            {
                if(originalQuake[j].magnitude>originalQuake[i].maxNearQuake)
                {
                    originalQuake[i].maxNearQuake=originalQuake[j].magnitude;
                    originalQuake[i].timeBetweenMajorQuake=l1-l2;
                    originalQuake[i].distanceFromMajorQuake=distance;
                }

                iminDistance++;
                dminDistance+=distance;
                dminMagnitude+=originalQuake[j].magnitude;
            }
        }
        if(iminDistance>0)
        {
		originalQuake[i].meanDistance= dminDistance/iminDistance;
            originalQuake[i].nearQuakesInThePast=iminDistance;
            originalQuake[i].meanQuakesInThePast=dminMagnitude/iminDistance;
        }
        else
        {
		originalQuake[i].meanDistance= 0.0;
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
    s2<<"11\n"<<finalQuake.size()<<"\n";
    for(int i=0;i<finalQuake.size();i++)
    {
        Quake record = finalQuake[i];
        s2<<(record.year-1970)*1.0/(2025.0-1970.0)<<" "<<
            record.month/12.0<<" "<<
            record.day/31.0<<" "<<
            record.lat<<" "<<
            record.lon<<" "<<
            record.depth<<" "<<
            record.nearQuakesInThePast<<" "<<
            record.meanQuakesInThePast<<"  "<<
            record.maxNearQuake<<" "<<
            record.timeBetweenMajorQuake*1.0/(3600.0 * 1000.0)<<" "<<
            record.distanceFromMajorQuake/minDistance<<" "<<
            (round(record.magnitude)>=3.0?1.0:0.0)<<"\n";
    }
    fout.close();
    //years
    QVector<int> years;
    for(int i=0;i<finalQuake.size();i++)
    {
        Quake record = finalQuake[i];
	int year = record.year;
	bool found=false;
	for(int j=0;j<years.size();j++)
	{
		if(years[j]==year) {found=true;break;}
	}
	if(!found) years<<year;
    }
    for(int j=0;j<years.size();j++)
    {
	    int icount = 0;
	    for(int i=0;i<finalQuake.size();i++)
		    if(finalQuake[i].year == years[j])icount++;
   	QFile fout(QString("quake")+QString::number(years[j])+".data");
    	fout.open(QIODevice::WriteOnly);
    	QTextStream s2(&fout);
    	s2<<"10\n"<<icount<<"\n";
    	for(int i=0;i<finalQuake.size();i++)
    	{
        	Quake record = finalQuake[i];
		if(record.year!=years[j]) continue;
            s2<<record.month/12.0<<" "<<
            record.day/31.0<<" "<<
            record.lat<<" "<<
            record.lon<<" "<<
            record.depth<<" "<<
            record.nearQuakesInThePast<<" "<<
            record.meanQuakesInThePast<<"  "<<
            record.maxNearQuake<<" "<<
            record.timeBetweenMajorQuake*1.0/(3600.0 * 1000.0)<<" "<<
            record.distanceFromMajorQuake/minDistance<<" "<<
            (round(record.magnitude)>=3?1.0:0.0)<<"\n";
    	}
	fout.close();
    }
    return 0;
}
