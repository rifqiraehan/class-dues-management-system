#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

int total_uang = 0;

void finish_with_error(MYSQL * con) {
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
}

void enter_dua_kali() {
    int enterCount = 0;
    while (enterCount < 2) {
        char c = getchar();
        if (c == '\n') {
            enterCount++;
        } else {
            enterCount = 0;
        }
    }
}

MYSQL * connect_to_database() {
    MYSQL * con = mysql_init(NULL);

    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, "localhost", "root", "admin", "project_c", 0, NULL, 0) == NULL) {
        finish_with_error(con);
    }

    return con;
}

void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int tampilan_tabel() {
    MYSQL * con = connect_to_database();

    if (mysql_query(con, "SELECT * FROM kasKelas WHERE nrp BETWEEN 1 AND 10")) {
        finish_with_error(con);
    }

    MYSQL_RES * result = mysql_store_result(con);

    if (result == NULL) {
        finish_with_error(con);
    }

    MYSQL_ROW row;
    MYSQL_FIELD * fields = mysql_fetch_fields(result);
    int num_fields = mysql_num_fields(result);
    int i;

    printf("TABEL TRANSAKSI MINGGUAN\n");

    for (i = 0; i < num_fields; i++) {
        printf("+------------");
    }
    printf("+\n");

    for (i = 0; i < mysql_num_fields(result); i++) {
        printf("| %-10s ", fields[i].name);
    }
    printf("|\n");

    for (i = 0; i < num_fields; i++) {
        printf("+------------");
    }
    printf("+\n");

    while ((row = mysql_fetch_row(result))) {
        for (i = 0; i < num_fields; i++) {
            printf("| %-10s ", row[i] ? row[i] : "NULL");
        }
        printf("|\n");
    }

    for (i = 0; i < num_fields; i++) {
        printf("+------------");
    }
    printf("+\n");

    char query[200];
    snprintf(query, sizeof(query), "SELECT SUM(5000 * (%s", fields[2].name);

    for (i = 3; i < num_fields; i++) {
        snprintf(query + strlen(query), sizeof(query) - strlen(query), " + %s", fields[i].name);
    }

    snprintf(query + strlen(query), sizeof(query) - strlen(query), ")) AS total FROM kasKelas WHERE nrp BETWEEN 1 AND 10");

    if (mysql_query(con, query)) {
        finish_with_error(con);
    }

    MYSQL_RES *total_result = mysql_store_result(con);
    MYSQL_ROW total_row = mysql_fetch_row(total_result);

    total_uang = (total_row && total_row[0]) ? atoi(total_row[0]) : 0;

    char pengeluaran_query[100];
    snprintf(pengeluaran_query, sizeof(pengeluaran_query), "SELECT COALESCE(SUM(nominal), 0) FROM pengeluaran");

    char final_query[300];
    snprintf(final_query, sizeof(final_query), "SELECT (total - (%s)) AS final_total FROM (%s) AS uang, (%s) AS pengeluaran", pengeluaran_query, query, pengeluaran_query);

    if (mysql_query(con, final_query)) {
        finish_with_error(con);
    }

    MYSQL_RES *final_result = mysql_store_result(con);
    MYSQL_ROW final_row = mysql_fetch_row(final_result);

    int final_total = (final_row && final_row[0]) ? atoi(final_row[0]) : 0;

    char update_query[100];
    snprintf(update_query, sizeof(update_query), "UPDATE transaksi SET total_uang = %d WHERE id = 1", final_total);

    if (mysql_query(con, update_query)) {
        finish_with_error(con);
    }

    printf("\nTotal: Rp %d\n", final_total);
    printf("\nTekan enter 2 kali untuk kembali...\n");

    clearBuffer();
    enter_dua_kali();

    return menu_utama();

    mysql_free_result(total_result);
    mysql_free_result(result);
    mysql_close(con);

    exit(0);
}

int menu_utama() {
    system("cls");

    int confirm;

    printf("SELAMAT DATANG DI SISTEM MANAJEMEN KAS KELAS\n\nPilih Menu:\n1. Lihat Tabel Transaksi Mingguan\n2. Masukkan Uang\n3. Keluarkan Uang\n4. Tambah Minggu\n5. Pengeluaran\n\nSilakan pilih nomor menu yang diinginkan: ");
    scanf("%d", &confirm);

    return confirm;
}

int penambahan_kolom() {
    MYSQL *con = connect_to_database();

    char nama_kolom[50];
    printf("Masukkan nama kolom yang ingin ditambahkan: ");
    scanf("%s", &nama_kolom);

    char query[100];
    snprintf(query, sizeof(query), "ALTER TABLE kasKelas ADD COLUMN %s tinyint(1) NOT NULL DEFAULT 0", nama_kolom);

    if (mysql_query(con, query)) {
        finish_with_error(con);
    }

    printf("Kolom %s berhasil ditambahkan!\n", nama_kolom);

    printf("\nTekan enter 2 kali untuk kembali...\n");

    clearBuffer();
    enter_dua_kali();
    mysql_close(con);

    return menu_utama();
}

int uang_masuk() {
    MYSQL *con = connect_to_database();

    char mingguInput[10];
    int mingguExists;

    do {
        printf("Minggu ke berapa? (e.g., minggu4): ");
        scanf("%s", &mingguInput);

        char checkMingguQuery[200];
        snprintf(checkMingguQuery, sizeof(checkMingguQuery), "SHOW COLUMNS FROM kasKelas LIKE '%s'", mingguInput);

        if (mysql_query(con, checkMingguQuery)) {
            finish_with_error(con);
        }

        MYSQL_RES *checkMingguResult = mysql_store_result(con);
        mingguExists = mysql_num_rows(checkMingguResult);

        if (mingguExists == 0) {
            printf("Nama minggu tidak ditemukan di tabel transaksi. Silakan coba lagi.\n\n");
        }

        mysql_free_result(checkMingguResult);

    } while(mingguExists == 0);

    printf("Ketik 'n' untuk kembali ke menu utama\n");

    int nrpExists;
    int nrp;

    do{
        do {
            printf("Masukkan NRP: ");

            if (scanf("%d", &nrp) != 1) {
                char response[2];
                scanf("%1s", &response);

                if (response[0] == 'n' || response[0] == 'N') {
                    return menu_utama();
                } else {
                    printf("NRP tidak valid. Silakan coba lagi.\n");
                    continue;
                }
            }

            char checkNRPQuery[200];
            snprintf(checkNRPQuery, sizeof(checkNRPQuery), "SELECT COUNT(*) FROM kasKelas WHERE nrp = %d", nrp);

            if (mysql_query(con, checkNRPQuery)) {
                finish_with_error(con);
            }

            MYSQL_RES *checkNRPResult = mysql_store_result(con);
            MYSQL_ROW checkNRPRow = mysql_fetch_row(checkNRPResult);
            nrpExists = atoi(checkNRPRow[0]);

            if (nrpExists == 0) {
                printf("NRP tidak ditemukan di tabel transaksi. Silakan coba lagi.\n\n");
            }

            mysql_free_result(checkNRPResult);
        } while (nrpExists == 0);

        char updateQuery[200];
        snprintf(updateQuery, sizeof(updateQuery), "UPDATE kasKelas SET %s = 1 WHERE nrp = %d", mingguInput, nrp);

        if (mysql_query(con, updateQuery)) {
            finish_with_error(con);
        }

        printf("Data berhasil dimasukkan!\n\n");
    } while(1);

    mysql_close(con);
}

int uang_keluar() {
    MYSQL *con = connect_to_database();

    char get_total_query[100];
    snprintf(get_total_query, sizeof(get_total_query), "SELECT total_uang FROM transaksi WHERE id = 1");

    if (mysql_query(con, get_total_query)) {
        finish_with_error(con);
    }

    MYSQL_RES *total_result = mysql_store_result(con);
    MYSQL_ROW total_row = mysql_fetch_row(total_result);

    total_uang = (total_row && total_row[0]) ? atoi(total_row[0]) : 0;

    printf("Total uang saat ini: Rp %d\n", total_uang);

    int nominal;
    printf("Masukkan nominal uang yang ingin diambil: Rp ");
    scanf("%d", &nominal);

    if (nominal > total_uang) {
        printf("Maaf, uang yang diminta melebihi total uang yang tersedia.\n");
    } else {
        total_uang -= nominal;

        char update_query[100];
        snprintf(update_query, sizeof(update_query), "UPDATE transaksi SET total_uang = %d WHERE id = 1", total_uang);

        if (mysql_query(con, update_query)) {
            finish_with_error(con);
        }

        char insert_query[200];
        snprintf(insert_query, sizeof(insert_query), "INSERT INTO pengeluaran (tanggal, nominal) VALUES (NOW(), %d)", nominal);

        if (mysql_query(con, insert_query)) {
            finish_with_error(con);
        }

        printf("Total uang berkurang menjadi: Rp %d\n", total_uang);
    }

    printf("\nTekan enter 2 kali untuk kembali...\n");

    clearBuffer();
    enter_dua_kali();
    mysql_free_result(total_result);
    mysql_close(con);

    return menu_utama();
}

int pengeluaran() {
    MYSQL *con = connect_to_database();

    if (mysql_query(con, "SELECT * FROM pengeluaran")) {
        finish_with_error(con);
    }

    MYSQL_RES *result = mysql_store_result(con);

    if (result == NULL) {
        finish_with_error(con);
    }

    MYSQL_ROW row;
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    int num_fields = mysql_num_fields(result);
    int i;

    printf("TABEL PENGELUARAN\n");

    for (i = 0; i < num_fields; i++) {
        printf("+------------");
    }
    printf("+\n");

    for (i = 0; i < mysql_num_fields(result); i++) {
        printf("| %-10s ", fields[i].name);
    }
    printf("|\n");

    for (i = 0; i < num_fields; i++) {
        printf("+------------");
    }
    printf("+\n");

    while ((row = mysql_fetch_row(result))) {
        for (i = 0; i < num_fields; i++) {
            printf("| %-10s ", row[i] ? row[i] : "NULL");
        }
        printf("|\n");
    }

    for (i = 0; i < num_fields; i++) {
        printf("+------------");
    }
    printf("+\n");

    if (mysql_query(con, "SELECT COALESCE(SUM(nominal), 0) AS total_pengeluaran FROM pengeluaran")) {
        finish_with_error(con);
    }

    MYSQL_RES *total_result = mysql_store_result(con);
    MYSQL_ROW total_row = mysql_fetch_row(total_result);

    int total_pengeluaran = (total_row && total_row[0]) ? atoi(total_row[0]) : 0;

    printf("\nTotal Pengeluaran: Rp %d\n", total_pengeluaran);

    mysql_free_result(result);
    mysql_free_result(total_result);
    mysql_close(con);

    printf("\nTekan enter 2 kali untuk kembali...\n");

    clearBuffer();

    enter_dua_kali();

    return menu_utama();
}

int main() {
    int confirm;
    confirm = menu_utama();

    do {
        switch (confirm) {
        case 1:
            system("cls");
            confirm = tampilan_tabel();
            break;
        case 2:
            system("cls");
            confirm = uang_masuk();
            break;
        case 3:
            system("cls");
            confirm = uang_keluar();
            break;
        case 4:
            system("cls");
            confirm = penambahan_kolom();
            break;
        case 5:
            system("cls");
            confirm = pengeluaran();
            break;
        default:
            break;
        }
    } while (confirm == 1 || confirm == 2 || confirm == 3 || confirm == 4 || confirm == 5);

    return 0;
}
