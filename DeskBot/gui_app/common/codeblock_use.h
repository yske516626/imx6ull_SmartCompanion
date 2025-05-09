
#ifndef __CODEBLOCK_USE_H_
#define __CODEBLOCK_USE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define USE_CODEBLOCK 0  //是否使用codeblock仿真器运行lvgl

#if USE_CODEBLOCK
#include "../../lvgl.h"
    typedef void (*lv_anim_completed_cb_t)(lv_anim_ready_cb_t*);
    static inline void lv_anim_set_completed_cb(lv_anim_t * a, lv_anim_completed_cb_t ready_cb)
    {
        a->ready_cb = ready_cb;
    }
#else

#include "../../lvgl/lvgl.h"


//#include "../../common/sys_manager/sys_manager.h"
//#include "../../common/gpio_manager/gpio_manager.h"
#endif
    


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // __FUNC_PAGEMANAGER_H_
