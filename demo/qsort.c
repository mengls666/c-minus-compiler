void qsort(int a[], int start, int stop) {
	int key;
	int i;
	int j;
	int flag;
    /* test comment */
	int temp;

	i = start;
	j = stop;
	key = a[start];
	flag = 0;

	while(i < j) {
		if(flag == 0) {
			if(a[j] < key) {
				temp = a[j];
				a[j] = a[i];
				a[i] = temp;
				flag = 1;
			} else {
				j = j - 1;
			}
		} else {
			if(a[i] > key) {
				temp = a[j];
				a[j] = a[i];
				a[i] = temp;
				flag = 0;
			} else {
				i = i + 1;
			}
		}
	}

	if(start < stop) {
		qsort(a, start, i-1);
		qsort(a, i+1, stop);
	}

}


int main(void) {

	int a[10];
	int i;

	i = 0;

	while(i < 10) {
		a[i] = input();
		i = i + 1;
	}

	qsort(a, 0, 9);

	i = 0;

	while(i < 10) {
		output(a[i]);
		i = i + 1;
	}	

	return 0;
}
