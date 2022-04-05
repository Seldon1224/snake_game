/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "stdio.h"
#include "music.h"
#include "gui.h"
#include "stdlib.h"
#include "adc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define EMPTY 0
#define SNAKE 1
#define FOOD  2
// direction define
#define RIGHT 0
#define LEFT 1
#define UP 2
#define DOWN 3
// define X, Y
#define X_POS 0
#define Y_POS 1
// game state
#define GAME_INIT 0
#define RUNNING 1
#define GAME_FAIL 2
#define GMAE_SUCCESS 3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
int s[2] = { 0, 0 };
unsigned char table[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f,
		0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x40, };
unsigned char score_str[] = { 0x6d, 0x39, 0x3f, 0x77, 0x79, 0x40 }; //SCORE-XX
int game_score = 0, choose = 1;
char music_state = 0;
// 0:menu, 1:game page, 2:game setting, 3:game history, 4: About
char display_choose = 0; // init = 0
char key_state = 0;
// x: y:1-14
char game_board[24][15] = { 0 };
int val_input = 0;
extern u8 check_flag, time_update_flag;
extern u16 update_time;
u8 time_m, time_s;
u8 direction = RIGHT;
u8 game_state = GAME_INIT, head_index, tail_index;
u8 snake[2][2000], snake_speed = 0;
u8 game_over_show = 0, show_board = 1;
u8 init_len = 3;
/* USER CODE END Variables */
osThreadId gameTaskHandle;
osThreadId musicTaskHandle;
osThreadId updateTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartGameTask(void const *argument);
void StartMusicTask(void const *argument);
void StartUpdateTask(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of gameTask */
	osThreadDef(gameTask, StartGameTask, osPriorityNormal, 0, 128);
	gameTaskHandle = osThreadCreate(osThread(gameTask), NULL);

	/* definition and creation of musicTask */
	osThreadDef(musicTask, StartMusicTask, osPriorityNormal, 0, 128);
	musicTaskHandle = osThreadCreate(osThread(musicTask), NULL);

	/* definition and creation of updateTask */
	osThreadDef(updateTask, StartUpdateTask, osPriorityNormal, 0, 128);
	updateTaskHandle = osThreadCreate(osThread(updateTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartGameTask */
void write_byte(unsigned char date) {
	unsigned char i;
	for (i = 0; i < 8; i++) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, (date >> (7 - i)) & 0x01);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
	}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0);
}

void updateDigitalNum(unsigned char num) {
	int num1 = num % 10;
	int num2 = (num / 10) % 10;
	s[0] = num2;
	s[1] = num1;
	for (char i = 0; i < 6; i++) {
		write_byte(score_str[i]);
		HAL_GPIO_WritePin(GPIOF, 0x01 << (15 - i), 0);
		delay_us(410);
		HAL_GPIO_WritePin(GPIOF, 0x01 << (15 - i), 1);
	}
	for (char i = 6; i < 8; i++) {
		write_byte(table[s[i - 6]]);
		HAL_GPIO_WritePin(GPIOF, 0x01 << (15 - i), 0);
		delay_us(410);
		HAL_GPIO_WritePin(GPIOF, 0x01 << (15 - i), 1);
	}
}

//KB1-KB4: 1-4
//Ka1-Ka5: 5-9
int key_scan() {
	u32 val = 0;
	float y = 0;
	char key_result = 0;
	int time = 12000;

	if (HAL_GPIO_ReadPin(KB1_GPIO_Port, KB1_Pin) == 0) {
		delay_us(time);
		if (HAL_GPIO_ReadPin(KB1_GPIO_Port, KB1_Pin) == 0) {
			key_result = 1;
		}
	} else if (HAL_GPIO_ReadPin(KB2_GPIO_Port, KB2_Pin) == 0) {
		delay_us(time);
		if (HAL_GPIO_ReadPin(KB2_GPIO_Port, KB2_Pin) == 0) {
			key_result = 2;
		}
	} else if (HAL_GPIO_ReadPin(KB3_GPIO_Port, KB3_Pin) == 0) {
		delay_us(time);
		if (HAL_GPIO_ReadPin(KB3_GPIO_Port, KB3_Pin) == 0) {
			key_result = 3;
		}
	} else if (HAL_GPIO_ReadPin(KB4_GPIO_Port, KB4_Pin) == 0) {
		delay_us(time);
		if (HAL_GPIO_ReadPin(KB4_GPIO_Port, KB4_Pin) == 0) {
			key_result = 4;
		}
	}
	HAL_ADC_Start(&hadc2);
	HAL_ADC_PollForConversion(&hadc2, 100);
	if (HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc2), HAL_ADC_STATE_REG_EOC)) {
		val = HAL_ADC_GetValue(&hadc2);
		y = val * 3.3 / 4096;
		int ka = ((int) (y * 10));
		if (ka == 4 || ka == 5) {
			key_result = 5;
		} else if (ka == 8 || ka == 9) {
			key_result = 6;
		} else if (ka == 11 || ka == 12) {
			key_result = 7;
		} else if (ka == 14) {
			key_result = 8;
		} else if (ka == 16) {
			key_result = 9;
		}
		return key_result;
	}
}

void drawSquare(u16 x1, u16 y1, u16 width, u16 color) {
	LCD_Fill(x1, y1, x1 + width, y1 + width, color);
}

void drawSquareInBoard(u16 x, u16 y, u16 width, u16 color) {
	u16 x_start = 70 + x * width;
	u16 y_start = 10 + y * width;
	LCD_Fill(y_start, x_start, y_start + width, x_start + width, color);
}

void setGameBoard(u8 x, u8 y, u8 state) {
	game_board[x][y] = state;
}

u8 getGameBoard(u8 x, u8 y) {
	return game_board[x][y];
}

void generateFood() {
	u8 x = rand() % 23;
	u8 y = rand() % 14;
	while (game_board[x][y] == SNAKE) {
		x = rand() % 23;
		y = rand() % 14;
	}
	game_board[x][y] = FOOD;
}
void LED_Flow(u8 speed) {
	for (int i = 0; i < 8; i++) {
		GPIOF->ODR &= 0xFFFFFF00;
		GPIOF->ODR |= 1 << i;
		if (i == 7)
			delay_us((101 - speed) * 600);
		else
			delay_us((101 - speed) * 1000);
	}
}

void updateSnakePos() {
	char next_x = snake[X_POS][head_index], next_y = snake[Y_POS][head_index];
	head_index++;
	if (head_index > 1999) {
		head_index = 0;
	}
	if (key_state == 1 && direction != RIGHT)
		direction = LEFT;
	else if (key_state == 2 && direction != DOWN)
		direction = UP;
	else if (key_state == 3 && direction != UP)
		direction = DOWN;
	else if (key_state == 4 && direction != LEFT)
		direction = RIGHT;
	// handler direction choose
	if (direction == RIGHT) {
		next_y++;
	} else if (direction == LEFT) {
		next_y--;
	} else if (direction == UP) {
		next_x--;
	} else if (direction == DOWN) {
		next_x++;
	}
	// check edge
	if (next_y == 14 || next_y == 255 || next_x == 23 || next_x == 255) {
		game_state = GAME_FAIL;
		game_over_show = 1;
		return;
	}
	// get food
	if (getGameBoard(next_x, next_y) == FOOD) {
		music_state = 1;
		snake[Y_POS][head_index] = next_y;
		snake[X_POS][head_index] = next_x;
		setGameBoard(next_x, next_y, SNAKE);
		head_index++;
		if (head_index > 1999) {
			head_index = 0;
		}
		if (direction == RIGHT) {
			next_y++;
		} else if (direction == LEFT) {
			next_y--;
		} else if (direction == UP) {
			next_x--;
		} else if (direction == DOWN) {
			next_x++;
		}
		generateFood();
		game_score++;
	}
	snake[Y_POS][head_index] = next_y;
	snake[X_POS][head_index] = next_x;
	setGameBoard(next_x, next_y, SNAKE);
	setGameBoard(snake[X_POS][tail_index], snake[Y_POS][tail_index], EMPTY);
	tail_index++;
	if (tail_index > 1999) {
		tail_index = 0;
	}
}

void showGameBoard() {
	for (u8 i = 0; i < 23; i++) {
		for (u8 j = 0; j < 14; j++) {
			if (game_board[i][j] == SNAKE)
				drawSquareInBoard(i, j, 10, RED);
			else if (game_board[i][j] == FOOD)
				drawSquareInBoard(i, j, 10, BROWN);
			else
				drawSquareInBoard(i, j, 10, WHITE);
		}
	}
}

void initGameState() {
	for (u8 i = 0; i < 23; i++) {
		for (u8 j = 0; j < 14; j++) {
			game_board[i][j] = EMPTY;
		}
	}
	// init snake position
	for (u8 i = 0; i < init_len; i++) {
		game_board[5][3 + i] = SNAKE;
		snake[X_POS][i] = 5;
		snake[Y_POS][i] = 3 + i;
	}
	tail_index = 0;
	head_index = init_len - 1;
	game_state = RUNNING;
	direction = RIGHT;
	game_score = 0;
	time_m = 0;
	time_s = 0;
	generateFood();
}
/**
 * @brief  Function implementing the gameTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGameTask */
void StartGameTask(void const *argument) {
	/* USER CODE BEGIN StartGameTask */
	/* Infinite loop */
	char buff[20];
	for (;;) {
		if (check_flag == 1 && display_choose == 1) {
			if (game_state == RUNNING) {
				updateSnakePos();
			}
			if (game_state == GAME_FAIL) {
				// game over
				if (game_over_show) {
					showGameBoard();
					LCD_Fill(38, 120, 180, 230, LIGHTGRAY);
					// show string
					Show_Str(80, 130, RED, BLUE, "Game Over", 16, 1);
					sprintf(buff, "Score: %d", game_score);
					Show_Str(40, 150, RED, BLUE, buff, 16, 1);
					sprintf(buff, "Time used: %.2d:%.2d", time_m, time_s);
					Show_Str(40, 170, RED, BLUE, buff, 16, 1);
					Show_Str(40, 190, MAGENTA, DARKBLUE, "Press KB1 return", 16,
							1);
					Show_Str(40, 210, MAGENTA, DARKBLUE, "Press KB2 restart",
							16, 1);
					game_over_show = 0;
				}
				// handle
				if (key_state == 1) {
					// return menu
					display_choose = 0;
					LCD_Clear(WHITE);
				} else if (key_state == 2) {
					// restart
					game_state = GAME_INIT;
					show_board = 1;
					LCD_Clear(WHITE);
				}
			}
			check_flag = 0;
		}

		if (time_update_flag
				== 1&& display_choose == 1 && game_state == RUNNING) {
			time_s++;
			if (time_s == 60) {
				time_m++;
				time_s = 0;
			}
			time_update_flag = 0;
		}

		// 数码管
		updateDigitalNum(game_score);
		osDelay(1);
	}
	/* USER CODE END StartGameTask */
}

/* USER CODE BEGIN Header_StartMusicTask */
/**
 * @brief Function implementing the musicTask thread.
 * @param argument: Not used
 * @retval None
 */

/* USER CODE END Header_StartMusicTask */
void StartMusicTask(void const *argument) {
	/* USER CODE BEGIN StartMusicTask */
	/* Infinite loop */

	for (;;) {
		 music_play(music_state);
		 music_state = 0;
		// 流水灯
		LED_Flow(snake_speed);
		osDelay(1);
	}
	/* USER CODE END StartMusicTask */
}

/* USER CODE BEGIN Header_StartUpdateTask */

/**
 * @brief Function implementing the updateTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartUpdateTask */
void StartUpdateTask(void const *argument) {
	/* USER CODE BEGIN StartUpdateTask */
	/* Infinite loop */
	char board_width = 10;
	char buff[10];
	u8 i = 0, j = 0, choose_setting = 1;
	tail_index = 0;
	head_index = init_len - 1;
	u32 val0 = 0;
	float x = 0;
	u8 update_speed = 0;
	snake_speed = 11 - (update_time / 10);
	for (;;) {
		//LCD_Clear(WHITE);
		// update snake speed
		if (display_choose == 0) {
			// game menu
			Gui_StrCenter(0, 30, RED, BLUE, "Greedy Snake Game", 16, 1); //居中显示
			Gui_StrCenter(0, 60, RED, BLUE, "Game Menu", 16, 1);      //居中显示

			Show_Str(60, 90, MAGENTA, DARKBLUE, "1. Start Game", 16, 1);
			Show_Str(60, 120, MAGENTA, DARKBLUE, "2. Game Setting", 16, 1);
			Show_Str(60, 150, MAGENTA, DARKBLUE, "3. About", 16, 1);
			Show_Str(30, 90 + (choose - 1) * 30, RED, BLUE, ">>>", 16, 1);
		} else if (display_choose == 1 && game_state == GAME_INIT) { // game init
			initGameState();
		} else if (display_choose == 1 && game_state == RUNNING) {
			// game page
			Gui_StrCenter(0, 30, RED, BLUE, "Greedy Snake Game", 16, 1); //居中显示
			// game board show
			if (show_board) {
				for (char i = 0; i < 16; i++) {
					drawSquare(0 + i * board_width, 60, board_width,
					DARKBLUE); // ----
					drawSquare(i * board_width, 300, board_width, DARKBLUE); // ----
				}
				for (char i = 0; i < 24; i++) {
					drawSquare(0, 60 + (i + 1) * board_width, board_width,
					DARKBLUE);            // |
					drawSquare(15 * board_width, 70 + i * board_width,
							board_width, DARKBLUE);   // |
				}
				show_board = 0;
			}
			// show score
			Show_Str(170, 180, MAGENTA, DARKBLUE, "Score:", 16, 1);
			sprintf(buff, "    %d", game_score);
			Show_Str(170, 200, BLUE, LGRAY, buff, 16, 0);
			// show time
			Show_Str(170, 220, MAGENTA, DARKBLUE, "Time:", 16, 1);
			sprintf(buff, "%.2d:%.2d", time_m, time_s);
			Show_Str(170, 240, BLUE, LGRAY, buff, 16, 0);
			// show snake
			showGameBoard();
		} else if (display_choose == 2) { //game setting
			// game setting
			Gui_StrCenter(0, 30, RED, BLUE, "Greedy Snake Game", 16, 1); //居中显示
			Gui_StrCenter(0, 60, RED, BLUE, "Game Setting", 16, 1);   //居中显示

			Show_Str(30, 90, MAGENTA, DARKBLUE, "1. Snake initial length:", 16,
					1);
			sprintf(buff, "<   %d   >", init_len);
			Show_Str(60, 120, BLUE, DARKBLUE, buff, 16, 1);
			Show_Str(30, 150, MAGENTA, DARKBLUE, "2. Snake speed:", 16, 1);
			sprintf(buff, "<   %d   >", snake_speed);
			Show_Str(60, 180, BLUE, DARKBLUE, buff, 16, 1);
			Show_Str(0, 150 + (choose_setting - 1) * 60, RED, BLUE, ">>>", 16,
					1);

			Show_Str(30, 210, MAGENTA, DARKBLUE, "Press Ka1 return menu", 16,
					1);
			// 修改update_time
			if (choose_setting == 1) {
				snake_speed = val_input / 10;
				update_time = (11 - snake_speed) * 10;
				sprintf(buff, "<   %d   >", snake_speed);
				Show_Str(60, 180, MAGENTA, WHITE, "          ", 16, 0);
				Show_Str(60, 180, BLUE, DARKBLUE, buff, 16, 1);
			} else if (choose_setting == 0) {
				if (key_state == 6) {
					init_len++;
					if (init_len > 5)
						init_len = 5;
				} else if (key_state == 7) {
					init_len--;
					if (init_len < 1)
						init_len = 1;
				}
				sprintf(buff, "<   %d   >", init_len);
				Show_Str(60, 120, MAGENTA, WHITE, "          ", 16, 0);
				Show_Str(60, 120, BLUE, DARKBLUE, buff, 16, 1);
			}

		} else if (display_choose == 3) {
			// game About
			Gui_StrCenter(0, 30, RED, BLUE, "Greedy Snake Game", 16, 1); //居中显示
			Gui_StrCenter(0, 60, RED, BLUE, "Game About", 16, 1);   //居中显示
			Gui_StrCenter(0, 90, RED, BLUE, "2018112651  LiXiaodong", 16, 1);   //居中显示
			Gui_StrCenter(0, 120, RED, BLUE, "2018112630  GaoZhichao", 16, 1);   //居中显示
			Gui_StrCenter(0, 150, RED, BLUE, "2018112657  YuanHuajun", 16, 1);  //居中显示
			Gui_StrCenter(0, 180, RED, BLUE, "2018112721  AiErken", 16, 1);   //居中显示
		}
		// 获取按键状态
		key_state = key_scan();
		if (key_state == 1) {
			if (display_choose == 2) {
				Show_Str(0, 150 + (choose_setting - 1) * 60, RED, WHITE, "   ",
						16, 0);
			} else if (display_choose == 0) {
				Show_Str(30, 90 + (choose - 1) * 30, RED, WHITE, "   ", 16, 0);
			}
			choose--;
			if (choose <= 0)
				choose = 3;
			choose_setting = (choose_setting == 1) ? 0 : 1;
		} else if (key_state == 2) {
			if (display_choose == 2) {
				Show_Str(0, 150 + (choose_setting - 1) * 60, RED, WHITE, "   ",
						16, 0);
			} else if (display_choose == 0) {
				Show_Str(30, 90 + (choose - 1) * 30, RED, WHITE, "   ", 16, 0);
			}
			choose++;
			if (choose > 3)
				choose = 1;
			choose_setting = (choose_setting == 1) ? 0 : 1;
		} else if ((key_state == 3 || key_state == 4) && display_choose == 0) {
			// comfirm
			display_choose = choose;
			if (display_choose == 1) {
				show_board = 1;
				game_state = GAME_INIT;
			}
			LCD_Clear(WHITE);
		} else if (key_state == 5
				&& (display_choose == 2 || display_choose == 3)) {
			// press ka1
			display_choose = 0;
			LCD_Clear(WHITE);
		}
		// 读取可调变压器值
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 100);
		if (HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC)) {
			val0 = HAL_ADC_GetValue(&hadc1);
			x = val0 * 3.3 / 4096;
			float get = ((x * 100)) / 3.2;
			val_input = ((int) get);
		}
	}
	osDelay(1);
}
/* USER CODE END StartUpdateTask */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
