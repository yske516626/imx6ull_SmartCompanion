#ifndef __FUNC_PAGEMANAGER_H_
#define __FUNC_PAGEMANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "../codeblock_use.h"

#include <stdio.h>
#include <stdlib.h>

/****************************历史页面栈管理***************************************************/
//用于实现历史页面的管理，加载新页面的时候，将上一页的数据暂存进该历史页面栈
#define PAGE_MANAGER_MAX_PAGE_HISTORY 5  //最多能存放的历史页面
// 栈节点类型
typedef struct page_stack_node_t {
    void *data;  // 任意数据指针，用于存放都指向的页面结构体：Page_t
} page_stack_node_t;

// 栈管理结构体
typedef struct {
    page_stack_node_t *stack; 	// 栈数组
    int top;                    // 栈顶索引
    int capacity;               // 栈的容量
} page_stack_t;

/****************************页面管理器***************************************************/
#define PAGE_MANAGER_MAX_PAGES 20        //最多实现20个页面

//单个页面结构体
typedef struct {
    char *name;                 // 页面名称
    void (*init)(void);         // 页面初始化函数
    void (*deinit)(void);       // 页面销毁函数
    lv_obj_t *page_obj;         // 页面上的主要 LVGL 对象
} Page_t;

// 页面管理器：历史页面的存放、所有实现的页面的相关定义存放
typedef struct {
    page_stack_t page_stack;                      // 页面栈，用于存储之前的历史页面
    Page_t *current_page;                 // 当前活动页面
    uint8_t cur_depth;                              // 当前页面深度
    Page_t *all_pages[PAGE_MANAGER_MAX_PAGES];   // 用于存储所有创建的页面
    uint8_t num_pages;                              // 注册页面数量
} Page_Manager_t;

/****************************接口***************************************************/
Page_t *Page_Manager_CheckName(const Page_Manager_t *manager, const char *name);
void Page_Manager_Init(Page_Manager_t *manager);
void Page_Manager_DeInit(Page_Manager_t *manager);
Page_t *Page_Manager_CreatePage(Page_Manager_t *manager, const char *name,
                                void (*init)(void), void (*deinit)(void),
                                lv_obj_t *page_obj);
void Page_Manager_LoadPage(Page_Manager_t *manager, Page_t *page, char *name);
void Page_Manager_RetPrePage(Page_Manager_t *manager);
Page_t *Page_Manager_GetCurrentPage(const Page_Manager_t *manager);
void Page_Manager_ReturnToBottom(Page_Manager_t *manager);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // __FUNC_PAGEMANAGER_H_
