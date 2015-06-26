int matrix[5][31];
int products[] = {
  5, 7, 10, 13
};
int product_count = 4;
int sum_range = 0;
int prod_index = 0;
int sum_lookup[31];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  int i = 0;
  
  initiate_matrix();
  fill_first_row();
  fill_matrix();

  int subset[10];
  find_subset(subset, 17);

  for (i=0; i < 10; i++) {
    Serial.print(subset[i] + ' ');
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}

void initiate_matrix() {
  int i = 0;
  int smallest = products[i];
  int max_sum = 0;

  // determine smallest and largest sums
  for (i = 0; i < product_count; i++) {
    max_sum += products[i];
    if (products[i] < smallest) {
      smallest = products[i];
    }
  }

  // create first row of matrix
  sum_range = max_sum - smallest + 1;
  for (i = 0; i < sum_range; i++) {
    matrix[0][i] = smallest + i;
  }

  // build lookup for sums
  for (i = 0; i < sum_range; i++) {
    sum_lookup[matrix[0][i]] = i;
  }

  // set all to False
  for (prod_index = 1; prod_index <= product_count; prod_index++) {
    for (i = 0; i < sum_range; i++) {
      matrix[prod_index][i] = 'F';
    }
  }
}

void fill_first_row() {
  int i;
  for (i = 0; i < sum_range; i++) {
    if (products[0] == matrix[0][i]) {
      matrix[1][i] = 'T';
    }
  }
}

void fill_matrix() {
  int i = 0;
  int row;
  for (row = 2; row <= product_count; row++) {
    prod_index = row - 1;
    for (i = 0; i < sum_range; i++) {
      if (products[prod_index] == matrix[0][i] ||
          matrix[row - 1][i] == 'T' ||
          current_sum_possible(row, prod_index, i)) {
        matrix[prod_index][i] = 'T';
      }
    }
  }
}

bool current_sum_possible(int row, int prod_index, int i) {
  int column_sum = matrix[0][i] - products[prod_index];
  int column_index = sum_lookup[column_sum];
  if (matrix[row - 1][column_index] == 'T') {
    return true;
  }
  return false;
}

void find_subset(int sub[], int sum) {
  int row;
  int subset_index = 0;
  int column_index = sum_lookup[sum];
  for (row = product_count; row > 0; row--) {
    if (matrix[row][column_index] == 'F') {
      return;
    }
    else if (matrix[row - 1][column_index] == 'T') {
      continue;
    }
    else {
      sub[subset_index] = products[row - 1];
      subset_index++;
      column_index = sum_lookup[matrix[0][column_index] - products[row-1]];
    }
  }
  return;
}

