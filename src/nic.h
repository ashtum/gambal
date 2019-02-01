#ifndef NIC_H
#define NIC_H

struct nic_statistics
{
    char dev_name[512];
    char rx_path[1024];
    char tx_path[1024];
    long rx_rates[100];
    long tx_rates[100];
    long last_rx_bytes;
    long last_tx_bytes;
    long bigest_rate;
    long total_rx_bytes;
    long total_tx_bytes;
};

void nic_init(struct nic_statistics *nic_s);
void nic_update_statistics(struct nic_statistics *nic_s);

#endif