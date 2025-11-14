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
    double prev_lat;
    double prev_lon;
    double 	depth;
    double 	prevmag;
    int 	regioncode;
    QString 	datetime;
    int 	same_region;
    int kpcode;
    int 	lithcode;
    double 	magnitude;
    int depthcode;
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


const int daysBetween = 10;
long timeInterval= daysBetween * (24 * 60 *60);//secs

const  double minDistance = 100000.0;//100km
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
        record.epoch=list[lcount++].toInt();
        record.day=list[lcount++].toInt();
        record.time=list[lcount++].toInt();
        record.lat=list[lcount++].toDouble();
        record.prev_lat=list[lcount++].toDouble();
        record.lon=list[lcount++].toDouble();

        record.prev_lon=list[lcount++].toDouble();
        record.depthcode=list[lcount++].toInt();
        record.magnitude=list[lcount++].toDouble();
        record.prevmag=list[lcount++].toDouble();
        record.regioncode=list[lcount++].toInt();
        record.same_region=list[lcount++].toInt();
        record.lithcode=list[lcount++].toInt();
        record.datetime=list[lcount++];
        record.kpcode=list[lcount++].toInt();
	record.lat=list[lcount++].toDouble();
	double d1,d2;
	d1=list[lcount++].toDouble();
	record.lon=list[lcount++].toDouble();
	d2=list[lcount++].toDouble();
    if(record.magnitude<=1) continue;
    //if(record.magnitude==3)
	{
        int r = rand() %10;
		if(r!=0) continue;
    }
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

            double distance = latlondistance(
                originalQuake[i].lat,
                originalQuake[i].lon,
                originalQuake[j].lat,
                originalQuake[j].lon
                );
            QString t1 = originalQuake[i].datetime;
            QString t2 = originalQuake[j].datetime;
            long l1 =  makeDateInSecs(t1);
            long l2 =  makeDateInSecs(t2);

            if(i%1000==0) printf("running %d from %d \n",i,N);
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
/*
    for(int i=0;i<N;i++)
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
            if(distance<minDistance &&
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
    }
    */
    finalQuake = originalQuake;
    //sort
    for(int i=0;i<finalQuake.size();i++)
    {
        for(int j=0;j<finalQuake.size();j++)
        {
            if(i==j) continue;
            int t1=finalQuake[i].year;
            int t2=finalQuake[j].year;
            if(t1<t2)
            {
                Quake t=finalQuake[i];
                finalQuake[i]=finalQuake[j];
                finalQuake[j]=t;
            }
        }
    }
   QFile fout("gct.data");
    fout.open(QIODevice::WriteOnly);
    QTextStream s2(&fout);
    s2<<"14\n"<<finalQuake.size()<<"\n";
    for(int i=0;i<finalQuake.size();i++)
    {
            Quake record = finalQuake[i];
            double dclass;
            if(record.magnitude<=2.0) dclass=1;
            else  dclass=2;
            s2<<(record.year-1970.0)/(2025.0-1970.0)<<" "<<
            record.epoch<<" "<<
            record.day<<" "<<
            record.time<<" "<<
            //record.lat<<" "<<
            //record.lon<<" "<<
            record.depthcode<<" "<<
            //record.prevmag<<" "<<
            record.regioncode<<" "<<
            //record.same_region<<" "<<
            record.lithcode<<" "<<
            record.kpcode<<" "<<
            record.maxNearQuake<<" "<<
            record.timeBetweenMajorQuake/(24 * 60 *60.0)<<" "<<
            record.distanceFromMajorQuake/minDistance<<" "<<
                record.meanDistance/minDistance<<" "<<
                record.nearQuakesInThePast<<" "<<
                record.meanQuakesInThePast<<" "<<
            dclass<<"\n";
            /**
             * 		originalQuake[i].meanDistance= dminDistance/iminDistance;
            originalQuake[i].nearQuakesInThePast=iminDistance;
            originalQuake[i].meanQuakesInThePast=dminMagnitude/iminDistance;*/
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
    //11 cases
    //1970-1975
    //1976-1980
    //1981-1985
    //1986-1990
    //1991-1995
    //1996-2000
    //2001-2005
    //2006-2010
    //2011-2015
    //2016-2020
    //2021-2025
    for(int j=0;j<years.size();j++)
    {
	    int icount = 0;
        int startYear;
        int endYear;
        QString fname;
        if(years[j]>=1970 && years[j]<=1975)
        {
            startYear=1970;
            endYear=1975;
            fname = "gct1970.data";
        }
        else
            if(years[j]>=1976 && years[j]<=1980)
        {
            startYear=1976;
            endYear=1980;
            fname = "gct1975.data";
        }
        else
            if(years[j]>=1981 && years[j]<=1985)
        {
            startYear=1981;
            endYear=1985;
            fname = "gct1980.data";
        }

        else
            if(years[j]>=1986 && years[j]<=1990)
            {
                startYear=1986;
                endYear=1990;
                fname = "gct1985.data";
            }
            else
            if(years[j]>=1991 && years[j]<=1995)
        {
            startYear=1991;
            endYear=1995;
            fname = "gct1990.data";
            }
            else
            if(years[j]>=1996 && years[j]<=2000)
                {
                    startYear=1996;
                    endYear=2000;
                    fname = "gct1995.data";
                }

            else
                if(years[j]>=2001 && years[j]<=2005)
                {
                    startYear=2001;
                    endYear=2005;
                    fname = "gct2000.data";
                }
            else
            if(years[j]>=2006 && years[j]<=2010)
             {
                startYear=2006;
                endYear=2010;
                fname = "gct2005.data";
            }
            else
            if(years[j]>=2011 && years[j]<=2015)
                    {
                        startYear=2011;
                        endYear=2015;
                        fname = "gct2010.data";
                    }
            else
                if(years[j]>=2016 && years[j]<=2020)
                {
                    startYear=2016;
                    endYear=2020;
                    fname = "gct2015.data";
                }
                    else
                        if(years[j]>=2021 && years[j]<=2025)
                        {
                            startYear=2021;
                            endYear=2025;
                            fname = "gct2020.data";
                        }
	    for(int i=0;i<finalQuake.size();i++)
        {
            if(finalQuake[i].year >=startYear && finalQuake[i].year<=endYear)
                icount++;
        }

        QFile fout(fname);
        printf("years = %d \n",years[j]);
        if(years[j]==startYear)
        {
            fout.open(QIODevice::WriteOnly);
        }
        else
        {
            fout.open(QIODevice::Append);
        }
    	QTextStream s2(&fout);
        if(years[j]==startYear) s2<<"14\n"<<icount<<"\n";

    	for(int i=0;i<finalQuake.size();i++)
    	{
        	Quake record = finalQuake[i];
            double dclass;
            if(record.magnitude<=2.0) dclass=1;
            else  dclass=2;

            if(record.year!=years[j]) continue;
            s2<<
                ((record.year-1970.0)/(2025.0-1970.0))<<" "<<
            record.epoch<<" "<<
            record.day<<" "<<
            record.time<<" "<<
            //record.lat<<" "<<
            //record.lon<<" "<<
            record.depthcode<<" "<<
            //record.prevmag<<" "<<
            record.regioncode<<" "<<
            //record.same_region<<" "<<
            record.lithcode<<" "<<
            record.kpcode<<" "<<
            record.maxNearQuake<<" "<<
            record.timeBetweenMajorQuake/(24 *60.0 * 60.0)<<" "<<
            record.distanceFromMajorQuake/minDistance<<" "<<
                record.meanDistance/minDistance<<" "<<
                record.nearQuakesInThePast<<" "<<
                record.meanQuakesInThePast<<" "<<
            dclass<<"\n";
    	}
	fout.close();
    }
    return 0;
}
