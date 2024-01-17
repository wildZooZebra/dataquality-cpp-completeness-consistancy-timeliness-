#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

//中间值
double midValue(std::vector<double>& vec) {
    if (vec.empty())        
      throw std::invalid_argument("Vector is empty");
    size_t size = vec.size();
    size_t middleIndex = size / 2;
    sort(vec.begin(), vec.end());
    if (size % 2 == 1)
        return vec[middleIndex];
    else
        return (vec[middleIndex - 1] + vec[middleIndex]) / 2.0;
}

//最小值
inline double findMin(double num1, double num2) {
    return (num1 < num2) ? num1 : num2;
}

class timeSeries{
private:
  vector<double> time;
  vector<double> value;
public:
  const int WINDOW_SIZE = 5;
  int cnt = 0;  //时间戳计数
  int missCnt = 0;  //时间戳缺失计数
  int specialCnt = 0;  //特殊值NaN，Inf计数
  int lateCnt = 0;  //时间戳延迟计数
  int redundancyCnt = 0;  //时间戳冗余计数
  int valueCnt = 0;
  int variationCnt = 0;
  int speedCnt = 0;
  int speedchangeCnt = 0;

  inline timeSeries(){
    time.resize(0);
    value.resize(0);
  }
  void init();  //时间戳序列初始化
  void NaNDetect();  //NaN的检测
  void NaNProcess();  //NaN的处理
  void timeDetect();   //时间戳的检测
  int findOutliers(double k);  //未使用
  inline double completeness(){
    return 1 
    - (missCnt + specialCnt) * 1.0 
    / (cnt + missCnt);
  }
  inline double consistency(){
    return 1 
    - redundancyCnt * 1.0 / cnt;
  }
  inline double timeliness(){
    return 1 
    - lateCnt * 1.0 / cnt;
  }
};



int main(){
  timeSeries ts;
  ts.init();    
  ts.NaNDetect();
  ts.timeDetect();
  cout << "completeness = " << ts.completeness() << endl;
  cout << "consistence = " << ts.consistency() << endl;
  cout << "timeliness = " << ts.timeliness() << endl;
  return 0;
}



void timeSeries::init(){
    cout << "cnt = ";
    cin >> cnt;
    time.resize(cnt);
    value.resize(cnt);
    for(size_t i = 0; i < cnt; ++i)
      cin >> time[i];
    for(size_t i = 0; i < cnt; ++i)
      cin >> value[i];
    cout << "time : ";
    for(size_t i = 0; i < cnt; ++i)
      cout << time[i] << " ";
    cout << endl;
    cout << "value : ";
    for(size_t i = 0; i < cnt; ++i)
      cout << value[i] << " ";
    cout << endl;
    }

  void timeSeries::NaNDetect(){
    for(int i = 0; i < cnt; ++i)
      if(isnan(value[i]))
        specialCnt++;
  }

  void timeSeries::NaNProcess(){
    //对NaN使用线性回归进行处理
    int index1 = 0, index2;
    while(index1 < cnt && isnan(value[index1]))
      index1++;
    index2 = index1 + 1;
    while(index2 < cnt && isnan(value[index2]))
      index2++;
    if(index2 >= cnt)
      return;
    for(int i = 0; i < index2; ++i){
      value[i] = value[index1] 
      + (value[index2] - value[index1]) 
      * (time[i] - time[index1])
      / (time[index2] - time[index1]);
    }
    for(int i = index2 + 1; i < cnt; ++i){
      if(!isnan(value[i])){
        index1 = index2;
        index2 = i;
        for(int j = index1 = 1; j < index2; ++j){
          value[j] = value[index1]
          + (value[index2] - value[index1])
          * (time[j] - time[index1])
          / (time[index2] - time[index1]);
        }
      }
    }
    for(int i = index2 + 1;i < cnt; ++i){
      value[i] = value[index1] 
      + (value[index2] - value[index1]) 
      * (time[i] - time[index1])
      / (time[index2] - time[index1]);
    }
  }

  void timeSeries::timeDetect(){
    cout << "timeDetect Start!" << endl;
    //cout << "time.size() = " << time.size() << endl;
    vector<double> interval(time.size() - 1, 0);
    for(int k = 0; k < interval.size(); ++k)
      interval[k] = time[k + 1] - time[k];
    double base = midValue(interval);   //base -> 时间戳差值的中间值
    //cout << "base = " << base << endl;
    vector<double> window(WINDOW_SIZE, 0);
    //cout << "window.size() = " << window.size() << endl;
    int i;
    for(i = 0; i < findMin(time.size(), WINDOW_SIZE); ++i){
      window[i] = time[i];
    }
    //for(int m = 0; m < window.size(); ++m) cout << window[m] << " "; cout << endl;
    while(window.size() > 1) {
      //times = 
      //（1）<= 0.5 -> 冗余
      //（2）(0.5, 2.0) -> 正常
      //（3）[2.0, 9.0] -> 缺失/延迟
      //（4）> 9.0 -> 关机
      double times = (window[1] - window[0]) / base;
      if(times <= 0.5) {
        //冗余
        window.erase(window.begin() + 1);
        redundancyCnt++;
      } else if (times >= 2.0 && times <= 9.0) {
        //判定为延迟或缺失
        int temp = 0;
        for(int j = 2; j < window.size(); ++j) {
          double times2 = (window[j] - window[j - 1]) / base;
          if(times2 >= 2.0)
            break;
          if(times2 <= 0.5) {
            temp++;
            window.erase(window.begin() + j);
            --j;
            if(temp == (int)round(times - 1))
              break;
          }
        }
        lateCnt += temp;
        missCnt += (round(times - 1) - temp);
      } else {
        //判定为关机
        cout << "Shut down!"<<endl;
        return;
      }
      window.erase(window.begin());
      while(window.size() < WINDOW_SIZE && i < cnt) {
        window.push_back(time[i]);
        ++i;
      }
    }
    printf("timeDetect Done!\n");
  }

  int timeSeries::findOutliers(double k){
    double mid = midValue(value);
    vector<double> derta(value.size(), 0);
    for(int i = 0; i < value.size(); ++i)
      derta[i] - abs(value[i] - mid);
    double sigma = midValue(derta);
    int num = 0;
    for(int i = 0; i < value.size(); ++i)
      if(abs(value[i] - mid) > k * sigma)
        num++;
    return num;
  }