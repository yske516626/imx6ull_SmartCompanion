#ifndef __UI_A_H
#define __UI_A_H

#ifdef __cplusplus
extern "C" {
#endif
#include "../../ui.h"

typedef struct {
    uint16_t witdh;                 // 屏幕宽度
    uint16_t height;                // 屏幕高度
    uint8_t page_number;  			// 页面数
    uint8_t index;   				 // 页面标志
    bool show_dropdown;             // 是否下拉状态栏状态
    bool scroll_busy;               // 避免滚动过快，同时发生btns点击事件
}DeskTop_data_t;

#define PAGE_NUMBER_MAX 2   //最多页面的数

void ui_HomePage_Init();
void ui_HomePage_Dinit();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif