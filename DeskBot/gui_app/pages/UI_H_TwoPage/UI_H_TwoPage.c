#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <time.h>
#include "UI_H_TwoPage.h"

/*******************************图标***********************************************/

/*******************************变量***********************************************/
#define STACK_DEPTH 5
#define MATRIX_SIZE 4
typedef struct
{
    uint16_t Data[STACK_DEPTH];
    uint8_t Top_Point;

}Stack_T;
typedef struct {
    uint16_t score;
    uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE];
    char    * btnm_map[MATRIX_SIZE * MATRIX_SIZE + MATRIX_SIZE];
    uint8_t     game_over;
} ui_Game2048_t;

#define GAME_2048_TEXT_BLACK_COLOR      lv_color_hex(0x6c635b)
#define GAME_2048_TEXT_WHITE_COLOR      lv_color_hex(0Xffffff)

#define GAME_2048_NUMBER_EMPTY_COLOR    lv_color_hex(0xc7b9ac)
#define GAME_2048_NUMBER_2_COLOR        lv_color_hex(0xeee4da)
#define GAME_2048_NUMBER_4_COLOR        lv_color_hex(0xede0c8)
#define GAME_2048_NUMBER_8_COLOR        lv_color_hex(0xf2b179)
#define GAME_2048_NUMBER_16_COLOR       lv_color_hex(0xf59563)
#define GAME_2048_NUMBER_32_COLOR       lv_color_hex(0xf67c5f)
#define GAME_2048_NUMBER_64_COLOR       lv_color_hex(0xf75f3b)
#define GAME_2048_NUMBER_128_COLOR      lv_color_hex(0xedcf72)
#define GAME_2048_NUMBER_256_COLOR      lv_color_hex(0xedcc61)
#define GAME_2048_NUMBER_512_COLOR      lv_color_hex(0xedc850)
#define GAME_2048_NUMBER_1024_COLOR     lv_color_hex(0xedc53f)
#define GAME_2048_NUMBER_2048_COLOR     lv_color_hex(0xedc22e)
static char* Str2048Nums[] = { "2","4","8","16","32","64","128","256","512","1024","2048" };

ui_Game2048_t Game_2048;

lv_obj_t * ui_Game2048Page;
lv_obj_t * ui_Game2048BtnM;
lv_obj_t * ui_Game2048ScLabel;


/*******************************内部静态接口***********************************************/
static void addRandom(uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE]);

static void Stack_init(Stack_T* stack)
{
    memset(stack->Data,0,STACK_DEPTH);
    stack->Top_Point = 0;
}

static uint8_t Stack_Push(Stack_T* stack, uint16_t datain)
{
  if(stack->Top_Point == STACK_DEPTH - 1)
    {return -1;}

    stack->Data[stack->Top_Point++] = datain;
    return 0;
}

static uint8_t Stack_Pop(Stack_T* stack)
{
  if(stack->Top_Point == 0)
    {return -1;}

    stack->Data[stack->Top_Point--] = NULL;
    return 0;
}

static uint8_t Stack_isEmpty(Stack_T* stack)
{
    if(stack->Top_Point == 0)
    {return 1;}

    return 0;
}

static void Stack_Clear(Stack_T* stack)
{
    while(!Stack_isEmpty(stack))
    {
        Stack_Pop(stack);
    }
}

static void init_matrix_num(uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    uint8_t x, y;

    for (x = 0; x < MATRIX_SIZE; x++) {
        for (y = 0; y < MATRIX_SIZE; y++) {
            matrix[x][y] = 0;
        }
    }

    /* 初始化两个随机位置的随机数 */
    addRandom(matrix);
    addRandom(matrix);
}

static void update_btnm_map(char * btnm_map[], uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    uint8_t x, y, index;
    index = 0;
    for (x = 0; x < MATRIX_SIZE; x++)
    {
        for (y = 0; y < MATRIX_SIZE; y++)
        {

            if(((index + 1) % 5) == 0)
                index++;

            if(matrix[x][y] != 0)
            {
                btnm_map[index] = Str2048Nums[matrix[x][y]-1];
            }
            else
                btnm_map[index] = " ";

            index++;
        }
    }
}

static void addRandom(uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    static uint8_t initialized = 0;
    uint16_t x, y;
    uint16_t r, len = 0;
    uint16_t n, list[MATRIX_SIZE * MATRIX_SIZE][2];

    if (!initialized)
    {
      srand((unsigned int)time(NULL));
      initialized = 1;
    }

    for (x = 0; x < MATRIX_SIZE; x++) {
        for (y = 0; y < MATRIX_SIZE; y++) {
            if (matrix[x][y] == 0) {
                list[len][0] = x;
                list[len][1] = y;
                len++;
            }
        }
    }

    if (len > 0) {
        r = rand() % len;
        x = list[r][0];
        y = list[r][1];
        n = ((rand() % 10) / 9) + 1;
        matrix[x][y] = n;
    }
}

static uint8_t move_up(uint16_t * score, uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    uint8_t moved = 0;
    uint8_t x,y;
    Stack_T num_stack;
    Stack_init(&num_stack);
    for (y = 0; y < MATRIX_SIZE; y++)
    {
        for(x=0; x<MATRIX_SIZE; x++)
        {
            if(matrix[MATRIX_SIZE-x-1][y] != 0)
                Stack_Push(&num_stack,matrix[MATRIX_SIZE-x-1][y]);
            else if(x > 0 && matrix[MATRIX_SIZE-x][y] != 0)
                moved = 1;
            if(num_stack.Top_Point>1)
            {
                if(num_stack.Data[num_stack.Top_Point-1] == num_stack.Data[num_stack.Top_Point-2])
                {
                    uint16_t temp;
                    temp = num_stack.Data[num_stack.Top_Point-1] + 1;
                    Stack_Pop(&num_stack);
                    Stack_Pop(&num_stack);
                    Stack_Push(&num_stack,temp);
                    *score += 1<<(temp-1);
                    moved = 1;
                }
            }
        }
        if(moved)
        {
            for(x=0; x<MATRIX_SIZE; x++)
            {
                if(!Stack_isEmpty(&num_stack))
                {
                    matrix[x][y] = num_stack.Data[num_stack.Top_Point-1];
                    Stack_Pop(&num_stack);
                }
                else
                {
                    matrix[x][y] = 0;
                }

            }
        }
        Stack_Clear(&num_stack);
    }
    return moved;
}

static uint8_t move_down(uint16_t * score, uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    uint8_t moved = 0;
    uint8_t x,y;
    Stack_T num_stack;
    Stack_init(&num_stack);
    for (y = 0; y < MATRIX_SIZE; y++)
    {
        for(x=0; x<MATRIX_SIZE; x++)
        {
            if(matrix[x][y] != 0)
                Stack_Push(&num_stack,matrix[x][y]);
            else if(x > 0 && matrix[x-1][y] != 0)
                moved = 1;
            if(num_stack.Top_Point>1)
            {
                if(num_stack.Data[num_stack.Top_Point-1] == num_stack.Data[num_stack.Top_Point-2])
                {
                    uint16_t temp;
                    temp = num_stack.Data[num_stack.Top_Point-1] + 1;
                    Stack_Pop(&num_stack);
                    Stack_Pop(&num_stack);
                    Stack_Push(&num_stack,temp);
                    *score += 1<<(temp-1);
                    moved = 1;

                }
            }
        }
        if(moved)
        {
            for(x=0; x<MATRIX_SIZE; x++)
            {
                if(!Stack_isEmpty(&num_stack))
                {
                    matrix[MATRIX_SIZE-x-1][y] = num_stack.Data[num_stack.Top_Point-1];
                    Stack_Pop(&num_stack);
                }
                else
                {
                    matrix[MATRIX_SIZE-x-1][y] = 0;
                }
            }
        }
        Stack_Clear(&num_stack);
    }
    return moved;
}

static uint8_t move_right(uint16_t * score, uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    uint8_t moved = 0;
    uint8_t x,y;
    Stack_T num_stack;
    Stack_init(&num_stack);
    for (x = 0; x < MATRIX_SIZE; x++)
    {
        for(y=0; y<MATRIX_SIZE; y++)
        {
            if(matrix[x][y] != 0)
                Stack_Push(&num_stack,matrix[x][y]);
            else if(y > 0 && matrix[x][y-1] != 0)
                moved = 1;
            if(num_stack.Top_Point>1)
            {
                if(num_stack.Data[num_stack.Top_Point-1] == num_stack.Data[num_stack.Top_Point-2])
                {
                    uint16_t temp;
                    temp = num_stack.Data[num_stack.Top_Point-1] + 1;
                    Stack_Pop(&num_stack);
                    Stack_Pop(&num_stack);
                    Stack_Push(&num_stack,temp);
                    *score += 1<<(temp-1);
                    moved = 1;

                }
            }
        }
        if(moved)
        {
            for(y=0; y<MATRIX_SIZE; y++)
            {
                if(!Stack_isEmpty(&num_stack))
                {
                    matrix[x][MATRIX_SIZE-y-1] = num_stack.Data[num_stack.Top_Point-1];
                    Stack_Pop(&num_stack);
                }
                else
                {
                    matrix[x][MATRIX_SIZE-y-1] = 0;
                }
            }
        }
        Stack_Clear(&num_stack);
    }
    return moved;
}

static uint8_t move_left(uint16_t * score, uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    uint8_t moved = 0;
    uint8_t x,y;
    Stack_T num_stack;
    Stack_init(&num_stack);
    for (x = 0; x < MATRIX_SIZE; x++)
    {
        for(y=0; y<MATRIX_SIZE; y++)
        {
            if(matrix[x][MATRIX_SIZE-y-1] != 0)
                Stack_Push(&num_stack,matrix[x][MATRIX_SIZE-y-1]);
            else if(y > 0 && matrix[x][MATRIX_SIZE-y] != 0)
                moved = 1;
            if(num_stack.Top_Point>1)
            {
                if(num_stack.Data[num_stack.Top_Point-1] == num_stack.Data[num_stack.Top_Point-2])
                {
                    uint16_t temp;
                    temp = num_stack.Data[num_stack.Top_Point-1] + 1;
                    Stack_Pop(&num_stack);
                    Stack_Pop(&num_stack);
                    Stack_Push(&num_stack,temp);
                    *score += 1<<(temp-1);
                    moved = 1;
                }
            }
        }
        if(moved)
        {
            for(y=0; y<MATRIX_SIZE; y++)
            {
                if(!Stack_isEmpty(&num_stack))
                {
                    matrix[x][y] = num_stack.Data[num_stack.Top_Point-1];
                    Stack_Pop(&num_stack);
                }
                else
                {
                    matrix[x][y] = 0;
                }

            }
        }
        Stack_Clear(&num_stack);
    }
    return moved;
}

static uint8_t game_over(uint16_t matrix[MATRIX_SIZE][MATRIX_SIZE])
{
  uint8_t x, y;
	uint8_t count = 0;

	for (x = 0; x < MATRIX_SIZE; x++) {
		for (y = 0; y < MATRIX_SIZE; y++) {
			if (matrix[x][y] == 0) {
				count++;
			}
		}
	}
	if(count==0)
	{
			return 1;
	}
	return 0;
}

static lv_color_t get_num_color(uint16_t num)
{
    lv_color_t color;

    switch (num)
    {
        case 0:     color = GAME_2048_NUMBER_EMPTY_COLOR;   break;
        case 1:     color = GAME_2048_NUMBER_EMPTY_COLOR;   break;
        case 2:     color = GAME_2048_NUMBER_2_COLOR;       break;
        case 4:     color = GAME_2048_NUMBER_4_COLOR;       break;
        case 8:     color = GAME_2048_NUMBER_8_COLOR;       break;
        case 16:    color = GAME_2048_NUMBER_16_COLOR;      break;
        case 32:    color = GAME_2048_NUMBER_32_COLOR;      break;
        case 64:    color = GAME_2048_NUMBER_64_COLOR;      break;
        case 128:   color = GAME_2048_NUMBER_128_COLOR;     break;
        case 256:   color = GAME_2048_NUMBER_256_COLOR;     break;
        case 512:   color = GAME_2048_NUMBER_512_COLOR;     break;
        case 1024:  color = GAME_2048_NUMBER_1024_COLOR;    break;
        case 2048:  color = GAME_2048_NUMBER_2048_COLOR;    break;
        default:    color = GAME_2048_NUMBER_2048_COLOR;    break;
    }
    return color;
}

static void Game_2048_init(void)
{
    Game_2048.score = 0;
    Game_2048.game_over = 0;

    memset(Game_2048.btnm_map,0,MATRIX_SIZE * MATRIX_SIZE + MATRIX_SIZE);

    Game_2048.btnm_map[4]="\n";
    Game_2048.btnm_map[9]="\n";
    Game_2048.btnm_map[14]="\n";
    Game_2048.btnm_map[19]="";

    init_matrix_num(Game_2048.matrix);
    update_btnm_map(Game_2048.btnm_map, Game_2048.matrix);
    lv_btnmatrix_set_map(ui_Game2048BtnM, Game_2048.btnm_map);

}




/*******************************动画***********************************************/


/*******************************event callback***********************************************/
static void ui_event_Game2048Page(lv_event_t * e)
{
   lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);

    if(event_code == LV_EVENT_GESTURE)
    {
		Game_2048.game_over = game_over(Game_2048.matrix);
        if(!Game_2048.game_over)
        {
            uint8_t moved = 0;
            if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT)
            {
                moved = move_left(&Game_2048.score,Game_2048.matrix);
            }
            else if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
            {
                moved = move_right(&Game_2048.score,Game_2048.matrix);
            }
            else if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP)
            {
                moved = move_up(&Game_2048.score,Game_2048.matrix);
            }
            else if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM)
            {
                moved = move_down(&Game_2048.score,Game_2048.matrix);
            }
            if(moved)
            {
                addRandom(Game_2048.matrix);
                update_btnm_map(Game_2048.btnm_map, Game_2048.matrix);
                lv_btnmatrix_set_map(ui_Game2048BtnM, Game_2048.btnm_map);
                uint8_t strbuf[11];
                sprintf(strbuf,"Sc: %4d",Game_2048.score);
                lv_label_set_text(ui_Game2048ScLabel,strbuf);
            }
        }
    }
}

static void ui_event_Game2048BtnM(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_DRAW_TASK_ADDED)
    {
        lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
        lv_draw_dsc_base_t * base_dsc = lv_draw_task_get_draw_dsc(draw_task);

        /*Change the draw descriptor the button*/
        if(base_dsc->id1 >= 0)
        {
            uint16_t x, y, num;
            x = (uint16_t)((base_dsc->id1) / MATRIX_SIZE);
            y = (base_dsc->id1) % MATRIX_SIZE;
            num = (uint16_t)(1 << (Game_2048.matrix[x][y]));

            lv_draw_fill_dsc_t * fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
            if(fill_draw_dsc)
            {
                fill_draw_dsc->color = get_num_color(num);
            }

            lv_draw_label_dsc_t * label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
            if(label_draw_dsc)
            {
                if (num > 516)
                    label_draw_dsc->color = GAME_2048_TEXT_BLACK_COLOR;
                else
                    label_draw_dsc->color = GAME_2048_TEXT_WHITE_COLOR;
            }

        }
    }
}

static void ui_event_new_game_btn(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_LONG_PRESSED)
    {
        Game_2048_init();
        uint8_t strbuf[11];
        sprintf(strbuf,"Sc: %4d",Game_2048.score);
        lv_label_set_text(ui_Game2048ScLabel,strbuf);
    }
}

static void ui_event_return_btn(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_LONG_PRESSED)
    {
        Page_Manager_RetPrePage(&page_manager);
    }
}


/*******************************初始化和销毁***********************************************/
void ui_TwoPage_Init(void)
{
	
    ui_Game2048Page = lv_obj_create(NULL);

    lv_obj_clear_flag(ui_Game2048Page,LV_OBJ_FLAG_SCROLLABLE);
    ui_Game2048BtnM = lv_btnmatrix_create(ui_Game2048Page);
    lv_obj_add_event_cb(ui_Game2048BtnM, ui_event_Game2048BtnM, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(ui_Game2048BtnM, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
    Game_2048_init();

    lv_obj_set_style_text_font(ui_Game2048BtnM, &lv_font_montserrat_44, 0);
    lv_btnmatrix_set_one_checked(ui_Game2048BtnM,true);
    lv_obj_clear_flag(ui_Game2048BtnM, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_width(ui_Game2048BtnM,0,0);
    lv_obj_set_style_bg_opa(ui_Game2048BtnM,0,0);
    lv_obj_set_size(ui_Game2048BtnM,480,480);
    lv_obj_set_x(ui_Game2048BtnM, -40);
    lv_obj_set_align(ui_Game2048BtnM,LV_ALIGN_BOTTOM_MID);


    lv_obj_t * new_game_btn = lv_btn_create(ui_Game2048Page);
    lv_obj_align(new_game_btn, LV_ALIGN_RIGHT_MID, -5, -100);
    lv_obj_set_size(new_game_btn,200,70);
    lv_obj_t * new_game_btn_label = lv_label_create(new_game_btn);
    lv_label_set_text(new_game_btn_label,"Restart");
    lv_obj_align(new_game_btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(new_game_btn_label, &lv_font_montserrat_44, 0);
    lv_obj_add_event_cb(new_game_btn, ui_event_new_game_btn, LV_EVENT_ALL, NULL);

    lv_obj_t * return_btn = lv_btn_create(ui_Game2048Page);
    lv_obj_align(return_btn, LV_ALIGN_RIGHT_MID, -5, 100);
    lv_obj_set_size(return_btn,200,70);
    lv_obj_t * return_btn_label = lv_label_create(return_btn);
    lv_label_set_text(return_btn_label,"Back");
    lv_obj_align(return_btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(return_btn_label, &lv_font_montserrat_44, 0);
    lv_obj_add_event_cb(return_btn, ui_event_return_btn, LV_EVENT_ALL, NULL);

    ui_Game2048ScLabel = lv_label_create(ui_Game2048Page);
    uint8_t strbuf[10];
    sprintf(strbuf,"Sc: %4d",Game_2048.score);
    lv_label_set_text(ui_Game2048ScLabel,strbuf);
    lv_obj_align(ui_Game2048ScLabel, LV_ALIGN_RIGHT_MID, -6, 0);
    lv_obj_set_style_text_font(ui_Game2048ScLabel, &lv_font_montserrat_44, 0);

    
    lv_obj_add_event_cb(ui_Game2048Page, ui_event_Game2048Page, LV_EVENT_ALL, NULL);
    
    

    // load page
    lv_scr_load_anim(ui_Game2048Page, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, true);

}

void ui_TwoPage_Dinit(void)
{

}