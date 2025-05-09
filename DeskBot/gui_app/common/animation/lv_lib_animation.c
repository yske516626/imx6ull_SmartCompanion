#include "lv_lib_animation.h"

/**
 * @brief  设置图形对象的 x 坐标（水平位
 * @note   
 * @param  var: 
 * @param  v: 
 * @retval None
 */
void lv_lib_anim_callback_set_x(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}


/**
 * @brief  设置图形对象的 y 坐标（垂直位置）
 * @note   
 * @param  var: 
 * @param  v: 
 * @retval None
 */
void lv_lib_anim_callback_set_y(void * var, int32_t v)
{
    lv_obj_set_y(var, v);
}

/**
 * @brief  设置图形对象的高度
 * @note   
 * @param  var: 
 * @param  v: 
 * @retval None
 */
void lv_lib_anim_callback_set_hight(void * var, int32_t v)
{
    lv_obj_set_height(var, v);
}

/**
 * @brief  设置图形对象的宽度
 * @note   
 * @param  var: 
 * @param  v: 
 * @retval None
 */
void lv_lib_anim_callback_set_width(void * var, int32_t v)
{
    lv_obj_set_width(var, v);
}

/**
 * @brief  设置图像的旋转角度
 * @note   
 * @param  var: 
 * @param  v: 
 * @retval None
 */
void lv_lib_anim_callback_set_image_angle(void * var, int32_t v)
{
#if USE_CODEBLOCK
    lv_img_set_angle(var, v);
#else
    lv_image_set_rotation(var, v);
    
#endif
}


/**
 * @brief  设置图像的缩放比例
 * @note   
 * @param  var: 
 * @param  v: 
 * @retval None
 */
void lv_lib_anim_callback_set_scale(void * var, int32_t v)
{
    
#if USE_CODEBLOCK
    lv_img_set_zoom(var, v);
#else
    lv_image_set_scale(var, v);
#endif
}

/**
 * @brief  设置图形对象的不透明度（透明度）
 * @note   
 * @param  var: 
 * @param  v: 
 * @retval None
 */
void lv_lib_anim_callback_set_opacity(void * var, int32_t v)
{
    lv_obj_set_style_opa(var, v, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/**
 * @brief  创建并启动一个自定义动画
 * @note   
 * @param  TagetObj: 需要执行动画的目标对象
 * @param  delay: 动画开始前的延迟时间
 * @param  time: 动画的持续时间（单位：毫秒）
 * @param  start_value: 动画开始时的值
 * @param  end_value: 动画结束时的值
 * @param  playback_delay: 播放回放的延迟时间
 * @param  playback_time: 播放回放的持续时间
 * @param  repeat_delay: 每次重复之间的延迟时间
 * @param  repeat_count: 动画重复的次数
 * @param  path_cb: 动画路径回调函数，决定动画的变化方式
 * @param  exec_cb: 执行回调函数，用来执行具体的操作
 * @param  completed_cb: 动画完成时调用的回调函数
 * @retval None
 */
void lv_lib_anim_user_animation(lv_obj_t * TagetObj, uint16_t delay, uint16_t time, int16_t start_value, int16_t end_value,
                                uint16_t playback_delay, uint16_t playback_time, uint16_t repeat_delay, uint16_t repeat_count,
                                lv_anim_path_cb_t path_cb, lv_anim_exec_xcb_t exec_cb, lv_anim_completed_cb_t completed_cb)
{
    lv_anim_t Animation;
    lv_anim_init(&Animation);
    lv_anim_set_var(&Animation, TagetObj);
    lv_anim_set_time(&Animation, time);
    lv_anim_set_values(&Animation, start_value, end_value);
    lv_anim_set_exec_cb(&Animation, exec_cb);
    lv_anim_set_path_cb(&Animation, path_cb);
    lv_anim_set_delay(&Animation, delay);
    lv_anim_set_playback_time(&Animation, playback_time);
    lv_anim_set_playback_delay(&Animation, playback_delay);
    lv_anim_set_repeat_count(&Animation, repeat_count);
    lv_anim_set_repeat_delay(&Animation, repeat_delay);
    lv_anim_set_early_apply(&Animation, false);
    lv_anim_set_completed_cb(&Animation, completed_cb);
    
    lv_anim_start(&Animation);
}

