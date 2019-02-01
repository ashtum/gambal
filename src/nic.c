#include "nic.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static long parse_str_file_to_long_int(char *file_address)
{
    FILE *file = fopen(file_address, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long length;
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        char tmp_string[length];
        fread(tmp_string, 1, length, file);
        fclose(file);
        return atol(tmp_string);
    }
    return 0;
}

void nic_init(struct nic_statistics *nic_s)
{
    int i;
    sprintf(nic_s->rx_path, "%s%s%s", "/sys/class/net/", nic_s->dev_name, "/statistics/rx_bytes");
    sprintf(nic_s->tx_path, "%s%s%s", "/sys/class/net/", nic_s->dev_name, "/statistics/tx_bytes");
    for (i = 0; i < 100; i++)
    {
        nic_s->rx_rates[i] = 0;
        nic_s->tx_rates[i] = 0;
    }
    nic_s->last_rx_bytes = LONG_MAX;
    nic_s->last_tx_bytes = LONG_MAX;
    nic_s->total_tx_bytes = 0;
    nic_s->total_rx_bytes = 0;
    nic_s->bigest_rate = 0;
}

void nic_update_statistics(struct nic_statistics *nic_s)
{
    int i;
    for (i = 99; i != 0; i--)
    {
        nic_s->rx_rates[i] = nic_s->rx_rates[i - 1];
        nic_s->tx_rates[i] = nic_s->tx_rates[i - 1];
    }

    long current_rx_bytes;
    current_rx_bytes = parse_str_file_to_long_int(nic_s->rx_path);
    if (nic_s->last_rx_bytes <= current_rx_bytes)
    {
        nic_s->rx_rates[0] = current_rx_bytes - nic_s->last_rx_bytes;
        nic_s->total_rx_bytes += nic_s->rx_rates[0];
    }
    nic_s->last_rx_bytes = current_rx_bytes;

    long current_tx_bytes;
    current_tx_bytes = parse_str_file_to_long_int(nic_s->tx_path);
    if (nic_s->last_tx_bytes <= current_tx_bytes)
    {
        nic_s->tx_rates[0] = current_tx_bytes - nic_s->last_tx_bytes;
        nic_s->total_tx_bytes += nic_s->tx_rates[0];
    }

    nic_s->last_tx_bytes = current_tx_bytes;

    nic_s->bigest_rate = 0;
    for (i = 0; i < 100; i++)
    {
        if (nic_s->rx_rates[i] > nic_s->bigest_rate)
            nic_s->bigest_rate = nic_s->rx_rates[i];

        if (nic_s->tx_rates[i] > nic_s->bigest_rate)
            nic_s->bigest_rate = nic_s->tx_rates[i];
    }
}
