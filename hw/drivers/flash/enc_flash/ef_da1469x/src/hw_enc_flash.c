/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <string.h>

#include <os/mynewt.h>
#include <mcu/da1469x_clock.h>

#include "enc_flash/enc_flash.h"
#include "enc_flash/enc_flash_int.h"
#include "mcu/da1469x_dma.h"
#include "mcu/da1469x_otp.h"

/*
 * Key slot is a config variable, passed in as syscfg or build time
 * param. Harcoded for now.
 *
 */
 #define AES_USER_DATA_SLOT 0

void
da1469x_enc_flash_setkey(struct hal_flash *h_dev, uint8_t *key)
{
    DMA_Type *dma_regs = DMA;

	/* enable OTP clock and set in read mode */
    da1469x_clock_amba_enable(CRG_TOP_CLK_AMBA_REG_OTP_ENABLE_Msk);
    da1469x_otp_set_mode(OTPC_MODE_READ);

    /* disable decrypt on the fly and program start and end addresses */
    QSPIC->QSPIC_CTR_CTRL_REG = 0;
    QSPIC->QSPIC_CTR_SADDR_REG = h_dev->hf_base_addr;
    QSPIC->QSPIC_CTR_EADDR_REG = h_dev->hf_size + h_dev->hf_base_addr;

    /* securely DMA hardware key from secret storage to QSPI decrypt engine */
    dma_regs->DMA_REQ_MUX_REG |= 0xf000;
    dma_regs->DMA7_LEN_REG = 8;
    dma_regs->DMA7_A_START_REG = MCU_OTPM_BASE + OTP_SEGMENT_USER_DATA_KEYS +
                                 (32 * (AES_USER_DATA_SLOT));
    dma_regs->DMA7_B_START_REG = (uint32_t *)key;
    dma_regs->DMA7_CTRL_REG = DMA_DMA7_CTRL_REG_AINC_Msk |
                              DMA_DMA7_CTRL_REG_BINC_Msk |
                              (MCU_DMA_BUS_WIDTH_4B << DMA_DMA7_CTRL_REG_BW_Pos) |
                              DMA_DMA7_CTRL_REG_DMA_ON_Msk;
    while (dma_regs->DMA7_IDX_REG != 8);

    /*
     * use the nonce in the ef_crypto.c. We should be able to use 0
     * but there is no API set to set the nonce in ef_crypto at the moment.
    */

    /* set OTP to standby and turn off clock */
    da1469x_otp_set_mode(OTPC_MODE_STBY);
    da1469x_clock_amba_disable(CRG_TOP_CLK_AMBA_REG_OTP_ENABLE_Msk);

    enc_flash_setkey_arch(HAL_TO_ENC(h_dev), key);
}
