#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
# include <stdio.h>
# include <math.h>
using namespace std;

struct Entry {
    double score;
    int label;
};

double computeROC(const vector<int>& y_true, const vector<double>& y_score,
                  vector<double>& fpr, vector<double>& tpr)
{
    int P = count(y_true.begin(), y_true.end(), 1);
    int N = y_true.size() - P;

    vector<Entry> data;
    for (size_t i = 0; i < y_true.size(); i++)
        data.push_back({y_score[i], y_true[i]});

    sort(data.begin(), data.end(),
         [](const Entry& a, const Entry& b){ return a.score > b.score; });

    int TP = 0, FP = 0;
    fpr.clear();
    tpr.clear();

    fpr.push_back(0);
    tpr.push_back(0);

    for (auto& e : data) {
        if (e.label == 1) TP++;
        else FP++;

        fpr.push_back(FP / (double)N);
        tpr.push_back(TP / (double)P);
    }

    double auc = 0.0;
    for (size_t i = 1; i < fpr.size(); i++)
        auc += (fpr[i] - fpr[i-1]) * (tpr[i] + tpr[i-1]) / 2.0;

    return auc;
}

double computePR(const vector<int>& y_true, const vector<double>& y_score,
                 vector<double>& precision, vector<double>& recall)
{
    int P = count(y_true.begin(), y_true.end(), 1);

    vector<Entry> data;
    for (size_t i = 0; i < y_true.size(); i++)
        data.push_back({y_score[i], y_true[i]});

    sort(data.begin(), data.end(),
         [](const Entry& a, const Entry& b){ return a.score > b.score; });

    int TP = 0, FP = 0;
    precision.clear();
    recall.clear();

    for (auto& e : data) {
        if (e.label == 1) TP++;
        else FP++;

        precision.push_back(TP / (double)(TP + FP));
        recall.push_back(TP / (double)P);
    }

    double ap = 0.0;
    for (size_t i = 1; i < precision.size(); i++)
        ap += precision[i] * (recall[i] - recall[i-1]);

    return ap;
}

int main(int argc,char **argv)
{

	//5 methods
    FILE *fp=fopen(argv[1],"r");
    if(!fp) return 1;
    vector<int> y_true1,y_true2,y_true3,y_true4,y_true5 ;
    vector<double> y_score1,y_score2,y_score3,y_score4,y_score5 ;


    double x[6];
    while(fscanf(fp,"%lf %lf %lf %lf %lf %lf",&x[0],&x[1],&x[2],&x[3],&x[4],&x[5])>1)
    {
	    int ac;
	    ac = (int)x[0];
	    y_true1.push_back(ac);
	    y_score1.push_back(x[1]);
	    ac=(int)x[2];
	    y_true2.push_back(ac);
	    y_score2.push_back(x[3]);
	    ac=(int)x[4];
	    y_true3.push_back(ac);
	    y_score3.push_back(x[5]);
	
	 }

    fclose(fp);

    //method - 1 
    vector<double> fpr1, tpr1, prec1, rec1;
    double roc_auc1 = computeROC(y_true1, y_score1, fpr1, tpr1);
    double pr_auc1 = computePR(y_true1, y_score1, prec1, rec1);
    // Write ROC points
    ofstream rocfile1("roc1.dat");
    for (size_t i = 0; i < fpr1.size(); i++)
        rocfile1 << fpr1[i] << " " << tpr1[i] << "\n";
    rocfile1.close();
    // Write PR points
    ofstream prfile1("pr1.dat");
    for (size_t i = 0; i < prec1.size(); i++)
        prfile1 << rec1[i] << " " << prec1[i] << "\n";
    prfile1.close();

    //method - 2 
    vector<double> fpr2, tpr2, prec2, rec2;
    double roc_auc2 = computeROC(y_true2, y_score2, fpr2, tpr2);
    double pr_auc2 = computePR(y_true2, y_score2, prec2, rec2);
    // Write ROC points
    ofstream rocfile2("roc2.dat");
    for (size_t i = 0; i < fpr2.size(); i++)
        rocfile2 << fpr2[i] << " " << tpr2[i] << "\n";
    rocfile2.close();
    // Write PR points
    ofstream prfile2("pr2.dat");
    for (size_t i = 0; i < prec2.size(); i++)
        prfile2 << rec2[i] << " " << prec2[i] << "\n";
    prfile2.close();

    //method - 3 
    vector<double> fpr3, tpr3, prec3, rec3;
    double roc_auc3 = computeROC(y_true3, y_score3, fpr3, tpr3);
    double pr_auc3 = computePR(y_true3, y_score3, prec3, rec3);
    // Write ROC points
    ofstream rocfile3("roc3.dat");
    for (size_t i = 0; i < fpr3.size(); i++)
        rocfile3 << fpr3[i] << " " << tpr3[i] << "\n";
    rocfile3.close();
    // Write PR points
    ofstream prfile3("pr3.dat");
    for (size_t i = 0; i < prec3.size(); i++)
        prfile3 << rec3[i] << " " << prec3[i] << "\n";
    prfile3.close();


    // Create gnuplot script
    ofstream gp("plot.gp");
    gp << "set terminal png size 900,700\n";
    gp <<"set key bottom right\n";
    gp << "set output 'roc.png'\n";
    gp << "set title 'TURKEY - ROC Curves \n";
    gp << "set xlabel 'False Positive Rate'\n";
    gp << "set ylabel 'True Positive Rate'\n";
    gp << 
	    "plot 'roc1.dat' with lines lw 2 title 'RBF(AUC="<<pr_auc1<<")'"<<
	    ",'roc2.dat' with lines lw 2 title 'NNC(AUC="<<pr_auc2<<")'"<<
	    ",'roc3.dat' with lines lw 2 title 'GENCLASS(AUC="<<pr_auc3<<")'"<<
	    "\n\n";

    gp << "set output 'pr.png'\n";
    gp <<"set key bottom right\n";
    gp << "set title 'TURKEY - Precision-Recall Curve \n";
    gp << "set xlabel 'Recall'\n";
    gp << "set ylabel 'Precision'\n";
    gp << 
	    "plot 'pr1.dat' with lines lw 2 title 'RBF(AP="<<pr_auc1<<")'"<<
	    ",'pr2.dat' with lines lw 2 title 'NNC(AP="<<pr_auc2<<")'"<<
	    ",'pr3.dat' with lines lw 2 title 'GENCLASS(AP="<<pr_auc3<<")'"<<
	    "\n\n";
    gp.close();

    cout << "Data saved to roc.dat, pr.dat\n";
    cout << "Run: gnuplot plot.gp\n";

    return 0;
}
