#include <HashMap.h>

int products[] = {
  2, 2, 2, 4, 4, 4, 8, 8, 16, 16, 32, 32, 32
};
const int PRODUCT_COUNT = 13;
const int MAX_SUM = 162;
const int SUM_RANGE = MAX_SUM-2+1;
int sum_range = 0;
int prod_index = 0;

//HashType<char*, int> hashRawArray[SUM_RANGE];
//HashMap<char*, int> sum_lookup = HashMap<char*, int>(hashRawArray, SUM_RANGE);

int sum_lookup[100];
int matrix[PRODUCT_COUNT+1][SUM_RANGE];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  int i = 0;
  int tmp = 0;
  
  initiate_matrix();

  fill_first_row();
  
  fill_matrix();
  // print_matrix();
  
  int subset[PRODUCT_COUNT] = {0};
  int Weight = 20;
  find_subset(subset, Weight);

  Serial.print("LOOKING FOR... ");
  Serial.print(Weight);
  Serial.println();
  Serial.print("SUBSET");
  Serial.println();
  for (i=0; i < PRODUCT_COUNT; i++) {
    Serial.print(subset[i]);
    Serial.print("  ");
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
  for (i = 0; i < PRODUCT_COUNT; i++) {
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
  for (i=0; i < 100; i++) {
      sum_lookup[i] = -1;
  }
  for (i = 0; i < sum_range; i++) {
    sum_lookup[matrix[0][i]] = i;
  }

  // set all to False
  for (prod_index = 1; prod_index <= PRODUCT_COUNT; prod_index++) {
    for (i = 0; i < sum_range; i++) {
      matrix[prod_index][i] = 'F';
    }
  }
}

int find_column_index(int sum) {
  if (sum_lookup[sum] < sum_range && sum_lookup[sum] >= 0) {
     return sum_lookup[sum];
  }
  return -1;
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
  for (row = 2; row <= PRODUCT_COUNT; row++) {
    prod_index = row - 1;
    for (i = 0; i < sum_range; i++) {
      if (products[prod_index] == matrix[0][i] ||
          matrix[row - 1][i] == 'T' ||
          current_sum_possible(row, prod_index, i)) {
        matrix[row][i] = 'T';
      }
    }
  }
}

bool current_sum_possible(int tmp_row, int tmp_index, int j) {
  int column_sum = matrix[0][j] - products[tmp_index];
  int column_index = find_column_index(column_sum);
  if (column_index == -1) return false;
  
  if (matrix[tmp_row - 1][column_index] == 'T') {
    return true;
  }
  else return false;
}

void find_subset(int subset[], int sum) {
  int row;
  int subset_index = 0;
  int column_index = find_column_index(sum);
  int tmp_sum;
  for (row = PRODUCT_COUNT; row > 0; row--) {
    if (column_index == -1) return;
    if (matrix[row][column_index] == 'F') {
      return;
    }
    else if (matrix[row - 1][column_index] == 'T') {
      continue;
    }
    else {
      subset[subset_index] = products[row - 1];
      subset_index++;
      tmp_sum = matrix[0][column_index] - products[row-1];
      column_index = find_column_index(tmp_sum);
    }
  }
  return;
}

void print_matrix() {
  int i=0;
  int tmp=0;
  Serial.print("MATRIX");
  Serial.println();
  for (i=0; i < 31; i++) {
    Serial.print(matrix[0][i]);
    Serial.print(" ");
  }
  Serial.println();

  for (tmp=1; tmp <= PRODUCT_COUNT; tmp++) {
    for(i=0; i <31; i++) {
      Serial.print(char(matrix[tmp][i]));
      Serial.print(" ");
    }
    Serial.println();
  }

  Serial.println();
}

