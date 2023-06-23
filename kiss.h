#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef KISS_H
#define KISS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  E_OK = 0,  /* No Error*/
  E_FAIL,    /* Generic Failure */
  E_OVERFLOW /* rx_buffer is full*/
} kiss_error_t;

typedef enum {
  CMD_DATA = 0x00,   /* Frame contains Data */
  CMD_CONF = 0x06,   /* Frame is Configuration */
  CMD_INVALID = 0xFF /* Frame is invalid */
} kiss_cmd_t;

typedef enum {
  FESC = 0xDB,  /* Escape Frame */
  TFESC = 0xDD, /* Transpose Escape Frame */
  FEND = 0xC0,  /* End Frame */
  TFEND = 0xDC  /* Transpose End Frame */
} kiss_flag_t;

typedef enum {
  S_WAIT,    /* Waiting for start */
  S_COMMAND, /* Reading Command */
  S_BODY,    /* Receiving Message Body */
  S_ESCAPE,  /* Escaping Byte */
} kiss_state_t;

typedef struct {
  /* Current position in buffer */
  size_t rx_index;
  /* Current state in receiver FSM */
  kiss_state_t state;
  /* Current frame type */
  kiss_cmd_t cmd;
} kiss_internal_t;

typedef void (*kiss_recv_callback_t)(kiss_cmd_t type, uint8_t *buffer,
                                     size_t len, bool overflow);
typedef kiss_error_t (*kiss_sender_t)(uint8_t byte);

typedef struct {
  /**
   * @brief Takes KISS output (byte-by-byte)
   */
  kiss_sender_t sender;
  /**
   * @brief Callback in event of successful message reception
   * Note: Called from `kiss_ingest_byte`
   */
  kiss_recv_callback_t callback;
  /**
   * @brief Protocol internal state
   */
  kiss_internal_t internal;
  size_t rx_buffer_len;
  uint8_t *rx_buffer;
} kiss_t;

/**
 * @brief
 *
 * @param kiss Pointer to KISS instance
 * @return kiss_error_t
 */
kiss_error_t kiss_init(kiss_t *const kiss);
/**
 * @brief
 *
 * @param kiss Pointer to KISS instance
 * @param byte incoming byte
 * @return kiss_error_t
 */
kiss_error_t kiss_ingest_byte(kiss_t *const kiss, uint8_t const byte);
/**
 * @brief
 *
 * @param kiss Pointer to KISS instance
 * @param buffer Byte buffer containing message body
 * @param len buffer size
 * @return kiss_error_t
 */
kiss_error_t kiss_send(kiss_t const * const kiss, kiss_cmd_t const cmd,
                       uint8_t const * const buffer, size_t const len);



#ifdef __cplusplus
}
#endif
#endif // KISS_H
