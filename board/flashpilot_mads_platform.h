#pragma once

// Real platform faults remain vetoes. Diagnostic counter increments and an
// ordinary harness ADC critical section are not independently steering faults.
static bool flashpilot_mads_platform_ready(void) {
  static uint32_t can_resets[PANDA_CAN_CNT];
  static bool ignition_line_valid = false;
  ENTER_CRITICAL();
  bool valid = (faults == 0U) && !power_save_enabled && !heartbeat_lost && !heartbeat_disabled && heartbeat_engaged_mads;
  valid = valid && (harness.status != HARNESS_STATUS_NC);
  // Retain the last completed GPIO observation only during the ADC critical
  // section; never wait for a lock from safety RX/TX. Native ignition/host
  // lifecycle and harness disconnect checks remain in effect.
  if (!harness.sbu_adc_lock) {
    ignition_line_valid = harness_check_ignition();
  }
  valid = valid && (ignition_line_valid || ignition_can);
  for (uint32_t i = 0U; i < PANDA_CAN_CNT; i++) {
    valid = valid && (can_health[i].bus_off == 0U) && (can_health[i].error_passive == 0U);
    valid = valid && (can_resets[i] == can_health[i].can_core_reset_cnt);
    can_resets[i] = can_health[i].can_core_reset_cnt;
  }
  EXIT_CRITICAL();
  return valid;
}
