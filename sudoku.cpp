/**
 * Written by Orçun Altınsoy on June 2011
 */

#include <stdio.h>
#include <stdlib.h>

#define ROW 729
#define COLUMN 324

#define CSIZE 1 // sütun isimleri boyutu
#define SSIZE 81 // çözüm boyutu
#define INFINITY 0x7FFFFFF

int numberofsolutions , sudoku[9][9];
int start[ROW][COLUMN]; // 1 ve 0'lar bu matriste oluşturulacak (başlangıç değerleri 0)

struct data
{
       int row; // her data nesnesi bulunduğu satır ve sütun numarasını tutuyor (zorunlu değil -
       int column; // - ama algoritmayı takip etmek mümkün ve kodlar daha anlaşılır oluyor)  
       data *L; // sol
       data *R; // sağ
       data *U; // yukarı
       data *D; // aşağı
       data *C; // sütun başlığı
       int S; // sütundaki 1'lerin sayısı (yalnızca sütun başlığında bulunuyor)
       char N[CSIZE]; // sütun ismi (kullanılmıyor) 
};

data *matrix[ROW + 1][COLUMN + 1]; 
// bellek büyük problem olduğu için matrix'de 0 olan yerlerde bellek tahsis edilmemeli
data *h; // kök (yalnızca L ve R elemanları kullanılıyor)

data *O[SSIZE]; // çözümler
 
void initialize() 
{
     int row , column;
     int i;
     data *c , *r;
// matrix için yer ayırma işlemleri (row ve column elemanları da atanıyor)
     for(row = 0 ; row <= ROW ; row++) for(column = 0 ; column <= COLUMN ; column++)
      if(!row)
      {
              matrix[row][column] = (data*)malloc(sizeof(data));
              matrix[row][column] -> row = row;
              matrix[row][column] -> column = column;
      }
      else if(row && column && start[row - 1][column - 1])
      {
           matrix[row][column] = (data*)malloc(sizeof(data));
           matrix[row][column] -> row = row;
           matrix[row][column] -> column = column;
      }    
// matrix bağları oluşturuluyor (0. satır = sütun başlıkları)    
     for(row = 0 ; row <= ROW ; row++) for(column = 0 ; column <= COLUMN ; column++) 
      if(matrix[row][column])
      { 
                             i = column;
                             do i ? --i : i = COLUMN; while(!matrix[row][i]);
                             matrix[row][column] -> L = matrix[row][i];
             
                             i = column; 
                             do ++i %= COLUMN + 1; while(!matrix[row][i]);
                             matrix[row][column] -> R = matrix[row][i];
             
                             i = row; 
                             do i ? --i : i = ROW; while(!matrix[i][column]); 
                             matrix[row][column] -> U = matrix[i][column];
             
                             i = row;
                             do ++i %= ROW + 1; while(!matrix[i][column]);
                             matrix[row][column] -> D = matrix[i][column];
             
                             if(row) matrix[row][column] -> C = matrix[0][column];
     }
// sütun büyüklükleri oluşturuluyor
     h = matrix[0][0];    
     for(c = h -> R ; c != h ; c = c -> R)
     {
           c -> S = 0;
           for(r = c -> D ; r != c ; r = r -> D) c -> S++;
     }
}

void showlinks() // kontrol amaçlı (zorunlu değil) bu fonksiyon istenilen yerde çağırılıp -
{ // - linklerin durumu kontrol edilebilir
     int row , column;
     
     printf("left links\n");
     for(row = 0 ; row <= ROW ; row++)
     {
             for(column = 0 ; column <= COLUMN ; column++) if(matrix[row][column])
              printf("(%d , %d) <- (%d , %d)  " , 
              matrix[row][column] -> row , matrix[row][column] -> column , 
              matrix[row][column] -> L -> row , matrix[row][column] -> L -> column);
             printf("\n");
     }
     
     printf("right links\n");
     for(row = 0 ; row <= ROW ; row++)
     {
             for(column = 0 ; column <= COLUMN ; column++) if(matrix[row][column])
              printf("(%d , %d) -> (%d , %d)  " , 
              matrix[row][column] -> row , matrix[row][column] -> column , 
              matrix[row][column] -> R -> row , matrix[row][column] -> R -> column);
             printf("\n");
     }
     
     printf("up links\n");
     for(column = 0 ; column <= COLUMN ; column++)
     {
                for(row = 0 ; row <= ROW ; row++) if(matrix[row][column])
                 printf("(%d , %d) /\\ (%d , %d)  " , 
                 matrix[row][column] -> row , matrix[row][column] -> column , 
                 matrix[row][column] -> U -> row , matrix[row][column] -> U -> column);
                printf("\n");
     }
     
     printf("down links\n");
     for(column = 0 ; column <= COLUMN ; column++)
     {
                for(row = 0 ; row <= ROW ; row++) if(matrix[row][column])
                 printf("(%d , %d) \\/ (%d , %d)  " , 
                 matrix[row][column] -> row , matrix[row][column] -> column , 
                 matrix[row][column] -> D -> row , matrix[row][column] -> D -> column);
                printf("\n");
     }
     
     printf("headers\n");
     for(row = 1 ; row <= ROW ; row++)
     {
             for(column = 0 ; column <= COLUMN ; column++) if(matrix[row][column])
              printf("(%d , %d) 's (%d , %d)  " , 
              matrix[row][column] -> row , matrix[row][column] -> column , 
              matrix[row][column] -> C -> row , matrix[row][column] -> C -> column);
             printf("\n");
     }
     
     printf("sizes\n");
     for(column = 1 ; column <= COLUMN ; column++) if(matrix[0][column])
      printf("(%d , %d) 's %d    " , 
             matrix[0][column] -> row , matrix[0][column] -> column , matrix[0][column] -> S);
     printf("\n"); 
}

void write(int solution[4]) // 4 sayıdan oluşan çözümü yorumlar ve sudoku dizisine yazar
{
     int i , j , t , min; 
// dizi sıralanıyor
     for(i = 0 ; i < 3 ; i++)
     {
           min = i;
           for(j = i + 1 ; j < 4 ; j++)
           {
               if(solution[min] > solution[j])
               min = j;
           }
           t = solution[min];
           solution[min] = solution[i];
           solution[i] = t;
     }
     // for(i = 0 ; i < 4 ; i++) printf("%d " , solution[i]); printf("\n");
/*
solution dizisindeki sayılardan en küçüğü kare numarasını belirtir. En küçük ikinci sayıdan da, 
hangi satırda hangi rakamın olduğu bilgisi olduğundan bu iki sayının işlenmesi yeterlidir. 
*/
     sudoku[(solution[0] - 1) / 9][(solution[0] - 1) % 9] = solution[1] % 9 ? solution[1] % 9 : 9;
}

void print(int k) // çözümü yaz
{
     printf("SOLUTION FOUND\n");
     
     int i , j; 
     data *r;
     
     for(i = 0 ; i < k ; i++)
     {
           int in = 0 , solution[4]; // her satır 4 sayıdan (sütun numaraları) oluşuyor
           solution[in++] = O[i] -> C -> column; // bu sayılar solution dizisine alınıyor
           for(r = O[i] -> R ; r != O[i] ; r = r -> R) solution[in++] = r -> C -> column;
           write(solution); // 4 sayıdan oluşan çözümü yorumlar
           // getchar();
     }
// sudokuyu göster    
     for(i = 0 ; i < 9 ; i++) 
     {
           for(j = 0 ; j < 9 ; j++) printf("%d " , sudoku[i][j]);
           printf("\n");
     }
     printf("Press 'Enter' to continue searching...\n");
     numberofsolutions++;
     
     getchar();
}

data* choose() // sütun seç
{
      int s = INFINITY;
      data *c;
      data *j;
// en küçük ilk sütun seçiliyor      
      for(j = h -> R ; j != h ; j = j -> R) if(j -> S < s)
      {
            c = j;
            s = j -> S;
      }
      
      return c;
}

void cover(data *c)
{
     // printf("covering column %d\n" , c -> column);
     
     data *i;
     data *j;
     
     // printf("removing (%d , %d) horizontally\n" , c -> row , c -> column);
     c -> R -> L = c -> L;
     c -> L -> R = c -> R;
     
     for(i = c -> D ; i != c ; i = i -> D) for(j = i -> R ; j != i ; j = j -> R)
     {
           // printf("removing (%d , %d) vertically\n" , j -> row , j -> column);
           j -> D -> U = j -> U;
           j -> U -> D = j -> D;
           j -> C -> S--;
     }
     
     // printf("finished covering column %d\n" , c -> column);
}

void uncover(data *c)
{
     // printf("uncovering column %s\n" , c -> N);
     
     data *i;
     data *j;
     
     for(i = c -> U ; i != c ; i = i -> U) for(j = i -> L ; j != i ; j = j -> L)
     {
           // printf("putting back (%d , %d) vertically\n" , j -> row , j -> column);
           j -> C -> S++;
           j -> D -> U = j;
           j -> U -> D = j;
     }
     
     // printf("putting back (%d , %d) horizontally\n" , c -> row , c -> column);
     c -> R -> L = c;
     c -> L -> R = c;
     
     // printf("finished uncovering column %s\n" , c -> N);
}

void search(int k)
{
     // printf("entering search(%d)\n" , k);
     
     if(h -> R == h)
     {
          print(k);
          return;
     }
     
     data *r;
     data *j;
     data *c = choose();
     
     // printf("choosing column %s\n" , c -> N);
     
     cover(c);
     
     for(r = c -> D ; r != c ; r = r -> D)
     {
           O[k] = r; // çözüm adayı
           // printf("choosing row (%d , %d) for temporary solution\n" , r -> row , r -> column);
           for(j = r -> R ; j != r ; j = j -> R) cover(j -> C);
           search(k + 1);
           r = O[k];
           // printf("reattaining row (%d , %d)\n" , r -> row , r -> column);
           c = r -> C;
           for(j = r -> L ; j != r ; j = j -> L) uncover(j -> C);
     }
     uncover(c);
     
     // printf("exiting search(%d)\n" , k);
}

void coverrow(data *r) // çözüm satırını kapa
{
     data *j;
     
     // printf("covering row %d\n" , r -> row);
     
     cover(r -> C);
     for(j = r -> R ; j != r ; j = j -> R) cover(j -> C);
     
     // printf("finished covering row %d\n" , r -> row);
}

void coverrows(char *s) // matrix'in, sudokuda verilen rakamlarla oluşan çözüm satırlarını kapa
{
     int i = 0 , k , r , c;
     
     for(k = 0 ; i < 81 ; k += 9 , i++) if(s[i] != '0')
     {
           r = k + s[i] - '0'; // bu satır kapanacak
           for(c = 1 ; c <= COLUMN ; c++) if(matrix[r][c]) { coverrow(matrix[r][c]); break; }
     }
}

int main()
{
    int i , j , k , l , m , n , o;
// sudoku s dizisine girilecek
    char *s = "300200000000107000706030500070009080900020004010800050009040301000702000000008006";
    FILE *f = fopen("sudoku.txt" , "w");
    
    k = 0;
    for(i = 0 ; i < 9 ; i++) for(j = 0 ; j < 9 ; j++) sudoku[i][j] = s[k++] - '0';
// sudokuyu göster
    printf("Given Sudoku...\n");    
    for(i = 0 ; i < 9 ; i++) 
    {
          for(j = 0 ; j < 9 ; j++) printf("%d " , sudoku[i][j]);
          printf("\n");
    }
    printf("\n");
// start oluşturuluyor
    for(i = 0 ; i < ROW ; i++) for(j = 0 ; j < 81 ; j++) if(i / 9 == j) start[i][j] = 1;
    k = l = m = 0;
    for(i = 0 ; i < ROW ; i++) for(j = 81 ; j < 162 ; j++) if(i - j + 81 == k)
    {
          start[i][j] = 1;
          l++;
          if(!(l % 9))
          {
                 l = 0; 
                 m++; 
                 if(m % 9) k += 9; 
          }
    }
    for(i = 0 ; i < ROW ; i++) for(j = 162 ; j < 243 ; j++) if(!((i - j + 162) % 81)) 
     start[i][j] = 1;
    k = l = m = n = o = 0;
    for(i = 0 ; i < ROW ; i++) for(j = 243 ; j < COLUMN ; j++) if(i - j + 243 == k)
    {
          start[i][j] = 1;
          l++;
          if(!(l % 9))
          {
                 l = 0;
                 m++;
                 n++;
                 if(!(n % 9)) 
                 {
                        o++;
                        if(o % 3) k += 27;
                        m = 0;
                 }
                 if(m % 3) k += 9;    
          }
    }

    initialize();
    coverrows(s);
    // showlinks();
    search(0);
    
    printf("%d SOLUTION%s FOUND\n" , numberofsolutions , numberofsolutions > 1 ? "S" : "");
    
    getchar();
}
