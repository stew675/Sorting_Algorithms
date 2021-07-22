#include <stdio.h>
#include <stdlib.h>

static uint32_t
maximum(uint32_t *array, int size)
{
	uint32_t max = 0;

	for(int curr = 0; curr < size; curr++) {
		if(array[curr] > max) {
			max = array[curr];
		}
	}
	return max;
}

void
countingSort(uint32_t *array, int size)
{
	int curr = 0;
	uint32_t max = maximum(array, size);
	uint32_t *counting_array = calloc(max, sizeof(*counting_array)); // Zeros out the array

	for(curr = 0; curr < size; curr++) {
		counting_array[array[curr]]++;
	}

	uint32_t num = 0;

	curr = 0;

	while(curr <= size) {
		while(counting_array[num] > 0) {
			array[curr] = num;
			counting_array[num]--;
			curr++;
			if(curr > size) {
				break;
			}
		}
		num++;
	}
} // countingSort
