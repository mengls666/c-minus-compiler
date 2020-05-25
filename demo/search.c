int search(int a[], int len, int target) {

	int i;
	int j;
	int mid;
	int loop;
	int index;

	loop = 1;
	i = 0;
	j = len-1;
	mid = (i + j)/2;
	index = 1000;

	while(((loop == 1) * (i < j)) == 1) {
		if(a[mid] == target) {
			loop = 0;
			index = mid;
		} else {
			if(a[mid] > target) {
				j = mid;
				mid = (i + j) / 2;
			} else {
				i = mid + 1;
				mid = (i + j) / 2;
			}
		}
	}

	return index;

}

int main(void) {

	int a[1000];
	int i;
	int index;

	int x;
	int y;

    x = 10;
	i = 0;

	while(i < x) {
		a[i] = input();
		i = i + 1;
	}

	y = input();

	index = search(a, x, y);

	output(index);

	return 0;

}
