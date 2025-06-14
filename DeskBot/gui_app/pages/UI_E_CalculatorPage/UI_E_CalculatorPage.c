#include "UI_E_CalculatorPage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*******************************图标***********************************************/



/*******************************变量***********************************************/

#define TEXT_FULL 10
StrStack_t CalStr;
NumStack_t NumStack;
SymStack_t SymStack;
static const char * ui_ComPageBtnmap[] ={"1", "2", "3", "+", "\n",
                                         "4", "5", "6", "-", "\n",
                                         "7", "8", "9", "×", "\n",
                                         ".", "0", "=", "÷", ""};
lv_obj_t * ui_CompageBtnM;

/*******************************内部静态接口***********************************************/


uint8_t strput(StrStack_t * st,char strin)
{
    if(st->Top_Point == 15 - 1)
    {return -1;}

    st->strque[st->Top_Point++] = strin;
    return 0;
}

uint8_t strdel(StrStack_t * st)
{
    if(st->Top_Point == 0)
    {return -1;}

    st->strque[--st->Top_Point] = NULL;
    return 0;
}

uint8_t strstack_isEmpty(StrStack_t* st)
{
    if(st->Top_Point == 0)
    {return 1;}

    return 0;
}

void strclear(StrStack_t* sq)
{
    while(!strstack_isEmpty(sq))
    {
        strdel(sq);
    }
}

static uint8_t NumStackPut(NumStack_t * st, float in)
{
   if(st->Top_Point == CAL_DEPTH - 1)
	{return -1;}

	st->data[st->Top_Point++] = in;
	return 0;
}

static uint8_t NumStackDel(NumStack_t * st)
{
    if(st->Top_Point == 0)
	{return -1;}
    st->data[st->Top_Point--] = 0;
	return 0;
}

static uint8_t NumStack_isEmpty(NumStack_t* st)
{
	if(st->Top_Point == 0)
	{return 1;}

	return 0;
}

static void NumStackClear(NumStack_t* st)
{
	while(!NumStack_isEmpty(st))
	{
		NumStackDel(st);
	}
}

static uint8_t SymStackPut(SymStack_t * st, char in)
{
	if(st->Top_Point == CAL_DEPTH - 1)
	{return -1;}

	st->data[st->Top_Point++] = in;
	return 0;
}

static uint8_t SymStackDel(SymStack_t * st)
{
  if(st->Top_Point == 0)
	{return -1;}
    st->data[st->Top_Point--] = 0;
	return 0;
}

static uint8_t SymStack_isEmpty(SymStack_t* st)
{
	if(st->Top_Point == 0)
	{return 1;}

	return 0;
}

static void SymStackClear(SymStack_t* st)
{
	while(!SymStack_isEmpty(st))
	{
		SymStackDel(st);
	}
}

static uint8_t SymisHighPriority(char top, char present)
{
    //乘除的优先级最大
    if(top == '*' || top == '/')
    {
        return 1;
    }
    else if(top == '+')
    {
        if(present == '-')
        {return 1;}
        else
        {return 0;}
    }
    else if(top == '-')
    {
        if(present == '+')
        {return 1;}
        else
        {return 0;}
    }
}

static void CalculateOne(NumStack_t * numstack, SymStack_t * symstack)
{
    caldata_t temp;
    temp.datatype = NUMBER_TYPE;
    temp.symbol = NULL;
    //计算数字栈中的顶部两数,结果存到temp中
    if(symstack->data[symstack->Top_Point-1] == '+')
        temp.number = (numstack->data[numstack->Top_Point-2]) + (numstack->data[numstack->Top_Point-1]);

    else if(symstack->data[symstack->Top_Point-1] == '-')
        temp.number = (numstack->data[numstack->Top_Point-2]) - (numstack->data[numstack->Top_Point-1]);

    else if(symstack->data[symstack->Top_Point-1] == '*')
        temp.number = (numstack->data[numstack->Top_Point-2]) * (numstack->data[numstack->Top_Point-1]);

    else if(symstack->data[symstack->Top_Point-1] == '/')
        temp.number = (numstack->data[numstack->Top_Point-2]) / (numstack->data[numstack->Top_Point-1]);

    //运算前两数出栈,运算结果数入栈
    NumStackDel(numstack);
    NumStackDel(numstack);
    NumStackPut(numstack,temp.number);
    SymStackDel(symstack);

}

static uint8_t NumSymSeparate(char * str, uint8_t strlen, NumStack_t * NumStack, SymStack_t * SymStack)
{
		NumStackClear(NumStack);
		SymStackClear(SymStack);
    caldata_t temp,temp_pre;
    char NumBehindPoint_Flag = 0;//数字是否在小数点后,后多少位
    temp.datatype = NUMBER_TYPE;
    temp.number = 0;
    temp.symbol = NULL;
    temp_pre = temp;
    temp_pre.datatype = SYMBOL_TYPE;
    if(str[0]>'9' || str[0]<'0')
        return 1;//erro
		int i;
    for(i=0;i<strlen;i++)
    {
        if(str[i]=='.')
        {
            temp.datatype = POINT_TYPE;
            if(temp_pre.datatype == NUMBER_TYPE)
            {}
            else
            {return 2;}
            temp_pre = temp;
        }
        if(str[i]<='9' && str[i]>='0')
        {
            //溢出报错
            if(NumStack->Top_Point>CAL_DEPTH || SymStack->Top_Point>CAL_DEPTH)
            {return 3;}
            //读取当前的字符到temp中
            temp.datatype = NUMBER_TYPE;
            temp.number = (str[i] - '0');
            temp.symbol = NULL;
            //如果为连续数字,需要进行进位,将数字栈顶读出进位，再加上现在位，再入栈
            if(temp_pre.datatype == NUMBER_TYPE)
            {
                if(!NumBehindPoint_Flag)
                {temp.number += NumStack->data[NumStack->Top_Point-1] * 10;}
                else
                {
                    NumBehindPoint_Flag += 1;
                    char i = NumBehindPoint_Flag;
                    while(i--)
                    {temp.number /= 10;}
                    temp.number += NumStack->data[NumStack->Top_Point-1];
                }
                NumStackDel(NumStack);
                NumStackPut(NumStack,temp.number);
            }
            //当前数字刚好是小数点后一位
            else if(temp_pre.datatype == POINT_TYPE)
            {
                NumBehindPoint_Flag = 1;
                temp.number /= 10;
                temp.number += NumStack->data[NumStack->Top_Point-1];
                NumStackDel(NumStack);
                NumStackPut(NumStack,temp.number);
            }
            //前一位不是数字或小数点,现在读取的这一位是数字，直接入栈
            else
            {
                NumStackPut(NumStack,temp.number);
            }
            temp_pre = temp;
        }
        else if(str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/')
        {
            //溢出报错
            if(NumStack->Top_Point>CAL_DEPTH || SymStack->Top_Point>CAL_DEPTH)
            {return 4;}
            //读取当前的字符到temp中
            temp.datatype = SYMBOL_TYPE;
            temp.symbol = str[i];
            temp.number = 0;
            NumBehindPoint_Flag = 0;//小数点计算已经结束
            //重复输入了运算符号
            if(temp_pre.datatype == SYMBOL_TYPE)
            {
                return 5 ;//erro
            }
            else
            {
                if((!SymStack_isEmpty(SymStack)) && SymisHighPriority(SymStack->data[SymStack->Top_Point-1],temp.symbol))
                {
                    CalculateOne(NumStack, SymStack);
                    SymStackPut(SymStack,temp.symbol);
                }
                else
                {
                    //符号压入符号栈
                    SymStackPut(SymStack,temp.symbol);
                }
                temp_pre = temp;
            }
        }
    }
    return 0;
}

static uint8_t StrCalculate(char * str,NumStack_t * NumStack, SymStack_t * SymStack)
{
    if(NumSymSeparate(str,strlen(str),NumStack,SymStack))
    {
        //erro, clear all
        NumStackClear(NumStack);
        SymStackClear(SymStack);
        return -1;
    }
    else
    {
        while(!SymStack_isEmpty(SymStack))
        {
            CalculateOne(NumStack,SymStack);
        }
    }
    return 0;
}

static uint8_t isIntNumber(float number)
{
	if(number == (int)number)
	{return 1;}
	return 0;
}



/*******************************动画***********************************************/


/*******************************event_cb***********************************************/

static void ui_enent_Gesture(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_GESTURE)
    {
        if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT || lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
        {
            Page_Manager_RetPrePage(&page_manager);
        }
    }
}

void ui_CompageBtnM_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * ui_CompageTextarea = lv_event_get_user_data(e);
    if(code == LV_EVENT_DRAW_TASK_ADDED)
    {
        lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
        lv_draw_dsc_base_t * base_dsc = lv_draw_task_get_draw_dsc(draw_task);
        if(base_dsc->part == LV_PART_ITEMS)
        {
            if(base_dsc->id1 >= 0)
            {
                lv_draw_fill_dsc_t * fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
                if(base_dsc->id1 == 3 || base_dsc->id1 == 7 || base_dsc->id1 == 11 || base_dsc->id1 == 14 || base_dsc->id1 == 15)
                {
                    if(fill_draw_dsc)
                    {
                        fill_draw_dsc->radius = LV_RADIUS_CIRCLE;
                        if (lv_btnmatrix_get_selected_btn(obj) == base_dsc->id1)
                        {
                            fill_draw_dsc->color = lv_palette_darken(LV_PALETTE_BLUE,3);
                            //lv_btnmatrix_set_selected_btn(ui_CompageBtnM,NULL);
                        }
                        else
                            fill_draw_dsc->color = lv_palette_main(LV_PALETTE_BLUE);
                    }
                }
            }
        }
    }
    if(code == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t btn_id = lv_btnmatrix_get_selected_btn(obj); // 获取当前选中的按键的id
        const char * txt = lv_btnmatrix_get_btn_text(obj, btn_id); // 获取当前按键的文本

        if (txt != NULL)
        {
            if (ui_CompageTextarea != NULL)
            {
                if(lv_textarea_get_cursor_pos(ui_CompageTextarea) <= TEXT_FULL)
                {
                    lv_textarea_add_text(ui_CompageTextarea, txt); // 文本框追加字符
                    switch(btn_id)
                    {
                        case 0:
                                strput(&CalStr,'1');
                                break;
                        case 1:
                                strput(&CalStr,'2');
                                break;
                        case 2:
                                strput(&CalStr,'3');
                                break;
                        case 3:
                                strput(&CalStr,'+');
                                break;
                        case 4:
                                strput(&CalStr,'4');
                                break;
                        case 5:
                                strput(&CalStr,'5');
                                break;
                        case 6:
                                strput(&CalStr,'6');
                                break;
                        case 7:
                                strput(&CalStr,'-');
                                break;
                        case 8:
                                strput(&CalStr,'7');
                                break;
                        case 9:
                                strput(&CalStr,'8');
                                break;
                        case 10:
                                strput(&CalStr,'9');
                                break;
                        case 11:
                                strput(&CalStr,'*');
                                break;
                        case 12:
                                strput(&CalStr,'.');
                                break;
                        case 13:
                                strput(&CalStr,'0');
                                break;
                        case 14:
                                strput(&CalStr,'=');
                                lv_textarea_add_text(ui_CompageTextarea,"\n");
                                strput(&CalStr,'\n');
                                break;
                        case 15:
                                strput(&CalStr,'/');
                                break;
                    }
                }
            }
        }

        if(lv_btnmatrix_get_selected_btn(obj) == 14)
        {
            //calculate
            if(StrCalculate(CalStr.strque,&NumStack,&SymStack))
            {lv_textarea_add_text(ui_CompageTextarea,"erro");}
            else
            {
                char strout[10];
                if(isIntNumber(NumStack.data[NumStack.Top_Point-1]))
                {sprintf(strout,"%.0f",NumStack.data[NumStack.Top_Point-1]);}
                else
                {sprintf(strout,"%.4f",NumStack.data[NumStack.Top_Point-1]);}
                lv_textarea_add_text(ui_CompageTextarea,strout);
            }
            strclear(&CalStr);
            lv_obj_clear_flag(ui_CompageBtnM,LV_OBJ_FLAG_CLICKABLE);
        }
    }
}


void ui_CompageBackBtn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * ui_CompageTextarea = lv_event_get_user_data(e);
    if(code == LV_EVENT_CLICKED)
    {
        if (ui_CompageTextarea != NULL)
        {
            if(!strstack_isEmpty(&CalStr))
            {
                lv_textarea_delete_char(ui_CompageTextarea);
                strdel(&CalStr);
            }
            else
            {
                int i = 0;
                for (i = 0; i < (TEXT_FULL*2); i++)
                {
                    lv_textarea_delete_char(ui_CompageTextarea);
                }
                lv_obj_add_flag(ui_CompageBtnM,LV_OBJ_FLAG_CLICKABLE);
            }
        }
    }
		if(code == LV_EVENT_LONG_PRESSED)
		{
			if (ui_CompageTextarea != NULL)
			{
				if(!strstack_isEmpty(&CalStr))
				{
                    strclear(&CalStr);
                    int i = 0;
                    for (i = 0; i < (TEXT_FULL*2); i++)
                    {
                        lv_textarea_delete_char(ui_CompageTextarea);
                    }
				}
			}
		}
}


/*******************************初始化和销毁***********************************************/

void ui_CalculatorPage_Init(void)
{
	strclear(&CalStr);
    NumStackClear(&NumStack);
    SymStackClear(&SymStack);
    lv_obj_t * ui_CalculatorPage = lv_obj_create(NULL);

    lv_obj_clear_flag(ui_CalculatorPage,LV_OBJ_FLAG_SCROLLABLE);
    ui_CompageBtnM = lv_btnmatrix_create(ui_CalculatorPage);
    lv_btnmatrix_set_map(ui_CompageBtnM, ui_ComPageBtnmap);

    lv_obj_set_style_text_font(ui_CompageBtnM, &ui_font_heiti24, 0);
    lv_btnmatrix_set_one_checked(ui_CompageBtnM,true);
    int i = 0;
    for (i = 0; i < 16; i++)
    {
        lv_btnmatrix_set_btn_ctrl(ui_CompageBtnM, i, LV_BTNMATRIX_CTRL_NO_REPEAT); // 长按按钮时禁用重复
    }
    lv_obj_clear_flag(ui_CompageBtnM, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_style_border_width(ui_CompageBtnM,0,0);
    lv_obj_set_style_bg_opa(ui_CompageBtnM,0,0);
    lv_obj_set_size(ui_CompageBtnM,480,480);
    lv_obj_set_align(ui_CompageBtnM,LV_ALIGN_LEFT_MID);


    lv_obj_t * ui_CompageTextarea = lv_textarea_create(ui_CalculatorPage);
    lv_textarea_set_one_line(ui_CompageTextarea, false); // 将文本区域配置为一行
    //lv_textarea_set_password_mode(obj_text_area, true); // 将文本区域配置为密码模式
    lv_textarea_set_max_length(ui_CompageTextarea, TEXT_FULL*2); // 设置文本区域可输入的字符长度最大值
    lv_obj_add_state(ui_CompageTextarea, LV_STATE_FOCUSED); // 显示光标
    lv_obj_set_style_radius(ui_CompageTextarea, 0, 0); // 设置样式的圆角弧度
    lv_obj_set_style_border_width(ui_CompageTextarea, 0, 0); //设置边框宽度
    lv_obj_set_style_bg_color(ui_CompageTextarea, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_CompageTextarea, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(ui_CompageTextarea, 200, 480); // 设置对象大小
    lv_obj_align(ui_CompageTextarea, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_clear_flag(ui_CompageTextarea,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_text_font(ui_CompageTextarea, &ui_font_heiti24, 0);
    lv_textarea_set_align(ui_CompageTextarea, LV_TEXT_ALIGN_RIGHT);

    lv_obj_t * ui_CompageBackBtn = lv_btn_create(ui_CalculatorPage);
    lv_obj_align(ui_CompageBackBtn,LV_ALIGN_RIGHT_MID,-10,-110);
    lv_obj_set_width(ui_CompageBackBtn,100);
    lv_obj_set_height(ui_CompageBackBtn,100);
    lv_obj_set_style_radius(ui_CompageBackBtn, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_CompageBackBtn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(ui_CompageBackBtn, LV_ALIGN_BOTTOM_RIGHT, -16, -12);
    lv_obj_set_style_bg_color(ui_CompageBackBtn, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * btnlabel = lv_label_create(ui_CompageBackBtn);
    lv_label_set_text(btnlabel, LV_SYMBOL_BACKSPACE);
    lv_obj_set_style_text_font(btnlabel, &lv_font_montserrat_44, 0);
    lv_obj_center(btnlabel);
    
    // event
    lv_obj_add_event_cb(ui_CalculatorPage, ui_enent_Gesture, LV_EVENT_ALL, NULL);

    lv_obj_add_flag(ui_CompageBtnM, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
    lv_obj_add_event_cb(ui_CompageBtnM, ui_CompageBtnM_event_cb, LV_EVENT_ALL, ui_CompageTextarea);
    lv_obj_add_event_cb(ui_CompageBackBtn, ui_CompageBackBtn_event_cb, LV_EVENT_ALL, ui_CompageTextarea);

    // load page
    lv_scr_load_anim(ui_CalculatorPage, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);

}
void ui_CalculatorPage_Dinit(void)
{

}