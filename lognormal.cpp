/****************************************************
	����N=100���ڣ�0,50�����������������̬�ֲ��������
/*****************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <time.h>
using namespace std;

#define N 1000 
#define MAX 180
#define MIN 0.1
#define MIU 2.76695
#define SIGMA 1.18678
#define PI 3.14159

int RandomFLAG = 1;

double AverageRandom(double min,double max)//����(min,max)֮����ȷֲ��������
{
        int MINnteger = (int)(min*10000);
        int MAXnteger = (int)(max*10000);
        int randInteger = rand()*rand();
        int diffInteger = MAXnteger - MINnteger;
        int resultInteger = randInteger % diffInteger + MINnteger;
        return resultInteger/10000.0;
}
double LogNormal(double x,double miu,double sigma) //������̬�ֲ������ܶȺ���
{
        return 1.0/(x*sqrt(2*PI)*sigma) * exp(-1*(log(x)-miu)*(log(x)-miu)/(2*sigma*sigma));
}
double Random_LogNormal(double miu,double sigma,double min,double max)//����������̬�ֲ������
{
        double x;
        double dScope;
        double y;
		if(!RandomFLAG)
			return 0;

        do
        {
        x = AverageRandom(min,max); 
        y = LogNormal(x, miu, sigma);
        dScope = AverageRandom(0, LogNormal(miu,miu,sigma));
        }while( dScope > y);
        return x;
}
double randomExponential(double lambda)
{
	double pV = 0.0;
	while(true)
	{
		pV = (double)rand()/(double)RAND_MAX;
		if (pV != 1)
		{
			break;
		}
	}
	pV = (-1.0/lambda)*log(1-pV);
	return pV;
}
//void main(void)
//{
//	srand(time(NULL));
//	int i,j;
//	double  n;
//	ofstream ofile;
//	ofile.open("A:/lognormal.txt",ios::app);
//	for(i=0,j=0;i<N;i++)
//	{
//		printf("%14f", n=Random_LogNormal(MIU,SIGMA,MIN,MAX));
//		j++;
//		ofile<<n<<endl;
//		if(j==5)
//		{
//			
//			printf("\n");	//ÿ����ʾ5����
//
//			j=0;
//		}
//
//	}
//	ofile.close();
//}
