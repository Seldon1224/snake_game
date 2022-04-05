#include "music.h"
#include "gpio.h"

int t_num = 30;
int k_num = 3;
void music_play(unsigned char flag) {
	unsigned int s1[] = {
	so, mi, so, 0, so, mi,
	so, 0, so, mi, la, so, 0, so, mi,
	fa, 0, fa, re, 0, re, fa, 0, fa, re,
	so, fa, mi, re, mi, Do, 0, Do, 0 };
	unsigned int s2[] = { 18, 18, 9, 2, 9, 18, 9, 2, 9, 9, 9, 9, 2, 9, 18, 9, 2,
			9, 9, 2, 9, 9, 2, 9, 18, 9, 9, 9, 9, 9, 9, 2, 9, 0 };
	int i, j, k;
	// get score sound
	if (flag == 1) {
		for (j = 0; j < 9 * 15; j++) {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
			for (k = 0; k < mi * 1; k++) {
			}
		}
	}
// fail sound
	else if (flag == 2) {
		i = 24;
		for (j = 0; j < s2[i] * 30; j++) {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
			for (k = 0; k < s1[i] * 15; k++)
				;
		}
	} else if (flag == 3) {
		i = 5;
		for (j = 0; j < s2[i] * 30; j++) {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
			for (k = 0; k < s1[i] * 15; k++)
				;
		}

	} else {
		// stop
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	}

}
