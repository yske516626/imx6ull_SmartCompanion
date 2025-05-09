#include "Func_PageManager.h"
#include <string.h>
#include <stdlib.h>




/////////////////////////////////////静态内部接口声明/////////////////////////////////////////////////////////////
/*
    用于实现历史页面的管理，加载新页面的时候，将上一页的数据暂存进该历史页面栈	
*/
static void Page_Stack_Init(page_stack_t *stack, int capacity);
static int Page_Stack_Is_Empty(const page_stack_t *stack);
static int Page_Stack_Is_Full(const page_stack_t *stack);
static void *Page_Stack_Pop(page_stack_t *stack);
static int Page_Stack_Push(page_stack_t *stack, void *data);
static void *Page_Stack_Get_Top(const page_stack_t *stack);
static void Page_Stack_Clear(page_stack_t *stack);
static void Page_Stack_Destroy(page_stack_t *stack);



//////////////////////////////////////////公共接口：页面管理器////////////////////////////////////////////////////////

/**
 * @brief  检查已经有该页面
 * @note   
 * @param  manager: 页面管理器
 * @param  name: 页面的名字
 * @retval 返回找到的页面Page_t
 */
Page_t* Page_Manager_CheckName(const Page_Manager_t* manager, const char* name) {
    if (!manager || !name || !manager->all_pages) return NULL;

    // 遍历所有页面，查找名称匹配的页面
    for (int i = 0; i < manager->num_pages; i++) {
        Page_t *page = manager->all_pages[i];
        if (page && page->name && strcmp(page->name, name) == 0) {
            return page;
        }
    }

    // 若未找到，返回 NULL
    return NULL;
}

/**
 * @brief  页面管理器初始化
 * @note   
 * @param  manager: 
 * @retval None
 */
void Page_Manager_Init(Page_Manager_t *manager) {
    if (!manager) {
        LV_LOG_WARN("Page manager initialization exception.");
        return;
    }

    // 初始化历史页面存储栈
    Page_Stack_Init(&manager->page_stack, PAGE_MANAGER_MAX_PAGE_HISTORY); // 设定页面栈的容量为
    manager->current_page = NULL;  // 当前页面为NULL
    manager->cur_depth = 0;        // 当前深度为 0
}


/**
 * @brief  页面管理器销毁
 * @note   
 * @param  manager: 
 * @retval None
 */
void Page_Manager_DeInit(Page_Manager_t *manager) {
   if (!manager) {
        LV_LOG_WARN("Page_Manager_DeInit:Page Manager Destruction Exception.");
        return;
    }

    // 销毁页面栈
    Page_Stack_Destroy(&manager->page_stack);
    manager->current_page = NULL; // 清空当前页面
    manager->cur_depth = 0;       // 重置深度
}

/**
 * @brief  创建页面
 * @note   
 * @param  manager: 页面管理器
 * @param  name: 创建页面的名称
 * @param  init: 页面的初始化函数
 * @param  deinit：页面的释放函数
 * @param  page_obj：页面的实现对象
 * @retval 
 */
Page_t* Page_Manager_CreatePage(Page_Manager_t *manager, const char *name, void (*init)(void), void (*deinit)(void), lv_obj_t *page_obj) {
    if (!manager || !name || !init || !deinit) {
        LV_LOG_WARN("Page_Manager_CreatePage: Invalid parameters for creating a page.");
        return NULL;
    }

    // 检查是否已有同名的页面
    if (Page_Manager_CheckName(manager, name)) {
        LV_LOG_WARN("Page_Manager_CreatePage: A page with the name '%s' already exists.", name);
        return NULL;
    }

    // 分配内存创建页面
    Page_t *page = (Page_t *)malloc(sizeof(Page_t));
    if (!page) {
        LV_LOG_WARN("Page_Manager_CreatePage: Failed to allocate memory for the page.");
        return NULL;
    }

    // 分配并复制页面名称
    page->name = (char *)malloc(strlen(name) + 1);
    if (!page->name) {
        free(page);
        LV_LOG_WARN("Page_Manager_CreatePage: Failed to allocate memory for the page name.");
        return NULL;
    }
    strcpy(page->name, name);

    // 设置页面的初始化和销毁函数
    page->init = init;
    page->deinit = deinit;
    page->page_obj = page_obj;
    // 将页面添加到所已创建的页面列表
    if (manager->num_pages < PAGE_MANAGER_MAX_PAGES) {  
        LV_LOG_INFO("Page created");
        manager->all_pages[manager->num_pages++] = page;
    } else {
        LV_LOG_WARN("Page_Manager_CreatePage: Unable to create page, exceeded limit.");
        free(page->name);
        free(page);
        return NULL;
    }

    return page;
}

/**
 * @brief  加载指定界面
 * @note   
 * @param  manager: 页面管理器
 * @param  page: 要加载的页面主体
 * @param  name: 要加载的页面名字
 * @retval None
 */
void Page_Manager_LoadPage(Page_Manager_t *manager, Page_t *page, char *name) {
    if (!manager) return;

    // 如果传入的 page 不为空，则优先使用 page
    if (page) {
        // 如果当前页面存在，先销毁它
        if (manager->current_page != NULL && manager->current_page->deinit) {
            manager->current_page->deinit(); // 调用当前页面的销毁函数
        }

        // 将当前页面压入栈中（存放进历史页面栈）
        Page_Stack_Push(&manager->page_stack, manager->current_page);

        // 设置新页面为当前页面，并调用其初始化函数
        manager->current_page = page;
        if (page->init) {
            page->init(); // 调用新页面的初始化函数
        }

        manager->cur_depth++; // 增加页面深度
        LV_LOG_INFO("Page_Manager_LoadPage:Opened page: %s, depth: %d", page->name, manager->cur_depth);

    } else if (name) { // 如果没有 page，使用 name 查找页面
        Page_t *found_page = Page_Manager_CheckName(manager, name);
        if (found_page) {
            // 如果找到了页面，打开它
            Page_Manager_LoadPage(manager, found_page, NULL); // 递归调用打开页面
        } else {
            LV_LOG_WARN("Page_Manager_LoadPage:Page with name '%s' not found.", name);
        }
    } else {
        LV_LOG_WARN("Page_Manager_LoadPage:Both page and name are NULL. Cannot open page.");
    }
}


/**
 * @brief  返回上一页
 * @note   从历史页面栈中取出上一页的页面数据
 * @param  manager: 
 * @retval None
 */
void Page_Manager_RetPrePage(Page_Manager_t *manager) {
    if (!manager) return;

    // 如果栈为空，表示没有历史页面，无法返回
    if (Page_Stack_Is_Empty(&manager->page_stack)) {
        LV_LOG_WARN("Page_Manager_RetPrePage:No previous page to return to.");
        return;
    }

    //从历史页面栈中取得上一页Page_t(出栈)
    Page_t *prev_page = (Page_t *)Page_Stack_Pop(&manager->page_stack);
    
    // 先销毁当前页面
    if (manager->current_page != NULL && manager->current_page->deinit) {
        manager->current_page->deinit(); // 销毁当前页面
    }

    // 设置出栈得来的上一页面为当前页面并初始化
    manager->current_page = prev_page;
    if (prev_page && prev_page->init) {
        prev_page->init(); // 初始化上一页面
    }

    manager->cur_depth--; // 减少当前页面深度
    LV_LOG_INFO("Page_Manager_RetPrePage:Returned to previous page, depth: %d .", manager->cur_depth);
}

/**
 * @brief  当前页面的Page_t
 * @note   
 * @param  manager: 
 * @retval 
 */
Page_t* Page_Manager_GetCurrentPage(const Page_Manager_t *manager) {
    if (!manager) return NULL;
    return manager->current_page;
}

/**
 * @brief  直接返回到历史页面栈栈底页面
 * @note   
 * @param  manager: 
 * @retval None
 */
void Page_Manager_ReturnToBottom(Page_Manager_t *manager) {
    if (!manager) return;

    // 如果当前页面存在，销毁它
    if (manager->current_page != NULL && manager->current_page->deinit) {
        manager->current_page->deinit();
    }

    // 获取栈底页面
    Page_t *bottom_page = (Page_t *)manager->page_stack.stack[0].data;
    if (!bottom_page) {
        LV_LOG_WARN("Page_Manager_ReturnToBottom:No pages in stack.");
        return;  // 如果栈中没有页面，直接返回
    }

    // 弹出所有
    while (!Page_Stack_Is_Empty(&manager->page_stack)) {
        Page_Stack_Pop(&manager->page_stack); // 弹出栈中的页面
    }

    // 设置栈底页面为当前页面并初始化
    manager->current_page = bottom_page;
    if (manager->current_page->init) {
        manager->current_page->init(); // 初始化栈底页面
    }

    manager->cur_depth = 1; // 当前深度恢复为 1, 因为只保留了栈底页面
    LV_LOG_INFO("Page_Manager_ReturnToBottom:Returned to bottom page.");
}

///////////////////////////////////////静态内部接口/////////////////////////////////////////////////////////////
/**
 * @brief  初始化栈
 * @note   这个栈是用于存放当前界面之前的页面
 * @param  stack: 
 * @param  capacity: 所开辟的最多历史界面存放数量
 * @retval None
 */
static void Page_Stack_Init(page_stack_t *stack, int capacity) {
    // 分配内存给栈数组并检查是否分配成功
    stack->stack = (page_stack_node_t *)malloc(sizeof(page_stack_node_t) * capacity);
    if (!stack->stack) {
        LV_LOG_WARN("Page_Stack_Init:Page_Stack_Init is error.");
        stack->top = -1; // 初始化为空栈
        stack->capacity = 0;
        return;
    }

    // 初始化栈的状态
    stack->top = -1;
    stack->capacity = capacity;
}


/**
 * @brief 判断历史页面栈是否为空，为空说明当前为主页
 * @param stack 指向栈结构体的指针
 * @return      1: 栈为空, 0: 栈不为空
 */
static int Page_Stack_Is_Empty(const page_stack_t *stack) {
    if (!stack->stack) {
        LV_LOG_WARN("Page_Stack_Is_Empty:Page_Stack_Is_Empty is Error.");
        return 1; // 栈为空或未初始化
    }
    return stack->top == -1; // 栈为空时，top=-1
}

/**
 * @brief 历史页面栈是否已满
 * @note 满的话就无法将当前页面放进去，然后去加载新的页面
 * @param stack 指向栈结构体的指针
 * @return      1: 栈已满, 0: 栈未满
 */
static int Page_Stack_Is_Full(const page_stack_t *stack) {
    if (!stack->stack) {
        LV_LOG_WARN("Page_Stack_Is_Full:Page_Stack_Is_Full is error.");
        return 1; // 栈未初始化
    }
    return stack->top == stack->capacity - 1; // 栈满时，top=capacity-1
}


/**
 * @brief 上一页的页面数据（Page_t）出栈
 * @param stack 指向栈结构体的指针
 * @return      栈顶的数据指针，若栈为空返回NULL
 */
static void *Page_Stack_Pop(page_stack_t *stack) {
    if (!stack->stack) {
        LV_LOG_WARN("Page_Stack_Pop:Stack not initialized.");
        return NULL; // 栈未初始化
    }
    if (Page_Stack_Is_Empty(stack)) {
        LV_LOG_INFO("Page_Stack_Pop:Stack underflow! No data to pop.");
        return NULL; // 栈为空
    }

    // 获取栈顶数据并更新栈顶索引
    return stack->stack[stack->top--].data;
}

/**
 * @brief 页面入栈操作
 * @param stack 历史页面栈
 * @param data  要压入栈的数据指针（页面Page_t）
 * @return      0: 成功, -1: 失败（如栈已满）
 */
int Page_Stack_Push(page_stack_t *stack, void *data) {
    if (!stack->stack) {
        LV_LOG_WARN("Page_Stack_Push:Stack not initialized.");
        return -1; // 栈未初始化
    }
    if (Page_Stack_Is_Full(stack)) {
        LV_LOG_ERROR("Page_Stack_Push:Stack overflow! Cannot push data.");
        return -1; // 栈已满
    }

    // 将数据压入栈中
    stack->stack[++stack->top].data = data;
    return 0; // 入栈成功
}


/**
 * @brief 获取栈顶数据(上一页)
 * @param stack 指向栈结构体的指针
 * @return      栈顶的数据指针，若栈为空返回NULL
 */
static void *Page_Stack_Get_Top(const page_stack_t *stack) {
    if (!stack->stack) {
        LV_LOG_WARN("Page_Stack_Get_Top:Stack not initialized.");
        return NULL; // 栈未初始化
    }
    if (Page_Stack_Is_Empty(stack)) {
        LV_LOG_INFO("Page_Stack_Get_Top:Stack is empty! No data on top.");
        return NULL; // 栈为空
    }

    // 返回栈顶数据
    return stack->stack[stack->top].data;
}


/**
 * @brief 所保留的所有历史页面
 * @param stack 指向栈结构体的指针
 */
static void Page_Stack_Clear(page_stack_t *stack) {
    if (!stack->stack) {
        LV_LOG_WARN("Page_Stack_Clear:Stack not initialized.");
        return; // 栈未初始化
    }
    stack->top = -1; // 清空栈内容
}


/**
 * @brief  释放销毁
 * @note   
 * @param  stack: 
 * @retval None
 */
static void Page_Stack_Destroy(page_stack_t* stack) {
    if (stack->stack) {
        free(stack->stack);  // 释放内存
        stack->stack = NULL;  // 设置栈数组为NULL
        stack->top = -1;      // 重置栈顶索引
        stack->capacity = 0;  // 重置栈容量
    }
}