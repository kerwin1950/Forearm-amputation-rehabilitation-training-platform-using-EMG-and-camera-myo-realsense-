#pragma once
#include"Common.h"


class DataCollector : public myo::DeviceListener {//Éè±¸¼àÌýÆ÷
public:
	DataCollector();
	void onUnpair(myo::Myo* myo, uint64_t timestamp);
	void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg);
	void print();//bool once);
	void collect(int n);
	//void DataRead(float x[8]);
	std::array<int8_t, 8> emgData;
	//bool once = true;
};


