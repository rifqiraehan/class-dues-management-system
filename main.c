#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

// Menyimpan total uang dan pengeluaran sebagai variabel global
int total_uang = 0;
int pengeluaran = 0;

void finish_with_error(MYSQL * con) {
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
}

int showing_table() {
    MYSQL * con = mysql_init(NULL);

    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, "localhost", "root", "admin", "project_c", 0, NULL, 0) == NULL) {
        finish_with_error(con);
    }

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

    // Print column names
    for (i = 0; i < num_fields; i++) {
        printf("+------------");
    }
    printf("+\n");

    for (i = 0; i < mysql_num_fields(result); i++) {
        printf("| %-10s ", fields[i].name);
    }
    printf("|\n");

    // Print separator line
    for (i = 0; i < num_fields; i++) {
        printf("+------------");
    }
    printf("+\n");

    // Print rows
    while ((row = mysql_fetch_row(result))) {
        for (i = 0; i < num_fields; i++) {
            printf("| %-10s ", row[i] ? row[i] : "NULL");
        }
        printf("|\n");
    }

    // Print bottom line
    for (i = 0; i < num_fields; i++) {
        printf("+------------");
    }
    printf("+\n");

    // Query to dynamically calculate and print total
    char query[200];
    snprintf(query, sizeof(query), "SELECT SUM(5000 * (%s", fields[2].name);

    for (i = 3; i < num_fields; i++) {
        snprintf(query + strlen(query), sizeof(query) - strlen(query), " + %s", fields[i].name);
    }

    snprintf(query + strlen(query), sizeof(query) - strlen(query), ")) AS total FROM kasKelas WHERE nrp BETWEEN 1 AND 10");

    // Query to calculate and print total
    if (mysql_query(con, query)) {
        finish_with_error(con);
    }

    MYSQL_RES * total_result = mysql_store_result(con);
    MYSQL_ROW total_row = mysql_fetch_row(total_result);

    // Mengambil total uang dari hasil query
    total_uang = (total_row && total_row[0]) ? atoi(total_row[0]) : 0;

    // Update total uang pada tabel transaksi
    char update_query[250];
	snprintf(update_query, sizeof(update_query), "UPDATE transaksi SET total_uang = %d - IF((SELECT SUM(nominal) FROM pengeluaran WHERE id = (SELECT MAX(id) FROM transaksi)) IS NULL, 0, (SELECT SUM(nominal) FROM pengeluaran WHERE id = (SELECT MAX(id) FROM transaksi))) WHERE id = 1", total_uang);


    if (mysql_query(con, update_query)) {
        finish_with_error(con);
    }

	// Query untuk mengambil nilai total_uang yang sudah diperbarui
	char select_query[100];
	snprintf(select_query, sizeof(select_query), "SELECT total_uang FROM transaksi WHERE id = 1");

	if (mysql_query(con, select_query)) {
	    finish_with_error(con);
	}

	MYSQL_RES *select_result = mysql_store_result(con);
	MYSQL_ROW select_row = mysql_fetch_row(select_result);

	// Menampilkan nilai total_uang yang sudah diperbarui
	printf("\nTotal: Rp %s\n", select_row[0] ? select_row[0] : "NULL");
    printf("\nTekan enter 2 kali untuk kembali...\n");

    // Membersihkan buffer stdin sebelum membaca input lagi
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    // Menunggu dua Enter untuk kembali ke menu utama
    int enterCount = 0;
    while (enterCount < 2) {
        char c = getchar();
        if (c == '\n') {
            enterCount++;
        } else {
            enterCount = 0; // Reset jika karakter selain Enter ditemukan
        }
    }

    // Kembali ke menu utama
    return hello();

	mysql_free_result(select_result);
    mysql_free_result(total_result);
    mysql_free_result(result);
    mysql_close(con);

    exit(0);
}

int hello() {
    system("cls");

    int confirm;

    printf("SELAMAT DATANG DI SISTEM MANAJEMEN KAS KELAS\n\nPilih Menu:\n1. Lihat Tabel Transaksi Mingguan\n2. Masukkan Uang\n3. Keluarkan Uang\n4. Tambah Minggu\n\nSilakan pilih nomor menu yang diinginkan: ");
    scanf("%d", & confirm);

    return confirm;
}

int tambah_kolom() {
    MYSQL *con = mysql_init(NULL);

    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, "localhost", "root", "admin", "project_c", 0, NULL, 0) == NULL) {
        finish_with_error(con);
    }

    // Gunakan input pengguna untuk menentukan nama kolom yang akan ditambahkan
    char nama_kolom[50];
    printf("Masukkan nama kolom yang ingin ditambahkan: ");
    scanf("%s", nama_kolom);

    // Query untuk menambahkan kolom baru ke tabel
    char query[100];
    snprintf(query, sizeof(query), "ALTER TABLE kasKelas ADD COLUMN %s tinyint(1) NOT NULL DEFAULT 0", nama_kolom);

    if (mysql_query(con, query)) {
        finish_with_error(con);
    }

    printf("Kolom %s berhasil ditambahkan!\n", nama_kolom);

    printf("\nTekan enter 2 kali untuk kembali...\n");

    // Membersihkan buffer stdin sebelum membaca input lagi
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    // Menunggu dua Enter untuk kembali ke menu utama
    int enterCount = 0;
    while (enterCount < 2) {
        char c = getchar();
        if (c == '\n') {
            enterCount++;
        } else {
            enterCount = 0; // Reset jika karakter selain Enter ditemukan
        }
    }

    // Kembali ke menu utama
    return hello();

    mysql_close(con);
}

// ubah fungsi ini
int keluarkan_uang() {
	MYSQL * con = mysql_init(NULL);

    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }

    if (mysql_real_connect(con, "localhost", "root", "admin", "project_c", 0, NULL, 0) == NULL) {
        finish_with_error(con);
    }

    printf("Total uang saat ini: Rp %d\n", total_uang);

    int nominal;
    printf("Masukkan nominal uang yang ingin diambil: Rp ");
    scanf("%d", &nominal);

    if (nominal > total_uang) {
        printf("Maaf, uang yang diminta melebihi total uang yang tersedia.\n");
    } else {
        total_uang -= nominal;
        // Update total uang pada tabel transaksi
        char update_query[100];
        snprintf(update_query, sizeof(update_query), "UPDATE transaksi SET total_uang = %d WHERE id = 1", total_uang);

        if (mysql_query(con, update_query)) {
            finish_with_error(con);
        }

        // Menyimpan detail transaksi ke dalam tabel riwayat_transaksi
        char insert_query[200];
        snprintf(insert_query, sizeof(insert_query), "INSERT INTO pengeluaran (tanggal, nominal) VALUES (NOW(), %d)", nominal);

        if (mysql_query(con, insert_query)) {
            finish_with_error(con);
        }

        pengeluaran += nominal;
        printf("Total uang berkurang menjadi: Rp %d\n", total_uang);
    }

    printf("\nTekan enter 2 kali untuk kembali...\n");

    // Membersihkan buffer stdin sebelum membaca input lagi
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    // Menunggu dua Enter untuk kembali ke menu utama
    int enterCount = 0;
    while (enterCount < 2) {
        char c = getchar();
        if (c == '\n') {
            enterCount++;
        } else {
            enterCount = 0; // Reset jika karakter selain Enter ditemukan
        }
    }

    // Kembali ke menu utama
    return hello();

    mysql_close(con);
}

int main() {
    int confirm;
    confirm = hello();

    do {
        switch (confirm) {
        case 1:
            system("cls");
            // Memanggil showing_table() dan menyimpan nilai yang dikembalikan
            confirm = showing_table();

            break;
        case 3:
            system("cls");
            // Memanggil keluarkan_uang() dan menyimpan nilai yang dikembalikan
            confirm = keluarkan_uang();

            break;
        case 4:
            system("cls");
            // Memanggil tambah_kolom() dan menyimpan nilai yang dikembalikan
            confirm = tambah_kolom();

            break;
        default:
            break;
        }
    } while (confirm == 1 || confirm == 2 || confirm == 3 || confirm == 4);

    return 0;
}
