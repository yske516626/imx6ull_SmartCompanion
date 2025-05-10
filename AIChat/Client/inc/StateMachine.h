#ifndef __STATEMACHINE_H_
#define __STATEMACHINE_H_

#include <functional>
#include <unordered_map>
#include <string>

using enterStateFun = std::function<void()>;  //进入状态调用的回调函数
using exitStateFun = std::function<void()>;   //退出该状态调用的回调函数

//实现一个状态机，根据事件event在不同状态之间的转换
class StateMachine {
public:

	/**
	 * @brief  使用成员初始化列表，在构建状态机的时候初始化当前状态
     * @note
     * @param  initState_:构建状态机传入的初始化状态
     * @retval
     */
	StateMachine(int initState_);
	~StateMachine();

	/**
	 * @brief  初始化
	 * @note   调用初始状态的状态的进入回调函数
	 * @retval None
	 */
	void Init_State();


	/**
	 * @brief  添加新的状态
	 * @note   放进成员：state
	 * @param  newState_: 状态对应的标识
	 * @param  enter_: 状态进入时调用的函数
	 * @param  exit_:  状态退出时调用的函数
	 * @retval None
	 */
	void Add_State(int newState_,enterStateFun enterFunc_,exitStateFun exitFunc_);


	/**
	 * @brief  添加转换事件
	 * @note   first_会因遇到event_而进入secoond_
	 *		   如果first_传的-1，表示任何状态都可以因为事件event_进入secoond_状态
	 * @param  first_: 起始状态
	 * @param  event_: 遇到的事件
	 * @param  secoond_: 目标状态
	 * @retval None
	 */
	void Add_TransitionEvent(int first_, int event_, int second_);

	/**
	 * @brief  当遇到event，在transition中查找，找到cutrrentState要转换的下一个状态
	 * @note   
	 * @param  event: 状态转换事件
	 * @retval true:处理成功
	 */
	bool EventHandle(int event);


	/**
	 * @brief  获取当前的状态
	 * @note   
	 * @retval 返回当前状态（int类型）
	 */
	int Get_CutrrentState();

private:
	void Change_State(int newState_)
	{
		if (state.find(cutrrentState) != state.end())
		{
			state[cutrrentState].second(); //调用当前状态的退出函数exitStateFun
		}
		cutrrentState = newState_; //新的状态赋值到当前状态
		if (state.find(cutrrentState) != state.end())
		{
			state[cutrrentState].first();  //调用新状态的进入回调函数enterStateFun
		}
	}

	int cutrrentState; //当前状态
	//记录每个状态（int）对应的是进入和退出调用的函数(pair<enterStateFun, exitStateFun>)
	std::unordered_map<int, std::pair<enterStateFun, exitStateFun>> state;
	//记录每个状态（key：int）对于每个事件（unordered_map的key：int）的可能转换状态（unordered_map的value：int）。
	std::unordered_map<int, std::unordered_map<int, int>> transition;

};


#endif
