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

void extrageStatisticiLegatura(const char *nume_legatura, int fout)
{
    struct stat legatura_stat, fisier_stat;

    // informații despre legătura simbolică
    if (lstat(nume_legatura, &legatura_stat) < 0)
    {
        perror("Eroare la citirea informatiilor despre legatura simbolica\n");
        return;
    }

    if (stat(nume_legatura, &fisier_stat) < 0)
    {
        perror("Eroare la citirea informatiilor despre fisierul target al legaturii simbolice\n");
        return;
    }

    // adaug inf in buffer_out
    char buffer_out[2048];
    sprintf(buffer_out, "nume legatura: %s\ndimensiune legatura: %ld\ndimensiune fisier target: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n",
            nume_legatura, legatura_stat.st_size, fisier_stat.st_size,
            (legatura_stat.st_mode & S_IRUSR) ? 'R' : '-', (legatura_stat.st_mode & S_IWUSR) ? 'W' : '-', (legatura_stat.st_mode & S_IXUSR) ? 'X' : '-',
            (legatura_stat.st_mode & S_IRGRP) ? 'R' : '-', (legatura_stat.st_mode & S_IWGRP) ? 'W' : '-', (legatura_stat.st_mode & S_IXGRP) ? 'X' : '-',
            (legatura_stat.st_mode & S_IROTH) ? 'R' : '-', (legatura_stat.st_mode & S_IWOTH) ? 'W' : '-', (legatura_stat.st_mode & S_IXOTH) ? 'X' : '-');

    // scriu in fisierul de iesire
    if (write(fout, &buffer_out, strlen(buffer_out)) < 0)
    {
        perror("Eroare la scrierea in fisier\n");
    }
}

void extrageStatisticiFisier(int input_file, int output_file, const char *file_name)
{
    header bmp_header;
    char buffer_out[MAX_BUFFER_SIZE];
    struct stat file_stat;
    int len = 0;

    if (lstat(file_name, &file_stat) < 0)
    {
        perror("Eroare la citirea informatiilor despre fisier\n");
        return;
    }

    if (S_ISLNK(file_stat.st_mode))
    {

        extrageStatisticiLegatura(file_name, output_file);
    }
    else if (S_ISREG(file_stat.st_mode))
    {
        // verific .bmp
        if (strstr(file_name, ".bmp") != NULL)
        {
            //  nume fișier
            strncpy(bmp_header.file_name, file_name, 100);

            // dimensiune fișier
            lseek(input_file, 2, SEEK_SET);             // Setarea offsetului corespunzător
            read(input_file, &bmp_header.file_size, 4); // Citirea dimensiunii fișierului

            // câmp lungime imagine
            lseek(input_file, WIDTH_OFFSET, SEEK_SET); // Setarea offsetului corespunzător
            read(input_file, &bmp_header.width, 4);    // Citirea câmpului de lungime

            // câmp înălțime imagine
            lseek(input_file, HEIGHT_OFFSET, SEEK_SET); // Setarea offsetului corespunzător
            read(input_file, &bmp_header.height, 4);    // Citirea câmpului de înălțime

            // adaug in buffer_out
            stat(file_name, &file_stat);
            len = sprintf(buffer_out, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %d\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n",
                          bmp_header.file_name, bmp_header.height, bmp_header.width, bmp_header.file_size, file_stat.st_uid, ctime(&file_stat.st_mtime), file_stat.st_nlink,
                          (file_stat.st_mode & S_IRUSR) ? 'R' : '-', (file_stat.st_mode & S_IWUSR) ? 'W' : '-', (file_stat.st_mode & S_IXUSR) ? 'X' : '-',
                          (file_stat.st_mode & S_IRGRP) ? 'R' : '-', (file_stat.st_mode & S_IWGRP) ? 'W' : '-', (file_stat.st_mode & S_IXGRP) ? 'X' : '-',
                          (file_stat.st_mode & S_IROTH) ? 'R' : '-', (file_stat.st_mode & S_IWOTH) ? 'W' : '-', (file_stat.st_mode & S_IXOTH) ? 'X' : '-');
        }
        else
        {
            // n-are .bmp
            stat(file_name, &file_stat);

            char base_name[100];
            strncpy(base_name, file_name, 100);
            char *file_part = basename(base_name);
            len = sprintf(buffer_out, "nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n",
                          file_part, file_stat.st_size, file_stat.st_uid, ctime(&file_stat.st_mtime), file_stat.st_nlink,
                          (file_stat.st_mode & S_IRUSR) ? 'R' : '-', (file_stat.st_mode & S_IWUSR) ? 'W' : '-', (file_stat.st_mode & S_IXUSR) ? 'X' : '-',
                          (file_stat.st_mode & S_IRGRP) ? 'R' : '-', (file_stat.st_mode & S_IWGRP) ? 'W' : '-', (file_stat.st_mode & S_IXGRP) ? 'X' : '-',
                          (file_stat.st_mode & S_IROTH) ? 'R' : '-', (file_stat.st_mode & S_IWOTH) ? 'W' : '-', (file_stat.st_mode & S_IXOTH) ? 'X' : '-');
        }
    }
    buffer_out[len] = '\0';

    if (write(output_file, &buffer_out, strlen(buffer_out)) < 0)
    {
        perror("Eroare la scrierea in fisier\n");
    }
}

void extrageStatisticiDirector(const char *dir_name, int output_file, const char *dir_base)
{
    DIR *dir;
    struct dirent *intrare;

    if ((dir = opendir(dir_name)) == NULL)
    {
        perror("Eroare la deschiderea directorului\n");
        exit(-1);
    }

    char buffer_out[MAX_BUFFER_SIZE];
    struct stat dir_stat;
    stat(dir_name, &dir_stat);
    int len = sprintf(buffer_out, "nume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c-\ndrepturi de acces altii: %c%c%c\n",
                      dir_name, dir_stat.st_uid,
                      (dir_stat.st_mode & S_IRUSR) ? 'R' : '-', (dir_stat.st_mode & S_IWUSR) ? 'W' : '-', (dir_stat.st_mode & S_IXUSR) ? 'X' : '-',
                      (dir_stat.st_mode & S_IRGRP) ? 'R' : '-', (dir_stat.st_mode & S_IWGRP) ? 'W' : '-', (dir_stat.st_mode & S_IXGRP) ? 'X' : '-',
                      (dir_stat.st_mode & S_IROTH) ? 'R' : '-', (dir_stat.st_mode & S_IWOTH) ? 'W' : '-', (dir_stat.st_mode & S_IXOTH) ? 'X' : '-');

    buffer_out[len] = '\0';

    if (strcmp(dir_name, dir_base) != 0)
    {
        // Afișează informațiile doar dacă directorul nu este directorul de bază
        if (write(output_file, &buffer_out, strlen(buffer_out)) < 0)
        {
            perror("Eroare la scrierea in fisier\n");
        }
    }

    while ((intrare = readdir(dir)) != NULL)
    {
        if (strcmp(intrare->d_name, ".") == 0 || strcmp(intrare->d_name, "..") == 0)
        {
            continue;
        }

        char file_path[512];
        sprintf(file_path, "%s/%s", dir_name, intrare->d_name);

        struct stat file_stat;
        stat(file_path, &file_stat);

        if (S_ISREG(file_stat.st_mode))
        {
            if (S_ISLNK(file_stat.st_mode))
            {
                extrageStatisticiLegatura(file_path, output_file);
            }
            else
            {
                extrageStatisticiFisier(open(file_path, O_RDONLY), output_file, intrare->d_name);
            }
        }
        else if (S_ISDIR(file_stat.st_mode))
        {
            extrageStatisticiDirector(file_path, output_file, dir_base);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Nr insuficient de argumente\n");
        exit(-1);
    }

    int fout;
    fout = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (fout < 0)
    {
        perror("Eroare fisier de scriere\n");
        exit(-1);
    }

    if (stat(argv[1], &info) < 0)
    {
        perror("Eroare la citirea informatiilor despre fisier\n");
        exit(-1);
    }

    if (S_ISDIR(info.st_mode))
    {

        extrageStatisticiDirector(argv[1], fout, argv[1]);
    }
    else
    {
        printf("Calea nu este un director\n");
    }

    close(fout);
    return 0;
}
