#pragma once

// Read-only physical/platform veto, evaluated before Ford RX refresh, TX and
// permission publication. Changes in loss/error counters invalidate selection.
// Never sample the ignition GPIO while the harness ADC lock is held.
typedef struct {
  uint32_t rx_overflow;
  uint32_t tx_overflow;
  uint16_t spi_errors;
  uint32_t can_errors[PANDA_CAN_CNT];
  uint32_t tx_checksums[PANDA_CAN_CNT];
  uint32_t resets[PANDA_CAN_CNT];
  uint32_t rx_lost[PANDA_CAN_CNT];
  uint32_t tx_lost[PANDA_CAN_CNT];
} FlashPilotMadsFaultSnapshot;

static bool flashpilot_mads_platform_ready(void) {
  static FlashPilotMadsFaultSnapshot fp_mads_faults;
  ENTER_CRITICAL();
  bool valid = (faults == 0U) && !power_save_enabled && !heartbeat_lost && !heartbeat_disabled && heartbeat_engaged_mads;
  valid = valid && !harness.sbu_adc_lock && (harness.status != HARNESS_STATUS_NC);
  if (valid) {
    valid = harness_check_ignition() || ignition_can;
  }
  valid = valid && (fp_mads_faults.rx_overflow == rx_buffer_overflow) && (fp_mads_faults.tx_overflow == tx_buffer_overflow) &&
                   (fp_mads_faults.spi_errors == spi_error_count);
  fp_mads_faults.rx_overflow = rx_buffer_overflow;
  fp_mads_faults.tx_overflow = tx_buffer_overflow;
  fp_mads_faults.spi_errors = spi_error_count;
  for (uint32_t i = 0U; i < PANDA_CAN_CNT; i++) {
    valid = valid && (can_health[i].bus_off == 0U) && (can_health[i].error_passive == 0U);
    valid = valid && (fp_mads_faults.can_errors[i] == can_health[i].total_error_cnt) &&
                    (fp_mads_faults.tx_checksums[i] == can_health[i].total_tx_checksum_error_cnt) &&
                    (fp_mads_faults.resets[i] == can_health[i].can_core_reset_cnt) &&
                    (fp_mads_faults.rx_lost[i] == can_health[i].total_rx_lost_cnt) &&
                    (fp_mads_faults.tx_lost[i] == can_health[i].total_tx_lost_cnt);
    fp_mads_faults.can_errors[i] = can_health[i].total_error_cnt;
    fp_mads_faults.tx_checksums[i] = can_health[i].total_tx_checksum_error_cnt;
    fp_mads_faults.resets[i] = can_health[i].can_core_reset_cnt;
    fp_mads_faults.rx_lost[i] = can_health[i].total_rx_lost_cnt;
    fp_mads_faults.tx_lost[i] = can_health[i].total_tx_lost_cnt;
  }
  EXIT_CRITICAL();
  return valid;
}
