/**
 * ashmon is a free and open source tool for bandwidth monitoring in linux with a light gui .
 * (C) 2015 by Mohammad Nejati www.github.com/ashtum
 * Released under the GPL v2.0
 *
 *  compile command : gcc ashmon.c window.c nic.c -o ashmon -lX11 -pthread
 *  	You would need to install libx11-dev . You can install it with :
 *  		sudo apt-get install libx11-dev
 * 		    Or
 *  		sudo yum install libX11-devel
 */

#include "ashmon.h"
#include "nic.h"
#include "window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

struct window_display_struct window_properties;
struct nic_statistics main_nic;

static char init_configs(struct window_display_struct *win_prop, struct nic_statistics *nic_s, char *cfg_path);
static void *refresher(void *p);

int main(int argc, char *argv[])
{

    char config_patch[1024];
    strcpy(config_patch, getenv("HOME"));
    strcat(config_patch, "/.ashmon_config");

    if (!init_configs(&window_properties, &main_nic, config_patch))
    {
        if (!argv[1])
        {
            printf("Usage: ashmon [NIC]\n"
                   "replace [NIC] with your network interface card name.\n"
                   "Like : ashmon eth0 , ashmon wlan0 , ashmon ppp0 , etc \n");
            exit(0);
        }
    }

    if (argv[1])
        strcpy(main_nic.dev_name, argv[1]);

    save_configs_in_file(&window_properties, &main_nic, config_patch);

    window_init(&window_properties);
    nic_init(&main_nic);

    pthread_t link_updater;
    pthread_create(&link_updater, NULL, refresher, NULL);

    window_blocking_event_handler(&window_properties, &main_nic, config_patch);
    return 0;
}

static void *refresher(void *p)
{
    while (1)
    {
        struct timespec one_secound;
        clock_gettime(CLOCK_MONOTONIC, &one_secound);
        one_secound.tv_sec += 1;

        nic_update_statistics(&main_nic);
        window_redraw(&window_properties, &main_nic);

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &one_secound, NULL);
    }
    return NULL;
}

void save_configs_in_file(struct window_display_struct *win_prop, struct nic_statistics *nic_s, char *cfg_path)
{
    FILE *fptr = fopen(cfg_path, "wb+");
    if (fptr != NULL)
    {
        chmod(cfg_path, 0666);
        fprintf(fptr, "%s\n", nic_s->dev_name);
        fprintf(fptr, "%i\n", win_prop->opacity);
        fprintf(fptr, "%i\n", win_prop->last_window_x);
        fprintf(fptr, "%i\n", win_prop->last_window_y);
        fclose(fptr);
    }
}

static char init_configs(struct window_display_struct *win_prop, struct nic_statistics *nic_s, char *cfg_path)
{
    FILE *fptr = fopen(cfg_path, "r");
    if (fptr)
    {
        char line[512];
        int i = 0;
        while (fgets(line, 512, fptr) != NULL)
        {
            strtok(line, "\n");
            switch (i)
            {
            case 0:
                strcpy(nic_s->dev_name, line);
                break;
            case 1:
                win_prop->opacity = atoi(line);
                break;
            case 2:
                win_prop->last_window_x = atoi(line);
                break;
            case 3:
                win_prop->last_window_y = atoi(line);
                break;
            }
            i++;
        }
        fclose(fptr);
        return 1;
    }
    else
    {
        win_prop->last_window_x = 512;
        win_prop->last_window_y = 512;
        win_prop->opacity = 90;
        return 0;
    }
}
