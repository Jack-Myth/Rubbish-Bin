#include <stdio.h>
#include <vector>
#include <stdlib.h>

using std::vector;

class TaskClass
{
	int StartTime, EndTime, CommitTime, TimeRequest,TaskID,WaitTime;
public:
	TaskClass(int CommitTime, int TimeRequest, int TaskID);
	void ProcessTask(int& TimeLine);
	int GetCommitTime();
	int GetStartTime();
	int GetEndTime();
	int GetWaitTime();
	int GetTimeRequest();
	int GetTaskID();
};

class TaskQueue
{
	int TimeLine=0;
	vector<TaskClass> PreTaskPool,WorkingTaskPool,FinishedTaskPool;
public:
	void AddTask(TaskClass TaskToAdd);
	void BeginProcess();
	vector<TaskClass> GetFinishedTaskPool();
};


void SyncTask();
void AsyncTask();
int main()
{
	int TaskMode=0;
	while (1)
	{
		printf("输入任务模式(1.同时,2.不同时)：");
		scanf("%d", &TaskMode);
		switch (TaskMode)
		{
		case 1:
			SyncTask();
			break;
		case 2:
			AsyncTask();
		}
		system("pause");
		system("cls");
	}
}

void SyncTask()
{
	printf("输入任务所需的时间，输入0停止\n");
	int TaskTimeRequest=0;
	int TaskID = 1;
	TaskQueue mTaskQueue;
	while (1)
	{
		printf("输入ID为%d的任务所需的时间:", TaskID);
		scanf("%d", &TaskTimeRequest);
		if (!TaskTimeRequest)
			break;
		mTaskQueue.AddTask(TaskClass(0, TaskTimeRequest, TaskID));
		TaskID++;
	}
	mTaskQueue.BeginProcess();
	vector<TaskClass> TaskPool = mTaskQueue.GetFinishedTaskPool();
	int DelayTimeSum=0;
	for (int i=0;i<TaskPool.size();i++)
	{
		printf("任务%d:\n\t开始时间:%d\n\t结束时间:%d\n\t等待时间:%d\n", TaskPool[i].GetTaskID(),TaskPool[i].GetStartTime(), TaskPool[i].GetEndTime(), TaskPool[i].GetWaitTime());
		DelayTimeSum += TaskPool[i].GetWaitTime();
	}
	printf("平均等待时间:%f\n", (float)DelayTimeSum / TaskPool.size());
}

void AsyncTask()
{
	printf("输入任务的提交时间和所需时间，输入均为0时停止\n");
	int TaskTimeRequest = 0;
	int TaskID = 1;
	int TaskCommitTime = 0;
	TaskQueue mTaskQueue;
	while (1)
	{
		printf("输入ID为%d的任务的提交时间和所需时间:", TaskID);
		scanf("%d%d",&TaskCommitTime, &TaskTimeRequest);
		if (!TaskTimeRequest&&!TaskCommitTime)
			break;
		mTaskQueue.AddTask(TaskClass(TaskCommitTime, TaskTimeRequest, TaskID));
		TaskID++;
	}
	mTaskQueue.BeginProcess();
	vector<TaskClass> TaskPool = mTaskQueue.GetFinishedTaskPool();
	int DelayTimeSum = 0;
	for (int i = 0; i < TaskPool.size(); i++)
	{
		printf("任务%d:\n\t开始时间:%d\n\t结束时间:%d\n\t等待时间:%d\n", TaskPool[i].GetTaskID(), TaskPool[i].GetStartTime(), TaskPool[i].GetEndTime(), TaskPool[i].GetWaitTime());
		DelayTimeSum += TaskPool[i].GetWaitTime();
	}
	printf("平均等待时间:%f\n", (float)DelayTimeSum / TaskPool.size());
}

TaskClass::TaskClass(int CommitTime, int TimeRequest, int TaskID)
{
	TaskClass::CommitTime = CommitTime;
	TaskClass::TimeRequest = TimeRequest;
	TaskClass::TaskID = TaskID;
}

void TaskClass::ProcessTask(int & TimeLine)
{
	StartTime = TimeLine;
	TimeLine += TimeRequest;
	EndTime = TimeLine;
	WaitTime = StartTime - CommitTime;
}

int TaskClass::GetCommitTime()
{
	return CommitTime;
}

int TaskClass::GetStartTime()
{
	return StartTime;
}

int TaskClass::GetEndTime()
{
	return EndTime;
}

int TaskClass::GetWaitTime()
{
	return WaitTime;
}

int TaskClass::GetTimeRequest()
{
	return TimeRequest;
}

int TaskClass::GetTaskID()
{
	return TaskID;
}

void TaskQueue::AddTask(TaskClass TaskToAdd)
{
	PreTaskPool.push_back(TaskToAdd);
}

void TaskQueue::BeginProcess()
{
	do 
	{
		for (auto it=PreTaskPool.begin();it<PreTaskPool.end();)  //vector::iterator
		{
			if (it->GetCommitTime() <= TimeLine)
			{
				WorkingTaskPool.push_back(*it);
				it = PreTaskPool.erase(it);
			}
			else
				++it;
		}
		int IndexForMinTimeRequest=0;
		if (!WorkingTaskPool.size())
		{
			if (PreTaskPool.size())
			{
				int IndexForMinCommitTime=0;
				for (int i=0;i<PreTaskPool.size();i++)
				{
					if (PreTaskPool[i].GetCommitTime() < PreTaskPool[IndexForMinCommitTime].GetCommitTime())
						IndexForMinCommitTime = i;
				}
				TimeLine = PreTaskPool[IndexForMinCommitTime].GetCommitTime();
			}
			continue;
		}
		for (int i = 0; i < WorkingTaskPool.size(); i++)
		{
			if (WorkingTaskPool[i].GetTimeRequest()==WorkingTaskPool[IndexForMinTimeRequest].GetTimeRequest())
			{
				if (WorkingTaskPool[i].GetCommitTime() < WorkingTaskPool[IndexForMinTimeRequest].GetCommitTime())
					IndexForMinTimeRequest = i;
			}
			else if (WorkingTaskPool[i].GetTimeRequest()<WorkingTaskPool[IndexForMinTimeRequest].GetTimeRequest())
				IndexForMinTimeRequest = i;
		}
		WorkingTaskPool[IndexForMinTimeRequest].ProcessTask(TimeLine);
		FinishedTaskPool.push_back(WorkingTaskPool[IndexForMinTimeRequest]);
		WorkingTaskPool.erase(WorkingTaskPool.begin() + IndexForMinTimeRequest);
	} while (WorkingTaskPool.size()||PreTaskPool.size());
}

vector<TaskClass> TaskQueue::GetFinishedTaskPool()
{
	return FinishedTaskPool;
}
