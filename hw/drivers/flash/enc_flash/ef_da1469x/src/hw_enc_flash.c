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
#include "ef_da1469x/ef_da1469x.h"
#include <mcu/da1469x_dma.h>
#include <mcu/da1469x_otp.h>
#include <crypto/crypto.h>
#include "console/console.h"

#define EDEV_TO_DA1469X(dev)   (struct eflash_da1469x_dev *)dev
#define DA1469X_AES_KEYSIZE 256

struct time_tracker g_tt;

void
enc_flash_crypt_arch(struct enc_flash_dev *edev, uint32_t blk_addr,
                     const uint8_t *src, uint8_t *tgt, int off, int cnt)
{
    struct eflash_da1469x_dev *dev = EDEV_TO_DA1469X(edev);
    const struct hal_flash *h_dev = edev->efd_hwdev;
    uint32_t ctr[4] = {0};
    uint8_t blk[ENC_FLASH_BLK] = {0};
    const void *key = (void *)MCU_OTPM_BASE + OTP_SEGMENT_USER_DATA_KEYS +
                      (AES_MAX_KEY_LEN * (MYNEWT_VAL(USER_AES_SLOT)));
    uint32_t diff;

    g_tt.start = os_cputime_get32();

    ctr[0] = (uint32_t) ((blk_addr - h_dev->hf_base_addr) / ENC_FLASH_BLK);

    memcpy(blk + off, src, cnt);
    os_sem_pend(&dev->ef_sem, OS_TIMEOUT_NEVER);
    g_tt.ef_sem_pend = os_cputime_get32();
    crypto_encrypt_aes_ctr(dev->ecd_crypto, key, DA1469X_AES_KEYSIZE, ctr,
                            blk, blk, AES_BLOCK_LEN);
    g_tt.co_done = os_cputime_get32();
    os_sem_release(&dev->ef_sem);
    g_tt.end = os_cputime_get32();
    memcpy(tgt, blk + off, cnt);
    diff = g_tt.end - g_tt.start;
    if (diff > 1000) {
        console_printf("#enc: hff_write %d, cnt %d total %ld, efsp_s %ld, cod_efsp %ld, end_cod %ld\n\
            cmp_cos %ld, cdtb_cmp %ld, cdte_cdtb %ld, cwtf_cdte %ld, cwtfd_cwtf %ld, cmr_ctd %ld\n",
            g_tt.hff_write,
            cnt,
            diff,
            g_tt.ef_sem_pend - g_tt.start,
            g_tt.co_done - g_tt.ef_sem_pend,
            g_tt.end - g_tt.co_done,
            g_tt.crypto_mutex_pend - g_tt.crypto_op_start,
            g_tt.crypto_dma_tx_begin - g_tt.crypto_mutex_pend,
            g_tt.crypto_dma_tx_end - g_tt.crypto_dma_tx_begin,
            g_tt.crypto_wtf - g_tt.crypto_dma_tx_end,
            g_tt.crypto_wtf_done - g_tt.crypto_wtf,
            g_tt.crypto_mutex_rel - g_tt.crypto_wtf_done);
    }
}

/* Key is securely DMA transferred from OTP user data key slot */
void
enc_flash_setkey_arch(struct enc_flash_dev *h_dev, uint8_t *key)
{
    return;
}

int
enc_flash_init_arch(struct enc_flash_dev *edev)
{
    struct eflash_da1469x_dev *dev = EDEV_TO_DA1469X(edev);

    dev->ecd_crypto = (struct crypto_dev *) os_dev_open("crypto",
            OS_TIMEOUT_NEVER, NULL);
    assert(dev->ecd_crypto);
    os_sem_init(&dev->ef_sem, 1);
    return 0;
}
