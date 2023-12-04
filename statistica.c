#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>

// #pragma pack(1) este folosit pentru alinierea corespunzatoare a byte-tilor pentru a putea fi extrasi in structurile declarate mai jos
#pragma pack(1)
typedef struct
{
    char signature[2];
    int size;
    int reserved;
    int offset;
} BMPHeader;
#pragma pack()
typedef struct
{
    int headerSize;
    int width;
    int height;
    short planes;
    short bitsPerPixel;
    int compression;
    int imageSize;
    int xPixelsPerM;
    int yPixelsPerM;
    int colorsUsed;
    int colorsImportant;
} BMPInfoHeader;

#define WIDTH_OFFSET 18
#define HEIGHT_OFFSET 22
#define MAX_BUFFER_SIZE 2048

struct stat info;

typedef struct header
{
    char file_name[100];
    int file_size;
    int width;
    int height;
    int image_size;
} header;

void extrageStatisticiLegatura(char *nume_legatura, int fout, int *linii)
{
    struct stat legatura_stat, fisier_stat;
    int fin = open(nume_legatura, O_RDONLY);
    if (fin < 0)
    {
        perror("Eroare deschidere fisier intrare!");
        exit(-1);
    }

    // informații despre legătura simbolică
    if (lstat(nume_legatura, &legatura_stat) < 0)
    {
        perror("Eroare la citirea informatiilor despre legatura simbolica\n");
        exit(-1);
    }

    if (fstat(fin, &fisier_stat) < 0)
    {
        perror("Eroare la citirea informatiilor despre fisierul target al legaturii simbolice\n");
        exit(-1);
    }
    char *file_name = basename(nume_legatura);
    // adaug inf in buffer_out
    char buffer_out[MAX_BUFFER_SIZE];
    snprintf(buffer_out, sizeof(buffer_out), "nume legatura: %s\ndimensiune legatura: %ld\ndimensiune fisier target: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n\n",
             file_name, legatura_stat.st_size, fisier_stat.st_size,
             (legatura_stat.st_mode & S_IRUSR) ? 'R' : '-', (legatura_stat.st_mode & S_IWUSR) ? 'W' : '-', (legatura_stat.st_mode & S_IXUSR) ? 'X' : '-',
             (legatura_stat.st_mode & S_IRGRP) ? 'R' : '-', (legatura_stat.st_mode & S_IWGRP) ? 'W' : '-', (legatura_stat.st_mode & S_IXGRP) ? 'X' : '-',
             (legatura_stat.st_mode & S_IROTH) ? 'R' : '-', (legatura_stat.st_mode & S_IWOTH) ? 'W' : '-', (legatura_stat.st_mode & S_IXOTH) ? 'X' : '-');

    *linii = 6;
    // scriu in fisierul de iesire
    if (write(fout, &buffer_out, strlen(buffer_out)) < 0)
    {
        perror("Eroare la scrierea in fisier\n");
    }

    close(fin);
}

void extrageStatisticiFisierBMP(char *nume_fisier, int fout, int *linii)
{
    header bmp_header;
    char buffer_out[MAX_BUFFER_SIZE];
    struct stat file_stat;

    int fin = open(nume_fisier, O_RDWR);
    if (fin < 0)
    {
        perror("Eroare deschidere fisier intrare!");
        exit(-1);
    }

    if (fstat(fin, &file_stat) < 0)
    {
        perror("Eroare la citirea informatiilor despre fisier\n");
        exit(-1);
    }

    BMPHeader bmpHeader;
    if (read(fin, &bmpHeader, sizeof(BMPHeader)) != sizeof(BMPHeader))
    {
        perror("Eroare la citirea header-ului BMP");
        close(fin);
        return;
    }

    BMPInfoHeader bmpInfoHeader;
    if (read(fin, &bmpInfoHeader, sizeof(BMPInfoHeader)) != sizeof(BMPInfoHeader))
    {
        perror("Eroare la citirea informatiilor din header-ul BMP");
        close(fin);
        return;
    }

    //  nume fișier
    strncpy(bmp_header.file_name, nume_fisier, 100);

    // dimensiune fișier
    lseek(fin, 2, SEEK_SET);             // Setarea offsetului corespunzător
    read(fin, &bmp_header.file_size, 4); // Citirea dimensiunii fișierului

    // câmp lungime imagine
    lseek(fin, WIDTH_OFFSET, SEEK_SET); // Setarea offsetului corespunzător
    read(fin, &bmp_header.width, 4);    // Citirea câmpului de lungime

    // câmp înălțime imagine
    lseek(fin, HEIGHT_OFFSET, SEEK_SET); // Setarea offsetului corespunzător
    read(fin, &bmp_header.height, 4);    // Citirea câmpului de înălțime

    // adaug in buffer_out
    stat(nume_fisier, &file_stat);
    char *file_name = basename(nume_fisier);
    snprintf(buffer_out, sizeof(buffer_out), "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %d\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n\n",
             file_name, bmp_header.height, bmp_header.width, bmp_header.file_size, file_stat.st_uid, ctime(&file_stat.st_mtime), file_stat.st_nlink,
             (file_stat.st_mode & S_IRUSR) ? 'R' : '-', (file_stat.st_mode & S_IWUSR) ? 'W' : '-', (file_stat.st_mode & S_IXUSR) ? 'X' : '-',
             (file_stat.st_mode & S_IRGRP) ? 'R' : '-', (file_stat.st_mode & S_IWGRP) ? 'W' : '-', (file_stat.st_mode & S_IXGRP) ? 'X' : '-',
             (file_stat.st_mode & S_IROTH) ? 'R' : '-', (file_stat.st_mode & S_IWOTH) ? 'W' : '-', (file_stat.st_mode & S_IXOTH) ? 'X' : '-');

    *linii = 10;

    if (write(fout, &buffer_out, strlen(buffer_out)) < 0)
    {
        perror("Eroare la scrierea in fisier\n");
        exit(-1);
    }

    pid_t childPid = fork();

    if (childPid == -1)
    {
        perror("Eroare la crearea procesului fiu");
        close(fin);
        return;
    }

    if (childPid == 0)
    {

        lseek(fin, bmpHeader.offset, SEEK_SET);

        char pixel[3];
        ssize_t bytesRead;

        while ((bytesRead = read(fin, pixel, sizeof(pixel))) > 0)
        {
            char grayscale = 0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2];
            memset(pixel, grayscale, sizeof(pixel));
            lseek(fin, -bytesRead, SEEK_CUR);
            write(fin, pixel, sizeof(pixel));
        }

        close(fin);
        exit(0);
    }
    else
    {
        int status;
        waitpid(childPid, &status, 0);

        if (WIFEXITED(status))
        {
            printf("S-a încheiat procesul cu pid-ul %d și codul %d\n", childPid, WEXITSTATUS(status));
        }
        else
        {
            printf("Procesul cu pid-ul %d nu s-a încheiat normal\n", childPid);
        }
    }
    close(fin);
}

void extrageStatisticiFisierObisnuit(char *nume_fisier, int fout, int *linii)
{
    struct stat file_stat;

    int fin = open(nume_fisier, O_RDONLY);
    if (fin < 0)
    {
        perror("Eroare deschidere fisier intrare!");
        exit(-1);
    }

    if (fstat(fin, &file_stat) < 0)
    {
        perror("Eroare la citirea informatiilor despre fisier\n");
        exit(-1);
    }
    char buffer_out[MAX_BUFFER_SIZE];
    // n-are .bmp
    stat(nume_fisier, &file_stat);
    char *file_name = basename(nume_fisier);
    snprintf(buffer_out, sizeof(buffer_out), "nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n\n",
             file_name, file_stat.st_size, file_stat.st_uid, ctime(&file_stat.st_mtime), file_stat.st_nlink,
             (file_stat.st_mode & S_IRUSR) ? 'R' : '-', (file_stat.st_mode & S_IWUSR) ? 'W' : '-', (file_stat.st_mode & S_IXUSR) ? 'X' : '-',
             (file_stat.st_mode & S_IRGRP) ? 'R' : '-', (file_stat.st_mode & S_IWGRP) ? 'W' : '-', (file_stat.st_mode & S_IXGRP) ? 'X' : '-',
             (file_stat.st_mode & S_IROTH) ? 'R' : '-', (file_stat.st_mode & S_IWOTH) ? 'W' : '-', (file_stat.st_mode & S_IXOTH) ? 'X' : '-');
    *linii = 8;

    if (write(fout, &buffer_out, strlen(buffer_out)) < 0)
    {
        perror("Eroare la scrierea in fisier\n");
        exit(-1);
    }

    close(fin);
}

void extrageStatisticiDirector(char *nume_dir, int fout, char *dir_base, int *linii)
{
    DIR *dir;

    if ((dir = opendir(nume_dir)) == NULL)
    {
        perror("Eroare la deschiderea directorului\n");
        exit(-1);
    }

    char buffer_out[MAX_BUFFER_SIZE];
    struct stat dir_stat;
    if (stat(nume_dir, &dir_stat) < 0)
    {
        perror("Eroare obtinere informatii director");
        exit(-1);
    }
    stat(nume_dir, &dir_stat);
    char *file_name = basename(nume_dir);
    if (strcmp(nume_dir, dir_base) != 0)
    {
        snprintf(buffer_out, sizeof(buffer_out), "nume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c-\ndrepturi de acces altii: %c%c%c\n\n",
                 file_name, dir_stat.st_uid,
                 (dir_stat.st_mode & S_IRUSR) ? 'R' : '-', (dir_stat.st_mode & S_IWUSR) ? 'W' : '-', (dir_stat.st_mode & S_IXUSR) ? 'X' : '-',
                 (dir_stat.st_mode & S_IRGRP) ? 'R' : '-', (dir_stat.st_mode & S_IWGRP) ? 'W' : '-', (dir_stat.st_mode & S_IXGRP) ? 'X' : '-',
                 (dir_stat.st_mode & S_IROTH) ? 'R' : '-', (dir_stat.st_mode & S_IWOTH) ? 'W' : '-', (dir_stat.st_mode & S_IXOTH) ? 'X' : '-');

        // Afișează informațiile doar dacă directorul nu este directorul de bază
        *linii = 5;
        if (write(fout, &buffer_out, strlen(buffer_out)) < 0)
        {
            perror("Eroare la scrierea in fisier\n");
            exit(-1);
        }
    }

    closedir(dir);
}

void scrieStatistica(int statistica, int pidCopil, int linii_scrise)
{

    char buffer[MAX_BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "Pentru procesul fiu %d numarul de linii este %d.\n", pidCopil, linii_scrise);

    if (write(statistica, buffer, strlen(buffer)) < 0)
    {
        perror("Eroare la scrierea in fisierul de statistica");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Nr insuficient de argumente\n");
        exit(-1);
    }

    DIR *dir = opendir(argv[1]);
    if (!dir)
    {
        perror("Eroare deschidere director");
        exit(-1);
    }

    if (stat(argv[1], &info) < 0)
    {
        perror("Eroare la citirea informatiilor despre fisier\n");
        exit(-1);
    }

    char statisticFile[MAX_BUFFER_SIZE];
    snprintf(statisticFile, MAX_BUFFER_SIZE, "%s/statistica.txt", argv[1]);
    int statFd = open(statisticFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char full_path[MAX_BUFFER_SIZE];
        snprintf(full_path, sizeof(full_path), "%s/%s", argv[1], entry->d_name);

        pid_t pidCopil = fork();
        if (pidCopil < 0)
        {
            perror("Eroare la crearea procesului fiu\n");
            exit(-1);
        }
        if (pidCopil == 0)
        {
            int linii_scrise = 0;
            char output_file[MAX_BUFFER_SIZE];
            snprintf(output_file, MAX_BUFFER_SIZE, "%s/%s_statistica.txt", argv[2], entry->d_name);
            int out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

            if (out < 0)
            {
                perror("Eroare la deschiderea fisierului de iesire\n");
                exit(-1);
            }

            if (entry->d_type == DT_LNK)
            {
                extrageStatisticiLegatura(full_path, out, &linii_scrise);
            }
            else if (entry->d_type == DT_REG)
            {
                if (strstr(entry->d_name, ".bmp") != NULL)
                {
                    extrageStatisticiFisierBMP(full_path, out, &linii_scrise);
                }
                else
                {
                    extrageStatisticiFisierObisnuit(full_path, out, &linii_scrise);
                }
            }

            else if (entry->d_type == DT_DIR)
            {
                extrageStatisticiDirector(full_path, out, entry->d_name, &linii_scrise);
            }
            exit(linii_scrise);
        }
        else
        {
            int status = 0;
            waitpid(pidCopil, &status, 0);

            if (WIFEXITED(status))
            {
                printf("S-a încheiat procesul cu pid-ul %d și codul %d\n", pidCopil, WEXITSTATUS(status));
            }
            else
            {
                printf("Procesul cu pid-ul %d nu s-a încheiat normal\n", pidCopil);
            }
            scrieStatistica(statFd, pidCopil, WEXITSTATUS(status));
        }
    }
    close(statFd);
    closedir(dir);

    return 0;
}