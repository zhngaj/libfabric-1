/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _EFA_IO_H_
#define _EFA_IO_H_

#define EFA_IO_TX_DESC_NUM_BUFS              2
#define EFA_IO_TX_DESC_INLINE_MAX_SIZE       32
#define EFA_IO_TX_DESC_IMM_DATA_SIZE         4

enum efa_io_queue_type {
	/* send queue (of a QP) */
	EFA_IO_SEND_QUEUE                           = 1,
	/* recv queue (of a QP) */
	EFA_IO_RECV_QUEUE                           = 2,
};

enum efa_io_send_op_type {
	/* invalid op */
	EFA_IO_INVALID_OP                           = 0,
	/* send message */
	EFA_IO_SEND                                 = 1,
	/* RDMA read, future, not supported yet */
	EFA_IO_RDMA_READ                            = 2,
	/* RDMA write, future, not supported yet */
	EFA_IO_RDMA_WRITE                           = 3,
};

enum efa_io_comp_status {
	/* Successful completion */
	EFA_IO_COMP_STATUS_OK                       = 0,
	/* Flushed during QP destroy */
	EFA_IO_COMP_STATUS_FLUSHED                  = 1,
	/* Internal QP error */
	EFA_IO_COMP_STATUS_LOCAL_ERROR_QP_INTERNAL_ERROR = 2,
	/* Bad operation type */
	EFA_IO_COMP_STATUS_LOCAL_ERROR_INVALID_OP_TYPE = 3,
	/* Bad AH */
	EFA_IO_COMP_STATUS_LOCAL_ERROR_INVALID_AH   = 4,
	/* LKEY not registered or does not match IOVA */
	EFA_IO_COMP_STATUS_LOCAL_ERROR_INVALID_LKEY = 5,
	/* Message too long */
	EFA_IO_COMP_STATUS_LOCAL_ERROR_BAD_LENGTH   = 6,
	/* Destination ENI is down or does not run EFA */
	EFA_IO_COMP_STATUS_REMOTE_ERROR_BAD_ADDRESS = 7,
	/* Connection was reset by remote side */
	EFA_IO_COMP_STATUS_REMOTE_ERROR_ABORT       = 8,
	/* Bad dest QP number (QP does not exist or is in error state) */
	EFA_IO_COMP_STATUS_REMOTE_ERROR_BAD_DEST_QPN = 9,
	/* Destination resource not ready (no WQEs posted on RQ) */
	EFA_IO_COMP_STATUS_REMOTE_ERROR_RNR         = 10,
	/* Receiver SGL too short */
	EFA_IO_COMP_STATUS_REMOTE_ERROR_BAD_LENGTH  = 11,
	/* Unexpected status returned by responder */
	EFA_IO_COMP_STATUS_REMOTE_ERROR_BAD_STATUS  = 12,
};

/* Tx Meta descriptor. */
struct efa_io_tx_meta_desc {
	/* Verbs-generated Request ID */
	uint16_t req_id;

	/*
	 * control flags
	 * 3:0 : op_type - operation type: send/rdma/fast mem
	 *    ops/etc
	 * 4 : has_imm - immediate_data field carries valid
	 *    data.
	 * 5 : inline_msg - inline mode - inline message data
	 *    follows this descriptor (no buffer descriptors).
	 *    Note that it is different from immediate data
	 * 6 : meta_extension - Extended metadata. MBZ
	 * 7 : meta_desc - Indicates metadata descriptor.
	 *    Must be set.
	 */
	uint8_t ctrl1;

	/*
	 * control flags
	 * 0 : phase - phase bit.
	 * 1 : reserved25 - MBZ
	 * 2 : first - Indicates first descriptor in
	 *    transaction. Must be set.
	 * 3 : last - Indicates last descriptor in
	 *    transaction. Must be set.
	 * 4 : comp_req - Indicates whether completion should
	 *    be posted, after packet is transmitted. Valid only
	 *    for the first descriptor
	 * 7:5 : reserved29 - MBZ
	 */
	uint8_t ctrl2;

	/* destination QP number */
	uint16_t dest_qp_num;

	/*
	 * If inline_msg bit is set, length of inline message in bytes,
	 *    otherwise length of SGL (number of buffers).
	 */
	uint16_t len;

	/*
	 * immediate data: if has_imm is set, then this field is included
	 *    within Tx message and reported in remote Rx completion.
	 */
	uint32_t immediate_data;

	/* Address handle */
	uint16_t ah;

	uint16_t reserved;
};

/*
 * Tx buffer descriptor, for any transport type. Preceded by metadata
 * descriptor.
 */
struct efa_io_tx_buf_desc {
	/* length in bytes */
	uint16_t length;

	/*
	 * control flags
	 * 6:0 : reserved16
	 * 7 : meta_desc - MBZ
	 */
	uint8_t ctrl1;

	/*
	 * control flags
	 * 0 : phase - phase bit
	 * 1 : reserved25 - MBZ
	 * 2 : first - Indicates first descriptor in
	 *    transaction. MBZ
	 * 3 : last - Indicates last descriptor in transaction
	 * 7:4 : reserved28 - MBZ
	 */
	uint8_t ctrl;

	/* memory translation key */
	uint32_t lkey;

	/* Buffer address bits[31:0] */
	uint32_t buf_addr_lo;

	/*
	 * 15:0 : buf_addr_hi - Buffer Pointer[47:32]
	 * 31:16 : reserved - Reserved
	 */
	uint32_t buf_addr_hi;
};

/* Tx meta descriptor for UD */
struct efa_io_tx_ud_meta {
	/* Queue key */
	uint32_t qkey;

	uint8_t reserved[12];
};

/* Remote memory address */
struct efa_io_remote_mem_addr {
	/* length in bytes */
	uint16_t length;

	/*
	 * control flags
	 * 5:0 : reserved16
	 * 6 : meta_extension - Must be set
	 * 7 : meta_desc - Must be set
	 */
	uint8_t ctrl1;

	/*
	 * control flags
	 * 0 : phase - phase bit
	 * 1 : reserved25 - MBZ
	 * 2 : first - Indicates first descriptor in
	 *    transaction. MBZ
	 * 3 : last - Indicates last descriptor in transaction
	 * 7:4 : reserved28 - MBZ
	 */
	uint8_t ctrl;

	/* remote memory translation key */
	uint32_t rkey;

	/* Buffer address bits[31:0] */
	uint32_t buf_addr_lo;

	/* Buffer address bits[63:32] */
	uint32_t buf_addr_hi;
};

/*
 * Tx WQE, composed of tx meta descriptors followed by either tx buffer
 * descriptors or inline data
 */
struct efa_io_tx_wqe {
	/* TX meta */
	struct efa_io_tx_meta_desc common;

	union {
		/* Tx meta for UD */
		struct efa_io_tx_ud_meta ud;

		/* Reserved Tx meta for SRD */
		uint8_t srd_padding[16];

		/* RDMA memory address */
		struct efa_io_remote_mem_addr rdma_mem_addr;
	} u;

	union {
		/* buffer descriptors */
		struct efa_io_tx_buf_desc sgl[2];

		/* inline data */
		uint8_t inline_data[32];
	} data;
};

/*
 * Rx buffer descriptor; RX WQE is composed of one or more RX buffer
 * descriptors.
 */
struct efa_io_rx_desc {
	/* Buffer address bits[31:0] */
	uint32_t buf_addr_lo;

	/* Buffer Pointer[63:32] */
	uint32_t buf_addr_hi;

	/* Verbs-generated request id. */
	uint16_t req_id;

	/* Length in bytes. */
	uint16_t length;

	/*
	 * LKey and control flags
	 * 23:0 : lkey
	 * 29:24 : reserved - MBZ
	 * 30 : first - Indicates first descriptor in WQE
	 * 31 : last - Indicates last descriptor in WQE
	 */
	uint32_t lkey_ctrl;
};

/* Common IO completion descriptor */
struct efa_io_cdesc_common {
	/*
	 * verbs-generated request ID, as provided in the completed tx or rx
	 *    descriptor.
	 */
	uint16_t req_id;

	/* status */
	uint8_t status;

	/*
	 * flags
	 * 0 : phase - Phase bit
	 * 2:1 : q_type - enum efa_io_queue_type: send/recv
	 * 3 : has_imm - indicates that immediate data is
	 *    present - for RX completions only
	 * 4 : wide_completion - indicates that wide
	 *    completion format is used
	 * 7:5 : reserved29
	 */
	uint8_t flags;

	/* local QP number */
	uint16_t qp_num;

	/* Transferred length */
	uint16_t length;
};

/* Tx completion descriptor */
struct efa_io_tx_cdesc {
	/* Common completion info */
	struct efa_io_cdesc_common common;
};

/* Rx Completion Descriptor */
struct efa_io_rx_cdesc {
	/* Common completion info */
	struct efa_io_cdesc_common common;

	/* Remote Address Handle FW index, 0xFFFF indicates invalid ah */
	uint16_t ah;

	/* Source QP number */
	uint16_t src_qp_num;

	/* Immediate data */
	uint32_t imm;
};

/* Extended Rx Completion Descriptor */
struct efa_io_rx_cdesc_wide {
	/* Base RX completion info */
	struct efa_io_rx_cdesc rx_cdesc_base;

	/*
	 * Word 0 of remote (source) address, needed only for in-band
	 * ad-hoc AH support
	 */
	uint32_t src_addr_0;

	/*
	 * Word 1 of remote (source) address, needed only for in-band
	 * ad-hoc AH support
	 */
	uint32_t src_addr_1;

	/*
	 * Word 2 of remote (source) address, needed only for in-band
	 * ad-hoc AH support
	 */
	uint32_t src_addr_2;

	/*
	 * Word 3 of remote (source) address, needed only for in-band
	 * ad-hoc AH support
	 */
	uint32_t src_addr_3;
};

/* tx_meta_desc */
#define EFA_IO_TX_META_DESC_OP_TYPE_MASK                    GENMASK(3, 0)
#define EFA_IO_TX_META_DESC_HAS_IMM_SHIFT                   4
#define EFA_IO_TX_META_DESC_HAS_IMM_MASK                    BIT(4)
#define EFA_IO_TX_META_DESC_INLINE_MSG_SHIFT                5
#define EFA_IO_TX_META_DESC_INLINE_MSG_MASK                 BIT(5)
#define EFA_IO_TX_META_DESC_META_EXTENSION_SHIFT            6
#define EFA_IO_TX_META_DESC_META_EXTENSION_MASK             BIT(6)
#define EFA_IO_TX_META_DESC_META_DESC_SHIFT                 7
#define EFA_IO_TX_META_DESC_META_DESC_MASK                  BIT(7)
#define EFA_IO_TX_META_DESC_PHASE_MASK                      BIT(0)
#define EFA_IO_TX_META_DESC_FIRST_SHIFT                     2
#define EFA_IO_TX_META_DESC_FIRST_MASK                      BIT(2)
#define EFA_IO_TX_META_DESC_LAST_SHIFT                      3
#define EFA_IO_TX_META_DESC_LAST_MASK                       BIT(3)
#define EFA_IO_TX_META_DESC_COMP_REQ_SHIFT                  4
#define EFA_IO_TX_META_DESC_COMP_REQ_MASK                   BIT(4)

/* tx_buf_desc */
#define EFA_IO_TX_BUF_DESC_META_DESC_SHIFT                  7
#define EFA_IO_TX_BUF_DESC_META_DESC_MASK                   BIT(7)
#define EFA_IO_TX_BUF_DESC_PHASE_MASK                       BIT(0)
#define EFA_IO_TX_BUF_DESC_FIRST_SHIFT                      2
#define EFA_IO_TX_BUF_DESC_FIRST_MASK                       BIT(2)
#define EFA_IO_TX_BUF_DESC_LAST_SHIFT                       3
#define EFA_IO_TX_BUF_DESC_LAST_MASK                        BIT(3)
#define EFA_IO_TX_BUF_DESC_BUF_ADDR_HI_MASK                 GENMASK(15, 0)

/* remote_mem_addr */
#define EFA_IO_REMOTE_MEM_ADDR_META_EXTENSION_SHIFT         6
#define EFA_IO_REMOTE_MEM_ADDR_META_EXTENSION_MASK          BIT(6)
#define EFA_IO_REMOTE_MEM_ADDR_META_DESC_SHIFT              7
#define EFA_IO_REMOTE_MEM_ADDR_META_DESC_MASK               BIT(7)
#define EFA_IO_REMOTE_MEM_ADDR_PHASE_MASK                   BIT(0)
#define EFA_IO_REMOTE_MEM_ADDR_FIRST_SHIFT                  2
#define EFA_IO_REMOTE_MEM_ADDR_FIRST_MASK                   BIT(2)
#define EFA_IO_REMOTE_MEM_ADDR_LAST_SHIFT                   3
#define EFA_IO_REMOTE_MEM_ADDR_LAST_MASK                    BIT(3)

/* rx_desc */
#define EFA_IO_RX_DESC_LKEY_MASK                            GENMASK(23, 0)
#define EFA_IO_RX_DESC_FIRST_SHIFT                          30
#define EFA_IO_RX_DESC_FIRST_MASK                           BIT(30)
#define EFA_IO_RX_DESC_LAST_SHIFT                           31
#define EFA_IO_RX_DESC_LAST_MASK                            BIT(31)

/* cdesc_common */
#define EFA_IO_CDESC_COMMON_PHASE_MASK                      BIT(0)
#define EFA_IO_CDESC_COMMON_Q_TYPE_SHIFT                    1
#define EFA_IO_CDESC_COMMON_Q_TYPE_MASK                     GENMASK(2, 1)
#define EFA_IO_CDESC_COMMON_HAS_IMM_SHIFT                   3
#define EFA_IO_CDESC_COMMON_HAS_IMM_MASK                    BIT(3)
#define EFA_IO_CDESC_COMMON_WIDE_COMPLETION_SHIFT           4
#define EFA_IO_CDESC_COMMON_WIDE_COMPLETION_MASK            BIT(4)

#if !defined(DEFS_LINUX_MAINLINE)
static inline uint8_t get_efa_io_tx_meta_desc_op_type(const struct efa_io_tx_meta_desc *p)
{
	return p->ctrl1 & EFA_IO_TX_META_DESC_OP_TYPE_MASK;
}

static inline void set_efa_io_tx_meta_desc_op_type(struct efa_io_tx_meta_desc *p, uint8_t val)
{
	p->ctrl1 |= val & EFA_IO_TX_META_DESC_OP_TYPE_MASK;
}

static inline uint8_t get_efa_io_tx_meta_desc_has_imm(const struct efa_io_tx_meta_desc *p)
{
	return (p->ctrl1 & EFA_IO_TX_META_DESC_HAS_IMM_MASK) >> EFA_IO_TX_META_DESC_HAS_IMM_SHIFT;
}

static inline void set_efa_io_tx_meta_desc_has_imm(struct efa_io_tx_meta_desc *p, uint8_t val)
{
	p->ctrl1 |= (val << EFA_IO_TX_META_DESC_HAS_IMM_SHIFT) & EFA_IO_TX_META_DESC_HAS_IMM_MASK;
}

static inline uint8_t get_efa_io_tx_meta_desc_inline_msg(const struct efa_io_tx_meta_desc *p)
{
	return (p->ctrl1 & EFA_IO_TX_META_DESC_INLINE_MSG_MASK) >> EFA_IO_TX_META_DESC_INLINE_MSG_SHIFT;
}

static inline void set_efa_io_tx_meta_desc_inline_msg(struct efa_io_tx_meta_desc *p, uint8_t val)
{
	p->ctrl1 |= (val << EFA_IO_TX_META_DESC_INLINE_MSG_SHIFT) & EFA_IO_TX_META_DESC_INLINE_MSG_MASK;
}

static inline uint8_t get_efa_io_tx_meta_desc_meta_extension(const struct efa_io_tx_meta_desc *p)
{
	return (p->ctrl1 & EFA_IO_TX_META_DESC_META_EXTENSION_MASK) >> EFA_IO_TX_META_DESC_META_EXTENSION_SHIFT;
}

static inline void set_efa_io_tx_meta_desc_meta_extension(struct efa_io_tx_meta_desc *p, uint8_t val)
{
	p->ctrl1 |= (val << EFA_IO_TX_META_DESC_META_EXTENSION_SHIFT) & EFA_IO_TX_META_DESC_META_EXTENSION_MASK;
}

static inline uint8_t get_efa_io_tx_meta_desc_meta_desc(const struct efa_io_tx_meta_desc *p)
{
	return (p->ctrl1 & EFA_IO_TX_META_DESC_META_DESC_MASK) >> EFA_IO_TX_META_DESC_META_DESC_SHIFT;
}

static inline void set_efa_io_tx_meta_desc_meta_desc(struct efa_io_tx_meta_desc *p, uint8_t val)
{
	p->ctrl1 |= (val << EFA_IO_TX_META_DESC_META_DESC_SHIFT) & EFA_IO_TX_META_DESC_META_DESC_MASK;
}

static inline uint8_t get_efa_io_tx_meta_desc_phase(const struct efa_io_tx_meta_desc *p)
{
	return p->ctrl2 & EFA_IO_TX_META_DESC_PHASE_MASK;
}

static inline void set_efa_io_tx_meta_desc_phase(struct efa_io_tx_meta_desc *p, uint8_t val)
{
	p->ctrl2 |= val & EFA_IO_TX_META_DESC_PHASE_MASK;
}

static inline uint8_t get_efa_io_tx_meta_desc_first(const struct efa_io_tx_meta_desc *p)
{
	return (p->ctrl2 & EFA_IO_TX_META_DESC_FIRST_MASK) >> EFA_IO_TX_META_DESC_FIRST_SHIFT;
}

static inline void set_efa_io_tx_meta_desc_first(struct efa_io_tx_meta_desc *p, uint8_t val)
{
	p->ctrl2 |= (val << EFA_IO_TX_META_DESC_FIRST_SHIFT) & EFA_IO_TX_META_DESC_FIRST_MASK;
}

static inline uint8_t get_efa_io_tx_meta_desc_last(const struct efa_io_tx_meta_desc *p)
{
	return (p->ctrl2 & EFA_IO_TX_META_DESC_LAST_MASK) >> EFA_IO_TX_META_DESC_LAST_SHIFT;
}

static inline void set_efa_io_tx_meta_desc_last(struct efa_io_tx_meta_desc *p, uint8_t val)
{
	p->ctrl2 |= (val << EFA_IO_TX_META_DESC_LAST_SHIFT) & EFA_IO_TX_META_DESC_LAST_MASK;
}

static inline uint8_t get_efa_io_tx_meta_desc_comp_req(const struct efa_io_tx_meta_desc *p)
{
	return (p->ctrl2 & EFA_IO_TX_META_DESC_COMP_REQ_MASK) >> EFA_IO_TX_META_DESC_COMP_REQ_SHIFT;
}

static inline void set_efa_io_tx_meta_desc_comp_req(struct efa_io_tx_meta_desc *p, uint8_t val)
{
	p->ctrl2 |= (val << EFA_IO_TX_META_DESC_COMP_REQ_SHIFT) & EFA_IO_TX_META_DESC_COMP_REQ_MASK;
}

static inline uint8_t get_efa_io_tx_buf_desc_meta_desc(const struct efa_io_tx_buf_desc *p)
{
	return (p->ctrl1 & EFA_IO_TX_BUF_DESC_META_DESC_MASK) >> EFA_IO_TX_BUF_DESC_META_DESC_SHIFT;
}

static inline void set_efa_io_tx_buf_desc_meta_desc(struct efa_io_tx_buf_desc *p, uint8_t val)
{
	p->ctrl1 |= (val << EFA_IO_TX_BUF_DESC_META_DESC_SHIFT) & EFA_IO_TX_BUF_DESC_META_DESC_MASK;
}

static inline uint8_t get_efa_io_tx_buf_desc_phase(const struct efa_io_tx_buf_desc *p)
{
	return p->ctrl & EFA_IO_TX_BUF_DESC_PHASE_MASK;
}

static inline void set_efa_io_tx_buf_desc_phase(struct efa_io_tx_buf_desc *p, uint8_t val)
{
	p->ctrl |= val & EFA_IO_TX_BUF_DESC_PHASE_MASK;
}

static inline uint8_t get_efa_io_tx_buf_desc_first(const struct efa_io_tx_buf_desc *p)
{
	return (p->ctrl & EFA_IO_TX_BUF_DESC_FIRST_MASK) >> EFA_IO_TX_BUF_DESC_FIRST_SHIFT;
}

static inline void set_efa_io_tx_buf_desc_first(struct efa_io_tx_buf_desc *p, uint8_t val)
{
	p->ctrl |= (val << EFA_IO_TX_BUF_DESC_FIRST_SHIFT) & EFA_IO_TX_BUF_DESC_FIRST_MASK;
}

static inline uint8_t get_efa_io_tx_buf_desc_last(const struct efa_io_tx_buf_desc *p)
{
	return (p->ctrl & EFA_IO_TX_BUF_DESC_LAST_MASK) >> EFA_IO_TX_BUF_DESC_LAST_SHIFT;
}

static inline void set_efa_io_tx_buf_desc_last(struct efa_io_tx_buf_desc *p, uint8_t val)
{
	p->ctrl |= (val << EFA_IO_TX_BUF_DESC_LAST_SHIFT) & EFA_IO_TX_BUF_DESC_LAST_MASK;
}

static inline uint32_t get_efa_io_tx_buf_desc_buf_addr_hi(const struct efa_io_tx_buf_desc *p)
{
	return p->buf_addr_hi & EFA_IO_TX_BUF_DESC_BUF_ADDR_HI_MASK;
}

static inline void set_efa_io_tx_buf_desc_buf_addr_hi(struct efa_io_tx_buf_desc *p, uint32_t val)
{
	p->buf_addr_hi |= val & EFA_IO_TX_BUF_DESC_BUF_ADDR_HI_MASK;
}

static inline uint8_t get_efa_io_remote_mem_addr_meta_extension(const struct efa_io_remote_mem_addr *p)
{
	return (p->ctrl1 & EFA_IO_REMOTE_MEM_ADDR_META_EXTENSION_MASK) >> EFA_IO_REMOTE_MEM_ADDR_META_EXTENSION_SHIFT;
}

static inline void set_efa_io_remote_mem_addr_meta_extension(struct efa_io_remote_mem_addr *p, uint8_t val)
{
	p->ctrl1 |= (val << EFA_IO_REMOTE_MEM_ADDR_META_EXTENSION_SHIFT) & EFA_IO_REMOTE_MEM_ADDR_META_EXTENSION_MASK;
}

static inline uint8_t get_efa_io_remote_mem_addr_meta_desc(const struct efa_io_remote_mem_addr *p)
{
	return (p->ctrl1 & EFA_IO_REMOTE_MEM_ADDR_META_DESC_MASK) >> EFA_IO_REMOTE_MEM_ADDR_META_DESC_SHIFT;
}

static inline void set_efa_io_remote_mem_addr_meta_desc(struct efa_io_remote_mem_addr *p, uint8_t val)
{
	p->ctrl1 |= (val << EFA_IO_REMOTE_MEM_ADDR_META_DESC_SHIFT) & EFA_IO_REMOTE_MEM_ADDR_META_DESC_MASK;
}

static inline uint8_t get_efa_io_remote_mem_addr_phase(const struct efa_io_remote_mem_addr *p)
{
	return p->ctrl & EFA_IO_REMOTE_MEM_ADDR_PHASE_MASK;
}

static inline void set_efa_io_remote_mem_addr_phase(struct efa_io_remote_mem_addr *p, uint8_t val)
{
	p->ctrl |= val & EFA_IO_REMOTE_MEM_ADDR_PHASE_MASK;
}

static inline uint8_t get_efa_io_remote_mem_addr_first(const struct efa_io_remote_mem_addr *p)
{
	return (p->ctrl & EFA_IO_REMOTE_MEM_ADDR_FIRST_MASK) >> EFA_IO_REMOTE_MEM_ADDR_FIRST_SHIFT;
}

static inline void set_efa_io_remote_mem_addr_first(struct efa_io_remote_mem_addr *p, uint8_t val)
{
	p->ctrl |= (val << EFA_IO_REMOTE_MEM_ADDR_FIRST_SHIFT) & EFA_IO_REMOTE_MEM_ADDR_FIRST_MASK;
}

static inline uint8_t get_efa_io_remote_mem_addr_last(const struct efa_io_remote_mem_addr *p)
{
	return (p->ctrl & EFA_IO_REMOTE_MEM_ADDR_LAST_MASK) >> EFA_IO_REMOTE_MEM_ADDR_LAST_SHIFT;
}

static inline void set_efa_io_remote_mem_addr_last(struct efa_io_remote_mem_addr *p, uint8_t val)
{
	p->ctrl |= (val << EFA_IO_REMOTE_MEM_ADDR_LAST_SHIFT) & EFA_IO_REMOTE_MEM_ADDR_LAST_MASK;
}

static inline uint32_t get_efa_io_rx_desc_lkey(const struct efa_io_rx_desc *p)
{
	return p->lkey_ctrl & EFA_IO_RX_DESC_LKEY_MASK;
}

static inline void set_efa_io_rx_desc_lkey(struct efa_io_rx_desc *p, uint32_t val)
{
	p->lkey_ctrl |= val & EFA_IO_RX_DESC_LKEY_MASK;
}

static inline uint32_t get_efa_io_rx_desc_first(const struct efa_io_rx_desc *p)
{
	return (p->lkey_ctrl & EFA_IO_RX_DESC_FIRST_MASK) >> EFA_IO_RX_DESC_FIRST_SHIFT;
}

static inline void set_efa_io_rx_desc_first(struct efa_io_rx_desc *p, uint32_t val)
{
	p->lkey_ctrl |= (val << EFA_IO_RX_DESC_FIRST_SHIFT) & EFA_IO_RX_DESC_FIRST_MASK;
}

static inline uint32_t get_efa_io_rx_desc_last(const struct efa_io_rx_desc *p)
{
	return (p->lkey_ctrl & EFA_IO_RX_DESC_LAST_MASK) >> EFA_IO_RX_DESC_LAST_SHIFT;
}

static inline void set_efa_io_rx_desc_last(struct efa_io_rx_desc *p, uint32_t val)
{
	p->lkey_ctrl |= (val << EFA_IO_RX_DESC_LAST_SHIFT) & EFA_IO_RX_DESC_LAST_MASK;
}

static inline uint8_t get_efa_io_cdesc_common_phase(const struct efa_io_cdesc_common *p)
{
	return p->flags & EFA_IO_CDESC_COMMON_PHASE_MASK;
}

static inline void set_efa_io_cdesc_common_phase(struct efa_io_cdesc_common *p, uint8_t val)
{
	p->flags |= val & EFA_IO_CDESC_COMMON_PHASE_MASK;
}

static inline uint8_t get_efa_io_cdesc_common_q_type(const struct efa_io_cdesc_common *p)
{
	return (p->flags & EFA_IO_CDESC_COMMON_Q_TYPE_MASK) >> EFA_IO_CDESC_COMMON_Q_TYPE_SHIFT;
}

static inline void set_efa_io_cdesc_common_q_type(struct efa_io_cdesc_common *p, uint8_t val)
{
	p->flags |= (val << EFA_IO_CDESC_COMMON_Q_TYPE_SHIFT) & EFA_IO_CDESC_COMMON_Q_TYPE_MASK;
}

static inline uint8_t get_efa_io_cdesc_common_has_imm(const struct efa_io_cdesc_common *p)
{
	return (p->flags & EFA_IO_CDESC_COMMON_HAS_IMM_MASK) >> EFA_IO_CDESC_COMMON_HAS_IMM_SHIFT;
}

static inline void set_efa_io_cdesc_common_has_imm(struct efa_io_cdesc_common *p, uint8_t val)
{
	p->flags |= (val << EFA_IO_CDESC_COMMON_HAS_IMM_SHIFT) & EFA_IO_CDESC_COMMON_HAS_IMM_MASK;
}

static inline uint8_t get_efa_io_cdesc_common_wide_completion(const struct efa_io_cdesc_common *p)
{
	return (p->flags & EFA_IO_CDESC_COMMON_WIDE_COMPLETION_MASK) >> EFA_IO_CDESC_COMMON_WIDE_COMPLETION_SHIFT;
}

static inline void set_efa_io_cdesc_common_wide_completion(struct efa_io_cdesc_common *p, uint8_t val)
{
	p->flags |= (val << EFA_IO_CDESC_COMMON_WIDE_COMPLETION_SHIFT) & EFA_IO_CDESC_COMMON_WIDE_COMPLETION_MASK;
}

#endif /* !defined(DEFS_LINUX_MAINLINE) */
#endif /*_EFA_IO_H_ */
