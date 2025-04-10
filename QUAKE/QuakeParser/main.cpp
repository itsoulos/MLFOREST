#include <QCoreApplication>
# include <QFile>
# include <QTextStream>
# include <QDateTime>
# include <QDebug>
# include <stdio.h>
# include <math.h>

const double criticalDistance = 500;//500miles

struct QuakeRecord
{
    long seconds;
    double lat;
    double lon;
    double depth;
    int lith;
    int strat;
    int volcanic;
    int lava;
    int caldera;
    int complex;
    int compound;
    int shield;
    int pyro;
    int minor;
    int submarine;
    double magnitude;

    int countQuakesBefore=0;
    double meanQuakesBefore=0.0;
    double stdQuakesBefore=0.0;

    int magcode;
};
/**
 * @brief quake is the vector with all quake measurements.
 */
QVector<QuakeRecord> quake;
/**
 * @brief bigQuake is the vector with quakes having magcode==3
 */
QVector<QuakeRecord> bigQuake;

/**
 * @brief before
 * @param a
 * @param b
 * @return  true if quake a is before quake b.
 */
bool before(QuakeRecord &a,QuakeRecord &b)
{
    if(a.seconds < b.seconds) return true;
    return false;
}


double deg2rad(double deg) {
    return (deg * M_PI / 180.0); }

 double rad2deg(double rad) {
    return (rad * 180.0 / M_PI); }

double getDistance(QuakeRecord &a,QuakeRecord &b)
{
    double lon1=a.lon;
    double lon2 = b.lon;
    double lat1 = a.lat;
    double lat2 = b.lat;
    double theta = lon1 - lon2;
    double dist = sin(deg2rad(lat1))
                    * sin(deg2rad(lat2))
                    + cos(deg2rad(lat1))
                    * cos(deg2rad(lat2))
                    * cos(deg2rad(theta));
    dist = acos(dist);
    dist = rad2deg(dist);
    dist = dist * 60 * 1.1515;
    return (dist);
}

QVector<QuakeRecord> getQuakesBeforeMajor(QuakeRecord &major)
{
    QVector<QuakeRecord> list;
    for (QuakeRecord &x : quake)
    {
        if(x.magcode >= 3) continue;
        if(!before(x,major)) continue;
        double dist = getDistance(x,major);
        if(dist<criticalDistance)
            list.append(x);
    }
    return list;
}

QuakeRecord readLine(QString line)
{
    QStringList items = line.split(",");
    QuakeRecord x;
    int icount=0;
    int year = items[icount++].toInt();
    int month = items[icount++].toInt();
    int day = items[icount++].toInt();
    QString timeString = items[icount++];
    QStringList timeItems = timeString.split(":");
    int hours = timeItems[0].toInt();
    int mins  = timeItems[1].toInt();
    int secs  = timeItems[2].toInt();

    QDate tdate(year,month,day);
    QTime ttime(hours,mins,secs);
    QDateTime tdatetime(tdate,ttime);

    x.seconds = tdatetime.toSecsSinceEpoch();

    x.lat = items[icount++].toDouble();
    x.lon = items[icount++].toDouble();

    x.depth = items[icount++].toDouble();
    x.lith = items[icount++].toInt();
    x.strat = items[icount++].toInt();

    x.volcanic = items[icount++].toInt();
    x.lava = items[icount++].toInt();
    x.caldera = items[icount++].toInt();
    x.complex = items[icount++].toInt();

    x.compound = items[icount++].toInt();
    x.shield = items[icount++].toInt();

    x.pyro = items[icount++].toInt();
    x.minor = items[icount++].toInt();

    x.submarine = items[icount++].toInt();

    x.magnitude = items[icount++].toDouble();
    x.magcode = items[icount++].toInt();
    return x;
}


QString printQuake(const QuakeRecord &x)
{
    QString line="";
    //line+=QString::number(x.seconds)+" ";
    line+=QString::number(x.depth)+" ";
    line+=QString::number(x.lith)+" ";
    line+=QString::number(x.strat)+" ";
    line+=QString::number(x.volcanic)+" ";
    line+=QString::number(x.lava)+" ";
    line+=QString::number(x.caldera)+" ";
    line+=QString::number(x.complex)+" ";
    line+=QString::number(x.compound)+" ";
    line+=QString::number(x.shield)+" ";
    line+=QString::number(x.pyro)+" ";
    line+=QString::number(x.minor)+" ";
    line+=QString::number(x.submarine)+" ";
    if(x.countQuakesBefore!=0)
    {
        //information before major quake
        line+=QString::number(x.countQuakesBefore)+" ";
        line+=QString::number(x.meanQuakesBefore)+" ";
        line+=QString::number(x.stdQuakesBefore)+" ";
    }
    /** CLASSIFICATION PROBLEM
    if(x.magnitude>=6.0) line+=QString::number(6.0);
    else
    line+=QString::number(round(x.magnitude));
    **/
    /** REGRESSION PROBLEM **/
    line+=QString::number(x.magnitude);
    return line;
}

int countHeader(const QuakeRecord &x)
{
    QString line = printQuake(x);

    return line.count(" ");
}

double getMeanValueFromList(QVector<QuakeRecord> &list)
{
    double sum = 0.0;
    for(const QuakeRecord &x : list)
    {
        sum+=x.magnitude;
    }
    sum/=list.size();
    return sum;
}


double getStdValueFromList(QVector<QuakeRecord> &list)
{
    double m = getMeanValueFromList(list);
    double sum = 0.0;
    for(const QuakeRecord &x : list)
    {
        sum+=pow(x.magnitude-m,2.0);
    }
    sum =1.0/list.size() * sum;
    sum = sqrt(sum);
    return sum;
}

int main(int argc, char *argv[])
{
    if( argc==1)
        return fprintf(stderr,"File name not provided\n");
     QString filename = argv[1];
     QFile fp(filename);
     if(!fp.open(QIODevice::Text | QIODevice::ReadOnly))
         return fprintf(stderr,"File can not open \n");
     QTextStream st(&fp);


     QString line="";
     bool header=true;
     while(true)
     {
         line = st.readLine();
         if(line.size()==0)
             break;
         if(header)
         {
             header=false;
             continue;
         }
        QuakeRecord tt = readLine(line);
        quake.append(tt);
        if(tt.magcode>=3)
            bigQuake.append(tt);
     }
     fp.close();
     int icount=0;
     for(QuakeRecord & x : bigQuake)
     {
         QVector<QuakeRecord> list = getQuakesBeforeMajor(x);
         int count = list.size();
         double mean = getMeanValueFromList(list);
         double std = getStdValueFromList(list);
         x.countQuakesBefore = count;
         x.meanQuakesBefore = mean;
         x.stdQuakesBefore = std;
         icount++;
         double finishPercent =icount*100.0/bigQuake.size();
         if(icount%100==0)
             printf("Finished %.2lf%%\n",finishPercent);
     }

     for(const QuakeRecord &x : bigQuake)
     {
         QString line = printQuake(x);
         if(x.countQuakesBefore==0) continue;
         qDebug().noquote()<<line;
     }

     return 0;
}
