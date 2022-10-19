#include <string.h>
#include <vector>
#include <iostream>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <fstream>
#include "Traindir3.h"
#include "T_V.h"
using namespace std;
//#define  TractionAcc 1  
//#define  BrakingAcc  1
#define  MAX_SPEED 350
//double ProtectCurveSpeed[100000];
extern int twoByte2single(char *msg,unsigned short buff,int index);
extern vector<int> SpeedLimitStop;
extern vector<int> SpeedLimitValue;
extern vector<int> SpeedStart;
extern vector<int> SpeedEnd;
extern  long    current_time;

double  Func_s(double RunningTime, int RunningDistance, double TrainInitialSpeed, double TrainTerminalSpeed, int MainLineNo, Train *t);

        /// <summary>
        /// 生成站间曲线
        /// 输入参数：运行时间  RunningTime(s)
        /// 输入参数：运行距离 RunningDistance(m)
        /// 输入参数：初始速度TrainInitialSpeed (km/h)
        /// 输入参数：终点速度TrainTerminalSpeed (km/h)
        /// </summary> 
        struct ListNode
         {
			 int pos;
			 double SecPerMeter;
             double speed;
			 double totaltime;
             ListNode *next;
         };
		//struct Limit{
		//	int SpeedLimitValue;
		//	int StartPosition;
		//	int EndPosition;
		//	int MainLineNo;
		//};

         double  Func_s(double RunningTime, int RunningDistance, double TrainInitialSpeed, double TrainTerminalSpeed, int MainLineNo, Train *t)
		{
			int AccFlag = 1;
			double TempS =0, TempT = 0,TractionAcc = 1,BrakingAcc =1;
			if(wxStrcmp(t->pre_stop, t->entrance))//lrg 20211101
				delete[] t->CalculatedSpeedCurveValue;

            //////////////////计算平滑防护曲线（开始）//////////////
            vector<int> ListProtectCurveStart;
            vector<int> ListProtectCurveEnd;
			vector<int> ListCombinedSpeedLimitValue;
			vector<int> ListCombinedSpeedStart;
			vector<int> ListCombinedSpeedEnd;
			//double  *ProtectCurveSpeed;
			double  *ProtectCurveSpeed;
			ProtectCurveSpeed = new double[RunningDistance];
			//	} catch ( const bad_alloc& e ) {
			//		  return -1;
			//	}


			for(int i = 0;i<SpeedLimitValue.size();i++){
				if(SpeedLimitStop[i] == t->position->km){
				ListCombinedSpeedLimitValue.push_back(SpeedLimitValue[i]);
				ListCombinedSpeedStart.push_back(SpeedStart[i]);
				ListCombinedSpeedEnd.push_back(SpeedEnd[i]);
				}
			}
            vector<int> ListProtectCurvePosition(ListCombinedSpeedLimitValue.size());
            int *SpeedLimitBuffered;
			SpeedLimitBuffered = new int[ListCombinedSpeedLimitValue.size()];
            for (int i = 0; i < RunningDistance; i++)
            {
                ProtectCurveSpeed[i] = 1000;
            }
            copy(ListCombinedSpeedLimitValue.begin(), ListCombinedSpeedLimitValue.end(), SpeedLimitBuffered);
            for (unsigned int i = 0; i < ListCombinedSpeedLimitValue.size(); i++)
            {
                SpeedLimitBuffered[i] = SpeedLimitBuffered[i] - 3;
            }
            //计算停车速度曲线
                ListProtectCurveEnd.push_back(RunningDistance);  //记录结束位置
                double CurrentSpeed = TrainTerminalSpeed;
                int CurrentPostion = RunningDistance;
                ProtectCurveSpeed[CurrentPostion-1] = CurrentSpeed;
                int FlagStop = 0;
            while (FlagStop == 0)
            {
				
                CurrentSpeed = sqrt(CurrentSpeed / 3.6 * CurrentSpeed / 3.6 + 2 * BrakingAcc) * 3.6;//v=sqrt(v^2+2*a)
                CurrentPostion = CurrentPostion - 1;
		
				if (CurrentPostion == 0)  //没有找到结束点，则线路起始点为结束点 lrg 20211031由while最后几行移过来的，逻辑修改
                {
                    ListProtectCurveStart.push_back(0);
                    FlagStop = 1;
					break;//lrg 20211031
                }
                //ListProtectCurveSpeed.Add(CurrentSpeed);
                //ListProtectCurvePosition.Add(CurrentPostion);
                ProtectCurveSpeed[CurrentPostion - 1] = CurrentSpeed;
                for (int k = 0; k < ListCombinedSpeedLimitValue.size(); k++)
                {
                    if (CurrentPostion >= ListCombinedSpeedStart[k] && CurrentPostion < ListCombinedSpeedEnd[k]
                            && CurrentSpeed - SpeedLimitBuffered[k] > -0.01)
                    {
                        FlagStop = 1;
                        ListProtectCurveStart.push_back(CurrentPostion);
                        break;
                    }

                }


            }

            for (int i = ListCombinedSpeedLimitValue.size()-1; i >= 0; i--) //倒推计算限速防护曲线
            { 
                //if (SpeedLimitBuffered[i] < SpeedLimitBuffered[i - 1])//以下6行暂时注释，不理解
                //{
                //    if (ListProtectCurveEnd.size() != 0 && ProtectCurveSpeed[ListCombinedSpeedStart[i]] < SpeedLimitBuffered[i])  //判断上一防护曲线是否低于当前速度，如果是则i--
                //    {
                //        continue;
                //    }
                    ListProtectCurveEnd.push_back(ListCombinedSpeedStart[i]);  //记录结束位置

                    //ListProtectCurveSpeed.Add(SpeedLimitBuffered[i]);
                    //ListProtectCurvePosition.Add(ListCombinedSpeedStart[i]);
                    CurrentSpeed = SpeedLimitBuffered[i];
                    CurrentPostion = ListCombinedSpeedStart[i];
                    ProtectCurveSpeed[CurrentPostion] = CurrentSpeed;
                    FlagStop = 0;
                    while (FlagStop == 0)
                    {
						
                        CurrentSpeed = sqrt(CurrentSpeed / 3.6 * CurrentSpeed / 3.6 + 2 * BrakingAcc) * 3.6;//v=sqrt(v^2+2*a)
                        CurrentPostion = CurrentPostion - 1;
                        //ListProtectCurveSpeed.Add(CurrentSpeed);
                        //ListProtectCurvePosition.Add(CurrentPostion);
                        ProtectCurveSpeed[CurrentPostion] = CurrentSpeed;
                        for (int k = 0; k < ListCombinedSpeedLimitValue.size(); k++)
                        {
                            if (CurrentPostion >= ListCombinedSpeedStart[k] && CurrentPostion < ListCombinedSpeedEnd[k]
                                    && CurrentSpeed - SpeedLimitBuffered[k] > -0.01)
                            {
                                FlagStop = 1;
                                ListProtectCurveStart.push_back(CurrentPostion);
                                break;
                            }

                        }
                        if (CurrentPostion == 0)  //没有找到结束点，则线路起始点为结束点
                        {
                            ListProtectCurveStart.push_back(0);
                            FlagStop = 1;
                        }

                    }
                

			}

            //限速值扩展到每个点
            int *SpeedLimitWhole;
			SpeedLimitWhole = new int[RunningDistance];
			for (int i = 0; i < RunningDistance; i++)
            {
                SpeedLimitWhole[i] = 1000;
            }
            for (int i = 0; i < RunningDistance; i++)
            {
                for (int j = 0; j < ListCombinedSpeedLimitValue.size(); j++)
                {
                    if (i >= ListCombinedSpeedStart[j] && i < ListCombinedSpeedEnd[j])
                    {
                        SpeedLimitWhole[i] = SpeedLimitBuffered[j];
                    }
                }

            }

            //////////////////计算平滑防护曲线(结束)//////////////
            double CoastingSpeedLow = RunningDistance / RunningTime * 3.6;  //巡航速度设定为平均速度，最终的巡航速度会高于平均速度km/h
            double CoastingSpeedHigh = MAX_SPEED;//*max_element(SpeedLimitBuffered,SpeedLimitBuffered + ListCombinedSpeedLimitValue.size());
            double CoastingSpeedAverage = 0;
            double *CalculatedSpeedCurveValue;
			try {
             CalculatedSpeedCurveValue= new double[RunningDistance+1];
			} catch ( const bad_alloc& e ) {
				  return -1;
			}
			/*CalculatedSpeedCurveValue = new double[RunningDistance+1];*/
            double *CalculatedSecondPerMeter;
			CalculatedSecondPerMeter =new double[RunningDistance+1];
			double *TotalTime;
			TotalTime =new double[RunningDistance+1];

			vector<int> acc_turn;   //加速度（未用）
			vector<int> s_turn;
            int Flag = 0;
            double CalculatedRunningTime = 0;
			
			/*if(AccFlag == -1){
			 CoastingSpeedHigh = TrainInitialSpeed;
			 CoastingSpeedLow = 10;
			}*/

			if(CoastingSpeedLow >= CoastingSpeedHigh)
				CoastingSpeedLow = CoastingSpeedHigh-0.011;


            while (CoastingSpeedHigh - CoastingSpeedLow > 0.01)
            {
				s_turn.clear();
				acc_turn.clear();
				s_turn.push_back(0);
				acc_turn.push_back(1);
				
                //Console.WriteLine(CoastingSpeedHigh - CoastingSpeedLow);
                CalculatedRunningTime = 0;
                if (Flag == 0)
                {
                    CoastingSpeedAverage = CoastingSpeedLow;
                    Flag = 1;
                }
                else if (Flag == 1)
                {
                    CoastingSpeedAverage = CoastingSpeedHigh;
                    Flag = 2;
                }
                else if (Flag > 1)
                {
                    CoastingSpeedAverage = (CoastingSpeedLow + CoastingSpeedHigh) / 2;
                    Flag = 3;
                }
				if(CoastingSpeedLow<=0 || CoastingSpeedHigh<=0)
					CoastingSpeedAverage = CoastingSpeedLow = CoastingSpeedHigh = MAX_SPEED;

                CalculatedSpeedCurveValue[0] = TrainInitialSpeed; //速度初始值为0
                CalculatedSecondPerMeter[0] = 0;
				TotalTime[0] = 0;
                for (int i = 0; i < RunningDistance ; i++)
                {

                    if (CalculatedSpeedCurveValue[i] < SpeedLimitWhole[i] && CalculatedSpeedCurveValue[i] < ProtectCurveSpeed[i])
                    {
                        if (CalculatedSpeedCurveValue[i] < CoastingSpeedAverage)
                        {
							//TractionAcc = 0.0002425*2*CalculatedSpeedCurveValue[i]+0.5;
                            //CalculatedSpeedCurveValue[i + 1] = sqrt(CalculatedSpeedCurveValue[i] * CalculatedSpeedCurveValue[i] / 3.6 / 3.6 + 2 * TractionAcc) * 3.6;
							TempS = F_ACC_VS(CalculatedSpeedCurveValue[i],CalculatedSpeedCurveValue[i]+1); 
							TractionAcc = (CalculatedSpeedCurveValue[i]*2 + 1)/(3.6*3.6*2*TempS);

							CalculatedSpeedCurveValue[i + 1] = sqrt(CalculatedSpeedCurveValue[i] / 3.6 * CalculatedSpeedCurveValue[i] / 3.6 + 2 * TractionAcc) * 3.6;
                            CalculatedSecondPerMeter[i + 1] = 1 / (CalculatedSpeedCurveValue[i + 1] / 3.6);
                            CalculatedRunningTime = CalculatedRunningTime + 1 / (CalculatedSpeedCurveValue[i + 1] / 3.6);
							TotalTime[i+1] = CalculatedRunningTime;
							if(acc_turn.back() != 1){
							acc_turn.push_back(1);
							s_turn.push_back(i);
							}
							
                            //Console.WriteLine(CalculatedSpeedCurveValue[i + 1]);
                        }
						else if(CalculatedSpeedCurveValue[i] >  CoastingSpeedAverage)
						{
							TempS = F_DEC_VS(CalculatedSpeedCurveValue[i],CalculatedSpeedCurveValue[i]-1); 
							TractionAcc = (CalculatedSpeedCurveValue[i]*2 + 1)/(3.6*3.6*2*TempS);

							CalculatedSpeedCurveValue[i + 1] = sqrt(CalculatedSpeedCurveValue[i] * CalculatedSpeedCurveValue[i] / 3.6 / 3.6 - 2 * BrakingAcc) * 3.6;
                            CalculatedSecondPerMeter[i + 1] = 1 / (CalculatedSpeedCurveValue[i + 1] / 3.6);
                            CalculatedRunningTime = CalculatedRunningTime + 1 / (CalculatedSpeedCurveValue[i + 1] / 3.6);
							TotalTime[i+1] = CalculatedRunningTime;
						}
                        else
                        {
                            CalculatedSpeedCurveValue[i + 1] = CoastingSpeedAverage;
                            CalculatedSecondPerMeter[i + 1] = 1 / (CalculatedSpeedCurveValue[i + 1] / 3.6);
                            CalculatedRunningTime = CalculatedRunningTime + 1 / (CalculatedSpeedCurveValue[i + 1] / 3.6);
							TotalTime[i+1] = CalculatedRunningTime;
							if(acc_turn.back() !=0){
							acc_turn.push_back(0);  
							s_turn.push_back(i);
							}
                        }
                    }
                    else if (CalculatedSpeedCurveValue[i] >= SpeedLimitWhole[i] && SpeedLimitWhole[i] <= ProtectCurveSpeed[i]) //
                    {
                        CalculatedSpeedCurveValue[i + 1] = SpeedLimitWhole[i];
                        CalculatedSecondPerMeter[i + 1] = 1 / (CalculatedSpeedCurveValue[i + 1] / 3.6);
                        CalculatedRunningTime = CalculatedRunningTime + 1 / (CalculatedSpeedCurveValue[i + 1] / 3.6);
						TotalTime[i+1] = CalculatedRunningTime;
						if(acc_turn.back() != 0){
							acc_turn.push_back(0);
							s_turn.push_back(i);
							}
                    }
                    else if (CalculatedSpeedCurveValue[i] >= ProtectCurveSpeed[i] && ProtectCurveSpeed[i] <= SpeedLimitWhole[i]) //速度值达到防护速度
                    {
                        CalculatedSpeedCurveValue[i + 1] = ProtectCurveSpeed[i];
                        CalculatedSecondPerMeter[i + 1] = 1 / (CalculatedSpeedCurveValue[i + 1] / 3.6);
                        CalculatedRunningTime = CalculatedRunningTime + 1 / (CalculatedSpeedCurveValue[i + 1] / 3.6);
						TotalTime[i+1] = CalculatedRunningTime;
						if(acc_turn.back() != -1){
							acc_turn.push_back(-1);
							s_turn.push_back(i);
							}
						//cout<<CalculatedSpeedCurveValue[i + 1]<<" "<<CalculatedRunningTime<<endl;
                    }
                }
				if(Flag == 1 && CalculatedRunningTime < RunningTime)
				{
				CoastingSpeedHigh = CoastingSpeedAverage;
				CoastingSpeedLow = 10;
				//AccFlag = -1;
				}
                //CalculatedSpeedCurveValue[RunningDistance - 1] = TrainTerminalSpeed;
                if (Flag == 3)                //更新二分法对应的速度上限和下限值
                {
                    if (CalculatedRunningTime > RunningTime)
                    {
                        CoastingSpeedLow = CoastingSpeedAverage;
                    }
                    else
                    {
                        CoastingSpeedHigh = CoastingSpeedAverage;
                    }

                }

			}
			

           t->CalculatedSpeedCurveValue = CalculatedSpeedCurveValue;
		   t->estNextArrival = current_time + CalculatedRunningTime;
		   
			delete[] SpeedLimitBuffered;
			delete[] SpeedLimitWhole;
			delete[] CalculatedSecondPerMeter;  //时间  若要使用不能释放
			delete[] TotalTime;
			//delete[] node;
			//free(p1);
			memset(ProtectCurveSpeed,0,sizeof(double)*RunningDistance);//lrg 20211031添加，因ProtectCurveSpeed = new double[RunningDistance];47行报错
			return CalculatedSpeedCurveValue[RunningDistance];
        }