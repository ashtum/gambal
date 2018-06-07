#ifndef ASHMON_H
#define ASHMON_H

#include "nic.h"
#include "window.h"

void *refrasher(void *p);
void save_configs_in_file(struct window_display_struct *win_prop, struct nic_statistics *nic_s, char *cfg_path);
char init_configs(struct window_display_struct *win_prop, struct nic_statistics *nic_s, char *cfg_path);

#endif