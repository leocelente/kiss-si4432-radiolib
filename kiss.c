#include "kiss.h"

kiss_error_t kiss_init(kiss_t *const kiss) {
  kiss->internal.rx_index = 0;
  kiss->internal.state = S_WAIT;
  kiss->internal.cmd = CMD_INVALID;

  return (kiss->callback != NULL && kiss->sender != NULL) ? E_OK : E_FAIL;
}

kiss_error_t kiss_ingest_byte(kiss_t *const kiss, uint8_t const input) {
  size_t *const index = &kiss->internal.rx_index;
  kiss_state_t *const state = &kiss->internal.state;
  kiss_cmd_t *const cmd_type = &kiss->internal.cmd;
  if (*index >= kiss->rx_buffer_len) {
    *state = S_WAIT;
    size_t i = *index;
    *index = 0;
    kiss->callback(*cmd_type, kiss->rx_buffer, i, true);
    return E_OVERFLOW;
  }

  if (*state == S_WAIT && input == FEND) { /* start of frame */
    *state = S_COMMAND;
    return E_OK;
  } else if (*state == S_COMMAND) { /* command type byte */
    *cmd_type = input;
    *state = S_BODY;
    return E_OK;
  } else if (*state == S_BODY && input == FEND) { /* end of frame */
    kiss->callback(*cmd_type, kiss->rx_buffer, *index, false);
    *index = 0;
    *state = S_WAIT;
    return E_OK;
  } else if (*state == S_BODY && input == FESC) { /* escape next value */
    *state = S_ESCAPE;
    return E_OK;
  } else if (*state == S_ESCAPE && input == TFESC) { 
    kiss->rx_buffer[(*index)++] = FESC;
    *state = S_BODY;
    return E_OK;
  } else if (*state == S_ESCAPE && input == TFEND) { 
    kiss->rx_buffer[(*index)++] = FEND;
    *state = S_BODY;
    return E_OK;
  } else if (*state == S_BODY) { /* push to rx buffer */
    kiss->rx_buffer[(*index)++] = input;
    return E_OK;
  } else {
    *state = S_WAIT;
    return E_FAIL;
  }
}

kiss_error_t kiss_send(kiss_t const *const kiss, kiss_cmd_t const cmd,
                       uint8_t const *const buffer, size_t const len) {
  kiss_error_t e = E_OK;
  e |= kiss->sender(FEND);
  e |= kiss->sender(cmd);
  for (int i = 0; i < len; ++i) {
    if (buffer[i] == FEND) {
      e |= kiss->sender(FESC);
      e |= kiss->sender(TFEND);
      continue;
    } else if (buffer[i] == FESC) {
      e |= kiss->sender(FESC);
      e |= kiss->sender(TFESC);
      continue;
    }
    e |= kiss->sender(buffer[i]);
  }
  e |= kiss->sender(FEND);
  return e;
}
