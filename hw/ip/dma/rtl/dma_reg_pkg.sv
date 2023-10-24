// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Register Package auto-generated by `reggen` containing data structure

package dma_reg_pkg;

  // Param list
  parameter int NumIntClearSources = 11;
  parameter int NumAlerts = 1;

  // Address widths within the block
  parameter int BlockAw = 9;

  ////////////////////////////
  // Typedefs for registers //
  ////////////////////////////

  typedef struct packed {
    struct packed {
      logic        q;
    } dma_memory_buffer_limit;
    struct packed {
      logic        q;
    } dma_error;
    struct packed {
      logic        q;
    } dma_done;
  } dma_reg2hw_intr_state_reg_t;

  typedef struct packed {
    struct packed {
      logic        q;
    } dma_memory_buffer_limit;
    struct packed {
      logic        q;
    } dma_error;
    struct packed {
      logic        q;
    } dma_done;
  } dma_reg2hw_intr_enable_reg_t;

  typedef struct packed {
    struct packed {
      logic        q;
      logic        qe;
    } dma_memory_buffer_limit;
    struct packed {
      logic        q;
      logic        qe;
    } dma_error;
    struct packed {
      logic        q;
      logic        qe;
    } dma_done;
  } dma_reg2hw_intr_test_reg_t;

  typedef struct packed {
    logic        q;
    logic        qe;
  } dma_reg2hw_alert_test_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_source_address_lo_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_source_address_hi_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_destination_address_lo_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_destination_address_hi_reg_t;

  typedef struct packed {
    struct packed {
      logic [3:0]  q;
    } destination_asid;
    struct packed {
      logic [3:0]  q;
    } source_asid;
  } dma_reg2hw_address_space_id_reg_t;

  typedef struct packed {
    logic [31:0] q;
    logic        qe;
  } dma_reg2hw_enabled_memory_range_base_reg_t;

  typedef struct packed {
    logic [31:0] q;
    logic        qe;
  } dma_reg2hw_enabled_memory_range_limit_reg_t;

  typedef struct packed {
    logic        q;
  } dma_reg2hw_range_valid_reg_t;

  typedef struct packed {
    logic [3:0]  q;
  } dma_reg2hw_range_regwen_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_total_data_size_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_chunk_data_size_reg_t;

  typedef struct packed {
    logic [1:0]  q;
  } dma_reg2hw_transfer_width_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_destination_address_limit_lo_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_destination_address_limit_hi_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_destination_address_almost_limit_lo_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_destination_address_almost_limit_hi_reg_t;

  typedef struct packed {
    struct packed {
      logic        q;
      logic        qe;
    } go;
    struct packed {
      logic        q;
    } abort;
    struct packed {
      logic        q;
    } initial_transfer;
    struct packed {
      logic        q;
    } data_direction;
    struct packed {
      logic        q;
    } fifo_auto_increment_enable;
    struct packed {
      logic        q;
    } memory_buffer_auto_increment_enable;
    struct packed {
      logic        q;
    } hardware_handshake_enable;
    struct packed {
      logic [3:0]  q;
    } opcode;
  } dma_reg2hw_control_reg_t;

  typedef struct packed {
    struct packed {
      logic        q;
      logic        qe;
    } sha2_digest_valid;
    struct packed {
      logic        q;
      logic        qe;
    } error;
    struct packed {
      logic        q;
    } done;
    struct packed {
      logic        q;
    } busy;
  } dma_reg2hw_status_reg_t;

  typedef struct packed {
    logic [10:0] q;
  } dma_reg2hw_handshake_interrupt_enable_reg_t;

  typedef struct packed {
    logic [10:0] q;
  } dma_reg2hw_clear_int_src_reg_t;

  typedef struct packed {
    logic [10:0] q;
  } dma_reg2hw_clear_int_bus_reg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_int_source_addr_mreg_t;

  typedef struct packed {
    logic [31:0] q;
  } dma_reg2hw_int_source_wr_val_mreg_t;

  typedef struct packed {
    struct packed {
      logic        d;
      logic        de;
    } dma_done;
    struct packed {
      logic        d;
      logic        de;
    } dma_error;
    struct packed {
      logic        d;
      logic        de;
    } dma_memory_buffer_limit;
  } dma_hw2reg_intr_state_reg_t;

  typedef struct packed {
    logic [31:0] d;
    logic        de;
  } dma_hw2reg_source_address_lo_reg_t;

  typedef struct packed {
    logic [31:0] d;
    logic        de;
  } dma_hw2reg_source_address_hi_reg_t;

  typedef struct packed {
    logic [31:0] d;
    logic        de;
  } dma_hw2reg_destination_address_lo_reg_t;

  typedef struct packed {
    logic [31:0] d;
    logic        de;
  } dma_hw2reg_destination_address_hi_reg_t;

  typedef struct packed {
    logic [3:0]  d;
    logic        de;
  } dma_hw2reg_cfg_regwen_reg_t;

  typedef struct packed {
    struct packed {
      logic        d;
      logic        de;
    } initial_transfer;
    struct packed {
      logic        d;
      logic        de;
    } abort;
    struct packed {
      logic        d;
      logic        de;
    } go;
  } dma_hw2reg_control_reg_t;

  typedef struct packed {
    struct packed {
      logic        d;
      logic        de;
    } busy;
    struct packed {
      logic        d;
      logic        de;
    } done;
    struct packed {
      logic        d;
      logic        de;
    } aborted;
    struct packed {
      logic        d;
      logic        de;
    } error;
    struct packed {
      logic        d;
      logic        de;
    } sha2_digest_valid;
  } dma_hw2reg_status_reg_t;

  typedef struct packed {
    struct packed {
      logic        d;
      logic        de;
    } src_address_error;
    struct packed {
      logic        d;
      logic        de;
    } dst_address_error;
    struct packed {
      logic        d;
      logic        de;
    } opcode_error;
    struct packed {
      logic        d;
      logic        de;
    } size_error;
    struct packed {
      logic        d;
      logic        de;
    } bus_error;
    struct packed {
      logic        d;
      logic        de;
    } base_limit_error;
    struct packed {
      logic        d;
      logic        de;
    } range_valid_error;
    struct packed {
      logic        d;
      logic        de;
    } asid_error;
  } dma_hw2reg_error_code_reg_t;

  typedef struct packed {
    logic [31:0] d;
    logic        de;
  } dma_hw2reg_sha2_digest_mreg_t;

  // Register -> HW type
  typedef struct packed {
    dma_reg2hw_intr_state_reg_t intr_state; // [1169:1167]
    dma_reg2hw_intr_enable_reg_t intr_enable; // [1166:1164]
    dma_reg2hw_intr_test_reg_t intr_test; // [1163:1158]
    dma_reg2hw_alert_test_reg_t alert_test; // [1157:1156]
    dma_reg2hw_source_address_lo_reg_t source_address_lo; // [1155:1124]
    dma_reg2hw_source_address_hi_reg_t source_address_hi; // [1123:1092]
    dma_reg2hw_destination_address_lo_reg_t destination_address_lo; // [1091:1060]
    dma_reg2hw_destination_address_hi_reg_t destination_address_hi; // [1059:1028]
    dma_reg2hw_address_space_id_reg_t address_space_id; // [1027:1020]
    dma_reg2hw_enabled_memory_range_base_reg_t enabled_memory_range_base; // [1019:987]
    dma_reg2hw_enabled_memory_range_limit_reg_t enabled_memory_range_limit; // [986:954]
    dma_reg2hw_range_valid_reg_t range_valid; // [953:953]
    dma_reg2hw_range_regwen_reg_t range_regwen; // [952:949]
    dma_reg2hw_total_data_size_reg_t total_data_size; // [948:917]
    dma_reg2hw_chunk_data_size_reg_t chunk_data_size; // [916:885]
    dma_reg2hw_transfer_width_reg_t transfer_width; // [884:883]
    dma_reg2hw_destination_address_limit_lo_reg_t destination_address_limit_lo; // [882:851]
    dma_reg2hw_destination_address_limit_hi_reg_t destination_address_limit_hi; // [850:819]
    dma_reg2hw_destination_address_almost_limit_lo_reg_t
        destination_address_almost_limit_lo; // [818:787]
    dma_reg2hw_destination_address_almost_limit_hi_reg_t
        destination_address_almost_limit_hi; // [786:755]
    dma_reg2hw_control_reg_t control; // [754:743]
    dma_reg2hw_status_reg_t status; // [742:737]
    dma_reg2hw_handshake_interrupt_enable_reg_t handshake_interrupt_enable; // [736:726]
    dma_reg2hw_clear_int_src_reg_t clear_int_src; // [725:715]
    dma_reg2hw_clear_int_bus_reg_t clear_int_bus; // [714:704]
    dma_reg2hw_int_source_addr_mreg_t [10:0] int_source_addr; // [703:352]
    dma_reg2hw_int_source_wr_val_mreg_t [10:0] int_source_wr_val; // [351:0]
  } dma_reg2hw_t;

  // HW -> register type
  typedef struct packed {
    dma_hw2reg_intr_state_reg_t intr_state; // [702:697]
    dma_hw2reg_source_address_lo_reg_t source_address_lo; // [696:664]
    dma_hw2reg_source_address_hi_reg_t source_address_hi; // [663:631]
    dma_hw2reg_destination_address_lo_reg_t destination_address_lo; // [630:598]
    dma_hw2reg_destination_address_hi_reg_t destination_address_hi; // [597:565]
    dma_hw2reg_cfg_regwen_reg_t cfg_regwen; // [564:560]
    dma_hw2reg_control_reg_t control; // [559:554]
    dma_hw2reg_status_reg_t status; // [553:544]
    dma_hw2reg_error_code_reg_t error_code; // [543:528]
    dma_hw2reg_sha2_digest_mreg_t [15:0] sha2_digest; // [527:0]
  } dma_hw2reg_t;

  // Register offsets
  parameter logic [BlockAw-1:0] DMA_INTR_STATE_OFFSET = 9'h 0;
  parameter logic [BlockAw-1:0] DMA_INTR_ENABLE_OFFSET = 9'h 4;
  parameter logic [BlockAw-1:0] DMA_INTR_TEST_OFFSET = 9'h 8;
  parameter logic [BlockAw-1:0] DMA_ALERT_TEST_OFFSET = 9'h c;
  parameter logic [BlockAw-1:0] DMA_SOURCE_ADDRESS_LO_OFFSET = 9'h 10;
  parameter logic [BlockAw-1:0] DMA_SOURCE_ADDRESS_HI_OFFSET = 9'h 14;
  parameter logic [BlockAw-1:0] DMA_DESTINATION_ADDRESS_LO_OFFSET = 9'h 18;
  parameter logic [BlockAw-1:0] DMA_DESTINATION_ADDRESS_HI_OFFSET = 9'h 1c;
  parameter logic [BlockAw-1:0] DMA_ADDRESS_SPACE_ID_OFFSET = 9'h 20;
  parameter logic [BlockAw-1:0] DMA_ENABLED_MEMORY_RANGE_BASE_OFFSET = 9'h 24;
  parameter logic [BlockAw-1:0] DMA_ENABLED_MEMORY_RANGE_LIMIT_OFFSET = 9'h 28;
  parameter logic [BlockAw-1:0] DMA_RANGE_VALID_OFFSET = 9'h 2c;
  parameter logic [BlockAw-1:0] DMA_RANGE_REGWEN_OFFSET = 9'h 30;
  parameter logic [BlockAw-1:0] DMA_CFG_REGWEN_OFFSET = 9'h 34;
  parameter logic [BlockAw-1:0] DMA_TOTAL_DATA_SIZE_OFFSET = 9'h 38;
  parameter logic [BlockAw-1:0] DMA_CHUNK_DATA_SIZE_OFFSET = 9'h 3c;
  parameter logic [BlockAw-1:0] DMA_TRANSFER_WIDTH_OFFSET = 9'h 40;
  parameter logic [BlockAw-1:0] DMA_DESTINATION_ADDRESS_LIMIT_LO_OFFSET = 9'h 44;
  parameter logic [BlockAw-1:0] DMA_DESTINATION_ADDRESS_LIMIT_HI_OFFSET = 9'h 48;
  parameter logic [BlockAw-1:0] DMA_DESTINATION_ADDRESS_ALMOST_LIMIT_LO_OFFSET = 9'h 4c;
  parameter logic [BlockAw-1:0] DMA_DESTINATION_ADDRESS_ALMOST_LIMIT_HI_OFFSET = 9'h 50;
  parameter logic [BlockAw-1:0] DMA_CONTROL_OFFSET = 9'h 54;
  parameter logic [BlockAw-1:0] DMA_STATUS_OFFSET = 9'h 58;
  parameter logic [BlockAw-1:0] DMA_ERROR_CODE_OFFSET = 9'h 5c;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_0_OFFSET = 9'h 60;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_1_OFFSET = 9'h 64;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_2_OFFSET = 9'h 68;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_3_OFFSET = 9'h 6c;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_4_OFFSET = 9'h 70;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_5_OFFSET = 9'h 74;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_6_OFFSET = 9'h 78;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_7_OFFSET = 9'h 7c;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_8_OFFSET = 9'h 80;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_9_OFFSET = 9'h 84;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_10_OFFSET = 9'h 88;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_11_OFFSET = 9'h 8c;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_12_OFFSET = 9'h 90;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_13_OFFSET = 9'h 94;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_14_OFFSET = 9'h 98;
  parameter logic [BlockAw-1:0] DMA_SHA2_DIGEST_15_OFFSET = 9'h 9c;
  parameter logic [BlockAw-1:0] DMA_HANDSHAKE_INTERRUPT_ENABLE_OFFSET = 9'h a0;
  parameter logic [BlockAw-1:0] DMA_CLEAR_INT_SRC_OFFSET = 9'h a4;
  parameter logic [BlockAw-1:0] DMA_CLEAR_INT_BUS_OFFSET = 9'h a8;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_0_OFFSET = 9'h ac;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_1_OFFSET = 9'h b0;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_2_OFFSET = 9'h b4;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_3_OFFSET = 9'h b8;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_4_OFFSET = 9'h bc;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_5_OFFSET = 9'h c0;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_6_OFFSET = 9'h c4;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_7_OFFSET = 9'h c8;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_8_OFFSET = 9'h cc;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_9_OFFSET = 9'h d0;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_ADDR_10_OFFSET = 9'h d4;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_0_OFFSET = 9'h 12c;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_1_OFFSET = 9'h 130;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_2_OFFSET = 9'h 134;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_3_OFFSET = 9'h 138;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_4_OFFSET = 9'h 13c;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_5_OFFSET = 9'h 140;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_6_OFFSET = 9'h 144;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_7_OFFSET = 9'h 148;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_8_OFFSET = 9'h 14c;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_9_OFFSET = 9'h 150;
  parameter logic [BlockAw-1:0] DMA_INT_SOURCE_WR_VAL_10_OFFSET = 9'h 154;

  // Reset values for hwext registers and their fields
  parameter logic [2:0] DMA_INTR_TEST_RESVAL = 3'h 0;
  parameter logic [0:0] DMA_INTR_TEST_DMA_DONE_RESVAL = 1'h 0;
  parameter logic [0:0] DMA_INTR_TEST_DMA_ERROR_RESVAL = 1'h 0;
  parameter logic [0:0] DMA_INTR_TEST_DMA_MEMORY_BUFFER_LIMIT_RESVAL = 1'h 0;
  parameter logic [0:0] DMA_ALERT_TEST_RESVAL = 1'h 0;
  parameter logic [0:0] DMA_ALERT_TEST_FATAL_FAULT_RESVAL = 1'h 0;

  // Register index
  typedef enum int {
    DMA_INTR_STATE,
    DMA_INTR_ENABLE,
    DMA_INTR_TEST,
    DMA_ALERT_TEST,
    DMA_SOURCE_ADDRESS_LO,
    DMA_SOURCE_ADDRESS_HI,
    DMA_DESTINATION_ADDRESS_LO,
    DMA_DESTINATION_ADDRESS_HI,
    DMA_ADDRESS_SPACE_ID,
    DMA_ENABLED_MEMORY_RANGE_BASE,
    DMA_ENABLED_MEMORY_RANGE_LIMIT,
    DMA_RANGE_VALID,
    DMA_RANGE_REGWEN,
    DMA_CFG_REGWEN,
    DMA_TOTAL_DATA_SIZE,
    DMA_CHUNK_DATA_SIZE,
    DMA_TRANSFER_WIDTH,
    DMA_DESTINATION_ADDRESS_LIMIT_LO,
    DMA_DESTINATION_ADDRESS_LIMIT_HI,
    DMA_DESTINATION_ADDRESS_ALMOST_LIMIT_LO,
    DMA_DESTINATION_ADDRESS_ALMOST_LIMIT_HI,
    DMA_CONTROL,
    DMA_STATUS,
    DMA_ERROR_CODE,
    DMA_SHA2_DIGEST_0,
    DMA_SHA2_DIGEST_1,
    DMA_SHA2_DIGEST_2,
    DMA_SHA2_DIGEST_3,
    DMA_SHA2_DIGEST_4,
    DMA_SHA2_DIGEST_5,
    DMA_SHA2_DIGEST_6,
    DMA_SHA2_DIGEST_7,
    DMA_SHA2_DIGEST_8,
    DMA_SHA2_DIGEST_9,
    DMA_SHA2_DIGEST_10,
    DMA_SHA2_DIGEST_11,
    DMA_SHA2_DIGEST_12,
    DMA_SHA2_DIGEST_13,
    DMA_SHA2_DIGEST_14,
    DMA_SHA2_DIGEST_15,
    DMA_HANDSHAKE_INTERRUPT_ENABLE,
    DMA_CLEAR_INT_SRC,
    DMA_CLEAR_INT_BUS,
    DMA_INT_SOURCE_ADDR_0,
    DMA_INT_SOURCE_ADDR_1,
    DMA_INT_SOURCE_ADDR_2,
    DMA_INT_SOURCE_ADDR_3,
    DMA_INT_SOURCE_ADDR_4,
    DMA_INT_SOURCE_ADDR_5,
    DMA_INT_SOURCE_ADDR_6,
    DMA_INT_SOURCE_ADDR_7,
    DMA_INT_SOURCE_ADDR_8,
    DMA_INT_SOURCE_ADDR_9,
    DMA_INT_SOURCE_ADDR_10,
    DMA_INT_SOURCE_WR_VAL_0,
    DMA_INT_SOURCE_WR_VAL_1,
    DMA_INT_SOURCE_WR_VAL_2,
    DMA_INT_SOURCE_WR_VAL_3,
    DMA_INT_SOURCE_WR_VAL_4,
    DMA_INT_SOURCE_WR_VAL_5,
    DMA_INT_SOURCE_WR_VAL_6,
    DMA_INT_SOURCE_WR_VAL_7,
    DMA_INT_SOURCE_WR_VAL_8,
    DMA_INT_SOURCE_WR_VAL_9,
    DMA_INT_SOURCE_WR_VAL_10
  } dma_id_e;

  // Register width information to check illegal writes
  parameter logic [3:0] DMA_PERMIT [65] = '{
    4'b 0001, // index[ 0] DMA_INTR_STATE
    4'b 0001, // index[ 1] DMA_INTR_ENABLE
    4'b 0001, // index[ 2] DMA_INTR_TEST
    4'b 0001, // index[ 3] DMA_ALERT_TEST
    4'b 1111, // index[ 4] DMA_SOURCE_ADDRESS_LO
    4'b 1111, // index[ 5] DMA_SOURCE_ADDRESS_HI
    4'b 1111, // index[ 6] DMA_DESTINATION_ADDRESS_LO
    4'b 1111, // index[ 7] DMA_DESTINATION_ADDRESS_HI
    4'b 0001, // index[ 8] DMA_ADDRESS_SPACE_ID
    4'b 1111, // index[ 9] DMA_ENABLED_MEMORY_RANGE_BASE
    4'b 1111, // index[10] DMA_ENABLED_MEMORY_RANGE_LIMIT
    4'b 0001, // index[11] DMA_RANGE_VALID
    4'b 0001, // index[12] DMA_RANGE_REGWEN
    4'b 0001, // index[13] DMA_CFG_REGWEN
    4'b 1111, // index[14] DMA_TOTAL_DATA_SIZE
    4'b 1111, // index[15] DMA_CHUNK_DATA_SIZE
    4'b 0001, // index[16] DMA_TRANSFER_WIDTH
    4'b 1111, // index[17] DMA_DESTINATION_ADDRESS_LIMIT_LO
    4'b 1111, // index[18] DMA_DESTINATION_ADDRESS_LIMIT_HI
    4'b 1111, // index[19] DMA_DESTINATION_ADDRESS_ALMOST_LIMIT_LO
    4'b 1111, // index[20] DMA_DESTINATION_ADDRESS_ALMOST_LIMIT_HI
    4'b 1111, // index[21] DMA_CONTROL
    4'b 0001, // index[22] DMA_STATUS
    4'b 0001, // index[23] DMA_ERROR_CODE
    4'b 1111, // index[24] DMA_SHA2_DIGEST_0
    4'b 1111, // index[25] DMA_SHA2_DIGEST_1
    4'b 1111, // index[26] DMA_SHA2_DIGEST_2
    4'b 1111, // index[27] DMA_SHA2_DIGEST_3
    4'b 1111, // index[28] DMA_SHA2_DIGEST_4
    4'b 1111, // index[29] DMA_SHA2_DIGEST_5
    4'b 1111, // index[30] DMA_SHA2_DIGEST_6
    4'b 1111, // index[31] DMA_SHA2_DIGEST_7
    4'b 1111, // index[32] DMA_SHA2_DIGEST_8
    4'b 1111, // index[33] DMA_SHA2_DIGEST_9
    4'b 1111, // index[34] DMA_SHA2_DIGEST_10
    4'b 1111, // index[35] DMA_SHA2_DIGEST_11
    4'b 1111, // index[36] DMA_SHA2_DIGEST_12
    4'b 1111, // index[37] DMA_SHA2_DIGEST_13
    4'b 1111, // index[38] DMA_SHA2_DIGEST_14
    4'b 1111, // index[39] DMA_SHA2_DIGEST_15
    4'b 0011, // index[40] DMA_HANDSHAKE_INTERRUPT_ENABLE
    4'b 0011, // index[41] DMA_CLEAR_INT_SRC
    4'b 0011, // index[42] DMA_CLEAR_INT_BUS
    4'b 1111, // index[43] DMA_INT_SOURCE_ADDR_0
    4'b 1111, // index[44] DMA_INT_SOURCE_ADDR_1
    4'b 1111, // index[45] DMA_INT_SOURCE_ADDR_2
    4'b 1111, // index[46] DMA_INT_SOURCE_ADDR_3
    4'b 1111, // index[47] DMA_INT_SOURCE_ADDR_4
    4'b 1111, // index[48] DMA_INT_SOURCE_ADDR_5
    4'b 1111, // index[49] DMA_INT_SOURCE_ADDR_6
    4'b 1111, // index[50] DMA_INT_SOURCE_ADDR_7
    4'b 1111, // index[51] DMA_INT_SOURCE_ADDR_8
    4'b 1111, // index[52] DMA_INT_SOURCE_ADDR_9
    4'b 1111, // index[53] DMA_INT_SOURCE_ADDR_10
    4'b 1111, // index[54] DMA_INT_SOURCE_WR_VAL_0
    4'b 1111, // index[55] DMA_INT_SOURCE_WR_VAL_1
    4'b 1111, // index[56] DMA_INT_SOURCE_WR_VAL_2
    4'b 1111, // index[57] DMA_INT_SOURCE_WR_VAL_3
    4'b 1111, // index[58] DMA_INT_SOURCE_WR_VAL_4
    4'b 1111, // index[59] DMA_INT_SOURCE_WR_VAL_5
    4'b 1111, // index[60] DMA_INT_SOURCE_WR_VAL_6
    4'b 1111, // index[61] DMA_INT_SOURCE_WR_VAL_7
    4'b 1111, // index[62] DMA_INT_SOURCE_WR_VAL_8
    4'b 1111, // index[63] DMA_INT_SOURCE_WR_VAL_9
    4'b 1111  // index[64] DMA_INT_SOURCE_WR_VAL_10
  };

endpackage
