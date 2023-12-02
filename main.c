#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

int showing_table()
{
  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "mysql_init() failed\n");
    exit(1);
  }

  if (mysql_real_connect(con, "localhost", "root", "admin", "project_c", 0, NULL, 0) == NULL)
  {
    finish_with_error(con);
  }

  if (mysql_query(con, "SELECT * FROM kasKelas WHERE nrp BETWEEN 1 AND 10"))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  if (result == NULL)
  {
    finish_with_error(con);
  }

  MYSQL_ROW row;
  MYSQL_FIELD *fields = mysql_fetch_fields(result);
  int num_fields = mysql_num_fields(result);
  int i;

  printf("TABEL TRANSAKSI MINGGUAN\n");

  // Print column names
  for (i = 0; i < num_fields; i++)
  {
    printf("+------------");
  }
  printf("+\n");

  for (i = 0; i < mysql_num_fields(result); i++)
  {
    printf("| %-10s ", fields[i].name);
  }
  printf("|\n");

  // Print separator line
  for (i = 0; i < num_fields; i++)
  {
    printf("+------------");
  }
  printf("+\n");

  // Print rows
  while ((row = mysql_fetch_row(result)))
  {
    for (i = 0; i < num_fields; i++)
    {
      printf("| %-10s ", row[i] ? row[i] : "NULL");
    }
    printf("|\n");
  }

  // Print bottom line
  for (i = 0; i < num_fields; i++)
  {
    printf("+------------");
  }
  printf("+\n");

  // Query to dynamically calculate and print total
  char query[200];
  snprintf(query, sizeof(query), "SELECT SUM(5000 * (%s", fields[2].name);

  for (i = 3; i < num_fields; i++)
  {
    snprintf(query + strlen(query), sizeof(query) - strlen(query), " + %s", fields[i].name);
  }

  snprintf(query + strlen(query), sizeof(query) - strlen(query), ")) AS total FROM kasKelas WHERE nrp BETWEEN 1 AND 10");

  // Query to calculate and print total
  if (mysql_query(con, query))
  {
    finish_with_error(con);
  }

  MYSQL_RES *total_result = mysql_store_result(con);
  MYSQL_ROW total_row = mysql_fetch_row(total_result);

  printf("\nTotal: Rp %s\n", total_row[0] ? total_row[0] : "NULL");

  mysql_free_result(total_result);
  mysql_free_result(result);
  mysql_close(con);

  exit(0);
}

int main() {
  int confirm;
  printf("SELAMAT DATANG DI SISTEM MANAJEMEN KAS KELAS\n\nPilih Menu:\n1. Lihat Tabel Transaksi Mingguan\n2. Masukkan Uang\n3. Keluarkan Uang\n4. Tambah Minggu\n\nSilakan pilih nomor menu yang diinginkan: ");
  scanf("%d", &confirm);
  
  if (confirm == 1) {
  	system("cls");
  	showing_table();
  }
  return 0;
}

