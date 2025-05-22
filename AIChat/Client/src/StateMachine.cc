#include "../inc/StateMachine.h"
#include "../inc/Log.h"
#include <iostream>


StateMachine::StateMachine(int initState_): cutrrentState(initState_)
{
	USER_LOG_INFO("State machine created successfully.");
}

StateMachine::~StateMachine()
{
	USER_LOG_INFO("State machine destroyed successfully.");
}

void StateMachine::Init_State()
{
	//找到指定键的元素
	if (state.find(cutrrentState) != state.end())
	{
		state[cutrrentState].first();  //调用状态cutrrentState的进入回调函数
	}
}


void StateMachine::Add_State(int newState_,enterStateFun enterFunc_,exitStateFun exitFunc_)
{
	//添加新的状态
	state[newState_] = std::make_pair(enterFunc_, exitFunc_);
}

void StateMachine::Add_TransitionEvent(int first_, int event_, int second_)
{
	if (first_ == -1)
	{
		//遍历transition取出各个状态转换列表s_state并取出，添加新的转换：因event_而进入状态second_
		for (auto& s_state : transition)
		{
			//s_state.first：起始状态 --- key
			transition[s_state.first][event_] = second_; //添加新的value
		}
	}
	else
	{
		//添加状态事件转换
		transition[first_][event_] = second_;
	}
}

bool StateMachine::EventHandle(int event)
{
	//取出当前状态的状态转换字典：unordered_map<int, int>
	auto& handleTransition = transition[cutrrentState];
	if (handleTransition.find(event) == handleTransition.end())  
	{
		//找到尾部还是没找到该状态遇到该事件所要转换的状态
		USER_LOG_INFO("Event:%d the transition status of the event was not found", event);
		return false;
	}
	int nextState = handleTransition[event]; //找到键值（事件event）对应的value（要转换的状态）
	Change_State(nextState);
	return true;

}


int StateMachine::Get_CutrrentState()
{
	return cutrrentState;
}


